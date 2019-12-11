#include <osgDB/FileNameUtils>

#include <QCheckBox>
#include <QRadioButton>
#include <QWidget>
#include <QFile>
#include <QuiLoader>

#include "DiskDrawer.hpp"
#include "LasModel.hpp"

#include "LasModelScene.hpp"

LasModelScene::LasModelScene()
	: ModelScene()
{
}

void LasModelScene::buildScene()
{
	PCVR_Scene::buildScene();

	for (auto& path : _dataPaths)
	{
		osg::ref_ptr<LasModel> model = new LasModel(path);
		_models.push_back(model);
		_rootFrame->addChild(model);

		for (ModelPoint* p : model->getPoints())
		{
			_selectables.push_back(p);
		}
	}
}

std::vector<OpenFrames::Model*>& LasModelScene::getModels()
{
	return _models;
}

void LasModelScene::setupMenuEventListeners(PCVR_Controller* controller)
{
	ModelScene::setupMenuEventListeners(controller);
	QWidget* controllerWidget = controller->getPanelWidget();
	int cIndex = controller == PCVR_Controller::Left() ? 0 : 1;

	_circumferenceLabel[cIndex] = controllerWidget->findChild<QLabel*>("circumferenceLabel");

	QRadioButton* diskAction = controllerWidget->findChild<QRadioButton*>("diskButton");
	QObject::connect(diskAction, &QRadioButton::clicked, this,
		[=]() { switchToolTo(new DiskDrawer<SelectionDisk>(_FM, _selectables, controller)); });

	QCheckBox* colorCheckBox = controllerWidget->findChild<QCheckBox*>("colorByClassificationCheckBox");
	QObject::connect(colorCheckBox, &QCheckBox::stateChanged, this,
		[=](int state) { colorForest(state == Qt::CheckState::Checked); });
}

void LasModelScene::colorForest(bool b)
{
	osg::Vec4 BLACK = osg::Vec4(0, 0, 0, 1);
	osg::Vec4 WHITE = osg::Vec4(1, 1, 1, 1);
	osg::Vec4 RED = osg::Vec4(1, 0, 0, 1);
	osg::Vec4 GRAY = osg::Vec4(0.5, 0.5, 0.5, 1);

	for (PCVR_Selectable* sel : _selectables)
	{
		ModelPoint* p = static_cast<ModelPoint*>(sel);
		if (b)
		{
			uint8_t c = p->GetClassification().GetClass();
			switch (c)
			{
			case 1: // Unclassified
				p->setColor(BLACK);
				break;

			case 2: // Ground
				p->setColor(GRAY);
				break;

			case 3: // Unchanged
				p->setColor(WHITE);
				break;

			case 4: // Destroyed by Hurricane Maria
				p->setColor(RED);
				break;
			}
		}
		else {
			p->setColor(BLACK);
		}
	}
	static_cast<LasModel*>(_models[0])->getColors().dirty();
}