#pragma once

#include "LasModel.hpp"
#include "ModelScene.hpp"

#include <QLabel>

class LasModelScene : public ModelScene
{
public:
	LasModelScene();

	void buildScene() override;
	std::vector<OpenFrames::Model*>& getModels();

	// Qt
	QLabel* _circumferenceLabel[2];
	QLabel* _panelCircumferenceLabel;
	std::string _uiDialogFilePath = ":Qt/dialog/circumfDialog.ui";
	osg::ref_ptr<OpenFrames::QWidgetPanel> _circumfPanel;
	QWidget* _panelWidget;

private:

	void setupMenuEventListeners(PCVR_Controller* controller) override;
	void colorForest(bool b);
};