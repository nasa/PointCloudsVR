#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <QRadioButton>
#include <QStackedWidget>

#include "csv.h"
#include "MarsScene.hpp"

namespace fs = std::experimental::filesystem;

Seed::Seed(int offset, osg::ref_ptr<osg::Vec3Array> verts, osg::ref_ptr<osg::Vec4Array> colors)
	: frameOffset(offset)
	, ptVerts(verts)
	, ptColors(colors)
{
}

#define YELLOW osg::Vec4(1, 1, 0.5, 0)
#define TRAIL_COLOR osg::Vec4(0.227, 0.147, 0.472, 0)
#define TRAIL_DOT_COLOR osg::Vec4(1, 1, 0.5, 0)

MarsScene::MarsScene() : PCVR_Scene::PCVR_Scene()
{
	_sceneType = "mars";
}

void MarsScene::buildScene()
{
	_uiFilePath = ":Qt/menu/mars.ui";
	PCVR_Scene::buildScene();

	_windowProxy->getGridPosition(0, 0)->setBackgroundColor(0, 0, 0);
	_windowProxy->getGridPosition(0, 0)->setSkySphereStarData("../../data/images/Stars_HYGv3.txt", -2.0, 8.0, 40000, 1.0, 4.0, 0.1);

	_ptSwitch = new osg::Switch();
	std::unordered_map<std::string, Seed*> seedMap;
	int frameNum = 0;
	for (auto& path : _dataPaths)
	{
		for (auto& fname : fs::directory_iterator(path))
		{
			osg::ref_ptr<osg::Vec3Array> ptVerts = new osg::Vec3Array();
			osg::ref_ptr<osg::Vec4Array> ptColors = new osg::Vec4Array();

			if (fname.path().extension() == ".txt" || fname.path().extension() == ".csv")
			{
				io::CSVReader<6> in(fname.path().string());
				std::cout << "Reading " << in.next_line() << std::endl;
				in.next_line(); // Skip pointcount and timestamp

				double x, y, z;
				std::string seedId, lod, scalar;
				while (in.read_row(x, y, z, seedId, lod, scalar))
				{
					osg::Vec3 vert = osg::Vec3(x, y, z);

					Seed* seed;
					auto itr = seedMap.find(seedId);
					if (itr == seedMap.end())
					{
						seed = new Seed(frameNum, new osg::Vec3Array(), new osg::Vec4Array());
						seedMap.insert(std::make_pair(seedId, seed));
						_seeds.push_back(seed);
					}
					else {
						seed = itr->second;
					}
					seed->ptVerts->push_back(vert);
					seed->ptColors->push_back(TRAIL_COLOR);

					ptVerts->push_back(vert);
					ptColors->push_back(YELLOW);
				}
			}
			else if (fname.path().extension() == ".bin")
			{
				std::cout << "Reading binary data file: " << fname.path().string() << std::endl;
				// Read in header lines
				std::ifstream inFile(fname.path().string(), std::ios::binary);
				char filePath[76];
				inFile.read(reinterpret_cast<char*>(filePath), 76 * sizeof(char));
				// Read in vertices into array
				int numPoints;
				inFile.read(reinterpret_cast<char*>(&numPoints), sizeof(int));
				double julianDate;
				inFile.read(reinterpret_cast<char*>(&julianDate), sizeof(double));
				// Read in seeds, and insert
				std::vector<float> xVec;
				std::vector<float> yVec;
				std::vector<float> zVec;
				float coord;
				for (int i = 0; i < numPoints; i++)
				{
					inFile.read(reinterpret_cast<char*>(&coord), sizeof(float));
					xVec.push_back(coord);
					inFile.read(reinterpret_cast<char*>(&coord), sizeof(float));
					yVec.push_back(coord);
					inFile.read(reinterpret_cast<char*>(&coord), sizeof(float));
					zVec.push_back(coord);
				}
				int seedInt;
				for (int i = 0; i < numPoints; i++)
				{
					inFile.read(reinterpret_cast<char*>(&seedInt), sizeof(int));
					osg::Vec3 vert = osg::Vec3(xVec.at(i), yVec.at(i), zVec.at(i));

					Seed* seed;
					auto itr = seedMap.find(std::to_string(seedInt));
					if (itr == seedMap.end())
					{
						seed = new Seed(frameNum, new osg::Vec3Array(), new osg::Vec4Array());
						seedMap.insert(std::make_pair(std::to_string(seedInt), seed));
						_seeds.push_back(seed);
					}
					else {
						seed = itr->second;
					}
					seed->ptVerts->push_back(vert);
					seed->ptColors->push_back(TRAIL_COLOR);

					ptVerts->push_back(vert);
					ptColors->push_back(YELLOW);
				}

				// dispose of vertex array
				xVec.clear();
				yVec.clear();
				zVec.clear();
			}
			else
			{
				std::cout << "Unsupported file extension -- need .txt, .csv, or .bin" << std::endl;
				exit(-1);
			}
			
			// Set up geom and geode for point cloud animation.
			osg::ref_ptr<osg::Geometry> frameGeom = new osg::Geometry();
			frameGeom->setUseDisplayList(true);
			frameGeom->setUseVertexBufferObjects(true);
			frameGeom->setVertexArray(ptVerts);
			frameGeom->setColorArray(ptColors, osg::Array::BIND_PER_PRIMITIVE_SET);
			frameGeom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, ptVerts->size()));

			osg::ref_ptr<osg::Geode> frameGeode = new osg::Geode();
			frameGeode->addDrawable(frameGeom);
			_ptSwitch->addChild(frameGeode, false);

			frameNum++;
		}
	}

	_trailColors = new osg::Vec4Array();
	for (int i = 0; i < 10; i++)
	{
		float mixParam = i / 9.0;
		/*osg::Vec4 headColor = osg::Vec4(1, 0, 0, 1);
		osg::Vec4 tailColor = osg::Vec4(1, 1, 0.5, 0);*/
		osg::Vec4 tailColor = osg::Vec4(1, 0, 0, 0);
		osg::Vec4 headColor = osg::Vec4(1, 1, 0.5, 1.0);
		osg::Vec4 color = headColor * mixParam + tailColor * (1 - mixParam);
		_trailColors->push_back(color);
	}
	for (int i = 0; i < _seeds.size(); i += 50)
	{
		Seed* seed = _seeds[i];
		if (seed->ptVerts->size() > 9)
		{
			osg::ref_ptr<osg::Geometry> trailGeom = new osg::Geometry();
			trailGeom->setUseDisplayList(true);
			trailGeom->setUseVertexBufferObjects(true);
			trailGeom->setVertexArray(seed->ptVerts);
			trailGeom->setColorArray(_trailColors, osg::Array::BIND_PER_VERTEX);
			trailGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 10));
			trailGeom->setNodeMask(0);

			_shortTrailsGeode->addDrawable(trailGeom);
		}
	}
	_shortTrailsGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(0.5), osg::StateAttribute::ON);

	/*osg::ref_ptr<osg::Program> program = new osg::Program();
	osg::ref_ptr<osg::StateSet> state = _shortTrailsGeode->getOrCreateStateSet();
	state->setAttributeAndModes(program, osg::StateAttribute::ON);
	//state->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);*/
	osg::ref_ptr<osg::StateSet> stateSet = _shortTrailsGeode->getOrCreateStateSet();
	stateSet->setMode(GL_BLEND, osg::StateAttribute::ON); // Enable transparency
	stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	_shortTrailsGeode->setNodeMask(0);

	// Create Particle Trail Geom's and Geodes / fill in with data
	_trailsGeode = new osg::Geode();
	for (int i = 0; i < _seeds.size(); i += 500)
	{
		Seed* seed = _seeds[i];
		osg::ref_ptr<osg::Geometry> trailGeom = new osg::Geometry();
		trailGeom->setUseDisplayList(true);
		trailGeom->setUseVertexBufferObjects(true);
		trailGeom->setVertexArray(seed->ptVerts);
		trailGeom->setColorArray(seed->ptColors, osg::Array::BIND_PER_VERTEX);
		trailGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, seed->ptVerts->size()));
		_trailsGeode->addDrawable(trailGeom);
	}
	_trailsGeode->setNodeMask(0);

	// Create Mars
	_mars = new Mars(_ovrDevice);

	// Set Mars parameters
	_mars->setAutoLOD(true);
	_mars->setTextureMap("../../data/images/Mars.png");
	_mars->moveXAxis(osg::Vec3d(), 10.0, .1, .1, .2); // At the origin, with a length of 1000
	_mars->moveYAxis(osg::Vec3d(), 10.0, .1, .1, .2);
	_mars->moveZAxis(osg::Vec3d(), 10.0, .1, .1, .2);
	_mars->showNameLabel(false);
	showAxes(false);

	_rootFrame->addChild(_mars);
	_rootFrame->getGroup()->addChild(_shortTrailsGeode);
	_rootFrame->getGroup()->addChild(_trailsGeode);
	_rootFrame->getGroup()->addChild(_ptSwitch);
}

