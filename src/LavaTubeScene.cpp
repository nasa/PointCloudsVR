#include <filesystem>

#include <QComboBox>

#include "LavaTubeScene.hpp"

namespace fs = std::experimental::filesystem;

LavaTubeScene::LavaTubeScene() : PCVR_Scene::PCVR_Scene()
{
	_sceneType = "lavatube";
}

void LavaTubeScene::initWindowAndVR()
{
	PCVR_Scene::initWindowAndVR();

	_windowProxy->setWorldUnitsPerMeter(100);
}

void LavaTubeScene::buildScene()
{
	PCVR_Scene::buildScene();

	_windowProxy->getGridPosition(0, 0)->setBackgroundColor(0, 0, 0);
	_windowProxy->getGridPosition(0, 0)->setSkySphereStarData("../../data/images/Stars_HYGv3.txt", -2.0, 8.0, 40000, 1.0, 4.0, 0.1);

	// Set Earth parameters
	osg::ref_ptr<OpenFrames::Model> earth = new OpenFrames::Model("Earth", 0, 1, 0, 1);
	earth->setModel("../../data/osgearth/earthmap_finesse.earth");

	// Create / Add custom elevation-based color-ramp image layer
	// Adding sample one here for now -- need to add GUI later for setting region / elevation max / min
	osg::Node *earthNode = earth->getModel();
	_mapNode = osgEarth::MapNode::get(earthNode);
	osgEarth::LayerVector layers;
	_mapNode->getMap()->getLayers(layers);
	// Store Elevation Layers for Color Ramp association in GUI
	for (LayerVector::iterator iter = layers.begin(); iter != layers.end(); ++iter)
	{
		Layer* layer = iter->get();
		osg::Node* node = layer->getOrCreateNode();
		if (layer->getName().find("Elevation") != std::string::npos)
		{
			_elevationLayers.push_back(dynamic_cast<osgEarth::ElevationLayer*>(layer));
		}
	}
	// Get list of ramp files in ramp file directory
	std::string path = "../../data/osgearth/color_ramp_files";
	for (auto & fname : fs::directory_iterator(path))
	{
		std::string fileString = fname.path().filename().string();
		_colorRampFiles.push_back(fileString);
	}
	_rampImageLayer = dynamic_cast<osgEarth::ImageLayer*>(
		_mapNode->getMap()->getLayerByName("Color_Ramp_COTM_BD_Mushrooms_bin_avg_simp_1cm_32F_Hill"));
	osgEarth::TileSource* tileSource = _rampImageLayer->getTileSource();
	_colorRampTileSource = dynamic_cast<ColorRampTileSource*>(tileSource);
	_transferFunction = _colorRampTileSource->getTransferFunction();

	//// Green
	//_transferFunction->setColor(0, osg::Vec4(0, 1, 0, 1));
	//_transferFunction->setColor(1490, osg::Vec4(0, 1, 0, 1));
	//_transferFunction->setColor(1491, osg::Vec4(0, 1, 0, 1));

	// Yello
	/*_transferFunction->setColor(0, osg::Vec4(1, 1, 0, 1));
	_transferFunction->setColor(1490, osg::Vec4(1, 1, 0, 1));
	_transferFunction->setColor(1491, osg::Vec4(1, 1, 0, 1));*/

	//osgDB::ReaderWriter* readerWriter = osgDB::Registry::instance()->getReaderWriterForExtension("colorramp");

	// Eventually choose layers in GUI -- then use string name in code via Qt slot
	/*osgEarth::ElevationLayer* elevationLayer = dynamic_cast<osgEarth::ElevationLayer*>(
		mapNode->getMap()->getLayerByName("COTM_BD_Mushrooms_bin_avg_simp_1cm_32F"));
	osgEarth::DataExtentList dataExtents = elevationLayer->getDataExtents();
	osgEarth::GeoExtent geoExtent = dynamic_cast<osgEarth::Layer*>(elevationLayer)->getExtent();*/
	
	// Use data extents and geo extent to construct a new ImageLayer at desired min / max with color range and set transparency

	/*osg::TransferFunction1D* transfer(new osg::TransferFunction1D());
	std::ifstream in("../../../LandscapeData/VR - Mushrooms/elevation_ramp.clr");
	float value;
	unsigned int r, g, b, a;
	while (in >> value >> r >> g >> b >> a)
	{
		transfer->setColor(value, osg::Vec4((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0));
	}*/

	/*colorTransferFunction->setColor(-10000, osg::Vec4(0.0, 0.0, 120.0 / 255.0, 1.0));
	colorTransferFunction->setColor(0, osg::Vec4(0.0, 0.0, 1.0, 1.0));
	colorTransferFunction->setColor(1e-6, osg::Vec4(0.0, 150.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(500, osg::Vec4(0.0, 200.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(1000, osg::Vec4(110.0 / 255.0, 210.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(1500, osg::Vec4(220.0 / 255.0, 220.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(2000, osg::Vec4(220.0 / 255.0, 197.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(2500, osg::Vec4(220.0 / 255.0, 172.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(3000, osg::Vec4(220.0 / 255.0, 160.0 / 255.0, 0.0, 1.0));
	colorTransferFunction->setColor(3500, osg::Vec4(185.0 / 255.0, 155.0 / 255.0, 75.0 / 255.0, 1.0));
	colorTransferFunction->setColor(4000, osg::Vec4(170.0 / 255.0, 155.0 / 255.0, 110.0 / 255.0, 1.0));
	colorTransferFunction->setColor(4500, osg::Vec4(150.0 / 255.0, 150.0 / 255.0, 150.0 / 255.0, 1.0));
	colorTransferFunction->setColor(5000, osg::Vec4(190.0 / 255.0, 190.0 / 255.0, 190.0 / 255.0, 1.0));
	colorTransferFunction->setColor(5500, osg::Vec4(230.0 / 255.0, 230.0 / 255.0, 230.0 / 255.0, 1.0));
	colorTransferFunction->setColor(6000, osg::Vec4(250.0 / 255.0, 250.0 / 255.0, 250.0 / 255.0, 1.0));
	colorTransferFunction->setColor(9000, osg::Vec4(1.0, 1.0, 1.0, 1.0));
	colorTransferFunction->updateImage();*/

	/*osgEarth::Util::ContourMap* contourMap = new osgEarth::Util::ContourMap();
	contourMap->setTransferFunction(transfer);
	osgEarth::TerrainEngineNode* terrainEN = mapNode->getTerrainEngine();
	terrainEN->addEffect(contourMap);*/

	_rootFrame->addChild(earth);

	osg::BoundingSphere indianTunnelBound; // Cumulative bounding sphere for all tunnel tilesets

	osg::ref_ptr<OpenFrames::Model> tunnel = new OpenFrames::Model("Indian Tunnel", 1, 0, 0, 0.9);
	tunnel->setModel("../../../LandscapeData/IndianTunnel_tiled_full/tileset.lastile");

	osg::BoundingSphere bound = tunnel->getModel()->getBound();
	indianTunnelBound.expandBy(bound);

	// To visually align with ground
	osg::Vec3 center = bound._center * (bound._center.normalize() + 10);
	tunnel->setPosition(center);
	tunnel->showAxes(false);
	tunnel->showAxesLabels(false);
	tunnel->moveZAxis(osg::Vec3d(), 1.0);

	// Create a DrawableTrajectory to hold this tileset's center marker
	// Note that we can't share one DrawableTrajectory since it uses an
	// OpenGL uniform variable, which would be set to the last tileset's
	// position. So we must use one DrawableTrajectory per tileset.
	osg::ref_ptr<OpenFrames::DrawableTrajectory> drawcenter = new OpenFrames::DrawableTrajectory("center marker");
	drawcenter->showAxes(false);
	drawcenter->showAxesLabels(false);
	drawcenter->showNameLabel(false);

	osg::ref_ptr<OpenFrames::MarkerArtist> centermarker = new OpenFrames::MarkerArtist();
	centermarker->setMarkerShader("Shaders/Marker_CirclePulse.frag");
	centermarker->setMarkerSize(15);
	drawcenter->addArtist(centermarker);

	tunnel->addChild(drawcenter);
	earth->addChild(tunnel);

	// Create views
	osg::ref_ptr<OpenFrames::View> view4 = new OpenFrames::View(earth, earth);

	view4->setTrackball(_pcvrTrackball);

	// Set Window params
	_windowProxy->getGridPosition(0, 0)->addView(view4);

	/*mapNode->getMap()->removeLayer(imageLayer);
	mapNode->getMap()->addLayer(imageLayer);*/
	// Assign elevation layers and ramp files to ComboBoxes
	for (int i = 0; i < 2; i++)
	{
		for (auto itr = _elevationLayers.begin(); itr != _elevationLayers.end(); ++itr)
		{
			_elevationLayerComboBox[i]->addItem(QString((*itr)->getName().c_str()));
		}
		/*_elevationLayerComboBox[i]->setCurrentIndex(-1);
		_elevationLayerComboBox[i]->setCurrentIndex(0);*/
		for (auto itr = _colorRampFiles.begin(); itr != _colorRampFiles.end(); ++itr)
		{
			_colorRampFileComboBox[i]->addItem(QString((*itr).c_str()));
		}
		/*_colorRampFileComboBox[i]->setCurrentIndex(-1);
		_colorRampFileComboBox[i]->setCurrentIndex(0);*/
	}
}

