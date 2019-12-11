#pragma once

#include <QObject>
#include <QLayout>
#include <QCheckBox>

#include "SphereDrawer.hpp"

class GaiaSphere : public SelectionSphere
{
public:
	// For newly drawn stars for which we don't know how many stars they hold.
	GaiaSphere::GaiaSphere(osg::Vec3 pos);

	// For previously saved stars for which we do.
	GaiaSphere::GaiaSphere(osg::Vec3 pos, double rad, osg::Vec4 color, int numStars);

	void save(const std::string& path, const std::vector<PCVR_Selectable*>& points) override;

	virtual void remove() override;

private:
	bool _removable = true;	// only set to false in the constructor for previously saved stars
};