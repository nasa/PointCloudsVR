#pragma once

#include "LasModel.hpp"
#include "PCVR_Scene.hpp"

class ModelScene : public PCVR_Scene
{
public:
	ModelScene();

	void buildScene() override;

protected:
	std::vector<OpenFrames::Model*> _models;
};