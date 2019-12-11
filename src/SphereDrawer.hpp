#pragma once

#include <OpenFrames/Sphere.hpp>

#include "PCVR_Controller.hpp"
#include "PCVR_Tool.hpp"
#include "PCVR_Selection.hpp"

class SelectionSphere : public OpenFrames::Sphere, public PCVR_Selection
{
public:
	SelectionSphere(osg::Vec3 pos);

	virtual void save(const std::string& path, const std::vector<PCVR_Selectable*>& points) override;
	virtual void show(bool b) override;
	virtual void remove() override;
};

template <typename S>
class SphereDrawer : public DrawingTool
{
public:
	SphereDrawer(osg::ref_ptr<OpenFrames::FrameManager> fm);
	void update() override;
	void handleVREvent(const vr::VREvent_t& ovrEvent) override;

private:
	osg::ref_ptr<OpenFrames::FrameManager> _fm;
	SelectionSphere* _currentSphere = nullptr;
};

template <typename S>
SphereDrawer<S>::SphereDrawer(osg::ref_ptr<OpenFrames::FrameManager> fm)
	: _fm(fm)
{
}

template <typename S>
void SphereDrawer<S>::update()
{
	// Only update if a controller is pressing trigger and thus is in drawing state.
	// Also only update on every 5th frame to save GPU.
	static int tick = 0;
	if (_drawingController != nullptr && tick % 5 == 0)
	{
		osg::Vec3d sphereOrigin;
		_currentSphere->getSpherePosition(sphereOrigin);
		double radius = (_drawingController->getWorldPos() - sphereOrigin).length();

		_fm->lock();
		_currentSphere->setRadius(radius);
		_currentSphere->setName("Selection Sphere, Radius = " + std::to_string(radius));
		_fm->unlock();
	}
	tick++;
}

template <typename S>
void SphereDrawer<S>::handleVREvent(const vr::VREvent_t& ovrEvent)
{
	// Start drawing on trigger down.
	if (ovrEvent.eventType == vr::VREvent_ButtonPress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger
		&& _drawingController == nullptr)
	{
		if (_currentSphere != nullptr) _currentSphere->remove();

		_drawingController = PCVR_Controller::GetById(ovrEvent.trackedDeviceIndex);
		_currentSphere = new S(_drawingController->getWorldPos());

		_fm->lock();
		_fm->getFrame()->addChild(_currentSphere);
		_fm->unlock();
	}
	// Stop drawing on trigger up.
	else if (ovrEvent.eventType == vr::VREvent_ButtonUnpress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger
		&& _drawingController != nullptr
		&& ovrEvent.trackedDeviceIndex == _drawingController->getId())
	{
		_drawingController = nullptr;
	}
	/*else if (ovrEvent.eventType == vr::VREvent_ButtonUnpress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Touchpad)
	{
		if (PCVR_Scene::Instance->_sceneType == "gaia")
		{
			GaiaScene* gaiaScene = dynamic_cast<GaiaScene*>(PCVR_Scene::Instance);
			for (int i = 0; i < 2; i++)
			{
				QCheckBox* check = new QCheckBox(QString::fromStdString(fileName));
				QObject::connect(check, &QCheckBox::clicked, gaiaScene,
					[=](bool checked) {
					_currentSphere->show(checked);
				});
				gaiaScene->_spheres[i]->addWidget(check);
			}
		}
	}*/
}