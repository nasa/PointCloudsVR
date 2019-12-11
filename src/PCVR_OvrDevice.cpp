#include "PCVR_OvrDevice.hpp"

osg::ref_ptr<OpenFrames::WindowProxy> PCVR_OvrDevice::_WinProxy;

void PCVR_OvrDevice::Initialize(osg::ref_ptr<OpenFrames::WindowProxy> winProxy)
{
	_WinProxy = winProxy;
}

osg::ref_ptr<OpenFrames::OpenVRDevice> PCVR_OvrDevice::GetOvrDevice()
{
	return const_cast<OpenFrames::OpenVRDevice*>(_WinProxy->getOpenVRDevice());
}

PCVR_OvrDevice::PCVR_OvrDevice(vr::TrackedDeviceIndex_t id)
	: _id(id)
	, _devModel(GetOvrDevice()->getDeviceModel(id))
{
}

vr::TrackedDeviceIndex_t PCVR_OvrDevice::getId() const
{
	return _id;
}

osg::Vec3d PCVR_OvrDevice::getWorldPos() const
{
	// Cannot store trackball, must always get new one from current view.
	osg::ref_ptr<OpenFrames::OpenVRTrackball> vrTrackball = static_cast<OpenFrames::OpenVRTrackball*>(_WinProxy->getGridPosition(0, 0)->getCurrentView()->getTrackball());

	// Get the controller origin in room coordinates
	// Note that transforming the origin is the same as getting the translation component of the device transformation matrix
	osg::Vec3d roomPos = _devModel->_rawDeviceToWorld.getTrans() * GetOvrDevice()->getWorldUnitsPerMeter();

	// Get the controller origin in world coordinates by multiplying Room->Trackball with Trackball->World matrices
	// Here, the FollowingTrackball parent transforms from Trackball space to World space
	return roomPos * vrTrackball->getRoomToTrackballMatrix() * vrTrackball->FollowingTrackball::getMatrix();
}

osg::Quat PCVR_OvrDevice::getOrientation() const
{
	osg::ref_ptr<OpenFrames::OpenVRTrackball> vrTrackball = static_cast<OpenFrames::OpenVRTrackball*>(_WinProxy->getGridPosition(0, 0)->getCurrentView()->getTrackball());

	osg::Quat roomQuat = _devModel->_rawDeviceToWorld.getRotate();

	osg::Quat worldQuat;
	worldQuat.set(vrTrackball->getRoomToTrackballMatrix() * vrTrackball->FollowingTrackball::getMatrix());

	return roomQuat * worldQuat;
}