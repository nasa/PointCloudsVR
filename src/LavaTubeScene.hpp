#pragma once

#include "PCVR_Scene.hpp"

#include <osg/TransferFunction>

#include <osgEarthDrivers/colorramp/EXPORT>
#include <osgEarthDrivers/colorramp/ColorRampTileSource>

#include <OpenFrames/MarkerArtist.hpp>
#include <OpenFrames/Model.hpp>

#include <osgEarth/MapNode>
#include <osgEarthUtil/ContourMap>
#include <osgEarth/TerrainEngineNode>
#include <osgEarth/TileSource>

#include <QObject>

class LavaTubeScene : public PCVR_Scene
{
public:
	LavaTubeScene();

	void initWindowAndVR() override;
	void buildScene() override;
private:
	ColorRampTileSource* _colorRampTileSource;
	osg::TransferFunction1D* _transferFunction;
	std::vector<osgEarth::ElevationLayer*> _elevationLayers;	// defined in osgEarth .earth file
	std::vector<std::string> _colorRampFiles;					// color ramp text files used by osgEarth
	osgEarth::ImageLayer* _rampImageLayer;
	osgEarth::MapNode* _mapNode;
	QComboBox* _elevationLayerComboBox[2];
	QComboBox* _colorRampFileComboBox[2];

	void setupMenuEventListeners(PCVR_Controller* controller) override;
	void LavaTubeScene::setElevationLayer(int index);
	void LavaTubeScene::setColorRampFile(int index);
};
