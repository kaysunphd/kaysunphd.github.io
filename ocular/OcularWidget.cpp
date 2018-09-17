#include "OcularWidget.h"

#include <GL/glu.h>
#include <iomanip>
#include <array>
#include <map>
#include <chrono>
#include <thread>
#include <cstdio>

#include <boost/algorithm/string.hpp>
#include <Utils/CSVParser.h>

#include <osg/NodeVisitor>
#include <osg/NodeCallback>
#include <osgUtil/SmoothingVisitor>
#include <osgManipulator/Dragger>
#include <osgManipulator/Projector>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>
#include <osg/TriangleFunctor>
#include <osg/TriangleIndexFunctor>
#include <osg/BlendFunc>
#include <osg/ColorMask>
#include <osg/Material>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <Math/Constants.h>
#include <Math/MathUtils.h>
#include <Utils/FileDB.h>

#include "GFW/IconLoader.h"
#include <GFW/GFWApplication.h>
#include <GFW/GFWWorkbench.h>
#include <GFW/OsgViewWindowManager.h>
#include <GFW/OsgSceneGraphManager.h>
#include <GFW/MiscWidgets.h>
#include <GFW/SearchInTreeViewWidget.h>

#include "QOCoBi/QOCoBiFileManager.h"

#include <Utils/tokenizer.h>
#include <OsgVisualization/OsgHelper.h>
#include <OsgVisualization/OsgUtilFuns.h>
#include <OsgVisualization/ColorMaps.h>

#include "../Properties/BasicProperties.h"
#include "../Properties/MathProperties.h"

#include "GFW/OsgNodeProperty.h"
#include "GFW/PropertyEnabledTreeWidget.h"
#include "GFW/LogWidget.h"

#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <fstream>
#include <iostream>

// Initialize the pointer
OcularWidget* OcularWidget::_instance = NULL;

OcularWidget* OcularWidget::instance()
{
	if (_instance == NULL)
		_instance = new OcularWidget();
	return _instance;
}

OcularWidget::OcularWidget(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags)
{
	_instance = this;

	QTabWidget	*_tabWidget;

	this->setObjectName(QString::fromUtf8("Solver"));
	this->setMinimumSize(QSize(750, 100));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setSizePolicy(sizePolicy);

	QVBoxLayout *verticalLayout = new QVBoxLayout;
	this->setLayout(verticalLayout);

	this->setWindowTitle(QApplication::translate("MainWindow", " Ocular Drug Delivery", 0));

	//set up the table widget
	_tabWidget = new QTabWidget(this);
	_tabWidget->setObjectName(QString::fromUtf8("TabWidget"));

	_mainTab = new QWidget();
	_mainTab->setObjectName(QString::fromUtf8("PCA"));
	_tabWidget->addTab(_mainTab, "Solver");
	
	QVBoxLayout* vlay = new QVBoxLayout();
	_mainTab->setLayout(vlay);

	_shareTab = new QWidget();
	_shareTab->setObjectName(QString::fromUtf8("Share"));
	_tabWidget->addTab(_shareTab, "Share");

	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
	//sizePolicy1.setHorizontalStretch(0);
	//sizePolicy1.setVerticalStretch(0);
	_flowTab = new QWidget();
	_flowTab->setObjectName(QString::fromUtf8("Flow"));
	_tabWidget->addTab(_flowTab, "Flow");

	_heatTab = new QWidget();
	_heatTab->setObjectName(QString::fromUtf8("Heat"));
	_tabWidget->addTab(_heatTab, "Heat");

	_speciesTab = new QWidget();
	_speciesTab->setObjectName(QString::fromUtf8("Species"));
	_tabWidget->addTab(_speciesTab, "Species");

	verticalLayout->addWidget(_tabWidget);

	QHBoxLayout *horizontalLayout = new QHBoxLayout;

	//set up open buttom	
	_openButton = new QToolButton(this);
	_openButton->setObjectName(QString::fromUtf8("openToolButton"));
	_openButton->setSizePolicy(sizePolicy1);
	_openButton->setFixedSize(68, 68); // retina -k
	_openButton->setIconSize(QSize(68, 68));
	//_openButton->setMinimumSize(QSize(0, 48));
	_openButton->setLayoutDirection(Qt::LeftToRight);
	QIcon icon(createIconSet(QLatin1String("Open_Folder-64.png")));//, QSize(), QIcon::Normal, QIcon::Off);
	_openButton->setIcon(icon);
	_openButton->setToolTip(tr("open file"));
	horizontalLayout->addWidget(_openButton);
	//_openButton->hide();
	
	//set up open buttom
	_saveButton = new QToolButton(this);
	_saveButton->setObjectName(QString::fromUtf8("saveToolButton"));
	_saveButton->setSizePolicy(sizePolicy1);
	_saveButton->setFixedSize(68, 68); // retina -k
	_saveButton->setIconSize(QSize(68, 68));
	//_saveButton->setMinimumSize(QSize(0, 48));
	_saveButton->setLayoutDirection(Qt::LeftToRight);
	QIcon iconsave(createIconSet(QLatin1String("Save-64.png")));//, QSize(), QIcon::Normal, QIcon::Off);
	_saveButton->setIcon(iconsave);
	_saveButton->setToolTip(tr("save file"));
	horizontalLayout->addWidget(_saveButton);
	//_saveButton->hide();

	//set up open buttom
	
	_runButton = new QToolButton(this);
	_runButton->setObjectName(QString::fromUtf8("run"));
	_runButton->setSizePolicy(sizePolicy1);
	_runButton->setFixedSize(68, 68); // retina -k
	_runButton->setIconSize(QSize(68, 68));
	//_runButton->setMinimumSize(QSize(0, 48));
	_runButton->setLayoutDirection(Qt::LeftToRight);
	_runButton->setToolTip(tr("run simulation"));
	QIcon iconMF(createIconSet(QLatin1String("Calculator-96.png")));//, QSize(), QIcon::Normal, QIcon::Off);
	_runButton->setIcon(iconMF);
	horizontalLayout->addWidget(_runButton);
	
	//set up open buttom
	_clearButton = new QToolButton(this);
	_clearButton->setObjectName(QString::fromUtf8("cancel"));
	_clearButton->setSizePolicy(sizePolicy1);
	_clearButton->setFixedSize(68, 68); // retina -k
	_clearButton->setIconSize(QSize(68, 68));
	//_clearButton->setMinimumSize(QSize(0, 48));
	_clearButton->setLayoutDirection(Qt::LeftToRight);
	QIcon iconClear(createIconSet(QLatin1String("Clear_Symbol-96.png")));//, QSize(), QIcon::Normal, QIcon::Off);
	_clearButton->setIcon(iconClear);
	_clearButton->setToolTip(tr("clear"));
	horizontalLayout->addWidget(_clearButton);

	QSpacerItem* _horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(_horizontalSpacer);

	//set up open buttom
	_backwardButton = new QToolButton(this);
	_backwardButton->setObjectName(QString::fromUtf8("backward"));
	_backwardButton->setSizePolicy(sizePolicy1);
	_backwardButton->setFixedSize(68, 68); // retina -k
	_backwardButton->setIconSize(QSize(68, 68));
	//_backwardButton->setMinimumSize(QSize(0, 48));
	_backwardButton->setLayoutDirection(Qt::LeftToRight);
	QIcon iconBack(createIconSet(QLatin1String("Back-100.png")));//, QSize(), QIcon::Normal, QIcon::Off);
	_backwardButton->setIcon(iconBack);
	_backwardButton->setToolTip(tr("backward"));
	horizontalLayout->addWidget(_backwardButton);

	//set up open buttom
	_forwardButton = new QToolButton(this);
	_forwardButton->setObjectName(QString::fromUtf8("forward"));
	_forwardButton->setSizePolicy(sizePolicy1);
	_forwardButton->setFixedSize(68, 68); // retina -k
	_forwardButton->setIconSize(QSize(68, 68));
	//_clearButton->setMinimumSize(QSize(0, 48));
	_forwardButton->setLayoutDirection(Qt::LeftToRight);
	QIcon iconForward(createIconSet(QLatin1String("Forward-100.png")));//, QSize(), QIcon::Normal, QIcon::Off);
	_forwardButton->setIcon(iconForward);
	_forwardButton->setToolTip(tr("forward"));
	horizontalLayout->addWidget(_forwardButton);

	_resultView = 0;

	verticalLayout->addLayout(horizontalLayout);

	this->setWindowTitle(QApplication::translate("MainWindow", " Ocular", 0));
	_tabWidget->setCurrentIndex(_tabWidget->indexOf(_mainTab));

	_osgGroup = new osg::Group;
	OsgViewWindowManager* vm = dynamic_cast<OsgViewWindowManager*>(GFW_APP->workbench()->viewWindowManager());
	if (vm) {
		OsgViewWidget* osgw = vm->getActiveOsgViewWidget();
		if (osgw) {
			osgw->getSceneGraphManager()->addSceneData(_osgGroup);
			vm->actionLightingTwoSided()->toggle();
			vm->actionFront()->toggle();
			vm->onViewsAction(vm->actionFront());
			osgw->setLightingTwoSided(false);
			osgw->setView(QOSGWidget::VIEW_MAX);
		}
	}
	
	connect(_openButton, SIGNAL(clicked()), this, SLOT(onOpenSimFile()));
	connect(_saveButton, SIGNAL(clicked()), this, SLOT(onSaveSimFile()));
	connect(_runButton, SIGNAL(clicked()), this, SLOT(onRun()));
	connect(_clearButton, SIGNAL(clicked()), this, SLOT(onClear()));

	connect(_backwardButton, SIGNAL(clicked()), this, SLOT(onStepBackward()));
	connect(_forwardButton, SIGNAL(clicked()), this, SLOT(onStepForward()));

	onCreateSimulationInfo();

	_currentDirectory = "./";
	_currentDirectory = mf_utils::FileDB::instance()->resolveRelativePath(_currentDirectory);
	
	_colorTables.push_back(mf_mbd::LookupColorTables::getOrCreateLookupColorTable("default"));
	_colorTables.push_back(mf_mbd::LookupColorTables::getOrCreateLookupColorTable("pressure")); 
	_colorTables.push_back(mf_mbd::LookupColorTables::getOrCreateLookupColorTable("species"));

	_colorTables[PRESSURE]->setTableRange(0.0, 1.0);
	_colorTables[SPECIES]->setTableRange(0.0, 5000);
}

OcularWidget::~OcularWidget()
{
}

void OcularWidget::closeEvent(QCloseEvent *event)
{
	QWidget::closeEvent(event);
	emit widgetClosed(false);
}

void OcularWidget::currentView()
{
	for (size_t i = 0; i < _osgGroup->getNumChildren(); i++) {
		osg::ref_ptr<osg::Node> node = _osgGroup->getChild(i);
		mf_mbd::OsgUtilFuns::setVisible(*node, false);
	}
	osg::ref_ptr<osg::Node> node = _osgGroup->getChild(_resultView);
	mf_mbd::OsgUtilFuns::setVisible(*node, true);
}

void OcularWidget::onStepBackward()
{
	if (_resultView - 1 < 0) return;
	_resultView--;
	currentView();
	updateVTKLegend();
}

void OcularWidget::onStepForward()
{
	if (_resultView + 1 > _osgGroup->getNumChildren() - 1) return;
	_resultView++;
	currentView();
	updateVTKLegend();
}

