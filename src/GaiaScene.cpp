#include <filesystem>
#include <iostream>
#include <sstream>
#define _USE_MATH_DEFINES

#include <OpenFrames/CoordinateAxes.hpp>
#include <OpenFrames/MarkerArtist.hpp>

#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>

#include "csv.h"

#include "GaiaStar.hpp"
#include "GaiaSphere.hpp"
#include "SphereDrawer.hpp"

#include "GaiaScene.hpp"

namespace fs = std::experimental::filesystem;


// Stateless utility functions and variables

// Colors for known stars or other particles
const std::vector<osg::Vec4> COLORS = 
{
	{ 1.0, 0.0, 0.0, 1.0 }, // Red
	{ 0.0, 0.5, 0.0, 1.0 }, // Green
	{ 0.0, 0.0, 1.0, 1.0 }, // Blue
	{ 1.0, 0.0, 1.0, 1.0 }, // Purple
	{ 1.0, 1.0, 0.0, 1.0 }, // Yellow
	{ 0.0, 1.0, 1.0, 1.0 }, // Cyan
	{ 1.0, 0.5, 0.0, 1.0 }, // Orange
	{ 0.0, 0.0, 0.5, 1.0 }, // Navy
	{ 0.5, 0.5, 0.5, 1.0 }	// Gray
};

const std::vector<std::string> stringCOLORS =
{
	"red",		// Red
	"green",	// Green
	"blue",		// Blue
	"purple",	// Purple
	"yellow",	// Yellow
	"cyan",		// Cyan
	"orange",	// Orange
	"navy",		// Navy
	"gray"		// Gray
};

double deg2rad(double deg)
{
	return deg * M_PI / 180.0;
}

std::vector<std::string> split(const std::string &text, char sep)
{
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos)
	{
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}


// GaiaScene methods

GaiaScene::GaiaScene() : PCVR_Scene::PCVR_Scene()
{
	_sceneType = "gaia";
}

void GaiaScene::parseArgs(osg::ArgumentParser& args)
{
	PCVR_Scene::parseArgs(args);

	args.read("--minPc", _minPc);
	args.read("--maxPc", _maxPc);

	args.read("--minParallax", _minParallax);
	args.read("--maxParallax", _maxParallax);

	args.read("--minMag", _minMag);
	args.read("--maxMag", _maxMag);

	args.read("--minTeff", _minTeff);
	args.read("--maxTeff", _maxTeff);

	_magColor = args.read("--magColor");
}

void GaiaScene::initWindowAndVR()
{
	PCVR_Scene::initWindowAndVR();

	// Create a set of Coordinate Axes for time history plot
	osg::ref_ptr<OpenFrames::CoordinateAxes> axes = new OpenFrames::CoordinateAxes("Gaia Data Release 2 (Radial Velocities)", 0.0, 0.8, 0.8, 1);
	axes->setAxisLength(3000.0);
	axes->setTickSpacing(100.0, 10.0);
	axes->setTickSize(20, 10);
	axes->setTickShader("../../shaders/Marker_Plus.frag"); // Default to solid circle
	axes->setXLabel("X");
	axes->setYLabel("Y");
	axes->setZLabel("Z");
	_rootFrame->addChild(axes);

}

