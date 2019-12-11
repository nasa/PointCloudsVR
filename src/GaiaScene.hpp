#pragma once

#include <unordered_map>

#include <osg/Array>

#include <QObject>
#include <QApplication>
#include <QSlider>
#include <QLCDnumber>
#include <QFile>
#include <QUiLoader>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QLabel>
#include <QMenu>
#include <QThread>
#include <QFutureWatcher>
#include <QLayout>

#include "PCVR_Scene.hpp"
#include "GaiaStar.hpp"

typedef struct
{
	double abs_g_mag;
	double bp_rp;
	double mass;
} IsochroneEntry;

typedef struct
{
	std::string name;
	int millionYrs;	// parsed from name
	std::vector<IsochroneEntry*> entries;
	std::vector<GaiaStar*> starsMatchingTable;
} IsochroneTable;

// Stateless utility functions and variables
double deg2rad(double deg);
std::vector<std::string> split(const std::string &text, char sep);

class GaiaScene : public PCVR_Scene
{
	Q_OBJECT

public:
	GaiaScene();

	void parseArgs(osg::ArgumentParser& args) override;
	void initWindowAndVR() override;
	void buildScene() override;

	// Qt
	QLayout* _spheres[2];

private:
	// Command line parameters
	double _minPc = -DBL_MAX;
	double _maxPc = DBL_MAX;

	double _minParallax = 0.6;
	double _maxParallax = DBL_MAX;

	double _minMag = -DBL_MAX;
	double _maxMag = DBL_MAX;

	double _minTeff = -DBL_MAX;
	double _maxTeff = DBL_MAX;

	bool _magColor = false;

	// Qt
	QLabel* _positionValueLabel[2];
	QLabel* _yearLabel[2];
	QSlider* _yearIncSlider[2];
	QLayout* _groups[2];
	QLayout* _isochrones[2];

	osg::ref_ptr<osg::Switch> _ptSwitch = new osg::Switch();
	osg::ref_ptr<osg::Vec3Array> _ptVerts = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec3Array> _ptVertsOrig = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec3Array> _ptVels = new osg::Vec3Array();

	// Astrophysics variables
	int _yearIncrement = 0;
	long _currentYear = 0;
	int _straightVel = 0;

	std::vector<PCVR_Selectable*> _allStars;
	std::vector<std::vector<GaiaStar*>> _knownGroups;
	osg::ref_ptr<osg::Switch> _knownGroupSwitch = new osg::Switch();

	std::vector<IsochroneTable*> _isochroneTables;
	osg::ref_ptr<osg::Switch> _isochroneSwitch = new osg::Switch();

	void step(OpenFrames::FramerateLimiter& waitLimiter);
	void updateToYear(long year);

	void setKnownStars(std::unordered_map<long long, GaiaStar*>& knownStars);
	void readSpheres();
	void readIsochrones();

	void matchStarsInIsochrones();
	void markersForIsochrones();
	void markersForKnownStars();
	osg::Vec4 getHeatMapColor(double value);

	void setupMenuEventListeners(PCVR_Controller* controller) override;

	void setYearIncrement(int yearIncrement);
	void hideUnnamedStars(int state);
	void toggleIsochrone(bool checked, IsochroneTable* isoTable);

	void getPosition(PCVR_Controller* controller);
};