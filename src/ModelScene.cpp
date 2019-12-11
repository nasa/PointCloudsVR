#include <osgDB/FileNameUtils>

#include "ModelScene.hpp"

ModelScene::ModelScene() : PCVR_Scene::PCVR_Scene()
{
}

void ModelScene::buildScene()
{
	PCVR_Scene::buildScene();

	for (auto& path : _dataPaths)
	{
		// Create and load model
		std::string fname = osgDB::getSimpleFileName(path);
		osg::ref_ptr<OpenFrames::Model> model = new OpenFrames::Model(fname, 0.5, 0.5, 0.5, 0.9);
		model->setModel(path);

		// Reset model pivot so its origin coincides with the root scene origin
		model->setModelPivot(0.0, 0.0, 0.0);
		_rootFrame->addChild(model);
	}
}