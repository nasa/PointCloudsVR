#pragma once

#define _USE_MATH_DEFINES

#include <QPushButton>

#include <osg/Shape>
#include <osg/Vec3>
#include <osg/Quat>

#include <OpenFrames/Model.hpp>

#include "PCVR_Controller.hpp"
#include "PCVR_Tool.hpp"
#include "PCVR_Selection.hpp"
#include "PCVR_Scene.hpp"
#include "LasModelScene.hpp"

#include "PCVR_Math.hpp"

class SelectionDisk : public PCVR_Selection, public OpenFrames::ReferenceFrame
{
public:
	SelectionDisk(osg::Vec3 pos);

	double getRadius() const;
	double getHeight() const;
	osg::ShapeDrawable* getShapeDrawable();

	void setRadius(double rad);
	void setHeight(double height);
	//void setAttitude(const osg::Quat& att);

	virtual void save(const std::string& path, const std::vector<PCVR_Selectable*>& points) override;
	virtual void show(bool b) override;
	virtual void remove() override;

protected:
	bool _diskSaved = false;

	osg::ref_ptr<osg::ShapeDrawable> _sd;
};

template <typename D>
class DiskDrawer : public DrawingTool
{
public:
	DiskDrawer(osg::ref_ptr<OpenFrames::FrameManager> fm, std::vector<PCVR_Selectable*> selectables, PCVR_Controller* controller);
	void update() override;
	void handleVREvent(const vr::VREvent_t& ovrEvent) override;
	void stopUsingTool() override;

private:
	osg::ref_ptr<OpenFrames::FrameManager> _fm;
	osg::ref_ptr<SelectionDisk> _currentDisk = nullptr;
	std::vector<PCVR_Selectable*> _selectables;
	PCVR_Controller* _controller;
	osg::Vec3d _drawingControllerWorldPos;

	float CylTest_CapsFirst(const osg::Vec3 &pt1, const osg::Vec3 &pt2, float lengthsq, float radius_sq, const osg::Vec3 &testpt);
};

template <typename D>
DiskDrawer<D>::DiskDrawer(osg::ref_ptr<OpenFrames::FrameManager> fm, std::vector<PCVR_Selectable*> selectables,
	PCVR_Controller* controller)
	: _fm(fm), _selectables(selectables), _controller(controller)
{
}

template <typename D>
void DiskDrawer<D>::update()
{
	// Only update if a controller is pressing trigger and thus is in drawing state.
	// Also only update on every 5th frame to save GPU.
	static int tick = 0;
	if (_drawingController != nullptr && tick % 5 == 0)
	{
		osg::Vec3d diskOrigin; 
		_currentDisk->getPosition(diskOrigin);
		double radius = (_drawingController->getWorldPos() - diskOrigin).length();
		double height = std::abs(_drawingController->getWorldPos().y() - diskOrigin.y());
		
		_fm->lock();
		_currentDisk->setRadius(radius);
		_currentDisk->setHeight(height);
		_currentDisk->setAttitude(_drawingController->getOrientation());
		_currentDisk->setName("Selection Disk, Radius = " + std::to_string(radius)
						    + "\n              Height = " + std::to_string(height));
		_fm->unlock();
	}
	tick++;
}

