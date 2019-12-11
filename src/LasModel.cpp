#include <osg/MatrixTransform>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "PCVR_OvrDevice.hpp"

#include "LasModel.hpp"

ModelPoint::ModelPoint(liblas::Point const& p, int index, LasModel* parent)
	: liblas::Point(p)
	, _index(index)
	, _parent(parent)
{
}

osg::Vec3 ModelPoint::getPos() const
{
	return _parent->_verts->at(_index);
}

osg::Vec4 ModelPoint::getColor() const
{
	return _parent->_colors->at(_index);
}

void ModelPoint::setPos(osg::Vec3 pos)
{
	_parent->_verts->at(_index).set(pos);
}

void ModelPoint::setColor(osg::Vec4 color)
{
	_parent->_colors->at(_index) = color;
}

LasModel::LasModel(const std::string& path)
	: OpenFrames::Model(osgDB::getSimpleFileName(path), 0.5, 0.5, 0.5, 0.9)
{
	loadLasFile(path);
}

std::vector<ModelPoint*>& LasModel::getPoints()
{
	return _points;
}

osg::Vec4Array& LasModel::getColors()
{
	return *_colors;
}

void LasModel::loadLasFile(const std::string& path)
{
	// Open file and create reader
	std::string fileName = osgDB::findDataFile(path);
	if (fileName.empty()) return;

	std::cout << "Reading file " << fileName << "..." << std::endl;

	std::ifstream ifs;
	if (!liblas::Open(ifs, path)) return;

	liblas::Reader reader = liblas::ReaderFactory().CreateWithStream(ifs);
	liblas::Header const& h = reader.GetHeader();

	// Setup Geometry
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();

	typedef std::pair<double, double> minmax_t;
	minmax_t mx(DBL_MAX, -DBL_MAX);
	minmax_t my(DBL_MAX, -DBL_MAX);
	minmax_t mz(DBL_MAX, -DBL_MAX);

	// Read Points
	while (reader.ReadNextPoint())
	{
		liblas::Point const& p = reader.GetPoint();

		double x = p.GetRawX() * h.GetScaleX();
		double y = p.GetRawY() * h.GetScaleY();
		double z = p.GetRawZ() * h.GetScaleZ();
		_verts->push_back(osg::Vec3(x, y, z));

		mx.first = std::min<double>(mx.first, x);
		mx.second = std::max<double>(mx.second, x);
		my.first = std::min<double>(my.first, y);
		my.second = std::max<double>(my.second, y);
		mz.first = std::min<double>(mz.first, z);
		mz.second = std::max<double>(mz.second, z);

		liblas::Color c = p.GetColor();
		float r = ((float)c.GetRed()) / USHRT_MAX;
		float g = ((float)c.GetGreen()) / USHRT_MAX;
		float b = ((float)c.GetBlue()) / USHRT_MAX;
		float a = 255;    // default value, since LAS point has no alpha information
		_colors->push_back(osg::Vec4(r, g, b, a));

		_points.push_back(new ModelPoint(p, _verts->size() - 1, this));
	}

	for (ModelPoint* p : _points)
	{
		osg::Vec3 mids = osg::Vec3(mx.second + mx.first, my.second + my.first, mz.second + mz.first) * 0.5;
		p->setPos(p->getPos() - mids);
	}

	// Setup Geometry
	geometry->setUseDisplayList(true);
	geometry->setUseVertexBufferObjects(true);
	geometry->setVertexArray(_verts);
	geometry->setColorArray(_colors, osg::Array::BIND_PER_VERTEX);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, _verts->size()));

	osg::ref_ptr<osg::Program> program = new osg::Program();
	//program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, "../../shaders/SurfacePoint.vert"));
	//program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "../../shaders/SurfacePoint.frag"));

	osg::ref_ptr<osg::StateSet> state = geometry->getOrCreateStateSet();
	state->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
	state->setAttributeAndModes(program, osg::StateAttribute::ON);

	osg::ref_ptr<osg::Uniform> viewMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "viewMatrix");
	viewMatrix->setUpdateCallback(new ViewMatrixCallback());
	state->addUniform(viewMatrix);

	// Set new model
	geode->setDataVariance(osg::Object::STATIC);
	geode->addDrawable(geometry);
	_model = geode;

	// Add the new model to this frame
	_modelXform->addChild(_model.get());
	_xform->addChild(_modelXform.get());

	// Rescale normals in case we want to scale the model
	_model->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);

	// Set the model pivot at its geometric center, so that scales/rotations will make sense.
	osg::Vec3d center = _model->getBound()._center;
	setModelPivot(center[0], center[1], center[2]);
}