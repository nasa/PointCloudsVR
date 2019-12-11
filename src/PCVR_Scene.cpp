#include <filesystem>
#include <iostream>

#include <QCheckBox>
#include <QFile>
#include <QFont>
#include <QuiLoader>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>

#include "json.hpp"

#include "FreeformLineDrawer.hpp"
#include "StraightLineDrawer.hpp"
#include "SphereDrawer.hpp"
#include "DiskDrawer.hpp"
#include "PCVR_Trackball.hpp"

#include "PCVR_Scene.hpp"
#include "MarsScene.hpp"

namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

PCVR_Scene* PCVR_Scene::Instance;
osg::ref_ptr<OpenFrames::FrameManager> PCVR_Scene::_FM;

osg::ref_ptr<OpenFrames::FrameManager> PCVR_Scene::GetFrameManager()
{
	return _FM;
}

osg::ref_ptr<OpenFrames::WindowProxy> PCVR_Scene::getWinProxy() const
{
	return _windowProxy;
}

osg::ref_ptr<OpenFrames::ReferenceFrame> PCVR_Scene::getRootFrame() const
{
	return _rootFrame;
}


PCVR_Scene::PCVR_Scene()
{
	PCVR_Scene::Instance = this;
}

void PCVR_Scene::parseArgs(osg::ArgumentParser& args)
{
	_useVR = !args.read("--novr");
	args.read("--winRes", _winRes);

	std::string dataPath;
	while (args.read("--data", dataPath))
	{
		_dataPaths.push_back(dataPath);
	}

	_app = new QApplication(args.argc(), args.argv());
}

void PCVR_Scene::initWindowAndVR()
{
	// Create the window interface
	unsigned int x = 30, y = 30;
	unsigned int nrows = 1, ncols = 1;
	bool isEmbedded = false;
	_windowProxy = new OpenFrames::WindowProxy(x, y, _winRes, _winRes, nrows, ncols, isEmbedded, _useVR);

	_windowProxy->setWorldUnitsPerMeter(1.0);
	_windowProxy->setWorldUnitsPerMeterLimits(1.0, DBL_MAX);
	_windowProxy->setKeyPressCallback(keyPressCallback);
	_windowProxy->setWindowCaptureFile("../../data/images/screenshot", "png");

	_rootFrame = new OpenFrames::ReferenceFrame("Root");
	_FM = new OpenFrames::FrameManager(_rootFrame);
	_windowProxy->setScene(_FM, 0, 0);

	if (_useVR)
	{
		_ovrDevice = const_cast<OpenFrames::OpenVRDevice*>(_windowProxy->getOpenVRDevice());
		_ovrDevice->updateDeviceModels();
		_windowProxy->setVREventCallback(vrEventCallback);
		_pcvrTrackball = new PCVR_Trackball(_ovrDevice);

		PCVR_OvrDevice::Initialize(_windowProxy);
		PCVR_Controller::InitializeControllers();
	}
}

void PCVR_Scene::buildScene()
{
	_rootFrame->showAxes(false);
	_rootFrame->showAxesLabels(false);
	_rootFrame->showNameLabel(false);

	_mainView = new OpenFrames::View();
	_mainView->setTrackball(_pcvrTrackball);

	_windowProxy->getGridPosition(0, 0)->addView(_mainView);

	// Remove floor grid.
	//osg::ref_ptr<osg::MatrixTransform> floorGrid = _ovrDevice->getGroundPlane();
	//floorGrid->getParent(0)->removeChild(floorGrid);
	if (_useVR)
	{
		PCVR_Controller::Left()->buildControllerPanel(_uiFilePath);
		PCVR_Controller::Right()->buildControllerPanel(_uiFilePath);

		setupMenuEventListeners(PCVR_Controller::Left());
		setupMenuEventListeners(PCVR_Controller::Right());
	}
}

void PCVR_Scene::run()
{
	// Run OpenFrames
	const double desiredFPS = 90;
	OpenFrames::FramerateLimiter waitLimiter(desiredFPS);

	// Main Event Loop
	_windowProxy->start();
	while (_windowProxy->isRunning())
	{
		step(waitLimiter);
	}
}

// End of public interface

void PCVR_Scene::step(OpenFrames::FramerateLimiter& waitLimiter)
{
	QCoreApplication::processEvents(QEventLoop::AllEvents, 1000 / waitLimiter.getDesiredFramerate());
	if (_useVR)
	{
		while (!_eventQueue.empty())
		{
			handleVREvent(_eventQueue.front());
			_eventQueue.pop();
		}

		if (_currentTool != nullptr) _currentTool->update();
	}
	
	waitLimiter.frame();
}

