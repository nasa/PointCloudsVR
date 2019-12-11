//-------------------------------------------------------------------------------------------------
// FlowScene.cpp:
//	Author: Nargess Memarsadeghi, Code 586, NASA / GSFC, January-October 2018
//  This class supports reading and visualizng phytoplanton netcdf data sets obained from 
//  MIT's Darwin project.
//----------------------------------------------------------------------------------------------=
#include <iostream>
#include <filesystem>

#include <OpenFrames/MarkerArtist.hpp>
#include <OpenFrames/Model.hpp>

#include <unordered_map>

#include <osg/Array>

#include <QObject>
#include <QApplication>
#include <QSlider>
#include <QLCDnumber>
#include <QFile>
#include <QUiLoader>
#include <QPushButton>

#include <QProgressBar>
#include <QLabel>
#include <QMenu>
#include <QThread>
#include <QFutureWatcher>
#include <QLayout>

#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QComboBox>

#include "PCVR_Scene.hpp"
#include "FlowScene.hpp"
#include "DarwinNetcdfs.hpp"

using namespace std;

namespace fs = std::experimental::filesystem;

FlowScene::FlowScene() : PCVR_Scene::PCVR_Scene()
{
	_sceneType = "flow";
}

//using namespace std;
void FlowScene::parseArgs(osg::ArgumentParser& args)
{
	PCVR_Scene::parseArgs(args);

	darwinData._dino   = args.read("--dino");
	darwinData._coco   = args.read("--coco");
	darwinData._diatom = args.read("--diatom");
	darwinData._prok = args.read("--prok");

	if (!darwinData._dino && !darwinData._coco && !darwinData._diatom && !darwinData._prok)
	{
		// there is no filter, consider all
		darwinData._dino = 1;
		darwinData._coco = 1;
		darwinData._diatom = 1;
		darwinData._prok = 1;
	}

}
//----------------------------------------------------------------------------------------------------------------------
void FlowScene::buildScene()
{
	_uiFilePath = ":Qt/menu/flow.ui";
	PCVR_Scene::buildScene();

	//alice blue: 240, 248, 255, 
	//lavender 230, 230, 250
	//seashell 3: 205-197-191
	_windowProxy->getGridPosition(0, 0)->setBackgroundColor(0.0f, 0.0f, 0.0f);
	_windowProxy->getGridPosition(0, 0)->setSkySphereStarData("../../data/images/Stars_HYGv3.txt", -2.0, 8.0, 40000, 1.0, 4.0, 0.1);
	
	
	osg::ref_ptr<OpenFrames::Model> earth = new OpenFrames::Model("Earth", 0, 1, 0, 1);
	earth->setModel("../../data/osgearth/earthmap_finesse.earth");

	double _radiusScale = 100 ;
	earth->setModelScale(1 / _radiusScale, 1 / _radiusScale, 1/_radiusScale);

	_rootFrame->addChild(earth);

	
	_ptSwitch = new osg::Switch;

	for (auto& path : _dataPaths)
	{
		cout << "numFiles = " << _dataPaths.size() << endl;

		for (auto & fname : fs::directory_iterator(path))
		{
			std::string filename = fname.path().string();
			cout << "filename is: " << filename << endl;
			darwinData.readDarwinData(filename.c_str());
			
			cout << "------------>> dataset number is: <<--------" << _datasetNum << endl;
			
			ptVerts.push_back(new osg::Vec3Array());
			ptColors.push_back(new osg::Vec4Array());

			allCoco.push_back(std::vector<float>());
			allDino.push_back(std::vector<float>());
			allDiatom.push_back(std::vector<float>());
			allProk.push_back(std::vector<float>());

			filteredDensities.push_back(std::vector<float>());

			ptCount = 0;
			float max = -1;
			
			
			if (darwinData._dino && darwinData._coco && darwinData._diatom && darwinData._prok)
			{
				addAll();
			}
			else if (darwinData._diatom)
			{
				addPoints(darwinData.diatom, darwinData.diatomColor, darwinData.diatomScale);
				cout << "adding diatoms" << endl;
			}
			else if (darwinData._dino)
			{
				addPoints(darwinData.dino, darwinData.dinoColor, darwinData.dinoScale);
				cout << "adding dino" << endl;
			}
			else if (darwinData._coco)
			{
				addPoints(darwinData.coco, darwinData.cocoColor, darwinData.cocoScale);
				cout << "adding coco" << endl;
			}
			else if (darwinData._prok)
			{
				addPoints(darwinData.prok, darwinData.prokColor, darwinData.prokScale);
				cout << "adding prok" << endl;
			}
				

			osg::ref_ptr<osg::Geometry> frameGeom = new osg::Geometry();
			frameGeom->setUseDisplayList(true);
			frameGeom->setUseVertexBufferObjects(true);
			frameGeom->setVertexArray(ptVerts[_datasetNum]);
			frameGeom->setColorArray(ptColors[_datasetNum], osg::Array::BIND_PER_VERTEX);
			frameGeom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, ptVerts[_datasetNum]->size()));		

			osg::ref_ptr<osg::StateSet> stateSet = frameGeom->getOrCreateStateSet();
			stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
			stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

			osg::ref_ptr<osg::Geode> frameGeode = new osg::Geode();
			frameGeode->addDrawable(frameGeom);
			_ptSwitch->addChild(frameGeode, true);

			_datasetNum += 1;
		}
	}

	_rootFrame->getGroup()->addChild(_ptSwitch);
	_rootFrame->showNameLabel(false);
	_rootFrame->showAxes(false);
	_rootFrame->showAxesLabels(false);

	// Create views
	//osg::ref_ptr<OpenFrames::View> view4 = new OpenFrames::View(earth, earth);
	//view4->setTrackball(_pcvrTrackball);
	// Set Window params
	//_windowProxy->getGridPosition(0, 0)->addView(view4);

}
//--------------------------------------------------------------------------------------------------------------------------
void FlowScene::addPoints(const float phyto[DarwinNetcdfs::NZ][DarwinNetcdfs::NLAT][DarwinNetcdfs::NLON],  const osg::Vec4 phyto_color, float scaling_factor)
{
	int z, l, lo;
	double xx = 0, yy = 0, zz = 0;
	osg::Vec4 pcolor;

	for (z = 0; z < DarwinNetcdfs::NZ; z++)
	{
		for (l = 0; l < DarwinNetcdfs::NLAT; l++)
		{
			for (lo = 0; lo < DarwinNetcdfs::NLON; lo++)
			{
				darwinData.convertLatLongHeightToXYZ(darwinData.lats[l], darwinData.lons[lo], -darwinData.Zs[z], xx, yy, zz);
				osg::Vec3 vert = osg::Vec3(xx, yy, zz );



				if (phyto[z][l][lo] < 1E36 && phyto[z][l][lo] > 1E-3)
				{
					ptVerts[_datasetNum]->push_back(vert);
					filteredDensities[_datasetNum].push_back(phyto[z][l][lo]);
					pcolor = phyto_color * (phyto[z][l][lo] / scaling_factor);
					ptColors[_datasetNum]->push_back(pcolor);
					ptCount++;
				}
			}
		}
	}
	cout << "number of points added: " << ptCount << endl;
}
//----------------------------------------------------------------------------------------------------------------
void FlowScene::addAll()
{
	int z, l, lo;
	double xx = 0, yy = 0, zz = 0;
	float  dino_value=0, coco_value=0, prok_value=0, diatom_value=0;
	float  dino_color = 0, coco_color = 0, prok_color = 0, diatom_color = 0;

	osg::Vec4 phyto_color;
	osg::Vec3 vert;

	float nz = DarwinNetcdfs::NZ;

	for (z = 0; z < DarwinNetcdfs::NZ; z++)
	{
		alphaScale = (0.1< (nz -z-2 )  / nz ? (nz - z - 2) / nz:0.1);
	
		//cout <<"nz: "<<nz<< ", alhpaScale: " << alphaScale << endl;

		for (l = 0; l < DarwinNetcdfs::NLAT; l++)
		{
			//cout << "l is: " << l << endl;
			for (lo = 0; lo < DarwinNetcdfs::NLON; lo++)
			{
				darwinData.convertLatLongHeightToXYZ(darwinData.lats[l], darwinData.lons[lo], -darwinData.Zs[z], xx, yy, zz);
				vert = osg::Vec3(xx, yy, zz);

				diatom_value = darwinData.diatom[z][l][lo];
				dino_value = darwinData.dino[z][l][lo];
				coco_value = darwinData.coco[z][l][lo];
				prok_value = darwinData.prok[z][l][lo];

				if (diatom_value < 1E36 && diatom_value > 1E-3)
				{
				//	cout << "---> in add all 2 <---" << endl;
					diatom_color = diatom_value / darwinData.diatomScale;
				}
				else
					diatom_value = 0;

				if (dino_value < 1E36 && dino_value > 1E-3)
				{
			     //cout << "---> in add all 3 <---" << endl;
				  dino_color = dino_value / darwinData.dinoScale;
			     }
				else
					dino_value = 0;

				if (coco_value < 1E36 && coco_value > 1E-3)
				{
					//cout << "---> in add all 4 <---" << endl;
					coco_color = coco_value / darwinData.cocoScale;
				}
				else
					coco_value = 0;

				if (prok_value < 1E36 && prok_value > 1E-3)
				{
				//	cout << "---> in add all 5 <---" << endl;
					prok_color = prok_value / darwinData.prokScale;
				}
				else
					prok_value = 0;

				if (diatom_value || dino_value || coco_value || prok_value)
				{
				
					ptVerts[_datasetNum]->push_back(vert);
				    
					phyto_color = osg::Vec4(dino_color, diatom_color, coco_color, alphaScale);
					ptColors[_datasetNum]->push_back(phyto_color);
				
					allDiatom[_datasetNum].push_back(diatom_value);
					allCoco[_datasetNum].push_back(coco_value);
					allDino[_datasetNum].push_back(dino_value);
					allProk[_datasetNum].push_back(prok_value);
				
					ptCount++;
				}
			}
		}
	}

}
//-----------------------------------------------------------------------------------------------------------------------
void FlowScene::step(OpenFrames::FramerateLimiter& waitLimiter)
{
	PCVR_Scene::step(waitLimiter);

	static int tick = 0;
	if (!_paused && tick % 5 == 0)
	{
		_ptSwitch->setSingleChildOn(_datasetNum);
		_datasetNum = (_datasetNum + 1) % _ptSwitch->getNumChildren();
	}
	tick++;
}
//-----------------------------------------------------------------------------------------------------------------------
void FlowScene::setupMenuEventListeners(PCVR_Controller* controller)
{
	PCVR_Scene::setupMenuEventListeners(controller);

	QWidget* controllerWidget = controller->getPanelWidget();


	QLabel* redScaleLabel = controllerWidget->findChild<QLabel*>("redScaleText");
	QSlider* redScaleSlider = controllerWidget->findChild<QSlider*>("redScaleSlider");
	redScaleLabel->setText(QString::number(darwinData.dinoScale)); //set default
	redScaleSlider->setValue(darwinData.dinoScale*10.0); //set default
	QObject::connect(redScaleSlider, &QSlider::valueChanged, redScaleLabel,
		[=](float scale) { 
		redScale = scale/10.0; 
		redScaleLabel->setText(QString::number(redScale));
		updateColorChannel(redText, 0, redScale);
	});


	QLabel* greenScaleLabel = controllerWidget->findChild<QLabel*>("greenScaleText");
	QSlider* greenScaleSlider = controllerWidget->findChild<QSlider*>("greenScaleSlider");
	greenScaleLabel->setText(QString::number(darwinData.diatomScale)); //set default
	greenScaleSlider->setValue(darwinData.diatomScale*10.0); //set default
	QObject::connect(greenScaleSlider, &QSlider::valueChanged, greenScaleLabel,
		[=](float scale) {
		greenScale = scale / 10.0;
		greenScaleLabel->setText(QString::number(greenScale));
		updateColorChannel(greenText, 1, greenScale);
	});


	QLabel* blueScaleLabel = controllerWidget->findChild<QLabel*>("blueScaleText");
	QSlider* blueScaleSlider = controllerWidget->findChild<QSlider*>("blueScaleSlider");
	blueScaleLabel->setText(QString::number(darwinData.cocoScale)); //set default
	blueScaleSlider->setValue(darwinData.cocoScale*10.0); //set defalt
	QObject::connect(blueScaleSlider, &QSlider::valueChanged, blueScaleLabel,
		[=](float scale) {
		blueScale = scale / 10.0;
		blueScaleLabel->setText(QString::number(blueScale));
		updateColorChannel(blueText, 2, blueScale);
	});

	
	QComboBox* redChannelBox = controllerWidget->findChild<QComboBox*>("red_channel");
	redChannelBox->setCurrentText("Dinoflagellates"); // setting default
	QObject::connect(redChannelBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [=](const QString &text) {
		redText = text.toStdString(); 
		updateColorChannel(redText, 0, redScale);
	});
	
	QComboBox* greenChannelBox = controllerWidget->findChild<QComboBox*>("green_channel");
	greenChannelBox->setCurrentText("Diatoms"); // setting default
	QObject::connect(greenChannelBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [=](const QString &text) {
		greenText = text.toStdString();
		updateColorChannel(greenText, 1, greenScale);
	});

	QComboBox* blueChannelBox = controllerWidget->findChild<QComboBox*>("blue_channel");
	blueChannelBox->setCurrentText("Coccolithophores"); // setting default
	QObject::connect(blueChannelBox, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [=](const QString &text) {
		blueText = text.toStdString(); 
		updateColorChannel(blueText, 2, blueScale);
	});

	QPushButton* resetViewButton = controllerWidget->findChild<QPushButton*>("resetColorsButton");
	QObject::connect(resetViewButton, &QPushButton::clicked, this,
		[=]() { 

		switchToolTo(nullptr);
		Removable::RemoveAll();

		redChannelBox->setCurrentText("Dinoflagellates"); // setting defauredText
		greenChannelBox->setCurrentText("Diatoms"); // setting default
		blueChannelBox->setCurrentText("Coccolithophores"); // setting default

		redScaleSlider->setValue(darwinData.dinoScale*10.0);
		greenScaleSlider->setValue(darwinData.diatomScale*10.0);
		blueScaleSlider->setValue(darwinData.cocoScale*10.0);

		updateColorChannel("Dinoflagellates", 0, darwinData.dinoScale);
		updateColorChannel("Diatoms",         1, darwinData.diatomScale);
		updateColorChannel("Coccolithophores",     2, darwinData.cocoScale);
	    });


}
//----------------------------------------------------------------------------------------------------------
void FlowScene::updateColorChannel(std::string colorText, int colorIndex, float scale)
{
	//cout << "updating colors, color text is: " << colorText << ", color index is: " << colorIndex << endl;

	for (unsigned int fileIndex = 0; fileIndex < _ptSwitch->getNumChildren(); fileIndex++)
	{
		if (colorText == "Dinoflagellates")
		{
			for (int i = 0; i < ptColors[fileIndex]->size(); i++)
			{
				(ptColors[fileIndex]->at(i))[colorIndex] = allDino[fileIndex][i] / scale;
			}
		}
		else if (colorText == "Diatoms")
		{
			for (int i = 0; i < ptColors[fileIndex]->size(); i++)
			{
				(ptColors[fileIndex]->at(i))[colorIndex] = allDiatom[fileIndex][i] / scale;
			}

		}
		else if (colorText == "Coccolithophores")
		{
			for (int i = 0; i < ptColors[fileIndex]->size(); i++)
			{
				(ptColors[fileIndex]->at(i))[colorIndex] = allCoco[fileIndex][i] / scale;
			}
		}
		else if (colorText == "Prokaryotes")
		{
			for (int i = 0; i < ptColors[fileIndex]->size(); i++)
			{
				(ptColors[fileIndex]->at(i))[colorIndex] = allProk[fileIndex][i] / scale;
			}

		}
		ptColors[fileIndex]->dirty();
	}
}
//----------------------------------------------------------------------------------------------------------
void FlowScene::updateAlpha(float alpha)
{
	cout << "updating alpha: " << endl;

	alphaScale = alpha;

	cout << "alpha Scale is:" << alphaScale;

	for (unsigned int fileIndex = 0; fileIndex < _ptSwitch->getNumChildren(); fileIndex++)
	{

		for (int i = 0; i < ptColors[fileIndex]->size(); i++)
		{
			(ptColors[fileIndex]->at(i))[3] = alphaScale;
		}


		ptColors[fileIndex]->dirty();
	}
}