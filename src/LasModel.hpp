#pragma once

#include <liblas/liblas.hpp>

#include <OpenFrames/Model.hpp>

#include "PCVR_Selection.hpp"

class LasModel;

class ModelPoint : public liblas::Point, public PCVR_Selectable
{
public:
	ModelPoint(liblas::Point const& p, int index, LasModel* parent);

	osg::Vec3 getPos() const override;
	osg::Vec4 getColor() const;

	void setPos(osg::Vec3 pos) override;
	void setColor(osg::Vec4 color) override;

private:
	int _index;
	LasModel* _parent;
};

class LasModel : public OpenFrames::Model
{
	friend class ModelPoint;

public:
	LasModel(const std::string& path);

	std::vector<ModelPoint*>& getPoints();
	osg::Vec4Array& getColors();

private:
	osg::ref_ptr<osg::Vec3Array> _verts = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec4Array> _colors = new osg::Vec4Array();
	std::vector<ModelPoint*> _points;

	void loadLasFile(const std::string& path);
};