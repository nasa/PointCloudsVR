#include <filesystem>

#include "SphereDrawer.hpp"

namespace fs = std::experimental::filesystem;

SelectionSphere::SelectionSphere(osg::Vec3 pos)
	: OpenFrames::Sphere("Selection Sphere", 0.0f, 0.0f, 1.0f, 0.5f)
	, PCVR_Selection()
{
	showNameLabel(true);
	showAxes(false);
	showAxesLabels(false);
	setRadius(0);
	setSpherePosition(pos.x(), pos.y(), pos.z());

	osg::ref_ptr<osg::StateSet> stateSet = getGroup()->getOrCreateStateSet();
	stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

void SelectionSphere::show(bool b)
{
	showContents(b);
	showNameLabel(b);
}

void SelectionSphere::save(const std::string& path, const std::vector<PCVR_Selectable*>& points)
{
	int fileNum = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{}) + 1;
	std::string filepath = path + "/SelectionSphere" + std::to_string(fileNum) + ".csv";

	std::ofstream file;
	file.open(filepath);

	double x, y, z;
	getSpherePosition(x, y, z);
	file << "#Origin: " << x << " " << y << " " << z << std::endl;
	file << "#Radius: " << getRadius() << std::endl;

	osg::Vec3 selPos = osg::Vec3(x, y, z);
	for (PCVR_Selectable* p : points)
	{
		if ((p->getPos() - selPos).length() <= getRadius())
		{
			p->writeToStream(file);
		}
	}
}

void SelectionSphere::remove()
{
	PCVR_Selection::remove();

	getParent(0)->removeChild(this);
}