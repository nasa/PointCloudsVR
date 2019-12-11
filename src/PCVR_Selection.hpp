#pragma once

#include <ostream>
#include <vector>

#include <osg/Vec3>
#include <osg/Vec4>

#include "Removable.hpp"

class PCVR_Selectable
{
public:
	virtual osg::Vec3 getPos() const = 0;
	virtual osg::Vec4 getColor() const = 0;

	virtual void setPos(osg::Vec3 pos) = 0;
	virtual void setColor(osg::Vec4 color) = 0;

	virtual std::ostream& writeToStream(std::ostream& o) const;
};

class PCVR_Selection : public Removable
{
public:
	static void SaveAllSelections(const std::string& path, const std::vector<PCVR_Selectable*>& points);
	static void ShowAllSelections(bool b);

	PCVR_Selection();
	virtual void remove() override;
	virtual void save(const std::string& path, const std::vector<PCVR_Selectable*>& points) = 0;
	virtual void show(bool b) = 0;

protected:
	static std::list<PCVR_Selection*> _Selections;

	bool _saved = false;
};