void OcularWidget::onOpenSimFile()
{
	static QString prevPath;
	QStringList filters;
	filters.append(QString("Supported Files (*.sim *.vtk)"));
	filters.append(QString("Sim (*.sim)"));
	filters.append(QString("VTK (*.vtk)"));

	QStringList fileNames = QFileDialog::getOpenFileNames(GFW_APP->mainWindow(), QString("Select File(s) to Open "), prevPath, filters.join("\n"));
	if (fileNames.isEmpty()) return; //do nothing if user pressed cancel

	std::string fileName = fileNames.front().toStdString();
	if (fileName.empty()) return;

	std::string path, file, post, file_no_post;
	mf_utils::FilePathFinder::splitFilename(fileName, path, file, post, file_no_post);
	std::transform(post.begin(), post.end(), post.begin(), ::tolower);

	if (post == "sim") {
		onOpenSim(fileName);
	}
	else if (post == "vtk") {
		_osgGroup->removeChildren(0, _osgGroup->getNumChildren());
		_resultsMinMax.clear();

		for (size_t i = 0; i < fileNames.size(); i++) {
			fileName = fileNames[i].toStdString();
			mf_utils::FilePathFinder::splitFilename(fileName, path, file, post, file_no_post);

			std::pair<double, double> range;
			mf_mbd::VoxImageReader *voxImageReader = new mf_mbd::VoxImageReader;
			osg::ref_ptr<osg::Group> vtk = voxImageReader->readVTK(fileName, range);
			vtk->setName(file);

			if (vtk) {
				_osgGroup->addChild(vtk);
				NTF_NOTICE << fileName << " loaded";
			}
			else {
				NTF_NOTICE << fileName << " loading mesh failed";
			}

			_resultsMinMax.push_back(range);
		}
		for (size_t i = 1; i < _osgGroup->getNumChildren(); i++) { // initial display of just first one
			osg::ref_ptr<osg::Node> node = _osgGroup->getChild(i);
			mf_mbd::OsgUtilFuns::setVisible(*node, false);
		}
		_resultView = 0;

		VTKvisualizeSetting();
		updateVTKLegend();
	}
}

void OcularWidget::updateVTKLegend()
{
	const std::vector<mf_mbd::LookupColorTable::Ref>& tables = mf_mbd::LookupColorTables::getLookupColorTables();
	mf_mbd::LookupColorTable* ltb = NULL;
	for (size_t i = 0; i < tables.size(); ++i) {
		std::string name = tables[i]->getName();
		if ("species" == name) {
			ltb = tables[i].get();
			ltb->notifyObservers();
			break;
		}
	}

	OsgViewWindowManager* vm = dynamic_cast<OsgViewWindowManager*>(GFW_APP->workbench()->viewWindowManager());
	OsgViewWidget* osgw = vm->getActiveOsgViewWidget();

	float low = (float)_resultsMinMax[_resultView].first;
	float high = (float)_resultsMinMax[_resultView].second;

	int lowprecs = Math::find_precision(low);
	int highprecs = Math::find_precision(high);

	float fmin = std::stof(Math::to_string_with_precision(low, lowprecs));
	float fmax = std::stof(Math::to_string_with_precision(high, highprecs));

	ltb->setTableRange(fmin, fmax);
	ltb->buildTableFromHSV();

	mf_mbd::Legend* lgd = osgw->getLegend();
	if (lgd) {
		lgd->setNumMajorTick(lgd->getNumMajorTick());
		lgd->setLookupColorTable(*ltb);
	}
}

osg::ref_ptr<osg::Geometry> OcularWidget::extractGeometry(osg::ref_ptr<osg::Node> fnode)
{
	if (!fnode) return 0;

	mf_mbd::GeometryFinder finder;
	fnode->accept(finder);
	std::vector<osg::ref_ptr<osg::Geometry> > geos = finder.getAllNodes();
	if (geos.empty()) {
		NTF_WARN << "there is no geometry from osg node";
		return 0;
	}
	else if (geos.size() > 1) {
		NTF_WARN << "there is more than geometry from loaded osg node, the first one is picked";
		return geos[0];
	}
	else {
		return geos[0];
	}
}

void OcularWidget::VTKvisualizeSetting()
{
	OsgViewWindowManager* vm = dynamic_cast<OsgViewWindowManager*>(GFW_APP->workbench()->viewWindowManager());
	if (vm) {
		OsgViewWidget* osgw = vm->getActiveOsgViewWidget();
		if (osgw) {
			osgw->setUseLighting(false);
			if (vm->actionLightingOn()->isChecked()) vm->actionLightingOn()->toggle();
			osgw->enableDrawLines(true);
			if (!vm->actionWireframe()->isChecked()) vm->actionWireframe()->toggle();
			osgw->setView(QOSGWidget::VIEW_MAX);
			if (!vm->actionViewMax()->isChecked()) vm->actionViewMax()->toggle();
		}
	}
}

void OcularWidget::onOpenSim(const std::string& fname)
{
	std::string pfname = fname;

	if (fname.rfind('/') != std::string::npos || fname.rfind('\\') != std::string::npos) { //has path
		_openFilePath = fname.substr(0, fname.rfind('/') + 1);
		if (_openFilePath.size() == 0) {
			_openFilePath = fname.substr(0, fname.rfind('\\') + 1);
		}

		mf_utils::FileDB::instance()->setInpWorkDir(_openFilePath);
		NTF_NOTICE << "Set input file working directory: " << _openFilePath;
		bool succ = mf_utils::FileDB::instance()->getFilePathFinder()->addPath(_openFilePath);
		if (succ) {
			NTF_NOTICE << "opening " << fname;
		}

		pfname = fname.substr(_openFilePath.size(), fname.size());
		_openFileName = pfname;
	}

	std::string ext = osgDB::getFileExtension(fname);

	if (ext == "sim") {
		readSIM(fname);		
	}
	else {
		throw std::runtime_error("can not recognize file format");
	}
}


void OcularWidget::onSaveSimFile()
{
	_simFileName = "";
	static QString prevPath;
	QStringList filters;
	filters.append(QString("Supported Files (*.sim)"));

	QFileDialog dialog(GFW_APP->mainWindow());
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setWindowModality(Qt::WindowModal);
	dialog.setViewMode(QFileDialog::Detail);
	dialog.setNameFilter(QString(filters.join("\n")));

	QStringList fileNames;
	if (dialog.exec()) fileNames = dialog.selectedFiles();
	if (fileNames.isEmpty()) return;

	QString fileName = fileNames.front();
	std::string fname = fileName.toUtf8().data();
	std::replace(fname.begin(), fname.end(), char('/'), char('\\')); // windows have to use \

	std::string scenario = _scenarioBox->currentText().toStdString();
	if (scenario == "In-vitro Cornea Rabbit" || scenario == "In-vitro Cornea Human") {
		onWriteInVitroCorneaSimFile(fname);
	}
	else if (scenario == "2D Whole Eye Rabbit" || scenario == "2D Whole Eye Human") {
		onWrite2DEyeSimFile(fname);
	}
	
	copyDTF(fname, scenario);
	_simFileName = fname;

	NTF_NOTICE << "saved " << fname;
}

void OcularWidget::clearSpecies()
{
	_diffVitreousEdit = 0;
	_diffAqueousEdit = 0;
	_diffCiliaryEdit = 0;
	_diffRetinaEdit = 0;
	_diffIrisEdit = 0;
	_diffChoroidEdit = 0;
	_diffTrabecularEdit = 0;
	_diffCorneaEdit = 0;
	_diffScleraEdit = 0;

	_postionXEdit;
	_postionYEdit = 0;
	_radiusEdit = 0;
	_bolusEdit = 0;
}

void OcularWidget::onClear()
{
	QVBoxLayout* vlay = dynamic_cast<QVBoxLayout*>(_mainTab->layout());
	delete vlay;
	foreach(QObject * w, _mainTab->children()) {
		//delete w;
		w->deleteLater();
	}

	_osgGroup->removeChildren(0, _osgGroup->getNumChildren());

	_speciescheck = 0;
	_bilay = 0;

	_sweepsflow1Edit = 0;
	_sweepsflow2Edit = 0;
	_sweepsheat1Edit = 0;
	_sweepsspecies1Edit = 0;

	_leftVelocityEdit = 0;
	_rightVelocityEdit = 0;

	_trabecularEdit = 0;
	_permRetinaEdit = 0;
	_perfusionEdit = 0;

	clearSpecies();

	_iterEdit = 0;
	_tolerEdit = 0;
	_timestepEdit = 0;
	_stepsizeEdit = 0;
	_outputfreqEdit = 0;

	_gravityXEdit = 0;
	_gravityYEdit = 0;
	_gravityZEdit = 0;

	_sk1Edit = 0;
	_bk1Edit = 0;
	_sk3Edit = 0;
	_bk3Edit = 0;

	_diffTearEdit = 0;
	_diffEpiEdit = 0;
	_diffEpiParaEdit = 0;
	_diffStromaEdit = 0;
	_diffEndoEdit = 0;
	_diffEndoParaEdit = 0;
	_diffAHEdit = 0;

	_permTearEpiEdit = 0;
	_permEpiStrEdit = 0;
	_permStrEndoEdit = 0;
	_permEndoAHEdit = 0;

	_partitionTearEdit = 0;
	_partitionEpiEdit = 0;
	_partitionStrEdit = 0;
	_partitionEndoEdit = 0;
	_partitionAHEdit = 0;

	_ahEdit = 0;
	_tearEdit = 0;
	_ahPressureEdit = 0;
	_tearPressureEdit = 0;

	_BmaxEdit = 0;
	_KdEdit = 0;
	_powerEdit = 0;

	_scenarioBox = 0;
	_transBox = 0;

	onCreateSimulationInfo();

	NTF_NOTICE << "";
	NTF_NOTICE << "";
	NTF_NOTICE << "";
	NTF_NOTICE << "";
	NTF_NOTICE << "cleared ";
}

void OcularWidget::copyDTF(const std::string& fname, const std::string& scenario)
{
	std::string path, file, post, file_no_post;
	mf_utils::FilePathFinder::splitFilename(fname, path, file, post, file_no_post);

	std::string inmodelfile;
	std::string outmodelfile;
	if (scenario == "In-vitro Cornea Rabbit") {
		inmodelfile = _currentDirectory + "models/In-vitro-cornea-Rabbit/In-vitro-cornea-Rabbit.WFG";
		outmodelfile = path + "/" + file_no_post + ".WFG";
	}
	else if (scenario == "In-vitro Cornea Human") {
		inmodelfile = _currentDirectory + "models/In-vitro-cornea-Human/In-vitro-cornea-Human.WFG";
		outmodelfile = path + "/" + file_no_post + ".WFG";
	}
	else if (scenario == "2D Whole Eye Rabbit") {
		inmodelfile = _currentDirectory + "models/2D-whole-eye-Rabbit/2D-whole-eye-Rabbit.DTF";
		outmodelfile = path + "/" + file_no_post + ".DTF";
	}
	else if (scenario == "2D Whole Eye Human") {
		inmodelfile = _currentDirectory + "models/2D-whole-eye-Human/2D-whole-eye-Human.DTF";
		outmodelfile = path + "/" + file_no_post + ".DTF";
	}

	std::ifstream infile(inmodelfile.c_str(), std::ios::in | std::ios::binary);
	std::ofstream outfile(outmodelfile.c_str(), std::ios::out | std::ios::binary);
	outfile << infile.rdbuf();
	infile.close();
	outfile.close();

	NTF_NOTICE << "created " << outmodelfile;
}

void OcularWidget::readSIM(const std::string& fname)
{

	std::string scenario = _scenarioBox->currentText().toStdString();
	if (scenario == "In-vitro Cornea Rabbit" || scenario == "In-vitro Cornea Human") {
		readInVitroCornea(fname);
	}
	else if (scenario == "2D Whole Eye Rabbit" || scenario == "2D Whole Eye Human") {
		read2DEye(fname);
	}
}

