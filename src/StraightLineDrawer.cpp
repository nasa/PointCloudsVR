#include <iostream>

#include "PCVR_Scene.hpp"

#include "StraightLineDrawer.hpp"

StraightLine::StraightLine(osg::ref_ptr<OpenFrames::DrawableTrajectory> parent, osg::ref_ptr<OpenFrames::Trajectory> traj)
	: OpenFrames::CurveArtist(traj)
	, Removable()
	, _parent(parent)
{
	setWidth(4.0);
	setColor(1, 0, 1);
	_parent->addArtist(this);
}

void StraightLine::remove()
{
	Removable::remove();
	_parent->removeArtist(this);
}


osg::ref_ptr<OpenFrames::DrawableTrajectory> StraightLineDrawer::_DrawableTraj;

StraightLineDrawer::StraightLineDrawer(osg::ref_ptr<OpenFrames::ReferenceFrame> rootFrame)
{
	PCVR_Scene::GetFrameManager()->lock();
	// Create DrawableTrajectory and add to world if static _DrawableTraj has not yet been created.
	if (_DrawableTraj == nullptr) {
		_DrawableTraj = new OpenFrames::DrawableTrajectory("Straight Line DrawableTrajectory");
		_DrawableTraj->showNameLabel(false);
		_DrawableTraj->showAxes(false);
		_DrawableTraj->showAxesLabels(false);
		rootFrame->addChild(_DrawableTraj);
	}
	PCVR_Scene::GetFrameManager()->unlock();
}

void StraightLineDrawer::update()
{
	// Only update if a controller is pressing trigger and thus is in drawing state.
	if (_drawingController != nullptr)
	{
		//OpenFrames::Trajectory* traj = const_cast<OpenFrames::Trajectory*>(_DrawableTraj->getArtist(_DrawableTraj->getNumArtists() - 1)->getTrajectory());
		//OpenFrames::Trajectory* traj = const_cast<OpenFrames::Trajectory*>(_straightLine->getTrajectory());
		PCVR_Scene::GetFrameManager()->lock();

		OpenFrames::Trajectory* newTraj = new OpenFrames::Trajectory();

		// Add origin to Trajectory
		newTraj->addTime(time(nullptr));
		newTraj->addPosition(_origin.x(), _origin.y(), _origin.z());

		// Add new point to Trajectory
		osg::Vec3d cPos = _drawingController->getWorldPos();
		newTraj->addTime(time(nullptr));
		newTraj->addPosition(cPos.x(), cPos.y(), cPos.z());

		_straightLine->setTrajectory(newTraj);

		PCVR_Scene::GetFrameManager()->unlock();
		// If 0 or 1 points, then add current time (time since 1971), and pos., to trajectory.
		//if (traj->getNumPos() <= 1)
		//{
		//	traj->addTime(time(nullptr));

		//	osg::Vec3d cPos = _drawingController->getWorldPos();
		//	traj->addPosition(cPos.x(), cPos.y(), cPos.z());
		//}
		//else if (traj->getNumPos() == 2)  // Set last traj point to latest controller location
		//{
		//	traj->lockData();
		//	// Delete latest time and position
		//	//std::vector<double> times = traj->getTimeList();
		//	std::vector<double> points = traj->getPosOptList();
		//	//times.pop_back();
		//	points.pop_back();
		//	points.pop_back();
		//	points.pop_back();

		//	// Set latest time and position
		//	traj->addTime(time(nullptr));

		//	osg::Vec3d cPos = _drawingController->getWorldPos();
		//	/*traj->addPosition(cPos.x(), cPos.y(), cPos.z());*/
		//	points.push_back(cPos.x());
		//	points.push_back(cPos.y());
		//	points.push_back(cPos.z());
		//	//OpenFrames::CurveArtist* ca = static_cast<OpenFrames::CurveArtist*>(_straightLine.get());
		//	//osg::BoundingBox bb = ca->getBoundingBox();
		//	//OpenFrames::Trajectory::DataSource data;
		//	//data._src = OpenFrames::Trajectory::POSOPT;
		//	//// Iterate through each point and expand the bounding box to encompass it
		//	//OpenFrames::Trajectory::DataType point[3];
		//	//unsigned int maxPoints = traj->getNumPoints(&data);
		//	//if (maxPoints == UINT_MAX) maxPoints = 1;
		//	//for (unsigned int i = 0; i < maxPoints; ++i)
		//	//{
		//	//	traj->getPoint(i, &data, point);
		//	//	bb.expandBy(point[0], point[1], point[2]);
		//	//}
		//	//traj->informSubscribers();
		//	//_straightLine->dataAdded(traj);
		//	//_straightLine->dirtyBound();
		//	//_straightLine->dirtyDisplayList();
		//	_straightLine->dataCleared(traj);
		//	std::cout << "x = " << cPos.x() << std::endl;
		//	std::cout << "y = " << cPos.y() << std::endl;
		//	std::cout << "z = " << cPos.z() << std::endl;
		//	traj->unlockData();
		//	//std::cout << "Number of points: " << traj->getNumPos() << std::endl;
		//}
		//std::cout << "Number of points: " << traj->getNumPos() << std::endl;
	}
}

void StraightLineDrawer::handleVREvent(const vr::VREvent_t& ovrEvent)
{
	// Start drawing on trigger down.
	if (ovrEvent.eventType == vr::VREvent_ButtonPress
		&& ovrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger
		&& _drawingController == nullptr)
	{
		PCVR_Scene::GetFrameManager()->lock();
		_straightLine = new StraightLine(_DrawableTraj, new OpenFrames::Trajectory());
		_drawingController = PCVR_Controller::GetById(ovrEvent.trackedDeviceIndex);
		_origin = _drawingController->getWorldPos();
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