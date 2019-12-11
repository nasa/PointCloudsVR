/**********************************************************************
PointCloudsVR -- new code by Matt Brandt, Code 587, NASA / GSFC, 2018

Parts of PointsVR are based on ofviewer, ofpointclouds, and ofcontrols
by Ravi Mathur of Emergent Space Technologies

-----------------------------------------------------------------------
ofviewer, ofpointclouds, ofcontrols - Copyright 2017 Ravishankar Mathur

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing
permissions and limitations under the License.
-----------------------------------------------------------------------

**********************************************************************/

#include <iostream>

#include <osgDB/FileNameUtils>

#include "PCVR_Scene.hpp"
#include "GaiaScene.hpp"
#include "LavaTubeScene.hpp"
#include "MarsScene.hpp"
#include "ModelScene.hpp"
#include "LasModelScene.hpp"
#include "FlowScene.hpp"

PCVR_Scene* chooseScene(osg::ArgumentParser& args);
void checkArgs(osg::ArgumentParser& args);
void usage();

int main(int argc, char **argv)
{
	osg::ArgumentParser args(&argc, argv);

	if (args.argc() == 1 || args.read("--help")) usage();

	PCVR_Scene* scene = chooseScene(args);

	scene->parseArgs(args);
	checkArgs(args);

	scene->initWindowAndVR();
	scene->buildScene();

	scene->run();
}

PCVR_Scene* chooseScene(osg::ArgumentParser& args)
{
	std::string sceneArg;
	if (!args.read("--scene", sceneArg))
	{
		int i = args.find("--data");
		if (i != -1 &&
			osgDB::getLowerCaseFileExtension(args[i + 1]) == "las")
		{
			return new LasModelScene();
		}
		return new ModelScene();
	}
	if (sceneArg == "gaia")
	{
		return new GaiaScene();
	}
	if (sceneArg == "lavatube")
	{
		return new LavaTubeScene();
	}
	if (sceneArg == "mars")
	{
		return new MarsScene();
	}
	if (sceneArg == "flow")
	{
		return new FlowScene();
	}

	std::cout << "if --scene is present, it must be one of the following values: gaia, lavatube, mars, or flow" << std::endl;
	exit(1);
}

void checkArgs(osg::ArgumentParser& args)
{
	args.reportRemainingOptionsAsUnrecognized();
	if (args.argc() > 1)
	{
		std::cout << "Unrecognized option given." << std::endl;
		usage();
	}
}

void usage()
{
	std::cout <<
		"\n"
		"General options:\n"
		"    --help                             Show this text.\n"
		"    --scene <gaia | mars | juniper | flow>    Choose which scene to run. If option omitted, will simply display all data files at origin.\n"
		"    --data  <datafile or directory>    Choose which data to display(can appear multiple times, but must appear at least once).\n"
		"    --novr                             If present, means do not run in vr mode.\n"
		"    --winRes <num pixels>              Change width, height for square mirror window from default 600x600.\n"
		"                                           Higher resolution means worse performance, but better screenshots.\n"
		"\n"
		"Gaia options :\n"
		"    --minPc       <parsecs>            Filter out stars closer than --minPc or farther than --maxPc.\n"
		"    --maxPc       <parsecs>\n"
		"    --minParallax <parallax>           Filter out stars with parallax less than --minParallax or greater than --maxParallax.\n"
		"    --maxParallax <parallax>\n"
		"    --minMag      <abs mag>            Filter out stars with absolute magnitude less than --minMag or greater than --maxMag.\n"
		"    --maxMag      <abs mag>\n"
		"    --minTeff     <effective temp>     Filter out stars with effective temperature less than --minTeff or greater than --maxTeff.\n"
		"    --maxTeff     <effective temp>\n"
		"    --magColor                         If present, color stars by magnitude as opposed to by teff(which is the default)."
		"\n"
		"Flow options:\n"
		"    --diatom					Filter by showing only diatom Phytoplanktons.\n"
		"    --coco 					Filter by showing only Coccolithophores Phytoplanktons.\n"
		"    --prok 					Filter by showing only Prokaryotes Phytoplanktons.\n"
		"    --dino 					Filter by showing only Dinoflagellates Phytoplanktons.\n"
		"\n"
		<< std::endl;
	exit(1);
}