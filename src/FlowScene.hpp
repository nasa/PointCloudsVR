//-------------------------------------------------------------------------------------------------
// FlowScene.h:
//	Author: Nargess Memarsadeghi, Code 586, NASA / GSFC, January-October 2018
//  This class supports reading and visualizng phytoplanton netcdf data sets obained from 
//  MIT's Darwin project.
//----------------------------------------------------------------------------------------------=

#pragma once

#include <string>
#include "PCVR_Scene.hpp"
#include "DarwinNetcdfs.hpp"

#include <OpenFrames/DrawableTrajectory.hpp>
#include "PCVR_Selection.hpp"

class FlowScene : public PCVR_Scene
{
public:
	FlowScene();
	void parseArgs(osg::ArgumentParser& args) override;
	void buildScene() override;

	void addPoints(const float phyto[DarwinNetcdfs::NZ][DarwinNetcdfs::NLAT][DarwinNetcdfs::NLON], const osg::Vec4 colors, float scaling_factor);
	void addAll();

private:
	DarwinNetcdfs darwinData;

	int _datasetNum = 0;
	osg::ref_ptr<osg::Switch> _ptSwitch = new osg::Switch();

	std::vector<osg::ref_ptr<osg::Vec3Array>> ptVerts;
	std::vector<osg::ref_ptr<osg::Vec4Array>> ptColors;


	std::vector<vector<float>> filteredDensities;
	std::vector<vector<float>> allCoco;
	std::vector<vector<float>> allDino;
	std::vector<vector<float>> allDiatom;
	std::vector<vector<float>> allProk;

	//default values are set 
	float redScale = darwinData.dinoScale;
	float greenScale = darwinData.diatomScale ;
	float blueScale  = darwinData.cocoScale;
	float alphaScale = 1.0;

	string redText   = "Dinoflagellates";
	string blueText  = "Coccolithophores";
	string greenText ="Diatoms";

	long ptCount;


	void updateColorChannel(std::string colorText, int colorIndex,float scale);
	void FlowScene::updateAlpha(float alpha);

	void step(OpenFrames::FramerateLimiter& waitLimiter) override;
	void setupMenuEventListeners(PCVR_Controller* controller) override;
};