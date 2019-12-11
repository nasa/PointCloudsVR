#pragma once

#include <OpenFrames/DrawableTrajectory.hpp>
#include <OpenFrames/CurveArtist.hpp>

#include "Removable.hpp"
#include "PCVR_Tool.hpp"

class StraightLine : public OpenFrames::CurveArtist, public Removable
{
public:
	StraightLine(osg::ref_ptr<OpenFrames::DrawableTrajectory> parent, osg::ref_ptr<OpenFrames::Trajectory> traj);
	void remove() override;

private:
	osg::ref_ptr<OpenFrames::DrawableTrajectory> _parent;
};

class StraightLineDrawer : public DrawingTool
{
public:
	StraightLineDrawer(osg::ref_ptr<OpenFrames::ReferenceFrame> rootFrame);
	void update() override;
	void handleVREvent(const vr::VREvent_t& ovrEvent) override;

private:
	static osg::ref_ptr<OpenFrames::DrawableTrajectory> _DrawableTraj;
	osg::ref_ptr<StraightLine> _straightLine;
	osg::Vec3d _origin;
};