void OcularWidget::read2DEye(const std::string& fname)
{
	std::ifstream sim(fname.c_str());
	if (!sim.is_open()) {
		NTF_WARN << "can not open file " << fname;
		return;
	}

	bool flow = false;
	bool heat = false;
	bool species = false;

	while (sim) {

		std::string line;
		std::getline(sim, line);

		std::string buf;
		std::stringstream ss(line);
		std::vector<std::string> tokens;
		while (ss >> buf) tokens.push_back(buf); // to remove accidental whitespace
		if (tokens.size() == 0) continue;

		mf_utils::Tokenizer toks(line);

		//std::cout << toks.size() << " : " << line << std::endl;
		int ct = 0;

		if (mf_utils::StrConv::toType<std::string>(toks[ct]) == "#") {
			continue;
		}

		while (ct < toks.size()) {
			std::string key = toks[ct];
			ct++;
			if (key == "transient") {
				_timestepEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				_stepsizeEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				_outputfreqEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
			}
			else if (key == "iteration") {
				_iterEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
			}
			else if (key == "gravity") {
				std::string tkey = toks[ct++]; //const_gravity
				if (tkey == "const_gravity") {
					_gravityXEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					_gravityYEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					_gravityZEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "x0") {
				std::string tkey = toks[ct++]; //
				if (tkey == "=") {
					_postionXEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "y0") {
				std::string tkey = toks[ct++]; //
				if (tkey == "=") {
					_postionYEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "r") {
				std::string tkey = toks[ct++]; //
				if (tkey == "=") {
					_radiusEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "S0^") {
				std::string tkey = toks[ct++]; // =
				tkey = toks[ct++]; // dis
				tkey = toks[ct++]; // <=
				tkey = toks[ct++]; // r
				tkey = toks[ct++]; // ?
				if (tkey == "?") {
					_bolusEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "module") {
				std::string tkey = toks[ct++]; //flow
				if (tkey == "flow") {
					flow = true;
				}
				else if (tkey == "heat") {
					heat = true;
				}
				else if (tkey == "species") {
					species = true;
				}
			}
			else if (key == "sweeps" && flow) {
				_sweepsflow1Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				_sweepsflow2Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				flow = false;
			}
			else if (key == "sweeps" && heat) {
				_sweepsheat1Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				heat = false;
			}
			else if (key == "sweeps" && species) {
				_sweepsspecies1Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				species = false;
			}
			else if (key == "bc") {
				std::string tkey = toks[ct++]; //CiliaryInletCiliaryBoundsLeft
				if (tkey == "CiliaryInletCiliaryBoundsLeft") {
					tkey = toks[ct++]; //fix_inter_norvelocity
					if (tkey == "fix_inter_norvelocity") {
						tkey = toks[ct++];
						_leftVelocityEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
				}
				else if (tkey == "CiliaryInletCiliaryBoundsRight") {
					tkey = toks[ct++]; //fix_inter_norvelocity
					if (tkey == "fix_inter_norvelocity") {
						tkey = toks[ct++];
						_rightVelocityEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
				}
				else if (tkey == "RetinaVitreousBounds") {
					tkey = toks[ct++]; //fix_flux
					if (tkey == "fix_flux") {
						tkey = toks[ct++]; //0
						tkey = toks[ct++]; //permeability_coeff
						if (tkey == "permeability_coeff") {
							_permRetinaEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
					}
				}
			}
			else if (key == "vc") {
				std::string tkey = toks[ct++]; //CiliaryInletCiliaryBoundsLeft
				if (tkey == "Vitreous") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffVitreousEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Aqueous") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffAqueousEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Retina") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffRetinaEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Iris") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffIrisEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Ciliary") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffCiliaryEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Choroid") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffChoroidEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
					else if (ttkey == "blood_purfusion") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_perfusionEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Trabecular") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffTrabecularEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
					else if (ttkey == "const_porosity") {
						ttkey = toks[ct++];
						_trabecularEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
				}
				else if (tkey == "Cornea") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffCorneaEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "Sclera") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffScleraEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
			}
		}
	}

	sim.close();

	NTF_NOTICE << "read " << fname;
}

void OcularWidget::readInVitroCornea(const std::string& fname)
{
	std::ifstream sim(fname.c_str());
	if (!sim.is_open()) {
		NTF_WARN << "can not open file " << fname;
		return;
	}

	while (sim) {

		std::string line;
		std::getline(sim, line);

		std::string buf;
		std::stringstream ss(line);
		std::vector<std::string> tokens;
		while (ss >> buf) tokens.push_back(buf); // to remove accidental whitespace
		if (tokens.size() == 0 ) continue;

		mf_utils::Tokenizer toks(line);

		int ct = 0;

		if (mf_utils::StrConv::toType<std::string>(toks[ct]) == "#") {
			continue;
		}

		while (ct < toks.size()) {
			std::string key = toks[ct];
			ct++;
			if (key == "transient") {
				_timestepEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				_stepsizeEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				_outputfreqEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
			}
			else if (key == "iteration") {
				_iterEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
			}
			else if (key == "sk1") {
				std::string tkey = toks[ct++];
				if (tkey == "=") {
					_sk1Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "bk1") {
				std::string tkey = toks[ct++];
				if (tkey == "=") {
					_bk1Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "sk3") {
				std::string tkey = toks[ct++];
				if (tkey == "=") {
					_sk3Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "bk3") {
				std::string tkey = toks[ct++];
				if (tkey == "=") {
					_bk3Edit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
				}
			}
			else if (key == "bc") {
				std::string tkey = toks[ct++];
				if (tkey == "ah_back") {
					std::string ttkey = toks[ct++];
					if (ttkey == "fix_pressure") {
						_ahPressureEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
					else if (ttkey == "fix_value") {
						_ahEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
				}
				else if (tkey == "tear_front") {
					std::string ttkey = toks[ct++];
					if (ttkey == "fix_pressure") {
						_tearPressureEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
					else if (ttkey == "fix_value") {
						_tearEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
					}
				}
				else if (tkey == "epi_trans_left") {
					std::string ttkey = toks[ct++]; //fix_wiremembrane
					if (ttkey == "fix_wiremembrane") {
						ttkey = toks[ct++]; //flux
						ttkey = toks[ct++]; //Ps
						if (ttkey == "Ps") {
							_permTearEpiEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
						ttkey = toks[ct++]; //K1
						ttkey = toks[ct++]; //1
						ttkey = toks[ct++]; //volume
						ttkey = toks[ct++]; //tear
						ttkey = toks[ct++]; //K2
						if (ttkey == "K2") {
							_partitionEpiEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
					}
				}
				else if (tkey == "epi_trans_right") {
					std::string ttkey = toks[ct++]; //fix_wiremembrane
					if (ttkey == "fix_wiremembrane") {
						ttkey = toks[ct++]; //flux
						ttkey = toks[ct++]; //Ps
						if (ttkey == "Ps") {
							_permEpiStrEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
						ttkey = toks[ct++]; //K1
						ttkey = toks[ct++]; //1
						ttkey = toks[ct++]; //volume
						ttkey = toks[ct++]; //tear
						ttkey = toks[ct++]; //K2
						if (ttkey == "K2") {
							_partitionStrEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
					}
				}
				else if (tkey == "endo_trans_left") {
					std::string ttkey = toks[ct++]; //fix_wiremembrane
					if (ttkey == "fix_wiremembrane") {
						ttkey = toks[ct++]; //flux
						ttkey = toks[ct++]; //Ps
						if (ttkey == "Ps") {
							_permStrEndoEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
						ttkey = toks[ct++]; //K1
						ttkey = toks[ct++]; //1
						ttkey = toks[ct++]; //volume
						ttkey = toks[ct++]; //tear
						ttkey = toks[ct++]; //K2
						if (ttkey == "K2") {
							_partitionEndoEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
					}
				}
				else if (tkey == "endo_trans_right") {
					std::string ttkey = toks[ct++]; //fix_wiremembrane
					if (ttkey == "fix_wiremembrane") {
						ttkey = toks[ct++]; //flux
						ttkey = toks[ct++]; //Ps
						if (ttkey == "Ps") {
							_permEndoAHEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
						ttkey = toks[ct++]; //K1
						ttkey = toks[ct++]; //1
						ttkey = toks[ct++]; //volume
						ttkey = toks[ct++]; //tear
						ttkey = toks[ct++]; //K2
						if (ttkey == "K2") {
							_partitionAHEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
						}
					}
				}
			}
			else if (key == "vc") {
				std::string tkey = toks[ct++];
				if (tkey == "tear") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffTearEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ct++;
						}
					}
				}
				else if (tkey == "epi") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffEpiEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct])));
							ct++;
						}
					}
				}
				else if (tkey == "epi_para") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffEpiParaEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct])));
							ct++;
						}
					}
				}
				else if (tkey == "stroma") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffStromaEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct])));
							ct++;
						}
					}
					else if (ttkey == "bound_species") {
						ttkey = toks[ct++]; //C_tot
						ttkey = toks[ct++]; //Bmax
						if (ttkey == "Bmax") {
							_BmaxEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
							ttkey = toks[ct++];
							if (ttkey == "Kd") {
								_KdEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
								ttkey = toks[ct++];
								if (ttkey == "power_n") {
									_powerEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct++])));
								}
							}
						}	
					}
				}
				else if (tkey == "endo") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffEndoEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct])));
							ct++;
						}
					}
				}
				else if (tkey == "endo_para") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffEndoParaEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct])));
							ct++;
						}
					}
				}
				else if (tkey == "ah") {
					std::string ttkey = toks[ct++];
					if (ttkey == "const_diff") {
						if (mf_utils::StrConv::toType<int>(toks[ct]) > 0) {
							_diffAHEdit->setText(QString::fromStdString(mf_utils::StrConv::toType<std::string>(toks[ct])));
							ct++;
						}
					}
				}
			}
		}
	}

	sim.close();

	NTF_NOTICE << "read " << fname;

}

void OcularWidget::onCreateSimulationInfo()
{
	QVBoxLayout* vlay = dynamic_cast<QVBoxLayout*>(_mainTab->layout());
	if (!vlay) {
		vlay = new QVBoxLayout;
		_mainTab->setLayout(vlay);
	}

	vlay->setAlignment(Qt::AlignTop);

	QWidget* groupBox = new QWidget();
	vlay->addWidget(groupBox);

	QVBoxLayout *groupBoxLayout = new QVBoxLayout;
	groupBoxLayout->setAlignment(Qt::AlignTop);
	groupBox->setLayout(groupBoxLayout);

	QWidget*   bi = new QPushButton(tr("Simulation Scenario"));
	groupBoxLayout->addWidget(bi);

	QScrollArea* simarea = new QScrollArea();
	groupBoxLayout->addWidget(simarea);
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
	simarea->setSizePolicy(sizePolicy1);

	QVBoxLayout* bilay = new QVBoxLayout();
	QWidget* simulationInfo = new QWidget();
	simarea->setWidget(simulationInfo);
	simulationInfo->setLayout(bilay);

	simarea->setFrameShape(QFrame::Panel);
	simarea->setFrameShadow(QFrame::Raised);
	simarea->setWidgetResizable(true);
	simarea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	simarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	simarea->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* genLay = new QHBoxLayout();
	bilay->addLayout(genLay);
	
	QLabel *genLab = new QLabel("Ocular Model Selection :");
	genLay->addWidget(genLab);
	_scenarioBox = new QComboBox();
	_scenarioBox->addItem("Select One");
	_scenarioBox->addItem("2D Whole Eye Human");
	_scenarioBox->addItem("2D Whole Eye Rabbit");
	_scenarioBox->addItem("In-vitro Cornea Human");
	_scenarioBox->addItem("In-vitro Cornea Rabbit");

	genLay->addWidget(_scenarioBox);

	QLabel *iterations = new QLabel("	Iterations :  ");
	genLay->addWidget(iterations);
	_iterEdit = new QLineEdit();
	genLay->addWidget(_iterEdit);

	QHBoxLayout* transientLay = new QHBoxLayout();
	bilay->addLayout(transientLay);

	QLabel *steps = new QLabel("Time Steps :  ");
	transientLay->addWidget(steps);
	_timestepEdit = new QLineEdit();
	transientLay->addWidget(_timestepEdit);

	QLabel *size = new QLabel("     Step Size :  ");
	transientLay->addWidget(size);
	_stepsizeEdit = new QLineEdit();
	transientLay->addWidget(_stepsizeEdit);

	QLabel *output = new QLabel("     Output frequency :  ");
	transientLay->addWidget(output);
	_outputfreqEdit = new QLineEdit();
	transientLay->addWidget(_outputfreqEdit);

	connect(_scenarioBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeScenario()));

}

