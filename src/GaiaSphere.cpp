#include <filesystem>

#include "GaiaSphere.hpp"
#include "GaiaScene.hpp"

namespace fs = std::experimental::filesystem;

GaiaSphere::GaiaSphere(osg::Vec3 pos)
	: SelectionSphere(pos)
{
}

GaiaSphere::GaiaSphere(osg::Vec3 pos, double rad, osg::Vec4 color, int numStars)
	: GaiaSphere(pos)
{
	_saved = true;

	setColor(color);
	setRadius(rad);
	setName("Selection Sphere Radius (pc): " + std::to_string(rad)
		+ " Number of Stars: " + std::to_string(numStars));
	_removable = false;
	SelectionSphere::Removable::remove(); // remove from the list of removables, so it won't get removed ;)
}

void GaiaSphere::save(const std::string& path, const std::vector<PCVR_Selectable*>& points)
{
	int fileNum = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{}) + 1;
	std::string filepath = path + "/SelectionSphere" + std::to_string(fileNum) + ".csv";

	std::ofstream file;
	file.open(filepath);

	double x, y, z;
	getSpherePosition(x, y, z);
	file << "#Origin: " << x << " " << y << " " << z << std::endl
		 << "#Radius: " << getRadius() << std::endl
	     << "id, x, y, z, u, v, w, ra, dec, parallax, teff, l, b" << std::endl;

	osg::Vec3 selPos = osg::Vec3(x, y, z);
	for (PCVR_Selectable* p : points)
	{
		if ((p->getPos() - selPos).length() <= getRadius())
		{
			p->writeToStream(file);
		}
	}
	// Put saved sphere into GUI list
	GaiaScene* gaiaScene = dynamic_cast<GaiaScene*>(PCVR_Scene::Instance);
	for (int i = 0; i < 2; i++)
	{
		QCheckBox* check = new QCheckBox(QString::fromStdString("SelectionSphere" + std::to_string(fileNum)));
		QObject::connect(check, &QCheckBox::clicked, gaiaScene,
			[=](bool checked) {
			this->show(checked);
		});
		gaiaScene->_spheres[i]->addWidget(check);
	}
}

void GaiaSphere::remove()
{
	if (_removable)
	{
		SelectionSphere::remove();
	}
}