void MarsScene::step(OpenFrames::FramerateLimiter& waitLimiter)
{
	if (_mode == ShowMode::FullTrails)
	{
		const double desiredFPS = 30;
		OpenFrames::FramerateLimiter waitLimiterFullTrails(desiredFPS);
		PCVR_Scene::step(waitLimiterFullTrails);
	}
	else
	{
		PCVR_Scene::step(waitLimiter);
	}
	//PCVR_Scene::step(waitLimiter);

	static int tick = 0;
	if (!_paused && tick % 2 == 0)
	{
		switch (_mode)
		{
		case ShowMode::Particles:
			_ptSwitch->setSingleChildOn(_datasetNum);
			_datasetNum = (_datasetNum + 1) % _ptSwitch->getNumChildren();
			break;

		case ShowMode::ShortTrails:
			animateShortTrails();
			_datasetNum = (_datasetNum + 1) % (_ptSwitch->getNumChildren() - 9);
			break;

		case ShowMode::FullTrails:
			animateFullTrails();
			_datasetNum = (_datasetNum + 1) % _ptSwitch->getNumChildren();
			/*if (tick % 100 == 0)
			{
				animateFullTrails();
				_datasetNum = (_datasetNum + 1) % _ptSwitch->getNumChildren();
			}*/
			break;
		}
	}
	tick++;
}

