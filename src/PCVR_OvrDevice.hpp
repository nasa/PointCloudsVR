#pragma once

#include <OpenFrames/OpenVRDevice.hpp>
#include <OpenFrames/WindowProxy.hpp>

#include <openvr.h>

class PCVR_OvrDevice
{
public:
	static void Initialize(osg::ref_ptr<OpenFrames::WindowProxy> winProxy);
	static osg::ref_ptr<OpenFrames::OpenVRDevice> GetOvrDevice();

	PCVR_OvrDevice(vr::TrackedDeviceIndex_t id);
	vr::TrackedDeviceIndex_t getId() const;

	osg::Vec3d getWorldPos() const;
	osg::Quat getOrientation() const;

protected:
	static osg::ref_ptr<OpenFrames::WindowProxy> _WinProxy;

	const vr::TrackedDeviceIndex_t _id;
	const OpenFrames::OpenVRDevice::DeviceModel* _devModel;
};

class ViewMatrixCallback : public osg::UniformCallback
{
public:
	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv) {
		uniform->set(PCVR_OvrDevice::GetOvrDevice()->getHMDPoseMatrix());
	}
};