void GaiaScene::buildScene()
{
	_uiFilePath = ":Qt/menu/gaia.ui";
	PCVR_Scene::buildScene();

	_windowProxy->getGridPosition(0, 0)->setBackgroundColor(0, 0, 0);
	_windowProxy->getGridPosition(0, 0)->setSkySphereStarData("../../data/images/Stars_HYGv3.txt", -2.0, 8.0, 40000, 1.0, 4.0, 0.1);

	std::unordered_map<long long, GaiaStar*> knownStars;
	setKnownStars(knownStars);	// Fill in _knownStars
	readSpheres();		// Read in existing selection spheres
	readIsochrones();	// Read in Isochrone tables

	osg::ref_ptr<osg::Vec4Array> ptColors = new osg::Vec4Array();
	for (auto& dataPath : _dataPaths)
	{
		for (auto & fname : fs::directory_iterator(dataPath))
		{
			std::string fileString = fname.path().string();
			std::cout << "Reading data file: " << fileString << std::endl;

			io::CSVReader<15> in(fileString);
			try
			{
				// Note radial_velocity is only valid for DR2
				in.read_header(io::ignore_extra_column,
					"source_id", "ra", "dec", "l", "b", "parallax", "pmra", "pmdec", "phot_g_mean_mag",
					"radial_velocity", "teff_val", "phot_bp_mean_mag", "phot_rp_mean_mag", "a_g_val",
					"e_bp_min_rp_val");
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}

			long long source_id;
			double ra, dec, l, b, parallax, pmra, pmdec, a_g_val, e_bp_min_rp_val,
				phot_g_mean_mag, rv, teff, phot_bp_mean_mag, phot_rp_mean_mag;

			while (in.read_row(source_id, ra, dec, l, b, parallax, pmra, pmdec, phot_g_mean_mag, rv,
				teff, phot_bp_mean_mag, phot_rp_mean_mag, a_g_val, e_bp_min_rp_val))
			{
				if (_minParallax <= parallax && parallax <= _maxParallax)
				{
					// Compute dependent columns and store array values and points
					double dist = 1000.0 / parallax;
					double phi = 90.0 - b;

					double abs_g_mag = phot_g_mean_mag + 5 * (log10(parallax / 1000) + 1);

					double x = dist * cos(deg2rad(l)) * sin(deg2rad(phi));
					double y = dist * sin(deg2rad(l)) * sin(deg2rad(phi));
					double z = dist * cos(deg2rad(phi));
					osg::Vec3 pos = osg::Vec3(x, y, z);

					double c1 = sin(deg2rad(27.12825)) * cos(deg2rad(dec)) - cos(deg2rad(27.12825)) *
						sin(deg2rad(dec)) * cos(deg2rad(ra - 192.85948));
					double c2 = cos(deg2rad(27.12825)) * sin(deg2rad(ra - 192.85948));
					double pml = (1 / cos(deg2rad(b))) * (c1 * pmra + c2 * pmdec);
					double thetadot = pml / cos(deg2rad(b));
					double pmb = (1 / cos(deg2rad(b))) * (c1 * pmdec - c2 * pmra);

					const double pc_per_1kYR_to_km_per_sec = 977.813106;
					double u = (rv / pc_per_1kYR_to_km_per_sec) * x / dist - (dist / 206264.806) * (sin(deg2rad(phi)) *
						sin(deg2rad(l)) * thetadot - cos(deg2rad(phi)) * cos(deg2rad(l)) * (-pmb));
					double v = (rv / pc_per_1kYR_to_km_per_sec) * y / dist + (dist / 206264.806) * (sin(deg2rad(phi)) *
						cos(deg2rad(l)) * thetadot + cos(deg2rad(phi)) * sin(deg2rad(l)) * (-pmb));
					double w = (rv / pc_per_1kYR_to_km_per_sec) * z / dist - dist * sin(deg2rad(phi)) * (-pmb) / 206264.806;
					osg::Vec3 vel = osg::Vec3(u, v, w);

					// And only stars that are known or satisfy command line parameters.
					if (knownStars.count(source_id) ||
						_minPc <= pos.length() && pos.length() <= _maxPc &&
						_minMag <= abs_g_mag && abs_g_mag <= _maxMag &&
						_minTeff <= teff && teff <= _maxTeff)
					{
						double normMag = (abs_g_mag - (-7.0)) / (12.0 - (-7.0));
						double normTeff = (teff - (3000.0)) / (10000.0 - (3000.0));
						double colorVal = _magColor ? normMag : normTeff;

						// If star source_id is found in known stars, then fill pos and vel in groups to which it belongs.
						auto itr = knownStars.find(source_id);
						if (itr != knownStars.end()) {
							for (auto& group : _knownGroups)
							{
								auto i = std::find_if(group.begin(), group.end(),
									[=](GaiaStar* star) { return star->sourceId == source_id; });
								if (i != group.end())
								{
									(*i)->pos = pos;
									(*i)->vel = vel;
									(*i)->found = true;
									(*i)->starVert = pos;
									(*i)->starVertOrig = pos;
								}
							}
						}

						// Add to Vector of All Stars
						GaiaStar* star = new GaiaStar();
						star->sourceId = source_id;

						star->pos = pos;
						star->vel = vel;
						star->ra = ra;
						star->dec = dec;
						star->parallax = parallax;
						star->teff = teff;
						star->l = l;
						star->b = b;

						star->starVert = pos;
						star->starVertOrig = pos;
						star->abs_g_mag = abs_g_mag;
						star->a_g_val = a_g_val;
						star->e_bp_min_rp_val = e_bp_min_rp_val;
						star->phot_bp_mean_mag = phot_bp_mean_mag;
						star->phot_rp_mean_mag = phot_rp_mean_mag;
						_allStars.push_back(star);

						_ptVertsOrig->push_back(pos);
						_ptVerts->push_back(pos);
						_ptVels->push_back(vel);
						ptColors->push_back(getHeatMapColor(1 - colorVal));
					}
				}
			}
		}
	}

	osg::ref_ptr<osg::Geometry> ptGeom = new osg::Geometry();
	ptGeom->setUseDisplayList(true);
	ptGeom->setUseVertexBufferObjects(true);
	ptGeom->setVertexArray(_ptVerts);
	ptGeom->setColorArray(ptColors, osg::Array::BIND_PER_VERTEX);
	ptGeom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, _ptVerts->size()));

	osg::ref_ptr<osg::Geode> ptGeode = new osg::Geode();
	ptGeode->addDrawable(ptGeom);
	_ptSwitch->addChild(ptGeode, true);
	_rootFrame->getGroup()->addChild(_ptSwitch);

	std::cout << "Total Number of Stars (with filtering, if used): " << _ptVerts->size() << std::endl;

	matchStarsInIsochrones();
	markersForIsochrones();
	markersForKnownStars();
}

