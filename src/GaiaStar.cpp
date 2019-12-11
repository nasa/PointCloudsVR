#include "GaiaStar.hpp"

osg::Vec3 GaiaStar::getPos() const
{
	return pos;
}

osg::Vec4 GaiaStar::getColor() const
{
	return color;
}

void GaiaStar::setPos(osg::Vec3 position)
{
	pos = position;
}

void GaiaStar::setColor(osg::Vec4 col)
{
	color = col;
}

std::ostream& GaiaStar::writeToStream(std::ostream& o) const
{
	return o << sourceId << ',' <<
			pos.x() << ',' << pos.y() << ',' << pos.z() << ',' <<
			vel.x() << ',' << vel.y() << ',' << vel.z() << ',' <<
			ra << ',' <<
			dec << ',' <<
			parallax << ',' <<
			teff << ',' <<
			l << ',' <<
			b << std::endl;
}