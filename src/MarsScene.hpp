#pragma once

#include <unordered_map>

#include <QCheckBox>
#include <QLCDNumber>
#include <QSlider>

#include "Mars.hpp"
#include "PCVR_Scene.hpp"

struct Seed
{
	Seed::Seed(int offset, osg::ref_ptr<osg::Vec3Array> verts, osg::ref_ptr<osg::Vec4Array> colors);

	int frameOffset;
	osg::ref_ptr<osg::Vec3Array> ptVerts;
	osg::ref_ptr<osg::Vec4Array> ptColors;
};

enum ShowMode {
	Particles,
	ShortTrails,
	FullTrails
};

class MarsScene : public PCVR_Scene
{
public:
	MarsScene();

	void buildScene() override;
	//void setMode(ShowMode mode);
	void showMode(ShowMode mode);
	void showAxes(bool show);
	void showMars(bool show);

private:
	osg::ref_ptr<Mars> _mars;
	osg::ref_ptr<osg::Switch> _ptSwitch;
	osg::ref_ptr<osg::Geode> _shortTrailsGeode = new osg::Geode();
	osg::ref_ptr<osg::Geode> _trailsGeode;
	osg::ref_ptr<osg::Vec4Array> _trailColors;

	std::vector<Seed*> _seeds;

	int _datasetNum = 0;
	ShowMode _mode = Particles;


	void step(OpenFrames::FramerateLimiter& waitLimiter) override;
	void animateShortTrails();
	void animateFullTrails();

	void setupMenuEventListeners(PCVR_Controller* controller) override;
	void setLOD(int lod, QLCDNumber* lodLCD);
};