void PCVR_Scene::setupMenuEventListeners(PCVR_Controller* controller)
{
	QWidget* controllerWidget = controller->getPanelWidget();

	QRadioButton* drawAction = controllerWidget->findChild<QRadioButton*>("drawButton");
	QObject::connect(drawAction, &QRadioButton::clicked, this,
		[=]() { switchToolTo(new FreeformLineDrawer(_rootFrame)); });

	QRadioButton* drawStraightAction = controllerWidget->findChild<QRadioButton*>("drawStraightButton");
	QObject::connect(drawStraightAction, &QRadioButton::clicked, this,
		[=]() { switchToolTo(new StraightLineDrawer(_rootFrame)); });

	QPushButton* resetToolsAction = controllerWidget->findChild<QPushButton*>("resetButton");
	QObject::connect(resetToolsAction, &QPushButton::clicked, this,
		[=]() {
		switchToolTo(nullptr);
		Removable::RemoveAll();
	});
}

void PCVR_Scene::switchToolTo(PCVR_Tool* t)
{
	if (_currentTool != nullptr) _currentTool->stopUsingTool();
	_currentTool = t;
}

void PCVR_Scene::showSkyBox(bool b)
{
	osg::ref_ptr<OpenFrames::SkySphere> skySphere = _windowProxy->getGridPosition(0, 0)->getSkySphere();
	if (b)
	{
		skySphere->setDrawMode(2);
	}
	else {
		skySphere->setDrawMode(0);
	}
}

void PCVR_Scene::handleVREvent(const vr::VREvent_t& ovrEvent)
{
	if (_currentTool != nullptr) _currentTool->handleVREvent(ovrEvent);

	if (ovrEvent.eventType == vr::VREvent_ButtonPress)
	{
		switch (ovrEvent.data.controller.button)
		{

		case vr::k_EButton_ApplicationMenu:
		{
			// If grip and menu button both clicked, take a screenshot (without changing menu state).
			vr::VRControllerState_t* controllerState = _ovrDevice->getDeviceModel(ovrEvent.trackedDeviceIndex)->_controllerState;
			if (controllerState->ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip))
			{
				_windowProxy->captureWindow();
				break;
			}

			// Menu button opens and closes menus.
			osg::ref_ptr<OpenFrames::QWidgetPanel> panel = PCVR_Controller::GetById(ovrEvent.trackedDeviceIndex)->getPanel();
			panel->showContents(!panel->getContentsShown());
			break;
		}

		case vr::k_EButton_SteamVR_Touchpad:
			// If no tool being used, then touchpad press will pause/play.
			if (_currentTool == nullptr)
			{
				_paused = !_paused;
			}
			else // If a tool is being used, then touchpad press will stop using that tool.
			{
				switchToolTo(nullptr);
			}
			break;

		case vr::k_EButton_Grip:
			// If grip and touchpad are pressed at same time, then switch views 
			// Note that for this to work properly, the grip button should be pressed first, then touchpad.
			if (_ovrDevice->getDeviceModel(ovrEvent.trackedDeviceIndex)->_controllerState->ulButtonPressed &
				vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad))
			{
				_windowProxy->getGridPosition(0, 0)->nextView();
			}
			break;
		}
	}
}

osg::ref_ptr<OpenFrames::QWidgetPanel> PCVR_Scene::buildQtPanel(const std::string &name, 
	const std::string &uiPanelFile, QWidget* &panelWidget)
{
	// 1) Create, size, orient, position, set other attributes
	// and addChild() QWidgetPanel to controller _modelTransform.

	osg::ref_ptr<OpenFrames::QWidgetPanel> panel = new OpenFrames::QWidgetPanel(name);
	panel->showNameLabel(false);
	panel->showAxes(false);
	panel->showAxesLabels(false);
	panel->showContents(false); // set panel hidden initially

	// Make panel face user (panel +y -> controller -z)
	panel->setAttitude(osg::Quat(osg::PI_2, osg::Vec3d(-1.0, 0.0, 0.0)));
	panel->setColor(osg::Vec4(0, 0, 0, 0));
	//_devModel->_modelTransform->addChild(panel->getGroup());

	// 2) Load QWidget from .ui file and assign to QWidgetPanel
	QFile* uiFile = new QFile(QString(uiPanelFile.c_str()));
	uiFile->open(QIODevice::ReadOnly);
	QUiLoader loader;
	panelWidget = loader.load(uiFile);
	panelWidget->setAttribute(Qt::WA_TranslucentBackground);

	double scale = 0.25;
	panel->setSize(scale * panelWidget->width(), scale * panelWidget->height());
	//panel->setPosition(-scale * panelWidget->width() / 2.0, 0.05, 0.0);
	panel->setWidget(panelWidget);

	// 3) Create and set OpenVRImageHandler for panel
	osg::ref_ptr<OpenFrames::OpenVRImageHandler> ih = new OpenFrames::OpenVRImageHandler(PCVR_OvrDevice::GetOvrDevice(), panel->getImage());
	ih->setSelectedColor(osg::Vec4(0.0, 0.5, 0.9, 1.0));
	ih->setSelectedWidth(4.0);
	panel->setImageHandler(ih);
	return panel;
}

