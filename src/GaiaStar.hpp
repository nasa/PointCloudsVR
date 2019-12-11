#pragma once

#include <OpenFrames/DrawableTrajectory.hpp>

#include "PCVR_Selection.hpp"

class GaiaStar : public PCVR_Selectable
{
public:
	long long sourceId;		// Gaia Source ID number
	std::string name;		// star name
	double ra;
	double dec;
	double parallax;
	double teff;
	double l;
	double b;

	double abs_g_mag;			// converted from phot_g_mean_mag
	double a_g_val;
	double e_bp_min_rp_val;
	double phot_bp_mean_mag;
	double phot_rp_mean_mag;
	double mass;

	osg::ref_ptr<OpenFrames::DrawableTrajectory> traj;	// trajectory for star marker
	osg::Vec3 starVert;			// vertex in OSG 
	osg::Vec3 starVertOrig;		// vertex in OSG -- used to reset x, y, z at time 0
	osg::Vec3 pos;				// position
	osg::Vec3 vel;				// velocity
	osg::Vec4 color;
	bool found = false;			// assume star is not found in data initially
	int colorIndex = -1;

	osg::Vec3 getPos() const override;
	osg::Vec4 getColor() const override;
	void setPos(osg::Vec3 pos) override;
	void setColor(osg::Vec4 color) override;
	std::ostream& writeToStream(std::ostream& o) const override;
};