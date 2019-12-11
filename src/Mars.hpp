#pragma once

#include <OpenFrames/OpenVRDevice.hpp>
#include <OpenFrames/Sphere.hpp>

class Mars : public OpenFrames::Sphere
{
public:
	Mars(osg::ref_ptr<OpenFrames::OpenVRDevice> ovrDevice);
};