void OcularWidget::onCreate2DEye()
{
	QVBoxLayout* vlay = dynamic_cast<QVBoxLayout*>(_mainTab->layout());
	if (!vlay) {
		vlay = new QVBoxLayout;
		_mainTab->setLayout(vlay);
	}

	int vlaycount = vlay->count();
	if (vlaycount == 2) {
		delete vlay->itemAt(1)->widget();
	}
	vlaycount = vlay->count();

	vlay->setAlignment(Qt::AlignTop);

	QWidget* groupBox = new QWidget();
	vlay->addWidget(groupBox);

	QVBoxLayout *groupBoxLayout = new QVBoxLayout;
	groupBoxLayout->setAlignment(Qt::AlignTop);
	groupBox->setLayout(groupBoxLayout);

	QWidget*   bi = new QPushButton(_scenarioBox->currentText());
	groupBoxLayout->addWidget(bi);

	QScrollArea* eyearea = new QScrollArea();
	groupBoxLayout->addWidget(eyearea);
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
	eyearea->setSizePolicy(sizePolicy1);

	_bilay = new QVBoxLayout();
	QWidget* simulationInfo = new QWidget();
	eyearea->setWidget(simulationInfo);
	simulationInfo->setLayout(_bilay);

	eyearea->setFrameShape(QFrame::Panel);
	eyearea->setFrameShadow(QFrame::Raised);
	eyearea->setWidgetResizable(true);
	eyearea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	eyearea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	eyearea->setContentsMargins(0, 0, 0, 0);
	eyearea->setStyleSheet("background-color: white");

	QGroupBox* gravityGroupBox = new QGroupBox(QString::fromWCharArray(L"Gravity [m/s\x00B2]"));
	QHBoxLayout* gravityLay = new QHBoxLayout();
	gravityGroupBox->setLayout(gravityLay);
	_bilay->addWidget(gravityGroupBox);

	QLabel *gravityx = new QLabel("Gravity in X :  ");
	gravityLay->addWidget(gravityx);
	_gravityXEdit = new QLineEdit();
	gravityLay->addWidget(_gravityXEdit);

	QLabel *gravityy = new QLabel("     Gravity in Y :  ");
	gravityLay->addWidget(gravityy);
	_gravityYEdit = new QLineEdit();
	gravityLay->addWidget(_gravityYEdit);

	QLabel *gravityz = new QLabel("     Gravity in Z :  ");
	gravityLay->addWidget(gravityz);
	_gravityZEdit = new QLineEdit();
	gravityLay->addWidget(_gravityZEdit);

	QGroupBox* flowmoduleGroupBox = new QGroupBox(QString::fromWCharArray(L"Flow Module"));
	QHBoxLayout* flowmoduleLay = new QHBoxLayout();
	flowmoduleGroupBox->setLayout(flowmoduleLay);
	_bilay->addWidget(flowmoduleGroupBox);

	QLabel *sweeps1 = new QLabel(" Number of Sweeps for Flow :  ");
	flowmoduleLay->addWidget(sweeps1);
	_sweepsflow1Edit = new QLineEdit();
	flowmoduleLay->addWidget(_sweepsflow1Edit);

	QLabel *sweeps2 = new QLabel("     Number of Sweeps for Pressure :  ");
	flowmoduleLay->addWidget(sweeps2);
	_sweepsflow2Edit = new QLineEdit();
	flowmoduleLay->addWidget(_sweepsflow2Edit);

	QGroupBox* ahvelocityGroupBox = new QGroupBox(QString::fromWCharArray(L"Aqueous Humor Secretion Velocity [m/s]"));
	QHBoxLayout* ahvelocityLay = new QHBoxLayout();
	ahvelocityGroupBox->setLayout(ahvelocityLay);
	_bilay->addWidget(ahvelocityGroupBox);

	QLabel *leftVelocity = new QLabel(" Left Ciliary Body Inlet :  ");
	ahvelocityLay->addWidget(leftVelocity);
	_leftVelocityEdit = new QLineEdit();
	ahvelocityLay->addWidget(_leftVelocityEdit);

	QLabel *rightVelocity = new QLabel(" Right Ciliary Body Inlet :  ");
	ahvelocityLay->addWidget(rightVelocity);
	_rightVelocityEdit = new QLineEdit();
	ahvelocityLay->addWidget(_rightVelocityEdit);

	QGroupBox* trabecularGroupBox = new QGroupBox(QString::fromWCharArray(L"Hydraulic Permeability of Trabecular Meshwork [m\x00B2]"));
	QHBoxLayout* trabecularLay = new QHBoxLayout();
	trabecularGroupBox->setLayout(trabecularLay);
	_bilay->addWidget(trabecularGroupBox);

	QLabel *trabecular = new QLabel(" Hydraulic Permeability :  ");
	trabecularLay->addWidget(trabecular);
	_trabecularEdit = new QLineEdit();
	trabecularLay->addWidget(_trabecularEdit);

	QGroupBox* heatmoduleGroupBox = new QGroupBox(QString::fromWCharArray(L"Heat Module"));
	QHBoxLayout* heatmoduleLay = new QHBoxLayout();
	heatmoduleGroupBox->setLayout(heatmoduleLay);
	_bilay->addWidget(heatmoduleGroupBox);

	QLabel *sweeps3 = new QLabel(" Sweeps :  ");
	heatmoduleLay->addWidget(sweeps3);
	_sweepsheat1Edit = new QLineEdit();
	heatmoduleLay->addWidget(_sweepsheat1Edit);

	onCreateSpeciesModule(true);
}

void OcularWidget::onCreateSpeciesModule(bool flag)
{
	if (flag) {
		QGroupBox* speciesmoduleGroupBox = new QGroupBox(QString::fromWCharArray(L"Species Module"));
		QHBoxLayout* speciesLay = new QHBoxLayout();
		speciesmoduleGroupBox->setLayout(speciesLay);
		_bilay->addWidget(speciesmoduleGroupBox);

		QLabel *sweeps5 = new QLabel(" Sweeps to Species Module :  ");
		speciesLay->addWidget(sweeps5);
		_sweepsspecies1Edit = new QLineEdit();
		speciesLay->addWidget(_sweepsspecies1Edit);

		QGroupBox* diffusivityGroupBox = new QGroupBox(QString::fromWCharArray(L"Drug Diffusivity [m\x00B2/s]"));
		QGridLayout* diffusivityLay = new QGridLayout();
		diffusivityGroupBox->setLayout(diffusivityLay);
		_bilay->addWidget(diffusivityGroupBox);

		QLabel *diffCorneaLabel = new QLabel("Cornea :  ");
		diffusivityLay->addWidget(diffCorneaLabel, 0, 0);
		_diffCorneaEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffCorneaEdit, 0, 1);

		QLabel *diffTrabecularLabel = new QLabel("Trabecular :  ");
		diffusivityLay->addWidget(diffTrabecularLabel, 0, 2);
		_diffTrabecularEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffTrabecularEdit, 0, 3);

		QLabel *diffAqueousLabel = new QLabel("Aqueous :  ");
		diffusivityLay->addWidget(diffAqueousLabel, 0, 4);
		_diffAqueousEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffAqueousEdit, 0, 5);

		QLabel *diffCiliaryLabel = new QLabel("Ciliary Body :  ");
		diffusivityLay->addWidget(diffCiliaryLabel, 1, 0);
		_diffCiliaryEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffCiliaryEdit, 1, 1);

		QLabel *diffIrisLabel = new QLabel("Iris :  ");
		diffusivityLay->addWidget(diffIrisLabel, 1, 2);
		_diffIrisEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffIrisEdit, 1, 3);

		QLabel *diffVitreousLabel = new QLabel("Vitreous :  ");
		diffusivityLay->addWidget(diffVitreousLabel, 1, 4);
		_diffVitreousEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffVitreousEdit, 1, 5);

		QLabel *diffRetinaLabel = new QLabel("Retina :  ");
		diffusivityLay->addWidget(diffRetinaLabel, 2, 0);
		_diffRetinaEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffRetinaEdit, 2, 1);

		QLabel *diffChoroidLabel = new QLabel("Choroid :  ");
		diffusivityLay->addWidget(diffChoroidLabel, 2, 2);
		_diffChoroidEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffChoroidEdit, 2, 3);

		QLabel *diffScleraLabel = new QLabel("Sclera :  ");
		diffusivityLay->addWidget(diffScleraLabel, 2, 4);
		_diffScleraEdit = new QLineEdit();
		diffusivityLay->addWidget(_diffScleraEdit, 2, 5);

		QGroupBox* posteriorGroupBox = new QGroupBox(QString::fromWCharArray(L"Drug Lost to Posterior Eye"));
		QHBoxLayout* posteriorLay = new QHBoxLayout();
		posteriorGroupBox->setLayout(posteriorLay);
		_bilay->addWidget(posteriorGroupBox);

		QLabel *permRetina = new QLabel(" Permeability at retina [m/s] :  ");
		posteriorLay->addWidget(permRetina);
		_permRetinaEdit = new QLineEdit();
		posteriorLay->addWidget(_permRetinaEdit);

		QLabel *perfusion = new QLabel(QString::fromWCharArray(L"     Rate of blood perfusion at choroid [m\x00B3/s] :  "));
		posteriorLay->addWidget(perfusion);
		_perfusionEdit = new QLineEdit();
		posteriorLay->addWidget(_perfusionEdit);

		QGroupBox* injectGroupBox = new QGroupBox(QString::fromWCharArray(L"Bolus Intravitreal Injection"));
		QGridLayout* injectLay = new QGridLayout();
		injectGroupBox->setLayout(injectLay);
		_bilay->addWidget(injectGroupBox);

		QLabel *postionX = new QLabel("Bolus Center Along X-Axis [m] :  ");
		injectLay->addWidget(postionX, 0, 0);
		_postionXEdit = new QLineEdit();
		injectLay->addWidget(_postionXEdit, 0, 1);

		QLabel *postionY = new QLabel("Bolus Center Along Y-Axis [m] :  ");
		injectLay->addWidget(postionY, 0, 2);
		_postionYEdit = new QLineEdit();
		injectLay->addWidget(_postionYEdit, 0, 3);

		QLabel *bolusRadius = new QLabel("Bolus Radius [m] :  ");
		injectLay->addWidget(bolusRadius, 1, 0);
		_radiusEdit = new QLineEdit();
		injectLay->addWidget(_radiusEdit, 1, 1);

		QLabel *bolusConc = new QLabel(QString::fromWCharArray(L"Bolus Concentration [mol/m\x00B3] :  "));
		injectLay->addWidget(bolusConc, 1, 2);
		_bolusEdit = new QLineEdit();
		injectLay->addWidget(_bolusEdit, 1, 3);
	}
	else {
		int vlaycount = _bilay->count();
		for (size_t ii = vlaycount; ii > 5; ii--) {
			delete _bilay->itemAt(ii-1)->widget();
		}
		clearSpecies();
	}
}