void LavaTubeScene::setupMenuEventListeners(PCVR_Controller* controller)
{
	PCVR_Scene::setupMenuEventListeners(controller);
	QWidget* controllerWidget = controller->getPanelWidget();
	int cIndex = controller == PCVR_Controller::Left() ? 0 : 1;

	_elevationLayerComboBox[cIndex] = controllerWidget->findChild<QComboBox*>("ElevationComboBox");
	_colorRampFileComboBox[cIndex] = controllerWidget->findChild<QComboBox*>("ColorRampFileComboBox");
	//// Assign elevation layers and ramp files to ComboBoxes
	//for (auto itr = _elevationLayers.begin(); itr != _elevationLayers.end(); ++itr)
	//{
	//	elevationLayerComboBox->addItem(QString((*itr)->getName().c_str()));
	//}
	//elevationLayerComboBox->setCurrentIndex(-1);
	//elevationLayerComboBox->setCurrentIndex(0);
	//for (auto itr = _colorRampFiles.begin(); itr != _colorRampFiles.end(); ++itr)
	//{
	//	colorRampFileComboBox->addItem(QString((*itr).c_str()));
	//}
	//colorRampFileComboBox->setCurrentIndex(-1);
	//colorRampFileComboBox->setCurrentIndex(0);
	QObject::connect(_elevationLayerComboBox[cIndex], static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
		[=](int index) {setElevationLayer(index);});
	QObject::connect(_colorRampFileComboBox[cIndex], static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
		[=](int index) {setColorRampFile(index);});
}