void GaiaScene::step(OpenFrames::FramerateLimiter& waitLimiter)
{
	PCVR_Scene::step(waitLimiter);

	static int tick = 0;
	if (_yearIncrement != 0 && !_paused && tick % 4 == 0)
	{
		updateToYear(_currentYear + (4 * _yearIncrement / waitLimiter.getFramerate()));
	}
	tick++;
}

void GaiaScene::updateToYear(long year)
{
	_currentYear = year;

	_yearLabel[0]->setText(QString::number(_currentYear / 1000000.0, 'f', 8));
	_yearLabel[1]->setText(QString::number(_currentYear / 1000000.0, 'f', 8));

	// Move all stars to proper location based on _currentYear.
	float t = _currentYear / 1000;
	const float omega = 2.828427e-5;
	const float nu = 7.5e-5;
	const float a = omega / 2;
	const float b = -omega / 2;
	const float kappa = std::sqrt(-4 * omega * b);

	if (_straightVel == 0 && t != 0)
	{
		// Only move all stars if they are visible.
		if (_ptSwitch->getChildValue(_ptSwitch->getChild(0)))
		{
			for (int i = 0; i < _ptVerts->size(); i++)
			{
				float x0 = _ptVerts->at(i).x();
				float y0 = _ptVerts->at(i).y();
				float z0 = _ptVerts->at(i).z();

				float u = _ptVels->at(i).x();
				float v = _ptVels->at(i).y();
				float w = _ptVels->at(i).z();

				float x = x0 + (v / 2 * b) * (1.0 - cos(kappa*t)) + (u / kappa) * sin(kappa*t);
				float y = y0 + 2 * a *(x0 + (v / (2 * b))) * t - (omega / (b*kappa))* v * sin(kappa*t)
					+ (2 * omega / (kappa*kappa)) * u * (1.0 - cos(kappa*t));
				float z = (w / nu) * sin(nu*t) + z0 * cos(nu * t);

				_ptVerts->at(i).set(x, y, z);
			}
			_ptVerts->dirty();
		}

		// Move other known star markers once each
		for (auto& group : _knownGroups)
		{
			for (auto star : group)
			{
				if (star->found)
				{
					float x0 = star->starVert.x();
					float y0 = star->starVert.y();
					float z0 = star->starVert.z();

					float u = star->vel.x();
					float v = star->vel.y();
					float w = star->vel.z();

					float x = x0 + (v / 2 * b) * (1.0 - cos(kappa*t)) + (u / kappa) * sin(kappa*t);
					float y = y0 + 2 * a *(x0 + (v / (2 * b))) * t - (omega / (b*kappa))* v * sin(kappa*t)
						+ (2 * omega / (kappa*kappa)) * u * (1.0 - cos(kappa*t));
					float z = (w / nu) * sin(nu*t) + z0 * cos(nu * t);

					star->traj->setPosition(x, y, z);

					star->starVert.set(osg::Vec3(x, y, z));
				}
			}
		}

		// Move markers for isochrone stars by epicyclic motion
		for (auto table : _isochroneTables)
		{
			for (auto star : table->starsMatchingTable)
			{
				float x0 = star->starVert.x();
				float y0 = star->starVert.y();
				float z0 = star->starVert.z();

				float u = star->vel.x();
				float v = star->vel.y();
				float w = star->vel.z();

				float x = x0 + (v / 2 * b) * (1.0 - cos(kappa*t)) + (u / kappa) * sin(kappa*t);
				float y = y0 + 2 * a *(x0 + (v / (2 * b))) * t - (omega / (b*kappa))* v * sin(kappa*t)
					+ (2 * omega / (kappa*kappa)) * u * (1.0 - cos(kappa*t));
				float z = (w / nu) * sin(nu*t) + z0 * cos(nu * t);

				star->traj->setPosition(x, y, z);

				star->starVert.set(osg::Vec3(x, y, z));
			}
		}
	}
	else if (t != 0)
	{
		if (_ptSwitch->getChildValue(_ptSwitch->getChild(0)))
		{
			// Move all stars but known ones first
			for (int i = 0; i < _ptVerts->size(); i++)
			{
				_ptVerts->at(i).set(_ptVertsOrig->at(i) + _ptVels->at(i) * t);
			}
			_ptVerts->dirty();
		}

		// Move other known star markers once each
		for (auto& group : _knownGroups)
		{
			for (auto star : group)
			{
				if (star->found)
				{
					star->traj->setPosition(star->starVert + star->vel * t);
				}
			}
		}

		// Move markers for isochrone stars that were in the selected table
		for (auto table : _isochroneTables)
		{
			for (auto star : table->starsMatchingTable)
			{
				star->traj->setPosition(star->starVert + star->vel * t);
			}
		}
	}
	

	if (t == 0)
	{
		for (int i = 0; i < _ptVerts->size(); i++)
		{
			_ptVerts->at(i).set(_ptVertsOrig->at(i));
		}
		_ptVerts->dirty();

		// Reset known groups to time 0
		for (auto& group : _knownGroups)
		{
			for (auto star : group)
			{
				if (star->found)
				{
					star->traj->setPosition(star->starVertOrig);
					star->starVert.set(star->starVertOrig);
				}
			}
		}

		// Reset Isochrone markers to time 0
		for (auto table : _isochroneTables)
		{
			for (auto star : table->starsMatchingTable)
			{
				star->traj->setPosition(star->starVertOrig);
				star->starVert.set(star->starVertOrig);
			}
		}
	}
}