void OcularWidget::onCreateInVitroCornea()
{
	QVBoxLayout* vlay = dynamic_cast<QVBoxLayout*>(_mainTab->layout());
	if (!vlay) {
		vlay = new QVBoxLayout;
		_mainTab->setLayout(vlay);
	}

	int vlaycount = vlay->count();
	if (vlaycount == 2) {
		delete vlay->itemAt(1)->widget();
	}
	vlaycount = vlay->count();

	vlay->setAlignment(Qt::AlignTop);

	QWidget* groupBox = new QWidget();
	vlay->addWidget(groupBox);

	QVBoxLayout *groupBoxLayout = new QVBoxLayout;
	groupBoxLayout->setAlignment(Qt::AlignTop);
	groupBox->setLayout(groupBoxLayout);

	QWidget*   bi = new QPushButton(_scenarioBox->currentText());
	groupBoxLayout->addWidget(bi);

	QScrollArea* corneaarea = new QScrollArea();
	groupBoxLayout->addWidget(corneaarea);

	QVBoxLayout* bilay = new QVBoxLayout();
	QWidget* simulationInfo = new QWidget();
	corneaarea->setWidget(simulationInfo);
	simulationInfo->setLayout(bilay);

	corneaarea->setFrameShape(QFrame::Panel);
	corneaarea->setFrameShadow(QFrame::Raised);
	corneaarea->setWidgetResizable(true);
	corneaarea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	corneaarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	corneaarea->setContentsMargins(0, 0, 0, 0);
	corneaarea->setStyleSheet("background-color: white");

	QGroupBox* diffusivityGroupBox = new QGroupBox(QString::fromWCharArray(L"Drug Diffusivity [m\x00B2/s]"));
	QGridLayout* diffusivityLay = new QGridLayout();
	diffusivityGroupBox->setLayout(diffusivityLay);
	bilay->addWidget(diffusivityGroupBox);

	QLabel *diffTearLabel = new QLabel("Tear :  ");
	diffusivityLay->addWidget(diffTearLabel,0,0);
	_diffTearEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffTearEdit,0,1);

	QLabel *diffEpiLabel = new QLabel("Epithelium :  ");
	diffusivityLay->addWidget(diffEpiLabel,0,2);
	_diffEpiEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffEpiEdit,0,3);

	QLabel *diffEpiParaLabel = new QLabel("Epithelium Paracellular :  ");
	diffusivityLay->addWidget(diffEpiParaLabel,0,4);
	_diffEpiParaEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffEpiParaEdit,0,5);

	QLabel *diffStromaLabel = new QLabel("Stroma :  ");
	diffusivityLay->addWidget(diffStromaLabel, 1, 0);
	_diffStromaEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffStromaEdit,1,1);

	QLabel *diffEndoLabel = new QLabel("Endothelium :  ");
	diffusivityLay->addWidget(diffEndoLabel, 1, 2);
	_diffEndoEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffEndoEdit,1, 3);

	QLabel *diffEndoParaLabel = new QLabel("Endothelium Paracellular :  ");
	diffusivityLay->addWidget(diffEndoParaLabel, 1, 4);
	_diffEndoParaEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffEndoParaEdit,1, 5);

	QLabel *diffAHLabel = new QLabel("Aqueous Humor :  ");
	diffusivityLay->addWidget(diffAHLabel, 2, 0);
	_diffAHEdit = new QLineEdit();
	diffusivityLay->addWidget(_diffAHEdit, 2, 1);

	QGroupBox* permeabilityGroupBox = new QGroupBox(QString::fromWCharArray(L"Drug Permeability [m/s]"));
	QGridLayout* permeabilityLay = new QGridLayout();
	permeabilityGroupBox->setLayout(permeabilityLay);
	bilay->addWidget(permeabilityGroupBox);

	QLabel *permTearEpiLabel = new QLabel("Tear-Epithelium :  ");
	permeabilityLay->addWidget(permTearEpiLabel, 0, 0);
	_permTearEpiEdit = new QLineEdit();
	permeabilityLay->addWidget(_permTearEpiEdit, 0, 1);

	QLabel *permEpiStrLabel = new QLabel("Epithelium-Stroma :  ");
	permeabilityLay->addWidget(permEpiStrLabel, 0, 2);
	_permEpiStrEdit = new QLineEdit();
	permeabilityLay->addWidget(_permEpiStrEdit, 0, 3);

	QLabel *permStrEndoParaLabel = new QLabel("Stroma-Endothelium :  ");
	permeabilityLay->addWidget(permStrEndoParaLabel, 1, 0);
	_permStrEndoEdit = new QLineEdit();
	permeabilityLay->addWidget(_permStrEndoEdit, 1, 1);

	QLabel *permEndoAHLabel = new QLabel("Endothelium-AH :  ");
	permeabilityLay->addWidget(permEndoAHLabel, 1, 2);
	_permEndoAHEdit = new QLineEdit();
	permeabilityLay->addWidget(_permEndoAHEdit, 1, 3);

	QGroupBox* partitionGroupBox = new QGroupBox(QString::fromWCharArray(L"Drug Partition Coefficient"));
	QGridLayout* partitionLay = new QGridLayout();
	partitionGroupBox->setLayout(partitionLay);
	bilay->addWidget(partitionGroupBox);

	QLabel *partitionTearLabel = new QLabel("Tear :  ");
	partitionLay->addWidget(partitionTearLabel, 0, 0);
	_partitionTearEdit = new QLineEdit();
	partitionLay->addWidget(_partitionTearEdit, 0, 1);

	QLabel *partitionEpiLabel = new QLabel("Epithelium :  ");
	partitionLay->addWidget(partitionEpiLabel, 0, 2);
	_partitionEpiEdit = new QLineEdit();
	partitionLay->addWidget(_partitionEpiEdit, 0, 3);

	QLabel *partitionStrLabel = new QLabel("Stroma :  ");
	partitionLay->addWidget(partitionStrLabel, 0, 4);
	_partitionStrEdit = new QLineEdit();
	partitionLay->addWidget(_partitionStrEdit, 0, 5);

	QLabel *partitionEndoLabel = new QLabel("Endothelium :  ");
	partitionLay->addWidget(partitionEndoLabel, 1, 0);
	_partitionEndoEdit = new QLineEdit();
	partitionLay->addWidget(_partitionEndoEdit, 1, 1);

	QLabel *partitionAHLabel = new QLabel("Aqueous Humor :  ");
	partitionLay->addWidget(partitionAHLabel, 1, 2);
	_partitionAHEdit = new QLineEdit();
	partitionLay->addWidget(_partitionAHEdit, 1, 3);

	QGroupBox* pressureGroupBox = new QGroupBox(tr("Pressure Condition [Pa]"));
	QHBoxLayout* pressureLay = new QHBoxLayout();
	pressureGroupBox->setLayout(pressureLay);
	bilay->addWidget(pressureGroupBox);

	QLabel *ahPressureLabel = new QLabel("Aqueous Humor :  ");
	pressureLay->addWidget(ahPressureLabel);
	_ahPressureEdit = new QLineEdit();
	pressureLay->addWidget(_ahPressureEdit);

	QLabel *tearPressureLabel = new QLabel("Tear :  ");
	pressureLay->addWidget(tearPressureLabel);
	_tearPressureEdit = new QLineEdit();
	pressureLay->addWidget(_tearPressureEdit);

	QGroupBox* loadingGroupBox = new QGroupBox(QString::fromWCharArray(L"Loading Condition [mol/m\x00B3]")); 
	QHBoxLayout* loadingLay = new QHBoxLayout();
	loadingGroupBox->setLayout(loadingLay);
	bilay->addWidget(loadingGroupBox);

	QLabel *ahLabel = new QLabel("Aqueous Humor :  ");
	loadingLay->addWidget(ahLabel);
	_ahEdit = new QLineEdit();
	loadingLay->addWidget(_ahEdit);

	QLabel *tearLabel = new QLabel("Tear :  ");
	loadingLay->addWidget(tearLabel);
	_tearEdit = new QLineEdit();
	loadingLay->addWidget(_tearEdit);

	QGroupBox* epiLipidGroupBox = new QGroupBox(tr("Corneal Epithelium Lipid"));
	QHBoxLayout* epilipidLay = new QHBoxLayout();
	epiLipidGroupBox->setLayout(epilipidLay);
	bilay->addWidget(epiLipidGroupBox);

	QLabel *sk1Label = new QLabel(QString::fromWCharArray(L"kᴮ [s\x207B\x00B9] :  "));
	epilipidLay->addWidget(sk1Label);
	_sk1Edit = new QLineEdit();
	epilipidLay->addWidget(_sk1Edit);

	QLabel *bk1Label = new QLabel(QString::fromWCharArray(L"Rᴮ :  "));
	epilipidLay->addWidget(bk1Label);
	_bk1Edit = new QLineEdit();
	epilipidLay->addWidget(_bk1Edit);

	QGroupBox* endoLipidGroupBox = new QGroupBox(tr("Corneal Endothelium Lipid"));
	QHBoxLayout* endolipidLay = new QHBoxLayout();
	endoLipidGroupBox->setLayout(endolipidLay);
	bilay->addWidget(endoLipidGroupBox);

	QLabel *sk3Label = new QLabel(QString::fromWCharArray(L"kᴮ [s\x207B\x00B9] :  "));
	endolipidLay->addWidget(sk3Label);
	_sk3Edit = new QLineEdit();
	endolipidLay->addWidget(_sk3Edit);

	QLabel *bk3Label = new QLabel(QString::fromWCharArray(L"Rᴮ :  "));
	endolipidLay->addWidget(bk3Label);
	_bk3Edit = new QLineEdit();
	endolipidLay->addWidget(_bk3Edit);

	QGroupBox* proteinGroupBox = new QGroupBox(tr("Protein Binding in Stroma"));
	QHBoxLayout* proteinLay = new QHBoxLayout();
	proteinGroupBox->setLayout(proteinLay);
	bilay->addWidget(proteinGroupBox);

	QLabel *BmaxLabel = new QLabel("Bmax :  ");
	proteinLay->addWidget(BmaxLabel);
	_BmaxEdit = new QLineEdit();
	proteinLay->addWidget(_BmaxEdit);

	QLabel *KdLabel = new QLabel("Kd :  ");
	proteinLay->addWidget(KdLabel);
	_KdEdit = new QLineEdit();
	proteinLay->addWidget(_KdEdit);

	QLabel *powerLabel = new QLabel("Power :  ");
	proteinLay->addWidget(powerLabel);
	_powerEdit = new QLineEdit();
	proteinLay->addWidget(_powerEdit);

}

void OcularWidget::onChangeScenario()
{
	_timestepEdit->setText("");
	_stepsizeEdit->setText("");
	_outputfreqEdit->setText("");

	std::string scenario = _scenarioBox->currentText().toStdString();
	if (scenario == "In-vitro Cornea Rabbit" || scenario == "In-vitro Cornea Human") {
		onCreateInVitroCornea();
	}
	else if (scenario == "2D Whole Eye Rabbit" || scenario == "2D Whole Eye Human") {
		onCreate2DEye();
	}
	loadVTKMesh(scenario);

	NTF_NOTICE << "selected scenario " << scenario;
}