void LavaTubeScene::setElevationLayer(int index)
{

}

void LavaTubeScene::setColorRampFile(int index)
{
	/*_transferFunction = _colorRampTileSource->loadCLRFile(
		"../../data/osgearth/color_ramp_files/" + _colorRampFiles.at(index));*/
	/*std::string filename = "../../data/osgearth/color_ramp_files/" + _colorRampFiles.at(index);
	std::ifstream in(filename.c_str());
	float value;
	unsigned int r, g, b, a;
	while (in >> value >> r >> g >> b >> a)
	{
		_transferFunction->setColor(value, osg::Vec4((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0));
	}*/
	_FM->lock();
	_mapNode->getMap()->removeLayer(_rampImageLayer);
	std::string filename = "../../data/osgearth/color_ramp_files/" + _colorRampFiles.at(index);
	std::ifstream in(filename.c_str());
	float value;
	unsigned int r, g, b, a;
	while (in >> value >> r >> g >> b >> a)
	{
		_transferFunction->setColor(value, osg::Vec4((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, (float)a / 255.0));
		//std::cout << value << (float)(r / 255.0) << (float)(g / 255.0) << (float)(b / 255.0) << (float)(a / 255.0) << "\n";
	}
	_mapNode->getMap()->addLayer(_rampImageLayer);
	_FM->unlock();
}