void GaiaScene::setKnownStars(std::unordered_map<long long, GaiaStar*>& knownStars)
{
	std::string path = "../../data/particles/KnownGaiaStars/DR2";

	int numFiles = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{});
	std::cout << "Number of known Gaia star files = " << numFiles << std::endl;

	int fileCounter = 0;	// use to set colorIndex below -- 1 color per file
	for (auto& fname : fs::directory_iterator(path))
	{
		std::string fileString = fname.path().string();
		std::string fileName = fname.path().stem().string();
		std::cout << "Reading known Gaia star datafile: " << fileString << std::endl;

		_knownGroups.push_back(std::vector<GaiaStar*>());

		io::CSVReader<2> in(fileString);
		try
		{
			in.read_header(io::ignore_extra_column,
				"Gaia_Source_ID", "Name");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

		long long source_id;
		std::string name;
		while (in.read_row(source_id, name))
		{
			GaiaStar* star = new GaiaStar();
			star->name = name;
			star->sourceId = source_id;
			star->colorIndex = fileCounter;
			knownStars.insert(std::make_pair(source_id, star));
			_knownGroups.back().push_back(star);
		}
		/*fileCounter++;*/

		osg::ref_ptr<osg::Group> starGroup = new osg::Group();
		_knownGroupSwitch->addChild(starGroup);

		for (int i = 0; i < 2; i++)
		{
			QCheckBox* check = new QCheckBox(QString::fromStdString(fileName));
			QString colorString = "QCheckBox { color: ";
			QString colorVal = QString::fromStdString(stringCOLORS[fileCounter % stringCOLORS.size()]);
			QString endString = " }";
			check->setStyleSheet(colorString + colorVal + endString);
			QObject::connect(check, &QCheckBox::clicked, this,
				[=](bool checked) {
				_knownGroupSwitch->setChildValue(starGroup, checked);
			});
			_groups[i]->addWidget(check);
		}

		fileCounter++;
	}
}

