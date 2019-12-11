#include "PCVR_Scene.hpp"

#include "FreeformLineDrawer.hpp"

FreeformLine::FreeformLine(osg::ref_ptr<OpenFrames::DrawableTrajectory> parent, osg::ref_ptr<OpenFrames::Trajectory> traj)
	: OpenFrames::CurveArtist(traj)
	, Removable()
	, _parent(parent)
{
	setWidth(4.0);
	setColor(1, 0, 1);
	_parent->addArtist(this);
}

void FreeformLine::remove()
{
	Removable::remove();
	_parent->removeArtist(this);
}


osg::ref_ptr<OpenFrames::DrawableTrajectory> FreeformLineDrawer::_DrawableTraj;

FreeformLineDrawer::FreeformLineDrawer(osg::ref_ptr<OpenFrames::ReferenceFrame> rootFrame)
{
	PCVR_Scene::GetFrameManager()->lock();
	// Create DrawableTrajectory and add to world if static _DrawableTraj has not yet been created.
	if (_DrawableTraj == nullptr) {
		_DrawableTraj = new OpenFrames::DrawableTrajectory("Freeform Line DrawableTrajectory");
		_DrawableTraj->showNameLabel(false);
		_DrawableTraj->showAxes(false);
		_DrawableTraj->showAxesLabels(false);
		rootFrame->addChild(_DrawableTraj);
	}
	PCVR_Scene::GetFrameManager()->unlock();
}

void FreeformLineDrawer::update()
{
	// Only update if a controller is pressing trigger and thus is in drawing state.
	if (_drawingController != nullptr)
	{
		OpenFrames::Trajectory* traj = const_cast<OpenFrames::Trajectory*>(_DrawableTraj->getArtist(_DrawableTraj->getNumArtists() - 1)->getTrajectory());

		// Add current time (time since 1971) to trajectory.
		traj->addTime(time(nullptr));

		osg::Vec3d cPos = _drawingController->getWorldPos();
		traj->addPosition(cPos.x(), cPos.y(), cPos.z());
	}
}

void FreeformLineDrawer::handleVREvent(const vr::VREvent_t& ovrEvent)
{
	// Start drawing on trigger down.
	if (ovrEvent.eventType == vr::VREvent_ButtonPress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger
		&& _drawingController == nullptr)
	{
		PCVR_Scene::GetFrameManager()->lock();
		new FreeformLine(_DrawableTraj, new OpenFrames::Trajectory());
		_drawingController = PCVR_Controller::GetById(ovrEvent.trackedDeviceIndex);
		PCVR_Scene::GetFrameManager()->unlock();
	}
	// Stop drawing on trigger up.
	else if (ovrEvent.eventType == vr::VREvent_ButtonUnpress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger
		&& _drawingController != nullptr
		&& ovrEvent.trackedDeviceIndex == _drawingController->getId())
	{
		_drawingController = nullptr;
	}
}