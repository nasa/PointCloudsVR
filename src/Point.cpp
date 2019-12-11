#include "Point.hpp"

std::ostream& Point::writeToStream(std::ostream& o) const
{
	osg::Vec3 pos = getPos();
	return o << pos.x() << ',' << pos.y() << ',' << pos.z() << std::endl;
}