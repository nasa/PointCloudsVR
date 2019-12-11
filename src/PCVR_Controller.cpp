#include <osgUtil/RayIntersector>

#include <QFile>
#include <QMenuBar>
#include <QuiLoader>

#include "PCVR_Controller.hpp"

// Override OpenVRImageHandler so to change default click behavior.
// Now clicks happen when the trigger is released, regardless of where it was first pressed.

PCVR_ImageHandler::PCVR_ImageHandler(osg::ref_ptr<OpenFrames::OpenVRDevice> ovrDevice, osg::Image* image)
	: OpenFrames::OpenVRImageHandler(ovrDevice, image)
{
}

bool PCVR_ImageHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
{
	// Check if incoming event is an OpenVR event
	const OpenFrames::OpenVREvent *event = dynamic_cast<const OpenFrames::OpenVREvent*>(&ea);
	osgViewer::View *view = dynamic_cast<osgViewer::View*>(&aa);
	if (event && view)
	{
		// Only process event if it's from a controller
		const vr::VREvent_t *ovrEvent = event->_ovrEvent;
		vr::TrackedDeviceIndex_t deviceID = ovrEvent->trackedDeviceIndex;
		const OpenFrames::OpenVRDevice::DeviceModel* deviceModel = _ovrDevice->getDeviceModel(deviceID);
		if ((deviceModel == nullptr) || (deviceModel->_class != OpenFrames::OpenVRDevice::CONTROLLER)) return false;

		// Don't process event if another controller is already being used for pick events
		if ((_pickData.mode != NONE) && (deviceID != _pickData.deviceID)) return false;

		// Get controller state
		const vr::VRControllerState_t *state = deviceModel->_controllerState;

		// Convert controller event types to view changes in VR space
		switch (ovrEvent->eventType)
		{
		case(vr::VREvent_ButtonTouch):
		{
			// If trigger is touched, then start mouse action processing on the image
			if (ovrEvent->data.controller.button == vr::k_EButton_SteamVR_Trigger)
			{
				saveCurrentPickData(PickMode::MOUSEACTION, view, nv, deviceID);
			}
			break;
		}

		//case(vr::VREvent_ButtonUnpress):
		case(vr::VREvent_ButtonUntouch):
		{
			// If trigger is untouched, then stop mouse action processing on the image
			if (ovrEvent->data.controller.button == vr::k_EButton_SteamVR_Trigger)
			{
				_pickData.mode = PickMode::NONE;
			}
			break;
		}
		}

		return false;
	}
	else // Otherwise process it as a regular trackball event
	{
		// Dispatch new image pick event at each frame
		if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
		{
			processImagePick();
			return false;
		}

		// Fallback to mouse-based image handler as needed
		else return osgViewer::InteractiveImageHandler::handle(ea, aa, obj, nv);
	}
}