void GaiaScene::readSpheres()
{
	std::string path = "../../data/particles/SelectionSpheres/DR2";

	int numFiles = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{});
	std::cout << "Number of Selection Spheres = " << numFiles << std::endl;
	int fileCounter = 0;	// use to set colorIndex below -- 1 color per file
	for (auto & fname : fs::directory_iterator(path))
	{
		std::string fileString = fname.path().string();
		std::string fileName = fname.path().stem().string();
		std::cout << "Reading Selection Sphere datafile: " << fileString << std::endl;

		std::ifstream ifs(fileString);
		std::string line;
		std::getline(ifs, line);

		std::string originComment, timeComment, radiusComment;
		std::istringstream is(line);

		double xOrigin, yOrigin, zOrigin, radius;
		is >> originComment >> xOrigin >> yOrigin >> zOrigin;
		std::getline(ifs, line);

		std::istringstream is2(line);
		is2 >> radiusComment >> radius;
		ifs.close();

		// Set known Stars (TW Hydrae association initially)
		io::CSVReader<13, io::trim_chars<' ', '\t'>, io::no_quote_escape<','>,
			io::throw_on_overflow, io::single_line_comment<'#'>> in(fileString);

		// Note radial_velocity is only valid for DR2
		in.read_header(io::ignore_no_column,
			"id", "x", "y", "z", "u", "v", "w", "ra", "dec", "parallax", "teff", "l", "b");

		long long sourceId;
		double x, y, z, u, v, w, ra, dec, parallax, teff, l, b;

		std::vector<GaiaStar*> sphereStars;
		int lines = 0;
		while (in.read_row(sourceId, x, y, z, u, v, w, ra, dec, parallax, teff, l, b))
		{
			GaiaStar* star = new GaiaStar();
			star->sourceId = sourceId;
			star->pos = osg::Vec3(x, y, z);
			star->vel = osg::Vec3(u, v, w);
			star->ra = ra;
			star->dec = dec;
			star->parallax = parallax;
			star->teff = teff;
			star->l = l;
			star->b = b;

			sphereStars.push_back(star);
			lines++;
		}
		std::cout << "Total number of sphere star lines: " << lines << std::endl;
		fileCounter++;

		// Make saved sphere (set to hidden by default).
		osg::Vec4 savedColor = osg::Vec4(0.545f, 0.0f, 0.0f, 0.5f);
		GaiaSphere* gSphere = new GaiaSphere(osg::Vec3(xOrigin, yOrigin, zOrigin),
			radius, savedColor, lines);
		gSphere->show(false);
		_rootFrame->addChild(gSphere);

		for (int i = 0; i < 2; i++)
		{
			QCheckBox* check = new QCheckBox(QString::fromStdString(fileName));
			QObject::connect(check, &QCheckBox::clicked, this,
				[=](bool checked) {
				gSphere->show(checked);
			});
			_spheres[i]->addWidget(check);
		}
	}
}

