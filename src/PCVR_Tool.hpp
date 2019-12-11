#pragma once

#include <osg/Vec3>

#include <openvr.h>

#include "PCVR_Controller.hpp"

class PCVR_Tool
{
public:
	virtual void update();
	virtual void handleVREvent(const vr::VREvent_t& ovrEvent);
	virtual void stopUsingTool();
};

class DrawingTool : public PCVR_Tool
{
protected:
	PCVR_Controller* _drawingController = nullptr;
};