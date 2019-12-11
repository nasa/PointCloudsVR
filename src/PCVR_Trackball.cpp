#include <openvr.h>

#include "PCVR_Trackball.hpp"

PCVR_Trackball::PCVR_Trackball(osg::ref_ptr<OpenFrames::OpenVRDevice> ovrDevice)
	: OpenFrames::OpenVRTrackball(ovrDevice)
{
}

void PCVR_Trackball::saveOrigMotionData(void)
{
	const OpenFrames::OpenVRDevice::DeviceModel *device1Model = _ovrDevice->getDeviceModel(_motionData._device1ID);
	_origMotionData._device1OrigPoseRaw = device1Model->_rawDeviceToWorld;

	if (_motionData._device2ID < _ovrDevice->getNumDeviceModels())
	{
		const OpenFrames::OpenVRDevice::DeviceModel *device2Model = _ovrDevice->getDeviceModel(_motionData._device2ID);
		_origMotionData._device2OrigPoseRaw = device2Model->_rawDeviceToWorld;
	}

	_origMotionData._origWorldUnitsPerMeter = _ovrDevice->getWorldUnitsPerMeter();
	_origMotionData._origRotation = osgGA::TrackballManipulator::getRotation();
	_origMotionData._origTrackball = _roomPose * osgGA::TrackballManipulator::getMatrix();
	_origMotionData._origRoomPose = _roomPose;
}

void PCVR_Trackball::restoreOrigMotionData()
{
	_motionData._device1OrigPoseRaw = _origMotionData._device1OrigPoseRaw;
	_motionData._device2OrigPoseRaw = _origMotionData._device2OrigPoseRaw;
	_motionData._origWorldUnitsPerMeter = _origMotionData._origWorldUnitsPerMeter;
	_motionData._origRotation = _origMotionData._origRotation;
	_motionData._origTrackball = _origMotionData._origTrackball;
	_motionData._origRoomPose = _origMotionData._origRoomPose;
	processMotion();
}

bool PCVR_Trackball::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	// Check if incoming event is an OpenVR event
	const OpenFrames::OpenVREvent *event = dynamic_cast<const OpenFrames::OpenVREvent*>(&ea);
	if (event)
	{
		// Only process event if it's from a controller
		const vr::VREvent_t *ovrEvent = event->_ovrEvent;
		vr::TrackedDeviceIndex_t deviceID = ovrEvent->trackedDeviceIndex;
		const OpenFrames::OpenVRDevice::DeviceModel* deviceModel = _ovrDevice->getDeviceModel(deviceID);
		if ((deviceModel == nullptr) || (deviceModel->_class != OpenFrames::OpenVRDevice::CONTROLLER)) return false;

		// Get controller state
		const vr::VRControllerState_t *state = deviceModel->_controllerState;

		// Convert controller event types to view changes in VR space
		switch (ovrEvent->eventType)
		{
		case(vr::VREvent_ButtonPress):
		{
			// Grip button pressed state transitions: No Motion -> Translate/Rotate -> Scale
			if (state->ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip))
			{
				// Go from No Motion -> Translate/Rotate when a controller's grip button is pressed
				if (_motionData._mode == NONE)
				{
					// Translate/Rotate uses Device 1 to control the view
					_motionData._mode = _motionData._prevMode;
					_motionData._device1ID = deviceID;
					_motionData._device2ID = UINT_MAX; // Ignore device 2
					saveCurrentMotionData();
				}

				// Go from Translate/Rotate -> Scale when the "other" controller's grip button is pressed
				else if (((_motionData._mode == TRANSLATE) || (_motionData._mode == ROTATE)) && (_motionData._device1ID != deviceID))
				{
					// Scale uses Device 1 & 2 to change the WorldUnits/Meter ratio
					_motionData._prevMode = _motionData._mode; // Save current mode
					_motionData._prevTime = event->getTime(); // Save event time
					_motionData._mode = SCALE;
					_motionData._device2ID = deviceID;
					saveCurrentMotionData();
				}
			}

			// If trigger is pressed, then pick a point on the ground grid
			else if (state->ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
			{
				// Go from No Motion -> Pick when a controller's trigger is pressed
				if (_motionData._mode == NONE)
				{
					// Pick using Device 1
					//_motionData._mode = PICK;
					_motionData._device1ID = deviceID;
					_motionData._device2ID = UINT_MAX; // Ignore device 2
					saveCurrentMotionData();
				}
			}

			break;
		}

		case(vr::VREvent_ButtonUnpress):
		{
			// Button unpressed state transitions: Scale -> Translate/Rotate -> No Motion
			if (state->ulButtonPressed == 0)
			{
				// Go from Translate/Rotate -> No Motion when controller's grip is unpressed
				if (((_motionData._mode == TRANSLATE) || (_motionData._mode == ROTATE)) && (_motionData._device1ID == deviceID))
				{
					// Save current mode for when button is pressed again
					_motionData._prevMode = _motionData._mode;
					_motionData._mode = NONE;
				}

				// Go from Scale -> Translate/Rotate when either controller's grip is unpressed
				else if ((_motionData._mode == SCALE) &&
					((_motionData._device1ID == deviceID) || (_motionData._device2ID == deviceID)))
				{
					// If the second grip button was just tapped, then switch translate/rotate modes
					// Otherwise it was actually pressed so keep the previous translate/rotate mode
					// Alternatively, if the WorldUnits/Meter scale was actually changed, then keep
					// the previous translate/rotate mode even if the grip button was just tapped.
					const double tapDuration = 0.25;
					const double maxScaleChange = 0.01;
					double pressDuration = event->getTime() - _motionData._prevTime;
					double scaleChange = std::abs(_motionData._origWorldUnitsPerMeter / _ovrDevice->getWorldUnitsPerMeter() - 1.0);
					if ((pressDuration >= tapDuration) || (scaleChange > maxScaleChange))
					{
						_motionData._mode = _motionData._prevMode;
					}
					//else if (_motionData._prevMode == TRANSLATE) _motionData._mode = ROTATE;
					else if (_motionData._prevMode == ROTATE) _motionData._mode = TRANSLATE;
					else
					{
						osg::notify(osg::WARN) << "OpenFrames::OpenVRTrackball WARNING: previous mode invalid. Defaulting to TRANSLATE." << std::endl;
						_motionData._mode = TRANSLATE;
					}

					// Translate/Rotate uses Device 1 to control the view, so indicate that the
					// controller with grip button still pressed should be used for view changes
					_motionData._device1ID = _motionData._device1ID + _motionData._device2ID - deviceID;
					_motionData._device2ID = UINT_MAX; // Ignore device 2
					saveCurrentMotionData();
				}
			}
			break;
		}

		case(vr::VREvent_ButtonTouch):
		{
			// If trigger is touched, then show the controller's laser
			if (state->ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger))
			{
				deviceModel->_laser->showLaser(true);
			}
			break;
		}

		case(vr::VREvent_ButtonUntouch):
		{
			// If trigger is untouched, then hide the controller's laser
			if ((state->ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)) == 0x0)
			{
				deviceModel->_laser->showLaser(false);
			}
			break;
		}
		}

		return false;
	}
	else // Otherwise process it as a regular trackball event
	{
		// Compute new motion-based view at each frame
		if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
			processMotion();

		return FollowingTrackball::handle(ea, us);
	}
}