void GaiaScene::readIsochrones()
{
	std::string path = "../../data/particles/Isochrones";

	int numFiles = std::distance(fs::directory_iterator(fs::path(path)), fs::directory_iterator{});
	std::cout << "Number of Isochrones = " << numFiles << std::endl;

	int fileCounter = 0;	// use to set colorIndex below -- 1 color per file
	for (auto& fname : fs::directory_iterator(path))
	{
		if (fileCounter++ == 0) continue;

		std::string fileString = fname.path().string();
		std::string fileName = fname.path().stem().string();
		std::cout << "Reading Isochrone datafile: " << fileString << std::endl;
		io::CSVReader<3, io::trim_chars<' ', '\t'>, io::no_quote_escape<','>,
			io::throw_on_overflow, io::single_line_comment<'#'>> in(fileString);
		try
		{
			in.read_header(io::ignore_extra_column, "absolute_g_mag", "bp-rp", "mass");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		double absolute_g_mag, bp_rp, mass;

		IsochroneTable* isochroneTable = new IsochroneTable();
		isochroneTable->name = fileName;
		while (in.read_row(absolute_g_mag, bp_rp, mass))
		{
			IsochroneEntry* entry = new IsochroneEntry();
			entry->abs_g_mag = absolute_g_mag;
			entry->bp_rp = bp_rp;
			entry->mass = mass;
			isochroneTable->entries.push_back(entry);
		}
		_isochroneTables.push_back(isochroneTable);

		// Parse years from name and save in table
		std::vector<std::string> nameTokens = split(isochroneTable->name, '_');
		isochroneTable->millionYrs = stoi(nameTokens.at(1));

		for (int i = 0; i < 2; i++)
		{
			QCheckBox* check = new QCheckBox(QString::fromStdString(fileName));
			QObject::connect(check, &QCheckBox::clicked, this,
				[=](bool checked) {
				toggleIsochrone(checked, isochroneTable);
			});
			_isochrones[i]->addWidget(check);
		}
	}
}

void GaiaScene::matchStarsInIsochrones()
{
	for (PCVR_Selectable* sel : _allStars)
	{
		GaiaStar* star = static_cast<GaiaStar*>(sel);

		for (auto table : _isochroneTables)
		{
			for (auto& entry : table->entries)
			{  //  + star->a_g_val   + star->e_bp_min_rp_val
				if (abs(star->abs_g_mag - entry->abs_g_mag + star->a_g_val) <= 0.25 &&
					abs((star->phot_bp_mean_mag - star->phot_rp_mean_mag 
						+ star->e_bp_min_rp_val) - entry->bp_rp) <= 0.25)
				{
					table->starsMatchingTable.push_back(star);
					break;
				}
			}
		}
	}
}

void GaiaScene::markersForIsochrones()
{
	int colorIndex = 0;
	for (auto table : _isochroneTables)
	{
		std::cout << "Now processing Isochrone Table: " << table->name << std::endl;
		std::cout << "Current table number of stars: " <<
			table->starsMatchingTable.size() << std::endl;

		osg::Group* tableGroup = new osg::Group();
		for (auto star : table->starsMatchingTable)
		{
			osg::ref_ptr<OpenFrames::MarkerArtist> starMarker = new OpenFrames::MarkerArtist();
			starMarker->setMarkerShader("Shaders/Marker_Circle.frag");
			starMarker->setMarkerSize(15);

			osg::Vec4 colorVec = COLORS[colorIndex];
			starMarker->setMarkerColor(OpenFrames::MarkerArtist::START, colorVec.r(), colorVec.g(), colorVec.b());

			star->traj = new OpenFrames::DrawableTrajectory(star->name);
			star->traj->showAxes(OpenFrames::ReferenceFrame::NO_AXES);
			star->traj->showAxesLabels(OpenFrames::ReferenceFrame::NO_AXES);
			star->traj->addArtist(starMarker);
			star->traj->setPosition(star->pos);
			tableGroup->addChild(star->traj->getGroup());
		}
		_isochroneSwitch->addChild(tableGroup);
		colorIndex++;
	}
	_isochroneSwitch->setAllChildrenOff();
	_rootFrame->getGroup()->addChild(_isochroneSwitch);
}

void GaiaScene::markersForKnownStars()
{
	for (auto& group : _knownGroups)
	{
		for (auto star : group)
		{
			if (star->found)		// only make marker if known star was found in data
			{
				osg::ref_ptr<OpenFrames::MarkerArtist> starMarker = new OpenFrames::MarkerArtist();
				starMarker->setMarkerShader("Shaders/Marker_CirclePulse.frag");
				starMarker->setMarkerSize(15);

				osg::Vec4 colorVec = COLORS[star->colorIndex % COLORS.size()];
				starMarker->setMarkerColor(OpenFrames::MarkerArtist::START, colorVec.r(), colorVec.g(), colorVec.b());

				osg::ref_ptr<OpenFrames::DrawableTrajectory> starTraj = new OpenFrames::DrawableTrajectory(star->name);
				starTraj->showAxes(OpenFrames::ReferenceFrame::NO_AXES);
				starTraj->showAxesLabels(OpenFrames::ReferenceFrame::NO_AXES);
				starTraj->showNameLabel(true);
				starTraj->addArtist(starMarker);
				starTraj->setPosition(star->pos);
				star->traj = starTraj;

				dynamic_cast<osg::Group*>(_knownGroupSwitch->getChild(star->colorIndex))
					->addChild(starTraj->getGroup());
			}
		}
	}
	_knownGroupSwitch->setAllChildrenOff();
	_rootFrame->getGroup()->addChild(_knownGroupSwitch);
}

osg::Vec4 GaiaScene::getHeatMapColor(double value)
{
	const int NUM_COLORS = 5;
	static double color[NUM_COLORS][3] = { { 0, 0, 1 },{ 0, 1, 1 },{ 0, 1, 0 },{ 1, 1, 0 },{ 1, 0, 0 } };

	// A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

	int idx1;        // |-- Our desired color will be between these two indexes in "color".
	int idx2;
	double fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

	if (value <= 0) { idx1 = idx2 = 0; }    // accounts for an input <=0
	else if (value >= 1) { idx1 = idx2 = NUM_COLORS - 1; }    // accounts for an input >=0
	else
	{
		value = value * (NUM_COLORS - 1);     // Will multiply value by 3.
		idx1 = floor(value);                  // Our desired color will be after this index.
		idx2 = idx1 + 1;                      // ... and before this index (inclusive).
		fractBetween = value - double(idx1);   // Distance between the two indexes (0-1).
	}

	double r = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
	double g = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
	double b = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
	return osg::Vec4(r, g, b, 0);
}

void GaiaScene::setupMenuEventListeners(PCVR_Controller* controller)
{
	PCVR_Scene::setupMenuEventListeners(controller);

	QWidget* controllerWidget = controller->getPanelWidget();
	int cIndex = controller == PCVR_Controller::Left() ? 0 : 1;

	QStackedWidget* stackedWidget = controllerWidget->findChild<QStackedWidget*>("stackedWidget");
	QObject::connect(controllerWidget->findChild<QRadioButton*>("viewRadioButton"), &QRadioButton::clicked, this,
		[=]() { stackedWidget->setCurrentIndex(0); });
	QObject::connect(controllerWidget->findChild<QRadioButton*>("toolsRadioButton"), &QRadioButton::clicked, this,
		[=]() { stackedWidget->setCurrentIndex(1); });
	QObject::connect(controllerWidget->findChild<QRadioButton*>("groupsRadioButton"), &QRadioButton::clicked, this,
		[=]() { stackedWidget->setCurrentIndex(2); });


	QRadioButton* sphereAction = controllerWidget->findChild<QRadioButton*>("sphereButton");
	QObject::connect(sphereAction, &QRadioButton::clicked, this,
		[=]() { switchToolTo(new SphereDrawer<GaiaSphere>(_FM)); });

	QPushButton* selectionShowAllAction = controllerWidget->findChild<QPushButton*>("showAllButton");
	QObject::connect(selectionShowAllAction, &QPushButton::clicked, this,
		[=]() { PCVR_Selection::ShowAllSelections(true); });

	QPushButton* selectionHideAllAction = controllerWidget->findChild<QPushButton*>("hideAllButton");
	QObject::connect(selectionHideAllAction, &QPushButton::clicked, this,
		[=]() { PCVR_Selection::ShowAllSelections(false); });

	_yearLabel[cIndex] = controllerWidget->findChild<QLabel*>("yearLabel");

	_yearIncSlider[cIndex] = controllerWidget->findChild<QSlider*>("horizontalSlider");
	QLabel* yearIncLabel = controllerWidget->findChild<QLabel*>("yearIncrementLabel");
	QObject::connect(_yearIncSlider[cIndex], &QSlider::valueChanged, yearIncLabel,
		[=](int yearInc) {
		_yearIncrement = 1000000 * (yearInc / 100.0);
		yearIncLabel->setText(QString("  ") + QString::number(yearInc / 100.0) + QString(" million"));
	});

	QPushButton* minusOneMillionYearsButton = controllerWidget->findChild<QPushButton*>("minusMillionYearsButton");
	QObject::connect(minusOneMillionYearsButton, &QPushButton::clicked, this,
		[=]() { updateToYear(_currentYear - 1000000); });

	QPushButton* plusOneMillionYearsButton = controllerWidget->findChild<QPushButton*>("plusMillionYearsButton");
	QObject::connect(plusOneMillionYearsButton, &QPushButton::clicked, this,
		[=]() { updateToYear(_currentYear + 1000000); });

	QCheckBox* hideOtherStars = controllerWidget->findChild<QCheckBox*>("hideUnnamedStarsCheckBox");
	QObject::connect(hideOtherStars, &QCheckBox::stateChanged, this, &GaiaScene::hideUnnamedStars);

	QPushButton* saveSourceIDsAction = controllerWidget->findChild<QPushButton*>("saveSelectionsButton");
	QObject::connect(saveSourceIDsAction, &QPushButton::clicked, this,
		[=]() { PCVR_Selection::SaveAllSelections("../../data/particles/SelectionSpheres/DR2", _allStars); });

	QPushButton* resetViewButton = controllerWidget->findChild<QPushButton*>("resetViewButton");
	QObject::connect(resetViewButton, &QPushButton::clicked, this,
		[=]() { _mainView->restoreTrackball(); });

	_positionValueLabel[cIndex] = controllerWidget->findChild<QLabel*>("positionValueLabel");

	QPushButton* getPositionButton[2];
	getPositionButton[cIndex] = controllerWidget->findChild<QPushButton*>("getPositionButton");
	QObject::connect(getPositionButton[cIndex], &QPushButton::clicked, this,
		[=]() {getPosition(controller); });

	_spheres[cIndex] = controllerWidget->findChild<QWidget*>("selectionArea")->layout();
	_groups[cIndex] = controllerWidget->findChild<QWidget*>("groupsArea")->layout();
	_isochrones[cIndex] = controllerWidget->findChild<QWidget*>("isochroneArea")->layout();

	QPushButton* resetToPresentButton = controllerWidget->findChild<QPushButton*>("resetToPresentButton");
	QObject::connect(resetToPresentButton, &QPushButton::clicked, this,
		[=]() {
		setYearIncrement(0);
		updateToYear(0);
	});

	QPushButton* showAllGroups = controllerWidget->findChild<QPushButton*>("showAllGroups");
	QObject::connect(showAllGroups, &QPushButton::clicked, this,
		[=]() {
		_knownGroupSwitch->setAllChildrenOn();
		/*QObjectList children = _groups[cIndex]->children();*/
		QLayout* groups = _groups[cIndex];
		if (groups)
		{
			for (int i = 0; i < groups->count(); i++)
			{
				QCheckBox* checkBox = qobject_cast<QCheckBox *>(groups->itemAt(i)->widget());
				checkBox->blockSignals(true);
				checkBox->setChecked(true);
				checkBox->blockSignals(false);
			}
		}
	});

	QPushButton* hideAllGroups = controllerWidget->findChild<QPushButton*>("hideAllGroups");
	QObject::connect(hideAllGroups, &QPushButton::clicked, this,
		[=]() {
		_knownGroupSwitch->setAllChildrenOff();
		QLayout* groups = _groups[cIndex];
		if (groups)
		{
			for (int i = 0; i < groups->count(); i++)
			{
				QCheckBox* checkBox = qobject_cast<QCheckBox *>(groups->itemAt(i)->widget());
				checkBox->blockSignals(true);
				checkBox->setChecked(false);
				checkBox->blockSignals(false);
			}
		}
	});

	QCheckBox* showSkyboxCheckBox = controllerWidget->findChild<QCheckBox*>("showSkyboxCheckBox");
	QObject::connect(showSkyboxCheckBox, &QCheckBox::stateChanged, this,
		[=](int state) { showSkyBox(state == Qt::CheckState::Checked); });
	QRadioButton* straightVelCheckBox = controllerWidget->findChild<QRadioButton*>("StraightVelRadioButton");
	QObject::connect(straightVelCheckBox, &QRadioButton::clicked, this,
		[=](int state) { 
		_straightVel = 1; 
	});
	QRadioButton* epicyclicCheckBox = controllerWidget->findChild<QRadioButton*>("EpicyclicRadioButton");
	QObject::connect(epicyclicCheckBox, &QRadioButton::clicked, this,
		[=](int state) { 
		_straightVel = 0;
	});

	//controllerWidget->findChild<QProgressBar*>("progressBar")->hide();
}

void GaiaScene::setYearIncrement(int yearIncrement)
{
	_yearIncrement = yearIncrement;
	_yearIncSlider[0]->setValue(0);
	_yearIncSlider[1]->setValue(0);
}

void GaiaScene::hideUnnamedStars(int state)
{
	if (state == Qt::CheckState::Checked) {
		_ptSwitch->setAllChildrenOff();
	}
	else {
		_ptSwitch->setAllChildrenOn();
	}
}

void GaiaScene::toggleIsochrone(bool checked, IsochroneTable* isoTable)
{
	int isoIndex = std::distance(_isochroneTables.begin(),
		std::find(_isochroneTables.begin(), _isochroneTables.end(), isoTable));
	_isochroneSwitch->setValue(isoIndex, checked);

	if (checked)
	{
		setYearIncrement(0);
		updateToYear(_currentYear - (1000000 * isoTable->millionYrs));
	}
	else  // reset to present time 0
	{
		setYearIncrement(0);
		updateToYear(0);
	}
}

void GaiaScene::getPosition(PCVR_Controller* controller)
{
	int cIndex = controller == PCVR_Controller::Left() ? 0 : 1;
	osg::Vec3d pos = controller->getWorldPos();
	_positionValueLabel[cIndex]->setText(QString("X: ") + QString::number(pos.x()) + QString("\n") +
		QString("Y: ") + QString::number(pos.y()) + QString("\n") +
		QString("Z: ") + QString::number(pos.z()));
}