void MarsScene::animateShortTrails()
{
	_FM->lock();
	int trailCounter = 0;
	for (int j = 0; j < _seeds.size(); j += 50)
	{
		Seed* seed = _seeds[j];
		int i = _datasetNum - seed->frameOffset;

		if (seed->ptVerts->size() > 9)
		{
			// Turn off trail once disappears in dataset.
			if (_datasetNum == 0 || i + 9 == seed->ptVerts->size())
			{
				 _shortTrailsGeode->getDrawable(trailCounter)->setNodeMask(0);
			}

			// Only update visible trails.
			if (i + 9 < seed->ptVerts->size())
			{
				//_FM->lock();
				osg::Geometry* geom = static_cast<osg::Geometry*>(_shortTrailsGeode->getDrawable(trailCounter));
				// Show trail on first appearance.
				if (i == 0)
				{
					geom->setNodeMask(~0);
				}
				static_cast<osg::DrawArrays*>(geom->getPrimitiveSet(0))->setFirst(i);
				//osg::Vec4Array* colorArray = new osg::Vec4Array(i + 9);
				osg::Vec4Array* colorArray = new osg::Vec4Array;
				for (int k = 0; k < i; k++)
				{
					colorArray->push_back(osg::Vec4(0, 0, 0, 0));
				}
				for (int k = 0; k < 10; k++)
				{
					colorArray->push_back(_trailColors->at(k));
				}
				//std::copy(std::begin(*_trailColors), std::end(*_trailColors), std::begin((*colorArray)) + i);
				geom->setColorArray(colorArray, osg::Array::BIND_PER_VERTEX);
				//geom->dirtyGLObjects();
				//_FM->unlock();
			}
			trailCounter++;
		}

	}
	_FM->unlock();
}

void MarsScene::animateFullTrails()
{
	for (int j = 0; j < _seeds.size(); j += 500)
	{
		Seed* seed = _seeds[j];
		int i = _datasetNum - seed->frameOffset;

		if (0 <= i && i < seed->ptVerts->size())
		{
			if (i != 0)
			{
				seed->ptColors->at(i - 1) = TRAIL_COLOR;
			}
			seed->ptColors->at(i) = TRAIL_DOT_COLOR;
			seed->ptColors->dirty();
		}
		if (_datasetNum == (seed->frameOffset + seed->ptVerts->size()) % _ptSwitch->getNumChildren())
		{
			seed->ptColors->back() = TRAIL_COLOR;
			seed->ptColors->dirty();
		}
	}
}

