#pragma once

#include <queue>

#include <osg/ArgumentParser>
#include <osg/ref_ptr>

#include <openvr.h>

#include <OpenFrames/CurveArtist.hpp>
#include <OpenFrames/DrawableTrajectory.hpp>
#include <OpenFrames/OpenVRDevice.hpp>
#include <OpenFrames/QWidgetPanel.hpp>
#include <OpenFrames/ReferenceFrame.hpp>
#include <OpenFrames/ReferenceFrame.hpp>
#include <OpenFrames/WindowProxy.hpp>

#include <QApplication>

#include "json.hpp"

#include "PCVR_Controller.hpp"
#include "PCVR_Selection.hpp"
#include "PCVR_Tool.hpp"
#include "PCVR_Trackball.hpp"

class PCVR_Scene : public QObject
{
	Q_OBJECT

	// Function Prototypes for Event Callbacks
	friend void keyPressCallback(unsigned int *winID,
		unsigned int *row, unsigned int *col, int *key);
	friend void vrEventCallback(unsigned int *winID,
		unsigned int *row, unsigned int *col, const OpenFrames::OpenVREvent *vrEvent);

public:
	PCVR_Scene();

	// Main public interface
	virtual void parseArgs(osg::ArgumentParser& args);
	virtual void initWindowAndVR();
	virtual void buildScene();
	virtual void run();

	// Create laser-triggered GUI panels for the VR world
	osg::ref_ptr<OpenFrames::QWidgetPanel> buildQtPanel(const std::string &name, const std::string &uiFile,
		QWidget* &panelWidget);

	static PCVR_Scene* Instance;
	std::string _sceneType;
	static osg::ref_ptr<OpenFrames::FrameManager> GetFrameManager();
	osg::ref_ptr<OpenFrames::WindowProxy> getWinProxy() const;
	osg::ref_ptr<OpenFrames::ReferenceFrame> getRootFrame() const;
	osg::ref_ptr<PCVR_Trackball> _pcvrTrackball;
	osg::ref_ptr<OpenFrames::OpenVRDevice> _ovrDevice;
	osg::ref_ptr<OpenFrames::WindowProxy> _windowProxy;

protected:
	// Arguments
	bool _useVR;
	unsigned int _winRes = 600;
	std::vector<std::string> _dataPaths;

	// OpenFrames
	static osg::ref_ptr<OpenFrames::FrameManager> _FM;
	osg::ref_ptr<OpenFrames::ReferenceFrame> _rootFrame;
	osg::ref_ptr<OpenFrames::View> _mainView;

	QApplication* _app;
	std::string _uiFilePath = ":Qt/menu/default.ui";

	bool _paused = false;
	PCVR_Tool* _currentTool = nullptr;
	std::queue<vr::VREvent_t> _eventQueue;
	std::vector<PCVR_Selectable*> _selectables;

	virtual void setupMenuEventListeners(PCVR_Controller* controller);
	virtual void switchToolTo(PCVR_Tool* t);
	virtual void handleVREvent(const vr::VREvent_t& ovrEvent);
	virtual void showSkyBox(bool b);

	virtual void step(OpenFrames::FramerateLimiter& waitLimiter);

	void saveProjectFile();
	virtual nlohmann::json& toJson();
};