void PCVR_Scene::saveProjectFile()
{
	std::string path = "../../data/projects/";
	int fileNum = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{}) + 1;
	std::string filepath = path + "project" + std::to_string(fileNum) + ".json";

	std::ofstream file;
	file.open(filepath);

	json j = {
		{"scene", toJson()}
	};
	file << j << std::endl;
}

json& PCVR_Scene::toJson()
{
	const OpenFrames::OpenVRDevice::DeviceModel* hmd;
	for (unsigned int j = 0; j < _ovrDevice->getNumDeviceModels(); j++)
	{
		hmd = _ovrDevice->getDeviceModel(j);
		if (hmd->_class == OpenFrames::OpenVRDevice::DeviceClass::HMD)
		{
			break;
		}
	}
	
	osg::ref_ptr<OpenFrames::OpenVRTrackball> vrTrackball = static_cast<OpenFrames::OpenVRTrackball*>(_windowProxy->getGridPosition(0, 0)->getCurrentView()->getTrackball());
	osg::Vec3 hmdRoomPos = hmd->_rawDeviceToWorld.getTrans() * _ovrDevice->getWorldUnitsPerMeter();
	osg::Vec3 hmdWorldPos = hmdRoomPos * vrTrackball->getRoomToTrackballMatrix() * vrTrackball->FollowingTrackball::getMatrix();

	json* j = new json {
		//{"pos", hmdWorldPos},
		//{"orientation", ""}
	};
	return *j;
}

// Callback to handle key press events
void keyPressCallback(unsigned int *winID,
	unsigned int *row, unsigned int *col, int *key)
{
	// Pause/unpause animation
	if (*key == 't')
	{
		//PCVR_Scene::Instance->_windowProxy->pauseTime(!PCVR_Scene::Instance->_windowProxy->isTimePaused());
		PCVR_Scene::Instance->_paused = !PCVR_Scene::Instance->_paused;
	}
	if (PCVR_Scene::Instance->_sceneType == "mars")
	{
		MarsScene* marsScene = dynamic_cast<MarsScene*>(PCVR_Scene::Instance);
		if (*key == 'a')
		{
			static bool axesShown = false;
			axesShown = !axesShown;
			marsScene->showAxes(axesShown);
		}
		if (*key == 'm')
		{
			static bool marsShown = true;
			marsShown = !marsShown;
			marsScene->showMars(marsShown);
		}
		if (*key == 'l')
		{
			marsScene->showMode(ShowMode::ShortTrails);
		}
		else if (*key == 'b')
		{
			marsScene->showMode(ShowMode::FullTrails);
		}
		else if (*key == 'p')
		{
			marsScene->showMode(ShowMode::Particles);
		}
	}
}

// Callback to handle VR controller events 
// This is called by WindowProxy when a new OpenVR event is detected
void vrEventCallback(unsigned int *winID, unsigned int *row, unsigned int *col,
	const OpenFrames::OpenVREvent *vrEvent)
{
	PCVR_Scene* scene = PCVR_Scene::Instance;

	// Get OpenVR event data; see openvr.h and online documentation/examples for details 
	const vr::VREvent_t* ovrEvent = vrEvent->_ovrEvent;  // OpenVR data type 

	// OpenVR will sometimes send a dummy event with an invalid device ID; ignore those
	if (ovrEvent->trackedDeviceIndex >= scene->_ovrDevice->getNumDeviceModels() ||
		// Ignore event if it came from device that's not a controller (e.g. headset or tracking events) s
		scene->_ovrDevice->getDeviceModel(ovrEvent->trackedDeviceIndex)->_class != OpenFrames::OpenVRDevice::CONTROLLER) {
		return;
	}

	scene->_eventQueue.push(*ovrEvent);
}