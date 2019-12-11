#include <filesystem>

#include "DiskDrawer.hpp"

namespace fs = std::experimental::filesystem;

SelectionDisk::SelectionDisk(osg::Vec3 pos)
	: OpenFrames::ReferenceFrame("SelectionDisk", osg::Vec4(0, 0., 1, 0.5f))
{
	_sd = new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(), 0, 0));
	_sd->setUseDisplayList(true);
	_sd->setUseVertexBufferObjects(true);
	_sd->setColor(getColor());

	osg::ref_ptr<osg::StateSet> stateSet = getGroup()->getOrCreateStateSet();
	stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(_sd);
	_xform->addChild(geode);

	setPosition(pos);
	showAxes(false);
}

double SelectionDisk::getRadius() const
{
	return static_cast<osg::Cylinder*>(_sd->getShape())->getRadius();
}

double SelectionDisk::getHeight() const
{
	return static_cast<osg::Cylinder*>(_sd->getShape())->getHeight();
}

osg::ShapeDrawable* SelectionDisk::getShapeDrawable()
{
	return _sd;
}

void SelectionDisk::setRadius(double rad)
{
	osg::ref_ptr<osg::Cylinder> cylinder = static_cast<osg::Cylinder*>(_sd->getShape());
	cylinder->setRadius(rad);
	_sd->dirtyBound();
	_sd->build();
}

void SelectionDisk::setHeight(double height)
{
	osg::ref_ptr<osg::Cylinder> cylinder = static_cast<osg::Cylinder*>(_sd->getShape());
	cylinder->setHeight(height);
	_sd->dirtyBound();
	_sd->build();
}

/*void SelectionDisk::setAttitude(const osg::Quat& att)
{
	//osg::ref_ptr<osg::Cylinder> cylinder = static_cast<osg::Cylinder*>(_sd->getShape());
	//cylinder->setRotation(att);
}*/

void SelectionDisk::save(const std::string& path, const std::vector<PCVR_Selectable*>& points)
{
	int fileNum = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{}) + 1;
	std::string filepath = path + "/SelectionDisk" + std::to_string(fileNum) + ".csv";

	std::ofstream file;
	file.open(filepath);

	osg::Vec3d c; getPosition(c);
	osg::Quat r; getAttitude(r);
	file << "#Origin: " << c.x() << " " << c.y() << " " << c.z() << std::endl;
	file << "#Radius: " << getRadius() << std::endl;
	file << "#Height: " << getHeight() << std::endl;
	file << "#Rotation: " << r.x() << r.y() << r.z() << r.w() << std::endl;

	for (PCVR_Selectable* p : points)
	{
		if ((p->getPos() - c).length() <= getRadius())
		{
			p->writeToStream(file);
		}
	}
}

void SelectionDisk::show(bool b)
{
	unsigned int mask = b ? 0xffffffff : 0x0;
	_xform->setNodeMask(mask);
}

void SelectionDisk::remove()
{
	PCVR_Selection::remove();

	_xform->getParent(0)->removeChild(_xform);
}