void OcularWidget::loadVTKMesh(std::string scenario)
{
	std::string vtkMeshFile;
	if (scenario == "In-vitro Cornea Rabbit") {
		vtkMeshFile = _currentDirectory + "models/In-vitro-cornea-Rabbit/In-vitro-cornea-Rabbit.vtk";
	}
	else if (scenario == "In-vitro Cornea Human") {
		vtkMeshFile = _currentDirectory + "models/In-vitro-cornea-Human/In-vitro-cornea-Human.vtk";
	}
	else if (scenario == "2D Whole Eye Rabbit") {
		vtkMeshFile = _currentDirectory + "models/2D-whole-eye-Rabbit/2D-whole-eye-Rabbit.vtk";
	}
	else if (scenario == "2D Whole Eye Human") {
		vtkMeshFile = _currentDirectory + "models/2D-whole-eye-Human/2D-whole-eye-Human.vtk";
	}

	std::pair<double, double> range;
	mf_mbd::VoxImageReader *voxImageReader = new mf_mbd::VoxImageReader;
	osg::ref_ptr<osg::Group> vtk = voxImageReader->readVTK(vtkMeshFile, range);
	_osgGroup->removeChildren(0, _osgGroup->getNumChildren());

	if (vtk) {
		_osgGroup->addChild(vtk);
		NTF_NOTICE << scenario << " mesh loaded";
	}
	else {
		NTF_NOTICE << scenario << " loading mesh failed";
	}

	VTKvisualizeSetting();
}

void OcularWidget::removeOldrsl()
{
	std::string path, file, post, file_no_post;
	mf_utils::FilePathFinder::splitFilename(_simFileName, path, file, post, file_no_post);
	std::string rslfname = path + "\\" + file_no_post + ".rsl";
	std::ifstream oldrsl(rslfname.c_str());
	if (oldrsl.good()) {
		oldrsl.close();
		int res = std::remove(rslfname.c_str());
	}
}

