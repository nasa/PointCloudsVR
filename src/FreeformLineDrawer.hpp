#pragma once

#include <OpenFrames/DrawableTrajectory.hpp>
#include <OpenFrames/CurveArtist.hpp>

#include "Removable.hpp"
#include "PCVR_Tool.hpp"

class FreeformLine : public OpenFrames::CurveArtist, public Removable
{
public:
	FreeformLine(osg::ref_ptr<OpenFrames::DrawableTrajectory> parent, osg::ref_ptr<OpenFrames::Trajectory> traj);
	void remove() override;

private:
	osg::ref_ptr<OpenFrames::DrawableTrajectory> _parent;
};

class FreeformLineDrawer : public DrawingTool
{
public:
	FreeformLineDrawer(osg::ref_ptr<OpenFrames::ReferenceFrame> rootFrame);
	void update() override;
	void handleVREvent(const vr::VREvent_t& ovrEvent) override;

private:
	static osg::ref_ptr<OpenFrames::DrawableTrajectory> _DrawableTraj;
};