#pragma once

#include <ostream>

#include <osg/Vec3>
#include <osg/Vec4>

class Point
{
public:
	virtual osg::Vec3 getPos() const = 0;
	virtual osg::Vec4 getColor() const = 0;

	virtual void setPos(osg::Vec3 pos) = 0;
	virtual void setColor(osg::Vec4 color) = 0;

	virtual std::ostream& writeToStream(std::ostream& o) const;
};