void OcularWidget::onProgressbar()
{
	std::string path, file, post, file_no_post;
	mf_utils::FilePathFinder::splitFilename(_simFileName, path, file, post, file_no_post);
	std::string rslfname = path + "\\" + file_no_post + ".rsl";

	ProgressBarDelegate progbar;
	progbar.setText("Running Simulation");
	progbar.setAllowProcessEvents(true);

	std::string analysistype;
	int totaltimestep = 0;
	int iterations = 0;
	iterations = std::stoi(_iterEdit->text().toStdString());
	if (!_timestepEdit->text().toStdString().empty()) {
		totaltimestep = std::stoi(_timestepEdit->text().toStdString());
		progbar.setRange(0, totaltimestep);
		analysistype = "transient";
	}
	else {
		analysistype = "steady-state";
		progbar.setRange(0, iterations);
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(50000)); // to give cobi time to write rsl

	std::ifstream rsl(rslfname.c_str());
	std::ios::streampos gpos = rsl.tellg();
	std::string line;
	bool done = false;

	std::cout << "total timestep: " << totaltimestep << ", iteration: " << iterations << " "<< rsl.good() <<std::endl;
	

	while (!done)
	{
		if (!std::getline(rsl, line) || rsl.eof())
		{
			rsl.clear();
			rsl.seekg(gpos);

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		gpos = rsl.tellg();

		std::string buf;
		std::stringstream ss(line);

		std::vector<std::string> tokens;
		while (ss >> buf) tokens.push_back(buf);

		bool istimestep = Math::isInteger(tokens.front());	

		int timestep;
		int iter;
		if (istimestep) {		
			if (analysistype == "transient") {
				timestep = std::stoi(tokens.front());
				iter = std::stoi(tokens[1]);
				std::cout << analysistype << ", timestep: "<< timestep << ", iteration: "<< iter << std::endl;
				progbar.setValue(timestep);
			}
			else if (analysistype == "steady-state") {
				iter = std::stoi(tokens[1]);
				std::cout << analysistype << ", iteration: " << iter << std::endl;
				progbar.setValue(iter);
			}	
		}
		if (analysistype == "transient" && timestep == totaltimestep && iter == iterations) {
			done = true;
		}
		else if (analysistype == "steady-state" && iter == iterations) {
			done = true;
		}
	}

	rsl.close();
	progbar.setText("Simulation");
}

void OcularWidget::runCobi()
{
	std::string call = _currentDirectory + "cobi\\cobi.exe " + _simFileName;
	std::cout << call << std::endl;
	system(call.c_str());
}

void OcularWidget::onRun()
{
	onSaveSimFile();

	if (_simFileName.empty() ) return;
	removeOldrsl();

	NTF_NOTICE << "Running simulation ... ";

	cobiThread = new OcularWidgetThread(this);
	std::string call = _currentDirectory + "cobi\\cobi.exe " + _simFileName;
	std::cout << call << std::endl;
	cobiThread->setSimFile(call);
	cobiThread->start();

	onProgressbar();

	NTF_NOTICE << "Simulation Complete";
	std::cout << "Simulation Complete" << std::endl;
}

void OcularWidget::loadResults()
{
	std::string path, file, post, file_no_post;
	mf_utils::FilePathFinder::splitFilename(_simFileName, path, file, post, file_no_post);

	std::string analysistype;
	int totaltimestep = 0;
	int iterations = 0;
	iterations = std::stoi(_iterEdit->text().toStdString());
	if (!_timestepEdit->text().toStdString().empty()) {
		totaltimestep = std::stoi(_timestepEdit->text().toStdString());
		analysistype = "transient";
	}
	else {
		analysistype = "steady-state";
	}
	
	for (size_t i = 0; i < totaltimestep; i++) {

		std::string vtkfname = path + "\\" + file_no_post + "." + _outputfreqEdit->text().toStdString() + ".vtk";
	}

}

void OcularWidget::onWrite2DEyeSimFile(const std::string& fname)
{
	std::ofstream out(fname, std::ios::out);

	int timesteps = 0;
	if (!_timestepEdit->text().toStdString().empty()) {
		timesteps = std::stoi(_timestepEdit->text().toStdString());
	}
	if (timesteps > 0) {
		out << "transient " << _timestepEdit->text().toStdString() << " " << _stepsizeEdit->text().toStdString() << " " << _outputfreqEdit->text().toStdString() << "\n";	
	}
	out << "iteration " << _iterEdit->text().toStdString() << "\n";
	out << "lengthunit mm " << "\n";
	out << "tolerance 1.0E-12 " << "\n";
	out << "gravity const_gravity " << _gravityXEdit->text().toStdString() <<" "<< _gravityYEdit->text().toStdString() <<" "<< _gravityZEdit->text().toStdString()<< "\n";
	out << "DTF" << "\n";
	out << "VTK" << "\n";
	out << "\n";

	out << "reaction C" << "\n";
	out << "  x0 = " << _postionXEdit->text().toStdString() << "\n";
	out << "  y0 = " << _postionYEdit->text().toStdString() << "\n";
	out << "  r = " << _radiusEdit->text().toStdString() << "\n";
	out << "  x  = coordinate_x" << "\n";
	out << "  y  = coordinate_y" << "\n";
	out << "  dis = sqrt((x - x0)*(x - x0) + (y - y0)*(y - y0))" << "\n";
	out << "  S0^ = dis <= r ? " << _bolusEdit->text().toStdString() << " : 0 #concentration" << "\n";
	out << "\n";

	out << "reaction O" << "\n";
	out << "  vAmean^ = vA_mean" << "\n";
	out << "  vVmean^ = vV_mean" << "\n";
	out << "  vCmean^ = vC_mean" << "\n";
	out << "compartment O reaction O" << "\n";
	out << "  vA_mean = volume Aqueous average_velocity" << "\n";
	out << "  vV_mean = volume Vitreous average_velocity" << "\n";
	out << "  vC_mean = volume Cornea average_velocity" << "\n";

	out << "\n";

	out << "module share" << "\n";
	out << "  vc Aqueous            temperature_based_dens 996 307 0.000337" << "\n";
	out << "  vc AqueousPetit       temperature_based_dens 996 307 0.000337" << "\n";
	out << "  vc CiliaryInlet       temperature_based_dens 996 307 0.000337" << "\n";
	out << "  vc Trabecular         temperature_based_dens 996 307 0.000337" << "\n";
	out << "  vc Vitreous           temperature_based_dens 1100 307 0.000337" << "\n";
	out << "  vc Retina             temperature_based_dens 1100 307 0.000337" << "\n";
	out << "  vc Cornea             temperature_based_dens 1050 307 0.000337" << "\n";
	out << "  vc Iris               temperature_based_dens 1050 307 0.000337" << "\n";
	out << "  vc Ciliary            temperature_based_dens 1050 307 0.000337" << "\n";
	out << "  vc ScleraFront        temperature_based_dens 1050 307 0.000337" << "\n";
	out << "  vc Sclera             temperature_based_dens 1050 307 0.000337" << "\n"; 
	out << "  vc Choroid            temperature_based_dens 1050 307 0.000337" << "\n"; 
	out << "\n";
	out << "  vc Lens               const_dens 1000" << "\n";
	out << "  vc Void               const_dens 1050" << "\n";
	out << "  vc VoidChoroid        const_dens 1050" << "\n";
	out << "\n";
	out << "  vc Aqueous            material_type fluid" << "\n";
	out << "  vc AqueousPetit       material_type fluid" << "\n";
	out << "  vc CiliaryInlet       material_type fluid" << "\n";
	out << "  vc Trabecular         material_type fluid" << "\n";
	out << "  vc Vitreous           material_type fluid" << "\n";
	out << "  vc Retina             material_type fluid" << "\n";
	out << "  vc Cornea             material_type fluid" << "\n";
	out << "  vc Iris               material_type fluid" << "\n";
	out << "  vc Ciliary            material_type fluid" << "\n";
	out << "  vc ScleraFront        material_type fluid" << "\n";
	out << "  vc Sclera             material_type fluid" << "\n";
	out << "  vc Choroid            material_type fluid" << "\n";
	out << "  vc VoidChoroid        material_type solid" << "\n";
	out << "  vc Lens				material_type solid" << "\n";
	out << "  vc Void				material_type solid" << "\n";
	out << "\n";
	out << "  vc Vitreous      reaction C ^S0" << "\n";
	out << "\n";

	out << "module flow" << "\n";
	out << "  sweeps "<< _sweepsflow1Edit->text().toStdString() <<" "<< _sweepsflow2Edit->text().toStdString() << "\n";
	out << "  linearsolver cgs amg forcedsolve" << "\n";
	out << "  solvertolerance 1e-12 1e-12" << "\n";
	out << "  relaxation 0.1 0.1 0.1 0.5" << "\n";
	out << "  diagrelaxation 0.3 0.3 0.3" << "\n";
	out << "\n";

	out << "  output_restart_data" << "\n";
	out << "  bouss_effect" << "\n";
	out << "\n";

	out << "  vc Vitreous     const_visc  7.4e-4" << "\n";
	out << "  vc Aqueous      const_visc  7.4e-4" << "\n";
	out << "  vc AqueousPetit const_visc  7.4e-4" << "\n";
	out << "  vc CiliaryInlet const_visc  7.4e-4" << "\n";
	out << "  vc Trabecular   const_visc  7.4e-4" << "\n";
	out << "  vc Retina       const_visc  7.4e-4" << "\n";
	out << "  vc Cornea       const_visc  7.4e-4" << "\n";
	out << "  vc Iris		  const_visc  7.4e-4" << "\n";
	out << "  vc Ciliary	  const_visc  7.4e-4" << "\n";
	out << "  vc ScleraFront  const_visc  7.4e-4" << "\n";
	out << "  vc Sclera       const_visc  7.4e-4" << "\n";
	out << "  vc Choroid      const_visc  7.4e-4" << "\n";
	out << "\n";

	out << "  vc Retina       const_porosity 1.0 1.75e-18 0 0 0 0" << "\n";
	out << "  vc Iris         const_porosity 1.0 2.0e-18 0 0 0 0" << "\n";
	out << "  vc Cornea       const_porosity 1.0 2.0e-18 0 0 0 0" << "\n";
	out << "  vc Ciliary      const_porosity 1.0 2.0e-18 0 0 0 0" << "\n";
	out << "  vc ScleraFront  const_porosity 1.0 2.0e-18 0 0 0 0" << "\n";
	out << "  vc Sclera       const_porosity 1.0 2.0e-18 0 0 0 0" << "\n";
	out << "  vc Choroid      const_porosity 1.0 1.75e-18 0 0 0 0" << "\n";
	out << "  vc Vitreous     const_porosity 1.0 1.0e-16 0 0 0 0" << "\n";
	out << "  vc Trabecular   const_porosity 1.0 "<< _trabecularEdit->text().toStdString() <<" 0 0 0 0" << "\n";
	out << "\n";

	out << "  bc AqueousLensBounds            fix_velocity    0 0 0   interpolate_pressure" << "\n";
	out << "  bc VitreousLensBounds           fix_velocity    0 0 0   interpolate_pressure" << "\n";
	out << "  bc VoidCorneaBounds             fix_velocity    0 0 0   #interpolate_pressure" << "\n";
	out << "  bc VoidScleraBounds             fix_velocity    0 0 0   interpolate_pressure" << "\n";
	out << "  bc VoidScleraBounds1            fix_velocity    0 0 0   #interpolate_pressure" << "\n";
	out << "  bc VoidBounds                   fix_velocity    0 0 0   #interpolate_pressure" << "\n";
	out << "  bc VoidScleraBounds             fix_velocity    0 0 0   #interpolate_pressure" << "\n";
	out << "  bc VoidChoroidBounds            fix_velocity    0 0 0   interpolate_pressure " << "\n";
	out << "\n";

	out << "  bc CiliaryIrisBounds            porousinterface" << "\n";
	out << "  bc AqueousCiliaryBounds         porousinterface" << "\n";
	out << "  bc CiliaryAqueousPetitBounds    porousinterface" << "\n";
	out << "  bc HyaloidAqueousBounds         porousinterface" << "\n";
	out << "  bc CiliaryInletCiliaryBounds    porousinterface" << "\n";
	out << "  bc HyaloidVitreousBounds        porousinterface" << "\n";
	out << "  bc RetinaAqueousPetitBounds     porousinterface" << "\n";
	out << "  bc RetinaVitreousBounds         porousinterface" << "\n";
	out << "  bc AqueousCorneaBounds          porousinterface" << "\n";
	out << "  bc TrabecularCorneaBounds       porousinterface" << "\n";
	out << "  bc TrabecularAqueousBounds      porousinterface" << "\n";
	out << "  bc TrabecularScleraBounds       porousinterface" << "\n";
	out << "  bc AqueousIrisBounds            porousinterface" << "\n";
	out << "  bc CiliaryInletAqueousBounds    porousinterface" << "\n";
	out << "  bc ScleraCiliaryBounds          porousinterface" << "\n";
	out << "  bc CiliaryChoroidBounds         porousinterface" << "\n";
	out << "  bc ChoroidRetinaBounds          porousinterface" << "\n";
	out << "\n";

	out << "  bc VoidChoroidBounds                 exp_inter_pressure positive" << "\n";
	out << "  bc CiliaryInletCiliaryBoundsLeft     fix_inter_norvelocity positive "<< _leftVelocityEdit->text().toStdString() << "\n";
	out << "  bc CiliaryInletCiliaryBoundsRight    fix_inter_norvelocity positive "<< _rightVelocityEdit->text().toStdString() << "\n";
	out << "  bc TrabecularOutletScleraBoundsLeft  fix_inter_pressure negative 1200" << "\n";
	out << "  bc TrabecularOutletScleraBoundsRight fix_inter_pressure negative 1200" << "\n";
	out << "  bc CorneaWall						   fix_pressure 0" << "\n";
	out << "  bc ScleraWall						   fix_pressure 1200" << "\n";
	out << "\n";

	out << "module heat temp" << "\n";
	out << "  sweeps " << _sweepsflow1Edit->text().toStdString() << "\n";
	out << "  linearsolver cgs" << "\n";
	out << "  solvertolerance 1e-12" << "\n";
	out << "  relaxation 0.0" << "\n";
	out << "  diagrelaxation 0.0" << "\n";
	out << "\n";

	out << "  output_restart_data" << "\n";
	out << "\n";

	out << "  vc Aqueous      const_conduct 0.58" << "\n";
	out << "  vc AqueousPetit const_conduct 0.58" << "\n";
	out << "  vc Choroid      const_conduct 1.0042" << "\n";
	out << "  vc Ciliary      const_conduct 1.0042" << "\n";
	out << "  vc CiliaryInlet const_conduct 0.58" << "\n";
	out << "  vc Cornea       const_conduct 0.58" << "\n";
	out << "  vc Void         const_conduct 0.58" << "\n";
	out << "  vc VoidChoroid  const_conduct 1.0042" << "\n";
	out << "  vc Iris         const_conduct 1.0042" << "\n";
	out << "  vc Lens         const_conduct 0.40" << "\n";
	out << "  vc Retina       const_conduct 1.0042" << "\n";
	out << "  vc Sclera       const_conduct 1.0042" << "\n";
	out << "  vc ScleraFront  const_conduct 1.0042" << "\n";
	out << "  vc Trabecular   const_conduct 0.58" << "\n";
	out << "  vc Vitreous     const_conduct 0.603" << "\n";
	out << "\n";

	out << "  vc Aqueous      const_capacity 3997" << "\n";
	out << "  vc AqueousPetit const_capacity 3997" << "\n";
	out << "  vc Choroid      const_capacity 3180" << "\n";
	out << "  vc Ciliary      const_capacity 3180" << "\n";
	out << "  vc CiliaryInlet const_capacity 3997" << "\n";
	out << "  vc Cornea       const_capacity 4178" << "\n";
	out << "  vc Void         const_capacity 4178" << "\n";
	out << "  vc VoidChoroid  const_capacity 3180" << "\n";
	out << "  vc Iris         const_capacity 3180" << "\n";
	out << "  vc Lens         const_capacity 3000" << "\n";
	out << "  vc Retina       const_capacity 3180" << "\n";
	out << "  vc Sclera       const_capacity 3180" << "\n";
	out << "  vc ScleraFront  const_capacity 3180" << "\n";
	out << "  vc Trabecular   const_capacity 3997" << "\n";
	out << "  vc Vitreous     const_capacity 1100" << "\n";
	out << "\n";

	out << "  bc CorneaWall    extern_flux 10 298" << "\n";
	out << "  bc CorneaWall    extern_rad_flux 0.975 298" << "\n";
	out << "  bc CorneaWall    fix_flux 40" << "\n";
	out << "  bc ScleraWall    extern_flux 65 310" << "\n";
	out << "\n";

	out << "  init Aqueous      fix_value 310" << "\n";
	out << "  init AqueousPetit fix_value 310" << "\n";
	out << "  init Choroid      fix_value 310" << "\n";
	out << "  init Ciliary      fix_value 310" << "\n";
	out << "  init CiliaryInlet fix_value 310" << "\n";
	out << "  init Cornea       fix_value 310" << "\n";
	out << "  init Iris         fix_value 310" << "\n";
	out << "  init Lens         fix_value 310" << "\n";
	out << "  init Retina       fix_value 310" << "\n";
	out << "  init Sclera       fix_value 310" << "\n";
	out << "  init ScleraFront  fix_value 310" << "\n";
	out << "  init Trabecular   fix_value 310" << "\n";
	out << "  init Vitreous     fix_value 310" << "\n";
	out << "  init Void         fix_value 310" << "\n";
	out << "  init VoidChoroid  fix_value 310" << "\n";
	out << "\n";

	int sweeps = std::stoi(_sweepsspecies1Edit->text().toStdString());
	out << "module species S" << "\n";
	out << "  sweeps "<< sweeps << "\n";
	out << "  linearsolver cgs" << "\n";
	out << "  solvertolerance 1e-10" << "\n";
	out << "  relaxation 0.02" << "\n";
	out << "  diagrelaxation 0.01" << "\n";
	out << "\n";

	out << "  output_restart_data" << "\n";
	out << "\n";

	out << "  vc Vitreous 				const_diff " << _diffVitreousEdit->text().toStdString() << "\n";
	out << "  vc Aqueous 				const_diff " << _diffAqueousEdit->text().toStdString() << "\n";
	out << "  vc AqueousPetit 			const_diff " << _diffAqueousEdit->text().toStdString() << "\n";
	out << "  vc CiliaryInlet 			const_diff " << _diffCiliaryEdit->text().toStdString() << "\n";
	out << "  vc Retina 				const_diff " << _diffRetinaEdit->text().toStdString() << "\n";
	out << "  vc Iris 					const_diff " << _diffIrisEdit->text().toStdString() << "\n";
	out << "  vc Ciliary 				const_diff " << _diffCiliaryEdit->text().toStdString() << "\n";
	out << "  vc Choroid 				const_diff " << _diffChoroidEdit->text().toStdString() << "\n";
	out << "  vc Trabecular 			const_diff " << _diffTrabecularEdit->text().toStdString() << "\n";
	out << "  vc Cornea 				const_diff " << _diffCorneaEdit->text().toStdString() << "\n";
	out << "  vc ScleraFront 			const_diff " << _diffScleraEdit->text().toStdString() << "\n";
	out << "  vc Sclera 				const_diff " << _diffScleraEdit->text().toStdString() << "\n";
	out << "\n";

	//if (_speciescheck->isChecked() && sweeps > 0) {
	if ( sweeps > 0) {
			out << "  vc Choroid 				blood_purfusion "<< _perfusionEdit->text().toStdString() <<" 0 " << "\n";
	}
	out << "\n";

	out << "  bc AqueousLensBounds            fix_flux 0" << "\n";
	out << "  bc VitreousLensBounds           fix_flux 0" << "\n";
	out << "  bc VoidCorneaBounds             fix_flux 0" << "\n";
	out << "  bc VoidScleraBounds             fix_flux 0" << "\n";
	out << "  bc VoidScleraBounds1            fix_flux 0" << "\n";
	out << "  bc VoidBounds                   fix_flux 0" << "\n";
	out << "  bc VoidCiliaryBounds            fix_flux 0" << "\n";
	out << "  bc VoidChoroidBounds            fix_flux 0" << "\n";
	out << "  bc TrabecularOutletScleraBoundsLeft  flow_exit 0" << "\n";
	out << "  bc TrabecularOutletScleraBoundsRight flow_exit 0" << "\n";
	out << "  bc RetinaVitreousBounds              flow_exit 0" << "\n";
	out << "\n";

	//if (_speciescheck->isChecked() && sweeps > 0) {
	if (sweeps > 0) {
		out << "  bc RetinaVitreousBounds fix_flux 0 permeability_coeff "<< _permRetinaEdit->text().toStdString() <<" value 0 include_convection" << "\n";
	}
	out << "  init Vitreous     fix_field S0" << "\n";
	out << "\n";

	out.close();

	NTF_NOTICE << "sim file written to " << fname;
}

void OcularWidget::onWriteInVitroCorneaSimFile(const std::string& fname)
{
	std::ofstream out(fname, std::ios::out);

	out <<"transient "<< _timestepEdit->text().toStdString()<<" "<< _stepsizeEdit->text().toStdString()<<" "<< _outputfreqEdit->text().toStdString() << "\n";
	out << "iteration "<< _iterEdit->text().toStdString() << "\n";
	out << "tolerance 1.0E-09 " << "\n";
	out << "tolerancecutoff 1.0E-15 " << "\n";
	out << "nocl" << "\n";
	out << "WFG" << "\n";
	out << "WriteNewDTF" << "\n";
	out << "wire_view_sides 20" << "\n";
	out << "VTK" << "\n";
	out << "\n";

	out << "reaction epi_lipid" << "\n";
	out << "  sk1 = "<<_sk1Edit->text().toStdString() << "\n";
	out << "  bk1 = " << _bk1Edit->text().toStdString() << "\n";
	out << "  rhs1 = sk1 * (C_tot - Cb_lipid / bk1)" << "\n";
	out << "  C_tot'  = -rhs1" << "\n";
	out << "  Cb_lipid'  =  rhs1" << "\n";
	out << "\n";

	out << "reaction endo_lipid" << "\n";
	out << "  sk3 = " << _sk3Edit->text().toStdString() << "\n";
	out << "  bk3 = " << _bk3Edit->text().toStdString() << "\n";
	out << "  rhs3 = sk3 * (C_tot - Cb_lipid / bk3)" << "\n";
	out << "  C_tot'  = -rhs3" << "\n";
	out << "  Cb_lipid'  =  rhs3" << "\n";
	out << "\n";

	out << "module share" << "\n";
	out << "  vc tear				const_dens 1000" << "\n";
	out << "  vc epi				const_dens 1000" << "\n";
	out << "  vc epi_para			const_dens 1000" << "\n";
	out << "  vc stroma				const_dens 1000" << "\n";
	out << "  vc endo				const_dens 1000" << "\n";
	out << "  vc endo_para			const_dens 1000" << "\n";
	out << "  vc ah					const_dens 1000" << "\n";
	out << "  vc epi				reaction epi_lipid" << "\n";
	out << "  vc endo				reaction endo_lipid" << "\n";
	out << "\n";

	out << "module flow" << "\n";
	out << "  sweeps 500 500" << "\n";
	out << "  linearsolver cgs amg forcedsolve" << "\n";
	out << "  solvertolerance 1e-8 1e-8" << "\n";
	out << "  relaxation 0.0 0.0 0.0 0.5" << "\n";
	out << "  diagrelaxation 0.1 0.1 0.1" << "\n";
	out << "\n";

	out << "  vc tear 				const_visc 1.0E-3" << "\n";
	out << "  vc epi 				const_visc 1.0E-3" << "\n";
	out << "  vc epi_para    		const_visc 1.0E-3" << "\n";
	out << "  vc stroma 	    	const_visc 1.0e-3" << "\n";
	out << "  vc endo  				const_visc 1.0e-3" << "\n";
	out << "  vc endo_para    		const_visc 1.0e-3" << "\n";
	out << "  vc ah     	    	const_visc 1.0e-3" << "\n";
	out << "\n";

	out << "  vc stroma					const_porosity	1 4.0E-18 0 0 0 0" << "\n";
	out << "  vc endo                  	const_porosity  1 1.00E-30 0 0 0 0" << "\n";
	out << "  vc epi                	const_porosity  1 1.00E-30 0 0 0 0" << "\n";
	out << "  vc endo_para          	const_porosity  1 5.00E-13  0 0 0 0" << "\n";
	out << "  vc epi_para           	const_porosity  1 1.41E-15  0 0 0 0" << "\n";

	out << "  bc ah_back			fix_pressure 	1961.33" << "\n";
	out << "  bc tear_front			fix_pressure    0" << "\n";

	out << "  bc tear_wall  		wire_sidewall	0 0 0" << "\n";
	out << "  bc epi_wall   		 wire_sidewall	0 0 0" << "\n";
	out << "  bc epi_para_wall       wire_sidewall	0 0 0" << "\n";
	out << "  bc stroma_wall  		 wire_sidewall	0 0 0" << "\n";
	out << "  bc endo_wall    		 wire_sidewall	0 0 0" << "\n";
	out << "  bc endo_para_wall      wire_sidewall	0 0 0" << "\n";
	out << "  bc ah_wall    		 wire_sidewall	0 0 0" << "\n";

	out << "  bc tear2epi      		fix_velocity 	0 0 0" << "\n";
	out << "  bc epi_para_left		fix_velocity 	0 0 0" << "\n";
	out << "  bc epi_para_right	 	fix_velocity 	0 0 0" << "\n";
	out << "  bc str_left			fix_velocity 	0 0 0" << "\n";
	out << "  bc str_right			fix_velocity 	0 0 0" << "\n";
	out << "  bc endo_para_left		fix_velocity 	0 0 0" << "\n";
	out << "  bc endo_para_right	fix_velocity 	0 0 0" << "\n";
	out << "  bc ah_left			fix_velocity 	0 0 0" << "\n";
	out << "  bc endo_trans_left	fix_velocity    0 0 0" << "\n";
	out << "  bc endo_trans_right	fix_velocity    0 0 0" << "\n";
	out << "  bc epi_trans_right	fix_velocity    0 0 0" << "\n";
	out << "  bc epi_trans_left		fix_velocity    0 0 0" << "\n";
	out << "\n";

	out << "module species C_tot" << "\n";
	out << "  sweeps 500" << "\n";
	out << "  linearsolver cgs" << "\n";
	out << "  solvertolerance 1e-8" << "\n";
	out << "  relaxation 0" << "\n";
	out << "  diagrelaxation 0" << "\n";
	out << "\n";

	out << "  vc tear 				const_diff " << _diffTearEdit->text().toStdString() << "\n";
	out << "  vc epi 				const_diff " << _diffEpiEdit->text().toStdString() << "\n";
	out << "  vc epi_para 			const_diff " << _diffEpiParaEdit->text().toStdString() << "\n";
	out << "  vc stroma 			const_diff " << _diffStromaEdit->text().toStdString() << "\n";
	out << "  vc endo 				const_diff " << _diffEndoEdit->text().toStdString() << "\n";
	out << "  vc endo_para 			const_diff " << _diffEndoParaEdit->text().toStdString() << "\n";
	out << "  vc ah 				const_diff " << _diffAHEdit->text().toStdString() << "\n";
	out << "\n";

	out << "  bc ah_back					fix_value " << _ahEdit->text().toStdString() << "\n";
	out << "  bc tear_front					fix_value " << _tearEdit->text().toStdString() << "\n";

	out << "  bc tear_wall  				fix_flux     	0" << "\n";
	out << "  bc epi_wall   				fix_flux     	0" << "\n";
	out << "  bc epi_para_wall   	   		fix_flux        0" << "\n";
	out << "  bc stroma_wall  				fix_flux        0" << "\n";
	out << "  bc endo_wall    				fix_flux        0" << "\n";
	out << "  bc endo_para_wall    	    	fix_flux        0" << "\n";
	out << "  bc ah_wall    		  		fix_flux        0" << "\n";
	out << "  bc tear2epi        			fix_flux        0" << "\n";
	out << "  bc epi_para_left				fix_flux        0" << "\n";
	out << "  bc epi_para_right	 	        fix_flux        0" << "\n";
	out << "  bc str_left					fix_flux        0" << "\n";
	out << "  bc str_right					fix_flux        0" << "\n";
	out << "  bc endo_para_left				fix_flux        0" << "\n";
	out << "  bc endo_para_right	        fix_flux        0" << "\n";
	out << "  bc ah_left        			fix_flux        0" << "\n";
	out << "\n";

	out << "  bc epi_trans_left	 	fix_wiremembrane flux Ps " << _permTearEpiEdit->text().toStdString() << " K1 1 volume tear K2 " << _partitionEpiEdit->text().toStdString() << " volume epi" << "\n";
	out << "  bc epi_trans_right			fix_wiremembrane flux Ps " << _permEpiStrEdit->text().toStdString() << " K1 1 volume tear K2 " << _partitionStrEdit->text().toStdString() << " volume stroma" << "\n";
	out << "  bc endo_trans_left		fix_wiremembrane flux Ps " << _permStrEndoEdit->text().toStdString() << " K1 1 volume tear K2 " << _partitionEndoEdit->text().toStdString() << " volume endo" << "\n";
	out << "  bc endo_trans_right			fix_wiremembrane flux Ps " << _permEndoAHEdit->text().toStdString() << " K1 1 volume tear K2 " << _partitionAHEdit->text().toStdString() << " volume ah" << "\n";
	out << "\n";

	//out << "init tear fix_value 5.5" << "\n";

	out << "  wire_center_line_output cell_data volume_base" << "\n";
	out << "\n";

	out << "module species Cb_lipid" << "\n";
	out << "  sweeps 500" << "\n";
	out << "  linearsolver cgs" << "\n";
	out << "  solvertolerance 1.e-8" << "\n";
	out << "  relaxation 0.0" << "\n";
	out << "  diagrelaxation 0.0" << "\n";
	out << "  stationary" << "\n";
	out << "  vc tear   		const_diff 0" << "\n";
	out << "  vc epi      		const_diff 0" << "\n";
	out << "  vc epi_para       const_diff 0" << "\n";
	out << "  vc stroma   		const_diff 0" << "\n";
	out << "  vc endo     		const_diff 0" << "\n";
	out << "  vc endo_para     	const_diff 0" << "\n";
	out << "  vc ah     		const_diff 0" << "\n";
	out << "  bc tear_wall      	fix_flux   0" << "\n";
	out << "  bc epi_wall       	fix_flux   0" << "\n";
	out << "  bc epi_para_wall  	fix_flux   0" << "\n";
	out << "  bc stroma_wall    	fix_flux   0" << "\n";
	out << "  bc endo_wall      	fix_flux   0" << "\n";
	out << "  bc endo_para_wall 	fix_flux   0" << "\n";
	out << "  bc ah_wall        	fix_flux   0" << "\n";
	out << "  wire_center_line_output cell_data volume_base" << "\n";
	out << "\n";

	out << "module species Cb_protein" << "\n";
	out << "  sweeps 0" << "\n";
	out << "  linearsolver cgs" << "\n";
	out << "  solvertolerance 1.e-8" << "\n";
	out << "  relaxation 1.0" << "\n";
	out << "  diagrelaxation 0.0" << "\n";
	out << "  vc stroma bound_species C_tot Bmax " << _BmaxEdit->text().toStdString() << " Kd " << _KdEdit->text().toStdString() << " power_n " << _powerEdit->text().toStdString() << "\n";
	out << "  wire_center_line_output cell_data volume_base" << "\n";
	out << "\n";

	out.close();

	NTF_NOTICE << "sim file written to " << fname;
}

OcularWidgetThread::OcularWidgetThread(QObject *parent, bool b)
	:QThread(parent), Stop(b)
{
}

void OcularWidgetThread::run()
{
	system(_simfile.c_str());
}

bool OcularModelToolWindow::_regSucc = OcularModelToolWindow::_registerCreateFunctor();
bool OcularModelToolWindow::_registerCreateFunctor() {
	std::string key = "OcularToolWindow";
	if (_getToolWindowFunctors().find(key) != _getToolWindowFunctors().end()) {
		return false;
	}
	_getToolWindowFunctors()[key] = &_createInstance;
	return true;
};



