#ifndef _PCA3DModel_WIDGET_H_
#define _PCA3DModel_WIDGET_H_

#include <QThread>

#include <Common/defaultmathtypes.h>

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidget>

#include <osg/Geometry>

#include <GFW/GFWToolwindow.h>
#include <GFW/PropertyEnabledTreeWidget.h>
#include <OsgVisualization/LookupColorTable.h>
#include <OsgVisualization/Legend.h>

#include <QtWidgets\qsizepolicy.h>

class QOCoBiMainWindow;
class OcularWidgetThread;
class LookupColorTable;
class LegendDrawable;

class OcularWidget : public QWidget
{
	Q_OBJECT

public:

	OcularWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~OcularWidget();

	// Create a singleton instance
	static OcularWidget* instance();

	OcularWidgetThread* cobiThread;

	void onProgressbar();
	osg::Group* getRootOsgGroup() { return _osgGroup.get(); }
	const osg::Group* getRootOsgGroup() const { return _osgGroup.get(); }
	void copyDTF(const std::string& fname, const std::string& scenario);
	void readInVitroCornea(const std::string& fname);
	void read2DEye(const std::string& fname);
	void runCobi();
	void onCreateSimulationInfo();
	void loadVTKMesh(std::string scenario);
	void clearSpecies();
	void removeOldrsl();
	void loadResults();
	void VTKvisualizeSetting();
	void currentView();
	void onWriteInVitroCorneaSimFile(const std::string& fname);
	void onWrite2DEyeSimFile(const std::string& fname);
	void updateLegned();
	void updateVTKLegend();

	static const int DEFAULT = 0;
	static const int PRESSURE = 1;
	static const int SPECIES = 2;

protected:
	virtual void closeEvent(QCloseEvent *event);
	osg::ref_ptr<osg::Geometry> extractGeometry(osg::ref_ptr<osg::Node> fnode);
	void readSIM(const std::string& fname);

Q_SIGNALS:
	void widgetClosed(bool flag);

	public slots:
	void onOpenSim(const std::string& fname);
	void onOpenSimFile();
	void onSaveSimFile();
	void onClear();
	void onChangeScenario();
	void onCreateInVitroCornea();
	void onCreate2DEye();
	void onCreateSpeciesModule(bool flag);

	void onRun();
	void onStepBackward();
	void onStepForward();

private:

	static OcularWidget* _instance;

	osg::ref_ptr<osg::Group> _osgGroup;

	QCheckBox *_speciescheck;
	QVBoxLayout *_bilay;

	QLineEdit *_sweepsflow1Edit;
	QLineEdit *_sweepsflow2Edit;
	QLineEdit *_sweepsheat1Edit;
	QLineEdit *_sweepsspecies1Edit;

	QLineEdit *_leftVelocityEdit;
	QLineEdit *_rightVelocityEdit;

	QLineEdit *_trabecularEdit;
	QLineEdit *_permRetinaEdit;
	QLineEdit *_perfusionEdit;

	QLineEdit *_diffVitreousEdit;
	QLineEdit *_diffAqueousEdit;
	QLineEdit *_diffCiliaryEdit;
	QLineEdit *_diffRetinaEdit;
	QLineEdit *_diffIrisEdit;
	QLineEdit *_diffChoroidEdit;
	QLineEdit *_diffTrabecularEdit;
	QLineEdit *_diffCorneaEdit;
	QLineEdit *_diffScleraEdit;

	QLineEdit *_postionXEdit;
	QLineEdit *_postionYEdit;
	QLineEdit *_radiusEdit;
	QLineEdit *_bolusEdit;

	QLineEdit *_iterEdit;
	QLineEdit *_tolerEdit;
	QLineEdit *_timestepEdit;
	QLineEdit *_stepsizeEdit;
	QLineEdit *_outputfreqEdit;

	QLineEdit *_gravityXEdit;
	QLineEdit *_gravityYEdit;
	QLineEdit *_gravityZEdit;

	QLineEdit *_sk1Edit;
	QLineEdit *_bk1Edit;
	QLineEdit *_sk3Edit;
	QLineEdit *_bk3Edit;

	QLineEdit *_diffTearEdit;
	QLineEdit *_diffEpiEdit;
	QLineEdit *_diffEpiParaEdit;
	QLineEdit *_diffStromaEdit;
	QLineEdit *_diffEndoEdit;
	QLineEdit *_diffEndoParaEdit;
	QLineEdit *_diffAHEdit;

	QLineEdit *_permTearEpiEdit;
	QLineEdit *_permEpiStrEdit;
	QLineEdit *_permStrEndoEdit;
	QLineEdit *_permEndoAHEdit;

	QLineEdit *_partitionTearEdit;
	QLineEdit *_partitionEpiEdit;
	QLineEdit *_partitionStrEdit;
	QLineEdit *_partitionEndoEdit;
	QLineEdit *_partitionAHEdit;

	QLineEdit *_ahEdit;
	QLineEdit *_tearEdit;
	QLineEdit *_ahPressureEdit;
	QLineEdit *_tearPressureEdit;

	QLineEdit *_BmaxEdit;
	QLineEdit *_KdEdit;
	QLineEdit *_powerEdit;

	QComboBox *_scenarioBox;
	QComboBox *_transBox;
	
	std::string _currentDirectory;
	std::string _openFileName;
	std::string _openFilePath;
	std::string _simFileName;

	QWidget		  *_mainTab;
	QWidget		  *_shareTab;
	QWidget		  *_flowTab;
	QWidget		  *_heatTab;
	QWidget		  *_speciesTab;
	
	//Buttons
	QToolButton   *_openButton;
	QToolButton   *_saveButton;
	QToolButton   *_runButton;
	QToolButton   *_clearButton;
	QToolButton   *_backwardButton;
	QToolButton   *_forwardButton;

	int _resultView;
	std::vector<mf_mbd::LookupColorTable::Ref> _colorTables;
	std::vector<std::pair<double, double>> _resultsMinMax;
	
};

class OcularWidgetThread : public QThread
{
	Q_OBJECT

public:
	explicit OcularWidgetThread(QObject * parent = 0, bool b = false);
	void run();
	void setSimFile(std::string xx) { _simfile = xx; }
	bool Stop;

signals:
	void valueChanged(int);

public slots:

private:
	std::string _simfile;
};

class OcularModelToolWindow : public GFWToolWindow
{
public:
	explicit OcularModelToolWindow(GFWWorkbench *workbench)
		: GFWToolWindow(workbench,
			new OcularWidget(),
			QLatin1String("OcularToolWindow"),
			QLatin1String("Ocular Drug Delivery"),
			QLatin1String("Ocular_tool_action"),
			Qt::LeftDockWidgetArea)
	{
	}

	virtual const char* getClassName() { return "OcularToolWindow"; }
protected:
	static GFWToolWindow* _createInstance(GFWWorkbench *workbench) {
		return new OcularModelToolWindow(workbench);
	};
	static bool _regSucc;
	static bool _registerCreateFunctor();

};

#endif