void MarsScene::setupMenuEventListeners(PCVR_Controller* controller)
{
	PCVR_Scene::setupMenuEventListeners(controller);

	QWidget* controllerWidget = controller->getPanelWidget();

	QStackedWidget* stackedWidget = controllerWidget->findChild<QStackedWidget*>("stackedWidget");
	QObject::connect(controllerWidget->findChild<QRadioButton*>("viewRadioButton"), &QRadioButton::clicked, this,
		[=]() { stackedWidget->setCurrentIndex(0); });
	QObject::connect(controllerWidget->findChild<QRadioButton*>("toolsRadioButton"), &QRadioButton::clicked, this,
		[=]() { stackedWidget->setCurrentIndex(1); });

	QLCDNumber* lodLCD = controllerWidget->findChild<QLCDNumber*>("lcdNumber");
	lodLCD->setAcceptDrops(false);
	lodLCD->setSegmentStyle(QLCDNumber::SegmentStyle::Filled);
	lodLCD->setStyleSheet("QLCDNumber{color:rgb(85, 85, 255);background-color:rgb(213, 205, 241);}");

	QSlider* lodSlider = controllerWidget->findChild<QSlider*>("horizontalSlider");
	QObject::connect(lodSlider, &QSlider::valueChanged, this,
		[=](int lod) { setLOD(lod, lodLCD); });

	QRadioButton* showParticlesCheckBox = controllerWidget->findChild<QRadioButton*>("showParticlesRadioButton");
	QObject::connect(showParticlesCheckBox, &QRadioButton::clicked, this,
		[=]() { showMode(ShowMode::Particles); });

	QRadioButton* showShortCheckBox = controllerWidget->findChild<QRadioButton*>("showShortTrailsRadioButton");
	QObject::connect(showShortCheckBox, &QRadioButton::clicked, this,
		[=]() { showMode(ShowMode::ShortTrails); });

	QRadioButton* showFullTrailsCheckBox = controllerWidget->findChild<QRadioButton*>("showFullTrailsRadioButton");
	QObject::connect(showFullTrailsCheckBox, &QRadioButton::clicked, this,
		[=]() { showMode(ShowMode::FullTrails); });

	QCheckBox* marsCheckBox = controllerWidget->findChild<QCheckBox*>("MarsCheckBox");
	QObject::connect(marsCheckBox, &QCheckBox::stateChanged, this,
		[=](int state) { _mars->showContents(state == Qt::CheckState::Checked); });

	QCheckBox* axesCheckBox = controllerWidget->findChild<QCheckBox*>("MarsAxesCheckBox");
	QObject::connect(axesCheckBox, &QCheckBox::stateChanged, this,
		[=](int state) { showAxes(state == Qt::CheckState::Checked); });

	QCheckBox* showSkyboxCheckBox = controllerWidget->findChild<QCheckBox*>("showSkyboxCheckBox");
	QObject::connect(showSkyboxCheckBox, &QCheckBox::stateChanged, this,
		[=](int state) { showSkyBox(state == Qt::CheckState::Checked); });
}

void MarsScene::setLOD(int lod, QLCDNumber* lodLCD)
{
	// LOD not implemented yet.
	lodLCD->display(lod);
}

//void MarsScene::setMode(ShowMode mode)
//{
//	// Switch off old mode.
//	switch (_mode)
//	{
//	case ShowMode::Particles:
//		_mode = mode;
//		break;
//
//	case ShowMode::ShortTrails:
//		_mode = mode;
//		break;
//
//	case ShowMode::FullTrails:
//		_mode = mode;
//		break;
//	}
//}

void MarsScene::showMode(ShowMode mode)
{
	// Switch off old mode.
	switch (_mode)
	{
	case ShowMode::Particles:
		_ptSwitch->setAllChildrenOff();
		break;

	case ShowMode::ShortTrails:
		_shortTrailsGeode->setNodeMask(0);
		break;

	case ShowMode::FullTrails:
		_trailsGeode->setNodeMask(0);
		break;
	}

	// Switch on new mode.
	switch (mode)
	{
	case ShowMode::ShortTrails:
	{
		_shortTrailsGeode->setNodeMask(~0);
		break;
	}

	case ShowMode::FullTrails:
		_trailsGeode->setNodeMask(~0);
		break;
	}
	_mode = mode;
}

void MarsScene::showAxes(bool show)
{
	unsigned int axes = show ? OpenFrames::ReferenceFrame::X_AXIS | OpenFrames::ReferenceFrame::Y_AXIS | OpenFrames::ReferenceFrame::Z_AXIS
		: OpenFrames::ReferenceFrame::NO_AXES;

	_mars->showAxes(axes);
	_mars->showAxesLabels(axes);
}

void MarsScene::showMars(bool show)
{
	_mars->showContents(show);
}