#pragma once

#include <osg/Vec3>

#include <OpenFrames/OpenVRDevice.hpp>
#include <OpenFrames/QWidgetPanel.hpp>
#include <OpenFrames/WindowProxy.hpp>

#include <openvr.h>

#include "PCVR_OvrDevice.hpp"

class PCVR_ImageHandler : public OpenFrames::OpenVRImageHandler
{
public:
	PCVR_ImageHandler(osg::ref_ptr<OpenFrames::OpenVRDevice> ovrDevice, osg::Image* image);
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv) override;

protected:
	PickMode _lastPickMode = PickMode::NONE;
	PickMode _lastLastPickMode = PickMode::NONE;
	double _lastX;
	double _lastY;

	void processImagePick();
};

class PCVR_Controller : public PCVR_OvrDevice
{
public:
	static void InitializeControllers();
	static PCVR_Controller* Left();
	static PCVR_Controller* Right();
	static PCVR_Controller* GetById(vr::TrackedDeviceIndex_t deviceID);

	PCVR_Controller(vr::TrackedDeviceIndex_t id);

	osg::ref_ptr<OpenFrames::QWidgetPanel> getPanel() const;
	QWidget* getPanelWidget() const;

	void buildControllerPanel(std::string uiFilePath);

private:
	static PCVR_Controller* _LeftController;
	static PCVR_Controller* _RightController;

	osg::ref_ptr<OpenFrames::QWidgetPanel> _panel;
	QWidget* _panelWidget;
};