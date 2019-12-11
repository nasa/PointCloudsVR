#ifndef _PCVR_TRACKBALL_
#define _PCVR_TRACKBALL_

#include <osg/ref_ptr>

#include <OpenFrames/OpenVRDevice.hpp>

class PCVR_Trackball : public OpenFrames::OpenVRTrackball
{
public:
	PCVR_Trackball(osg::ref_ptr<OpenFrames::OpenVRDevice> ovrDevice);

	// Handle event
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	void saveOrigMotionData();
	void restoreOrigMotionData();
	struct MotionData _origMotionData;
};

#endif // _PCVR_TRACKBALL_