void PCVR_ImageHandler::processImagePick()
{
	// Check if image pick processing is needed
	if (_pickData.mode == NONE && _lastPickMode == NONE && _lastLastPickMode == NONE) return;

	// Pick coordinates on image
	bool validPick = false;
	int x = 0, y = 0;

	const OpenFrames::OpenVRDevice::DeviceModel *deviceModel = _ovrDevice->getDeviceModel(_pickData.deviceID);

	// Get transform from laser space to controller space
	// This may be non-identity for VR controllers whose ideal pointing direction
	// is not parallel to the controller's x/y/z axes (e.g. Oculus Touch)
	const osg::Matrixd &matLaserToController = deviceModel->_laser->getTransform()->getMatrix();

	// Get transform from controller space to room space
	const osg::Matrixd &matControllerToRoom = _ovrDevice->getDeviceModel(_pickData.deviceID)->_rawDeviceToWorld;

	// Compute full transform from laser to room space
	osg::Matrixd matLaserToRoom = matLaserToController * matControllerToRoom;

	// Get controller laser start and end points in room space and world units
	osg::Vec3d startPoint = matLaserToRoom.getTrans() * _ovrDevice->getWorldUnitsPerMeter(); // Start point is origin
	osg::Vec3d endPoint = osg::Vec3d(0, 0, -1) * matLaserToRoom * _ovrDevice->getWorldUnitsPerMeter(); // Use unit length since we just want laser direction

																									   // Transform controller laser to the local coordinate space of the geometry containing the image
																									   // First determine if the geometry is in world space or room space. Note that room space includes attached to a VR controller
																									   // Do this by checking if the geometry's NodePath contains the topmost VR device transform
	osg::NodePath::iterator itrDevices = std::find(_pickData.nodePath.begin(), _pickData.nodePath.end(), _ovrDevice->getDeviceRenderModels());

	osg::Matrixd matRoomToGeomLocal; // Final transform from Room to Geometry's local space
	double ratioMetersPerLaserDistance; // Conversion from pick point distance to meters, depends on world vs room/controller space

										// Geometry in room/controller space
	if (itrDevices != _pickData.nodePath.end())
	{
		// Controller laser already in room space, so just get the additional NodePath to the geometry
		if ((itrDevices + 1) < (_pickData.nodePath.end() - 1))
		{
			osg::NodePath geomNodePath(itrDevices + 1, _pickData.nodePath.end() - 1); // Prune room-space NodePath to geometry
			matRoomToGeomLocal = osg::computeWorldToLocal(geomNodePath);
		}
		// else the topmost VR device transform is the geometry's direct parent and there is nothing more to do

		ratioMetersPerLaserDistance = 1.0;
	}
	else // Geometry in world space
	{
		// Transform to geometry's local space
		osg::Matrixd matRoomToWorld = _pickData.trackball->getRoomToTrackballMatrix() * _pickData.trackball->FollowingTrackball::getMatrix();
		if (_pickData.nodePath.size() > 1)
		{
			osg::NodePath geomNodePath(_pickData.nodePath.begin(), _pickData.nodePath.end() - 1); // Prune world-space NodePath to geometry
			osg::Matrixd matWorldToGeomLocal = osg::computeWorldToLocal(geomNodePath);
			matRoomToGeomLocal = matRoomToWorld * matWorldToGeomLocal;
		}
		else matRoomToGeomLocal = matRoomToWorld;

		ratioMetersPerLaserDistance = 1.0 / _ovrDevice->getWorldUnitsPerMeter();
	}

	// Compute the transformed laser start point and direction
	startPoint = startPoint * matRoomToGeomLocal;
	endPoint = endPoint * matRoomToGeomLocal;
	osg::Vec3d rayDir = endPoint - startPoint;

	// Perform the pick operation
	osg::ref_ptr<osgUtil::RayIntersector> intersector = new osgUtil::RayIntersector(startPoint, rayDir);
	osgUtil::IntersectionVisitor iv(intersector);
	_pickData.nodePath.back()->accept(iv);
	if (intersector->containsIntersections())
	{
		auto intersections = intersector->getIntersections();
		/*
		osg::notify(osg::NOTICE) << "Got " << intersections.size() << " intersections for " << _pickData.nodePath.back()->getName() << std::endl;
		for (auto&& intersection : intersections)
		{
		osg::notify(osg::NOTICE) << "  - Local intersection point = " << intersection.localIntersectionPoint << std::endl;
		}
		*/

		osg::Vec2 tc(0.5f, 0.5f); // Intersection tex coord

								  // Get nearest intersection
		const osgUtil::RayIntersector::Intersection& intersection = *(intersections.begin());
		osg::Drawable* drawable = intersection.drawable.get();
		osg::Geometry* geometry = drawable ? drawable->asGeometry() : nullptr;
		osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : nullptr;
		if (vertices)
		{
			// Get vertex indices
			const osgUtil::RayIntersector::Intersection::IndexList& indices = intersection.indexList;
			const osgUtil::RayIntersector::Intersection::RatioList& ratios = intersection.ratioList;

			if (indices.size() == 3 && ratios.size() == 3)
			{
				unsigned int i1 = indices[0];
				unsigned int i2 = indices[1];
				unsigned int i3 = indices[2];

				float r1 = ratios[0];
				float r2 = ratios[1];
				float r3 = ratios[2];

				osg::Array* texcoords = (geometry->getNumTexCoordArrays() > 0) ? geometry->getTexCoordArray(0) : nullptr;
				osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
				if (texcoords_Vec2Array)
				{
					// Compute intersection tex coord
					osg::Vec2 tc1 = (*texcoords_Vec2Array)[i1];
					osg::Vec2 tc2 = (*texcoords_Vec2Array)[i2];
					osg::Vec2 tc3 = (*texcoords_Vec2Array)[i3];
					tc = tc1 * r1 + tc2 * r2 + tc3 * r3;
				}

				// Compute intersection x,y coords on image
				if (_image.valid())
				{
					x = int(float(_image->s()) * tc.x());
					y = int(float(_image->t()) * tc.y());

					deviceModel->_laser->setLength(intersection.distance*ratioMetersPerLaserDistance); // Set laser length
					deviceModel->_laser->setWidth(_laserSelectedWidth);
					deviceModel->_laser->setColor(_laserSelectedColor);

					validPick = true;
				}
			}
		}
	}

	if (!validPick) return;

	// Dispatch the appropriate mouse event to the Image based on the trigger state
	if (_pickData.mode == MOUSEACTION) // Trigger pressed, not yet released: mouse move
	{
		_image->sendPointerEvent(x, y, 0);
		_lastX = x;
		_lastY = y;
	}
	else if (_lastPickMode == MOUSEACTION) // Trigger released: left-click
	{
		_image->sendPointerEvent(_lastX, _lastY, osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
	}
	else
	{
		_image->sendPointerEvent(_lastX, _lastY, 0);
		_image->sendPointerEvent(0, 0, 0);
	}
	_lastLastPickMode = _lastPickMode;
	_lastPickMode = _pickData.mode;
}


// Controller definition

PCVR_Controller* PCVR_Controller::_LeftController = nullptr;
PCVR_Controller* PCVR_Controller::_RightController = nullptr;


void PCVR_Controller::InitializeControllers()
{
	osg::ref_ptr<const OpenFrames::OpenVRDevice> ovrDevice = PCVR_OvrDevice::GetOvrDevice();

	int cIndex = 0;
	for (unsigned int j = 0; j < ovrDevice->getNumDeviceModels(); j++)
	{
		if (ovrDevice->getDeviceModel(j)->_class == OpenFrames::OpenVRDevice::CONTROLLER)
		{
			if (cIndex == 0) {
				_LeftController = new PCVR_Controller(j);
			}
			else {
				_RightController = new PCVR_Controller(j);
			}
			cIndex++;
		}
	}
	if (cIndex < 2) throw std::runtime_error("Could not find both controllers.");
}

PCVR_Controller* PCVR_Controller::Left()
{
	return _LeftController;
}

PCVR_Controller* PCVR_Controller::Right()
{
	return _RightController;
}

PCVR_Controller* PCVR_Controller::GetById(vr::TrackedDeviceIndex_t deviceID)
{
	return Left()->getId()  == deviceID ? Left()  :
		   Right()->getId() == deviceID ? Right() : nullptr;
}

PCVR_Controller::PCVR_Controller(vr::TrackedDeviceIndex_t id)
	: PCVR_OvrDevice(id)
{
}

osg::ref_ptr<OpenFrames::QWidgetPanel> PCVR_Controller::getPanel() const
{
	return _panel;
}

QWidget* PCVR_Controller::getPanelWidget() const
{
	return _panelWidget;
}

void PCVR_Controller::buildControllerPanel(std::string uiFilePath)
{
	// 1) Create, size, orient, position, set other attributes
	// and addChild() QWidgetPanel to controller _modelTransform.

	_panel = new OpenFrames::QWidgetPanel("ControllerPanel" + std::to_string(getId()));
	_panel->showNameLabel(false);
	_panel->showAxes(false);
	_panel->showAxesLabels(false);
	_panel->showContents(false); // set panel hidden initially

	// Make panel face user (panel +y -> controller -z)
	_panel->setAttitude(osg::Quat(osg::PI_2, osg::Vec3d(-1.0, 0.0, 0.0)));
	_panel->setColor(osg::Vec4(0, 0, 0, 0));
	_devModel->_modelTransform->addChild(_panel->getGroup());

	// 2) Load QWidget from .ui file and assign to QWidgetPanel
	QFile* uiFile = new QFile(QString(uiFilePath.c_str()));
	uiFile->open(QIODevice::ReadOnly);
	QUiLoader loader;
	_panelWidget = loader.load(uiFile);
	_panelWidget->setAttribute(Qt::WA_TranslucentBackground);

	double scale = 0.0005;
	_panel->setSize(scale * _panelWidget->width(), scale * _panelWidget->height());
	_panel->setPosition(-scale * _panelWidget->width() / 2.0, 0.05, 0.0);
	_panel->setWidget(_panelWidget);

	// 3) Create and set OpenVRImageHandler for panel
	osg::ref_ptr<OpenFrames::OpenVRImageHandler> ih = new OpenFrames::OpenVRImageHandler(PCVR_OvrDevice::GetOvrDevice(), _panel->getImage());
	ih->setSelectedColor(osg::Vec4(0.0, 0.5, 0.9, 1.0));
	ih->setSelectedWidth(4.0);
	_panel->setImageHandler(ih);
}