template <typename D>
void DiskDrawer<D>::handleVREvent(const vr::VREvent_t& ovrEvent)
{
	// Start drawing on trigger down.
	if (ovrEvent.eventType == vr::VREvent_ButtonPress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger
		&& _drawingController == nullptr)
	{
		if (_currentDisk != nullptr) _currentDisk->remove();

		_drawingController = PCVR_Controller::GetById(ovrEvent.trackedDeviceIndex);
		_currentDisk = new D(_drawingController->getWorldPos());
		_drawingControllerWorldPos = _drawingController->getWorldPos();

		_fm->lock();
		_fm->getFrame()->addChild(_currentDisk);
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
}

template <typename D>
void DiskDrawer<D>::stopUsingTool()
{
	// Disk drawing finished, now highlight model points under cylinder
	// Assume a single model file for now
	// Iterate through all PCVR_Selectable ModelPoints.
	// Set ones inside the cylinder to yellow.

	LasModelScene* modelScene = static_cast<LasModelScene*>(PCVR_Scene::Instance);
	LasModel* model = static_cast<LasModel*>(modelScene->getModels().at(0));	// single model assumed right now
	osg::Vec4Array& colors = model->getColors();
	osg::ShapeDrawable* shapeDrawable = _currentDisk->getShapeDrawable();
	int cIndex = _controller == PCVR_Controller::Left() ? 0 : 1;

	// Transform cylinder end cap centers by correct rotation and translation for controller
	osg::Vec3d pt1, pt2;
	pt1 = osg::Vec3d(0.0, 0.0, _currentDisk->getHeight() / 2);
	pt2 = osg::Vec3d(0.0, 0.0, -_currentDisk->getHeight() / 2);

	// Rotate first about Origin by controller orientation
	osg::Quat quat;
	_currentDisk->getAttitude(quat);
	pt1 = quat * pt1;
	pt2 = quat * pt2;

	// Then translate by disk center vector:
	osg::Vec3d diskCenter;
	_currentDisk->getPosition(diskCenter);
	pt1 = pt1 + diskCenter;
	pt2 = pt2 + diskCenter;

	double circumfSum = 0;
	double circumfAvg = 0;
	double radius = _currentDisk->getRadius();
	//std::cout << "Selectables Size: " << _selectables.size() << std::endl;
	long numPointsInDisk = 0;
	for (std::size_t i = 0; i < _selectables.size(); i++)
	{
		if (_currentDisk != nullptr)
		{
			osg::Vec3 point = _selectables.at(i)->getPos(); 
			
			//std::cout << "Circumference Sum: " << circumfSum << std::endl;
			
			if (CylTest_CapsFirst(pt1, pt2, _currentDisk->getHeight() * _currentDisk->getHeight(),
				_currentDisk->getRadius() *_currentDisk->getRadius(), point) != -1.0f)
			{
				colors.at(i) = osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow for now
				circumfSum += 2.0 * M_PI * ((double) (point - diskCenter).length());
				numPointsInDisk++;
			}
		}
	}
	circumfAvg = (circumfSum / numPointsInDisk) / 10;
	//std::cout << "Circumference Avg: " << circumfAvg << std::endl;
	modelScene->_circumferenceLabel[cIndex]->setText(QString::number(circumfAvg));
	// Create circumference panel and position next to disk.  Face panel toward user.
	_fm->lock();
	QWidget* panelWidget;
	modelScene->_circumfPanel = modelScene->buildQtPanel("CircumferencePanel", modelScene->_uiDialogFilePath,
		panelWidget);
	modelScene->_panelCircumferenceLabel = panelWidget->findChild<QLabel*>("circumferenceLabel");
	modelScene->_panelCircumferenceLabel->setText(QString::number(circumfAvg));
	
	QPushButton* circumfAction = panelWidget->findChild<QPushButton*>("dismissCircumferenceButton");
	QObject::connect(circumfAction, &QPushButton::clicked, modelScene,
		[=]() { modelScene->_circumfPanel->showContents(false); });

	const OpenFrames::OpenVRDevice::DeviceModel* hmd;
	for (unsigned int j = 0; j < modelScene->_ovrDevice->getNumDeviceModels(); j++)
	{
		hmd = modelScene->_ovrDevice->getDeviceModel(j);
		if (hmd->_class == OpenFrames::OpenVRDevice::DeviceClass::HMD)
		{
			break;
		}
	}
	osg::ref_ptr<OpenFrames::OpenVRTrackball> vrTrackball = static_cast<OpenFrames::OpenVRTrackball*>(modelScene->_windowProxy->getGridPosition(0, 0)->getCurrentView()->getTrackball());
	osg::Vec3 hmdRoomPos = hmd->_rawDeviceToWorld.getTrans() * modelScene->_ovrDevice->getWorldUnitsPerMeter();
	osg::Vec3 hmdWorldPos = hmdRoomPos * vrTrackball->getRoomToTrackballMatrix() * vrTrackball->FollowingTrackball::getMatrix();

	osg::Vec3d panelPos = _drawingControllerWorldPos;
	osg::Vec3d forward = _drawingControllerWorldPos - hmdWorldPos;
	osg::Vec3d left = - (forward ^ vrTrackball->getUpVector(vrTrackball->getCoordinateFrame(hmdWorldPos)));
	left.normalize();
	panelPos += left * 2 * radius;
	/*osg::Vec3d left = -(forward ^ vrTrackball->getUpVector(vrTrackball->getCoordinateFrame(hmdWorldPos)));
	left.normalize();
	panelPos += left * 2 * radius;*/
	//panelPos -= osg::Vec3d(5, 0, 0);
	//modelScene->_circumfPanel->setPosition(panelPos);

	osg::Quat panelQuat;
	osg::Quat roomQuat = hmd->_rawDeviceToWorld.getRotate();
	osg::Quat worldQuat;
	worldQuat.set(vrTrackball->getRoomToTrackballMatrix() * vrTrackball->FollowingTrackball::getMatrix());

	panelQuat = roomQuat * worldQuat;
	//quat.set(modelScene->_pcvrTrackball->getMatrix());
	modelScene->_circumfPanel->setAttitude(panelQuat);
	/*double width, height;
	modelScene->_circumfPanel->getSize(width, height);
	modelScene->_circumfPanel->setSize(width / 2, height / 2);*/
	modelScene->_circumfPanel->showContents(true);

	osg::Matrixd scaleMatrix;
	scaleMatrix.makeScale(0.15, 0.15, 0.15);
	osg::Matrixd posMatrix;
	posMatrix.setTrans(panelPos);
	osg::Matrixd dialogMatrix;


	osg::ref_ptr<osg::MatrixTransform> dialogXform = new osg::MatrixTransform;
	//osg::ref_ptr<osg::MatrixTransform> dialogXform2 = new osg::MatrixTransform;
	
	//osg::ref_ptr<osg::Matrixd> dialogMatrix = modelScene->_pcvrTrackball->getInverseMatrix();
	/*osg::Matrixd dialogMatrix;
	osg::Matrixd matrix90deg;
	osg::Matrixd scaleMatrix;
	scaleMatrix.makeScale(0.5, 0.5, 0.5);
	osg::Vec3d xAxis(1, 0, 0);
	double xAngle = 1.5708;
	matrix90deg.makeRotate(xAngle, xAxis);
	dialogMatrix.setRotate(panelQuat);
	dialogMatrix = dialogMatrix * matrix90deg * scaleMatrix;*/

	/*dialogMatrix.makeLookAt(panelPos, hmdWorldPos,
		modelScene->_pcvrTrackball->getUpVector(
		modelScene->_pcvrTrackball->getCoordinateFrame(hmdWorldPos)));
	dialogXform->setMatrix(dialogMatrix);*/
	//dialogXform->setMatrix(posMatrix * scaleMatrix);

	dialogXform->setMatrix(scaleMatrix * posMatrix);
	dialogXform->addChild(modelScene->_circumfPanel->getGroup());
	_fm->getFrame()->getGroup()->addChild(dialogXform);
	//_fm->getFrame()->addChild(modelScene->_circumfPanel);
	_fm->unlock();

	_currentDisk->show(false);
	colors.dirty();
}

template <typename D>
float DiskDrawer<D>::CylTest_CapsFirst(const osg::Vec3 &pt1, const osg::Vec3 &pt2, float lengthsq, float radius_sq, const osg::Vec3 &testpt)
{
	float dx, dy, dz;	// vector d  from line segment point 1 to point 2
	float pdx, pdy, pdz;	// vector pd from point 1 to test point
	float dot, dsq;

	dx = pt2.x() - pt1.x();	// translate so pt1 is origin.  Make vector from
	dy = pt2.y() - pt1.y();     // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2.z() - pt1.z();

	pdx = testpt.x() - pt1.x();		// vector from pt1 to test point.
	pdy = testpt.y() - pt1.y();
	pdz = testpt.z() - pt1.z();

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1.x, pt1.y, pt1.z

	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.

	if (dot < 0.0f || dot > lengthsq)
	{
		return(-1.0f);
	}
	else
	{
		// Point lies within the parallel caps, so find
		// distance squared from point to line, using the fact that sin^2 + cos^2 = 1
		// the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
		// Carefull: '*' means mult for scalars and dotproduct for vectors
		// In short, where dist is pt distance to cyl axis: 
		// dist = sin( pd to d ) * |pd|
		// distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
		// dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
		// dsq = pd * pd - dot * dot / lengthsq
		//  where lengthsq is d*d or |d|^2 that is passed into this function 

		// distance squared to the cylinder axis:

		dsq = (pdx*pdx + pdy * pdy + pdz * pdz) - dot * dot / lengthsq;

		if (dsq > radius_sq)
		{
			return(-1.0f);
		}
		else
		{
			return(dsq);		// return distance squared to axis
		}
	}
}