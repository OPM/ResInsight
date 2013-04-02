/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaStdInclude.h"

#include "cafLog.h"
#include "cafEffectCache.h"
#include "cafUtils.h"

#include "cvfPart.h"

#include "cvfStructGridGeometryGenerator.h"

#include "RiaVersionInfo.h"
#include "RiaBaseDefs.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuProcessMonitor.h"
#include "RiaPreferences.h"

#include "RimResultCase.h"
#include "RimInputCase.h"
#include "RimReservoirView.h"

#include "RigCaseData.h"
#include "RigCell.h"
#include "RigReservoirBuilderMock.h"

#include <QSettings>
#include "cafPdmDocument.h"
#include "RifReaderMockModel.h"

#include "cafCeetronNavigation.h"
#include "cafCadNavigation.h"
#include "RiaSocketServer.h"
#include "cafUiProcess.h"

#include "RimUiTreeModelPdm.h"
#include "RiaImageCompareReporter.h"
#include "RiaImageFileCompare.h"
#include "cafProgressInfo.h"

namespace caf
{
template<>
void AppEnum< RiaApplication::RINavigationPolicy >::setUp()
{
    addItem(RiaApplication::NAVIGATION_POLICY_CEETRON,  "NAVIGATION_POLICY_CEETRON",    "Ceetron");
    addItem(RiaApplication::NAVIGATION_POLICY_CAD,      "NAVIGATION_POLICY_CAD",        "CAD");
    setDefault(RiaApplication::NAVIGATION_POLICY_CAD);
}
}


namespace RegTestNames
{
    const QString generatedFolderName   = "RegTestGeneratedImages";
    const QString diffFolderName        = "RegTestDiffImages";
    const QString baseFolderName        = "RegTestBaseImages";
    const QString testProjectName       = "RegressionTest.rip";
    const QString testFolderFilter      = "TestCase*";
    const QString imageCompareExeName   = "compare";
    const QString reportFileName        = "ResInsightRegressionTestReport.html";
};


//==================================================================================================
///
/// \class RiaApplication
///
/// Application class
///
//==================================================================================================

 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaApplication::RiaApplication(int& argc, char** argv)
:   QApplication(argc, argv)
{
    // USed to get registry settings in the right place
    QCoreApplication::setOrganizationName(RI_COMPANY_NAME);
    QCoreApplication::setApplicationName(RI_APPLICATION_NAME);

    // For idle processing
//    m_idleTimerStarted = false;
    installEventFilter(this);

    //cvf::Trace::enable(false);

    m_preferences = new RiaPreferences;
    readPreferences();
    applyPreferences();

    if (useShaders())
    {
        caf::EffectGenerator::setRenderingMode(caf::EffectGenerator::SHADER_BASED);
    }
    else
    {
        caf::EffectGenerator::setRenderingMode(caf::EffectGenerator::FIXED_FUNCTION);
    }

    // Start with a project
    m_project = new RimProject;
 
    setWindowIcon(QIcon(":/AppLogo48x48.png"));

    m_socketServer = new RiaSocketServer( this);
    m_workerProcess = NULL;


    m_startupDefaultDirectory = QDir::homePath();

#ifdef WIN32
    //m_startupDefaultDirectory += "/My Documents/";
#endif
    setDefaultFileDialogDirectory("MULTICASEIMPORT", "/");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaApplication::~RiaApplication()
{
    delete m_preferences;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaApplication* RiaApplication::instance()
{
    return static_cast<RiaApplication*>qApp;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setWindowCaptionFromAppState()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    // The stuff being done here should really be handled by Qt automatically as a result of
    // setting applicationName and windowFilePath
    // Was unable to make this work in Qt4.4.0!

    QString capt = RI_APPLICATION_NAME;
#ifdef _DEBUG
    capt += " ##DEBUG##";
#endif

    {
        QString projFileName = m_project->fileName();
         if (projFileName.isEmpty()) projFileName = "Untitled project";

        capt = projFileName + QString("[*]") + QString(" - ") + capt;
    }

    mainWnd->setWindowTitle(capt);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::processNonGuiEvents()
{
    processEvents(QEventLoop::ExcludeUserInputEvents);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* RiaApplication::getVersionStringApp(bool includeCrtInfo)
{
    // Use static buf so we can return ptr
    static char szBuf[1024];

    cvf::String crtInfo;
    if (includeCrtInfo)
    {
#ifdef _MT
#ifdef _DLL
        crtInfo = " (DLL CRT)";
#else
        crtInfo = " (static CRT)";
#endif
#endif //_MT
    }

    cvf::System::sprintf(szBuf, 1024, "%s%s", STRPRODUCTVER, crtInfo.toAscii().ptr());

    return szBuf;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadProject(const QString& projectFileName)
{
    // First Close the current project

    if (!closeProject(true)) return false;

    // Open the project file and read the serialized data. 
    // Will initialize itself.

    if (!QFile::exists(projectFileName)) return false;

    m_project->fileName = projectFileName;
    m_project->readFile();
    m_project->fileName = projectFileName; // Make sure we overwrite the old filename read from the project file 

    // On error, delete everything, and bail out.

    if (m_project->projectFileVersionString().isEmpty())
    {
        closeProject(false);

        QString tmp = QString("Unknown project file version detected in file \n%1\n\nCould not open project.").arg(projectFileName);
        QMessageBox::warning(NULL, "Error when opening project file", tmp);

        RiuMainWindow* mainWnd = RiuMainWindow::instance();
        mainWnd->setPdmRoot(NULL);

        // Delete all object possibly generated by readFile()
        delete m_project;
        m_project = new RimProject;

        onProjectOpenedOrClosed();

        return true;
    }

    ///////
    // Load the external data, and initialize stuff that needs specific ordering

    m_preferences->lastUsedProjectFileName = projectFileName;
    writePreferences();

    for (size_t cgIdx = 0; cgIdx < m_project->caseGroups.size(); ++cgIdx)
    {
        // Load the Main case of each IdenticalGridCaseGroup

        RimIdenticalGridCaseGroup* igcg = m_project->caseGroups[cgIdx];
        igcg->loadMainCaseAndActiveCellInfo();
    }

    // Now load the ReservoirViews for the cases

    std::vector<RimCase*> casesToLoad;

    // Add all "native" cases in the project
    for (size_t cIdx = 0; cIdx < m_project->reservoirs().size(); ++cIdx)
    {
        casesToLoad.push_back(m_project->reservoirs()[cIdx]);
    }

    // Add all statistics cases as well
    for (size_t cgIdx = 0; cgIdx < m_project->caseGroups().size(); ++cgIdx)
    {
        if (m_project->caseGroups[cgIdx]->statisticsCaseCollection())
        {
            caf::PdmPointersField<RimCase*> & statCases = m_project->caseGroups[cgIdx]->statisticsCaseCollection()->reservoirs();
            for (size_t scIdx = 0; scIdx < statCases.size(); ++scIdx)
            {
                casesToLoad.push_back(statCases[scIdx]);
            }
        }
    }

    caf::ProgressInfo caseProgress(casesToLoad.size() , "Reading Cases");

    for (size_t cIdx = 0; cIdx < casesToLoad.size(); ++cIdx)
    {
        RimCase* ri = casesToLoad[cIdx];
        CVF_ASSERT(ri);

        caseProgress.setProgressDescription(ri->caseName());

        caf::ProgressInfo viewProgress(ri->reservoirViews().size() , "Creating Views");

        size_t j;
        for (j = 0; j < ri->reservoirViews().size(); j++)
        {
            RimReservoirView* riv = ri->reservoirViews[j];
            CVF_ASSERT(riv);

            viewProgress.setProgressDescription(riv->name());

            riv->loadDataAndUpdate();

            viewProgress.incrementProgress();
        }

        caseProgress.incrementProgress();
    }

    onProjectOpenedOrClosed();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadLastUsedProject()
{
    return loadProject(m_preferences->lastUsedProjectFileName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProject()
{
    CVF_ASSERT(m_project.notNull());

    if (!QFile::exists(m_project->fileName()))
    {
        return saveProjectPromptForFileName();
    }
    else
    {
        return saveProjectAs(m_project->fileName());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProjectPromptForFileName()
{
    //if (m_project.isNull()) return true;

    RiaApplication* app = RiaApplication::instance();

    QString startPath;
    if (!m_project->fileName().isEmpty())
    {
        QFileInfo fi(m_project->fileName());
        startPath = fi.absolutePath();
    }
    else
    {
        startPath = app->defaultFileDialogDirectory("BINARY_GRID");
    }

    startPath += "/ResInsightProject.rip";

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"), startPath, tr("Project Files (*.rip *.xml)"));
    if (fileName.isEmpty())
    {
        return false;
    }

    // Remember the directory to next time
    app->setDefaultFileDialogDirectory("BINARY_GRID", QFileInfo(fileName).absolutePath());

    bool bSaveOk = saveProjectAs(fileName);

    setWindowCaptionFromAppState();

    return bSaveOk;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProjectAs(const QString& fileName)
{
    m_project->fileName = fileName;
    m_project->writeFile();

    m_preferences->lastUsedProjectFileName = fileName;
    writePreferences();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::closeProject(bool askToSaveIfDirty)
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();

    terminateProcess();

    if (false)
    {
        QMessageBox msgBox(mainWnd);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The project being closed has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        //msgBox.setDefaultButton(QMessageBox::Save);

        int ret = msgBox.exec();
        if (ret == QMessageBox::Save)
        {
            //m_sceneManager->saveAll();
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }

    mainWnd->cleanupGuiBeforeProjectClose();

    caf::EffectGenerator::clearEffectCache();
    m_project->close();

    onProjectOpenedOrClosed();

    return true;
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::onProjectOpenedOrClosed()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    mainWnd->initializeGuiNewProjectLoaded();
    //mainWnd->redrawAllViews();

    setWindowCaptionFromAppState();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::currentProjectFileName() const
{
    return m_project->fileName();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openEclipseCaseFromFile(const QString& fileName)
{
    if (!QFile::exists(fileName)) return false;

    QFileInfo gridFileName(fileName);
    QString caseName = gridFileName.completeBaseName();

    return openEclipseCase(caseName, fileName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openEclipseCase(const QString& caseName, const QString& caseFileName)
{
    QFileInfo gridFileName(caseFileName);
    QString casePath = gridFileName.absolutePath();

    RimResultCase* rimResultReservoir = new RimResultCase();
    rimResultReservoir->caseName = caseName;
    rimResultReservoir->caseFileName = caseFileName;
    rimResultReservoir->caseDirectory = casePath;

    m_project->reservoirs.push_back(rimResultReservoir);

    RimReservoirView* riv = rimResultReservoir->createAndAddReservoirView();

    if (m_preferences->autocomputeSOIL)
    {
        // Select SOIL as default result variable
        riv->cellResult()->resultType = RimDefines::DYNAMIC_NATIVE;
        riv->cellResult()->resultVariable = "SOIL";
        riv->animationMode = true;
    }

    riv->loadDataAndUpdate();

    if (!riv->cellResult()->hasResult())
    {
        riv->cellResult()->resultVariable = RimDefines::undefinedResultName();
    }

    onProjectOpenedOrClosed();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openInputEclipseCase(const QString& caseName, const QStringList& caseFileNames)
{
    RimInputCase* rimInputReservoir = new RimInputCase();
    rimInputReservoir->caseName = caseName;
    rimInputReservoir->openDataFileSet(caseFileNames);

    m_project->reservoirs.push_back(rimInputReservoir);

    RimReservoirView* riv = rimInputReservoir->createAndAddReservoirView();

    riv->cellResult()->resultType = RimDefines::INPUT_PROPERTY;
    riv->animationMode = true;

    riv->loadDataAndUpdate();

    if (!riv->cellResult()->hasResult())
    {
        riv->cellResult()->resultVariable = RimDefines::undefinedResultName();
    }

    onProjectOpenedOrClosed();

    return true;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMockModel()
{
    openEclipseCase("Result Mock Debug Model Simple", "Result Mock Debug Model Simple");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createResultsMockModel()
{
    openEclipseCase("Result Mock Debug Model With Results", "Result Mock Debug Model With Results");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createLargeResultsMockModel()
{
    openEclipseCase("Result Mock Debug Model Large With Results", "Result Mock Debug Model Large With Results");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createInputMockModel()
{
    openInputEclipseCase("Input Mock Debug Model Simple", QStringList("Input Mock Debug Model Simple"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimReservoirView* RiaApplication::activeReservoirView() const
{
    return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirView* RiaApplication::activeReservoirView()
{
   return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setActiveReservoirView(RimReservoirView* rv)
{
    m_activeReservoirView = rv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setUseShaders(bool enable)
{
    m_preferences->useShaders = enable;
    writePreferences();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::useShaders() const
{
    if (!m_preferences->useShaders) return false;

    bool isShadersSupported = caf::Viewer::isShadersSupported();
    if (!isShadersSupported) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaApplication::RINavigationPolicy RiaApplication::navigationPolicy() const
{
    return m_preferences->navigationPolicy();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setShowPerformanceInfo(bool enable)
{
    m_preferences->showHud = enable;
    writePreferences();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::showPerformanceInfo() const
{
    return m_preferences->showHud;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::parseArguments()
{
    QStringList arguments = QCoreApplication::arguments();

    bool openLatestProject = false;
    QString projectFilename;
    QStringList caseNames;
    QString regressionTestPath;

    enum ArgumentParsingType
    {
        PARSE_PROJECT_FILE_NAME,
        PARSE_CASE_NAMES,
        PARSE_START_DIR,
        PARSE_REGRESSION_TEST_PATH,
        PARSING_NONE
    };

    ArgumentParsingType argumentParsingType = PARSING_NONE;

    bool showHelp = false;
    bool isSaveSnapshotsForAllViews = false;
    bool isRunRegressionTest = false;
    bool isUpdateRegressionTest = false;
    
    int i;
    for (i = 1; i < arguments.size(); i++)
    {
        QString arg = arguments[i];
        bool foundKnownOption = false;

        if (arg.toLower() == "-help" || arg.toLower() == "-?")
        {
            showHelp = true;
            foundKnownOption = true;
        }

        if (arg.toLower() == "-last")
        {
            openLatestProject = true;
            foundKnownOption = true;
        }
        else if (arg.toLower() == "-project")
        {
            argumentParsingType = PARSE_PROJECT_FILE_NAME;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-case")
        {
            argumentParsingType = PARSE_CASE_NAMES;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-startdir")
        {
            argumentParsingType = PARSE_START_DIR;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-savesnapshots")
        {
            isSaveSnapshotsForAllViews = true;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-regressiontest")
        {
            isRunRegressionTest = true; 

            argumentParsingType = PARSE_REGRESSION_TEST_PATH;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-updateregressiontestbase")
        {
            isUpdateRegressionTest = true; 

            argumentParsingType = PARSE_REGRESSION_TEST_PATH;

            foundKnownOption = true;
        }
        
        if (!foundKnownOption)
        {
            switch (argumentParsingType)
            {
            case PARSE_PROJECT_FILE_NAME:
                if (QFile::exists(arg))
                {
                    projectFilename = arg;
                }
                break;
            case PARSE_CASE_NAMES:
                {
                    caseNames.append(arg);
                }
                break;

            case PARSE_START_DIR:
                {
                    m_startupDefaultDirectory = arg;
                }
                break;
            case PARSE_REGRESSION_TEST_PATH:
                {
                   regressionTestPath = arg; 
                }
            }
        }
    }

    if (showHelp)
    {
        QString helpText = QString("\n%1 v. %2\n").arg(RI_APPLICATION_NAME).arg(getVersionStringApp(false));
        helpText += "Copyright Statoil ASA, Ceetron AS 2011, 2012\n\n";
        
        helpText +=
        "\nParameter              Description\n"
        "-----------------------------------------------------------------\n"
        "-last                    Open last used project\n"
        "\n"
        "-project <filename>      Open project file <filename>\n"
        "\n"
        "-case <casename>         Open Eclipse case <casename>\n"
        "                         (do not include .GRID/.EGRID)\n"
        "\n"
        "-startdir                The default directory for open/save commands\n"
        "\n"                      
        "-savesnapshots           Save snapshot of all views to 'snapshots' folder in project file folder\n"
        "                         Application closes after snapshots are written to file\n"
        "\n"
        "-regressiontest <folder> Run a regression test on all sub-folders starting with \"" + RegTestNames::testFolderFilter + "\" of the given folder: \n"
        "                         " + RegTestNames::testProjectName + " files in the sub-folders will be opened and \n"
        "                         snapshots of all the views is written to the sub-sub-folder " + RegTestNames::generatedFolderName + ". \n"
        "                         Then difference images is generated in the sub-sub-folder " + RegTestNames::diffFolderName + " based \n"
        "                         on the images in sub-sub-folder " + RegTestNames::baseFolderName + ".\n"
        "                         The results are presented in " + RegTestNames::reportFileName + " that is\n"
        "                         written in the given folder.\n"
        "\n"
        "-updateregressiontestbase <folder> For all sub-folders starting with \"" + RegTestNames::testFolderFilter + "\" of the given folder: \n"
        "                         Copy the images in the sub-sub-folder " + RegTestNames::generatedFolderName + " to the sub-sub-folder\n" 
        "                         " + RegTestNames::baseFolderName + " after deleting " + RegTestNames::baseFolderName + " completely.\n"
        "\n"
        "-help, -?                Displays help text\n"
        "-----------------------------------------------------------------";

        fprintf(stdout, "%s\n", helpText.toAscii().data());
        fflush(stdout);

        return false;
    }

    if (isRunRegressionTest)
    {
        RiuMainWindow* mainWnd = RiuMainWindow::instance();
        if (mainWnd)
        {
            mainWnd->hideAllDockWindows();
        }

        runRegressionTest(regressionTestPath);

        return false;
    }

    if (isUpdateRegressionTest)
    {
        updateRegressionTest(regressionTestPath);

        return false;
    }

    if (openLatestProject)
    {
        loadLastUsedProject();
    }
    
    if (!projectFilename.isEmpty())
    {
        loadProject(projectFilename);
    }
    
    if (!caseNames.isEmpty())
    {
        QString caseName;
        foreach (caseName, caseNames)
        {
            QString tmpCaseFileName = caseName + ".EGRID";

            if (QFile::exists(tmpCaseFileName))
            {
                openEclipseCaseFromFile(tmpCaseFileName);
            }
            else
            {
                tmpCaseFileName = caseName + ".GRID";
                if (QFile::exists(tmpCaseFileName))
                {
                    openEclipseCaseFromFile(tmpCaseFileName);
                }
            }
        }
    }

    if (m_project.notNull() && !m_project->fileName().isEmpty() && isSaveSnapshotsForAllViews)
    {
        saveSnapshotForAllViews("snapshots");

        // Returning false will exit the application
        return false;
    }


    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::scriptDirectory() const
{
    return m_preferences->scriptDirectory();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::scriptEditorPath() const
{
    return m_preferences->scriptEditorExecutable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::octavePath() const
{
    return m_preferences->octaveExecutable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::slotWorkerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    RiuMainWindow::instance()->processMonitor()->stopMonitorWorkProcess();

    // Execute delete later so that other slots that are hooked up
    // get a chance to run before we delete the object
    if (m_workerProcess)
    {
        m_workerProcess->close();
    }
    m_workerProcess = NULL;

    // Either the work process crashed or was aborted by the user
    if (exitStatus == QProcess::CrashExit)
    {
    //    MFLog::error("Simulation execution crashed or was aborted.");
        return;
    }

    // Exit code != 0 means we have an error
    if (exitCode != 0)
    {
      //  MFLog::error(QString("Simulation execution failed (exit code %1).").arg(exitCode));
        return;
    }

    //MFLog::info("Simulation completed successfully.");

    //MFMainWindow::instance()->slotLoadResultsFromSimulationFolder();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcess(const QString& program, const QStringList& arguments)
{
    if (m_workerProcess == NULL)
    {
        m_workerProcess = new caf::UiProcess(this);
        connect(m_workerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotWorkerProcessFinished(int, QProcess::ExitStatus)));

        RiuMainWindow::instance()->processMonitor()->startMonitorWorkProcess(m_workerProcess);

        m_workerProcess->start(program, arguments);
        if (!m_workerProcess->waitForStarted(1000))
        {
            m_workerProcess->close();
            m_workerProcess = NULL;

            RiuMainWindow::instance()->processMonitor()->stopMonitorWorkProcess();

            QMessageBox::warning(RiuMainWindow::instance(), "Script execution", "Failed to start script executable located at\n" + program);

            return false;
        }

        return true;
    }
    else
    {
        QMessageBox::warning(NULL, "Script execution", "An Octave process is still running. Please stop this process before executing a new script.");
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// Read fields of a Pdm object using QSettings
//--------------------------------------------------------------------------------------------------
void RiaApplication::readPreferences()
{
    QSettings settings;
    std::vector<caf::PdmFieldHandle*> fields;

    m_preferences->fields(fields);
    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];

        if (settings.contains(fieldHandle->keyword()))
        {
            QVariant val = settings.value(fieldHandle->keyword());
            fieldHandle->setValueFromUi(val);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Write fields of a Pdm object using QSettings
//--------------------------------------------------------------------------------------------------
void RiaApplication::writePreferences()
{
    QSettings settings;

    std::vector<caf::PdmFieldHandle*> fields;
    m_preferences->fields(fields);

    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];

        settings.setValue(fieldHandle->keyword(), fieldHandle->uiValue());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPreferences* RiaApplication::preferences()
{
    return m_preferences;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::applyPreferences()
{
    if (m_activeReservoirView && m_activeReservoirView->viewer())
    {
        if (m_preferences->navigationPolicy() == NAVIGATION_POLICY_CAD)
        {
            m_activeReservoirView->viewer()->setNavigationPolicy(new caf::CadNavigation);
        }
        else
        {
            m_activeReservoirView->viewer()->setNavigationPolicy(new caf::CeetronNavigation);
        }

        m_activeReservoirView->viewer()->enablePerfInfoHud(m_preferences->showHud());
    }

    if (useShaders())
    {
        caf::EffectGenerator::setRenderingMode(caf::EffectGenerator::SHADER_BASED);
    }
    else
    {
        caf::EffectGenerator::setRenderingMode(caf::EffectGenerator::FIXED_FUNCTION);
    }

    if (this->project())
    {
        this->project()->setUserScriptPath(m_preferences->scriptDirectory());
        RimUiTreeModelPdm* treeModel = RiuMainWindow::instance()->uiPdmModel();
        if (treeModel) treeModel->rebuildUiSubTree(this->project()->scriptCollection());
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::terminateProcess()
{
    if (m_workerProcess)
    {
        m_workerProcess->close();
    }

    m_workerProcess = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::defaultFileDialogDirectory(const QString& dialogName)
{
    QString defaultDirectory = m_startupDefaultDirectory;
    std::map<QString, QString>::iterator it;
    it = m_fileDialogDefaultDirectories.find(dialogName);
    
    if ( it != m_fileDialogDefaultDirectories.end())
    {
        defaultDirectory = it->second;
    }

    return defaultDirectory;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setDefaultFileDialogDirectory(const QString& dialogName, const QString& defaultDirectory)
{
    m_fileDialogDefaultDirectories[dialogName] = defaultDirectory;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::saveSnapshotPromtpForFilename()
{
    QString startPath;
    if (!m_project->fileName().isEmpty())
    {
        QFileInfo fi(m_project->fileName());
        startPath = fi.absolutePath();
    }
    else
    {
        startPath = defaultFileDialogDirectory("IMAGE_SNAPSHOT");
    }

    startPath += "/image.png";

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"), startPath, tr("Image files (*.bmp *.png * *.jpg)"));
    if (fileName.isEmpty())
    {
        return;
    }

    // Remember the directory to next time
    setDefaultFileDialogDirectory("IMAGE_SNAPSHOT", QFileInfo(fileName).absolutePath());

    saveSnapshotAs(fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::saveSnapshotAs(const QString& fileName)
{
    if (m_activeReservoirView && m_activeReservoirView->viewer())
    {
        QImage image = m_activeReservoirView->viewer()->grabFrameBuffer();
        if (image.save(fileName))
        {
            qDebug() << "Saved snapshot image to " << fileName;
        }
        else
        {
            qDebug() << "Error when trying to save snapshot image to " << fileName;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::copySnapshotToClipboard()
{
    if (m_activeReservoirView && m_activeReservoirView->viewer())
    {
        QImage image = m_activeReservoirView->viewer()->grabFrameBuffer();

        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            clipboard->setImage(image);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::saveSnapshotForAllViews(const QString& snapshotFolderName)
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    if (m_project.isNull()) return;

    if (m_project->fileName().isEmpty()) return;

    QFileInfo fi(m_project->fileName());
    QDir projectDir(fi.absolutePath());
    
    if (!projectDir.exists(snapshotFolderName))
    {
        if (!projectDir.mkdir(snapshotFolderName)) return;
    }

    QString snapshotPath = projectDir.absolutePath();
    snapshotPath += "/" + snapshotFolderName;

    for (size_t i = 0; i < m_project->reservoirs().size(); ++i)
    {
        RimCase* ri = m_project->reservoirs()[i];
        if (!ri) continue;

        for (size_t j = 0; j < ri->reservoirViews().size(); j++)
        {
            RimReservoirView* riv = ri->reservoirViews()[j];

            if (riv && riv->viewer())
            {
                setActiveReservoirView(riv);

                RiuViewer* viewer = riv->viewer();
                mainWnd->setActiveViewer(viewer);

                // Process all events to avoid a black image when grabbing frame buffer
                QCoreApplication::processEvents();

                QString fileName = ri->caseName() + "-" + riv->name();

                QString absoluteFileName = caf::Utils::constructFullFileName(snapshotPath, fileName, ".png");
                saveSnapshotAs(absoluteFileName);
            }
        }
    }
}

void removeDirectoryWithContent(QDir dirToDelete )
{
    QStringList files = dirToDelete.entryList();
    for (int fIdx = 0; fIdx < files.size(); ++fIdx)
    {
        dirToDelete.remove(files[fIdx]);
    }
    dirToDelete.rmdir(".");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::runRegressionTest(const QString& testRootPath)
{
    QString generatedFolderName = RegTestNames::generatedFolderName;
    QString diffFolderName      = RegTestNames::diffFolderName;    
    QString baseFolderName      = RegTestNames::baseFolderName;    
    QString regTestProjectName  = RegTestNames::testProjectName; 
    QString regTestFolderFilter = RegTestNames::testFolderFilter;

    // Find all sub folders
    
    QDir testDir(testRootPath); // If string is empty it will end up as cwd
    testDir.setFilter(QDir::Dirs);
    QStringList dirNameFilter;
    dirNameFilter.append(regTestFolderFilter);
    testDir.setNameFilters(dirNameFilter);

    QFileInfoList folderList = testDir.entryInfoList();

    // delete diff and generated images


    for (int i = 0; i < folderList.size(); ++i)
    {
        QDir testCaseFolder(folderList[i].filePath());

        QDir genDir(testCaseFolder.filePath(generatedFolderName));
        removeDirectoryWithContent(genDir);

        QDir diffDir(testCaseFolder.filePath(diffFolderName));
        removeDirectoryWithContent(diffDir);

        QDir baseDir(testCaseFolder.filePath(baseFolderName));
    }

    // Generate html report

    RiaImageCompareReporter imageCompareReporter;

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());

        QString testFolderName = testCaseFolder.dirName();
        QString reportBaseFolderName       = testCaseFolder.filePath(baseFolderName);
        QString reportGeneratedFolderName  = testCaseFolder.filePath(generatedFolderName);
        QString reportDiffFolderName       = testCaseFolder.filePath(diffFolderName);

        imageCompareReporter.addImageDirectoryComparisonSet(testFolderName.toStdString(), reportBaseFolderName.toStdString(), reportGeneratedFolderName.toStdString(), reportDiffFolderName.toStdString());
    }

    imageCompareReporter.generateHTMLReport(testDir.filePath(RegTestNames::reportFileName).toStdString());

    // Generate diff images

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());
        if (testCaseFolder.exists(regTestProjectName))
        {
             loadProject(testCaseFolder.filePath(regTestProjectName));
             saveSnapshotForAllViews(generatedFolderName);

             QDir baseDir(testCaseFolder.filePath(baseFolderName));
             QDir genDir(testCaseFolder.filePath(generatedFolderName));
             QDir diffDir(testCaseFolder.filePath(diffFolderName));
             if (!diffDir.exists()) testCaseFolder.mkdir(diffFolderName);
             baseDir.setFilter(QDir::Files);
             QStringList baseImageFileNames = baseDir.entryList();

             for (int fIdx = 0; fIdx < baseImageFileNames.size(); ++fIdx)
             {
                 QString fileName = baseImageFileNames[fIdx];
                 RiaImageFileCompare imgComparator(RegTestNames::imageCompareExeName);
                 bool ok = imgComparator.runComparison(genDir.filePath(fileName), baseDir.filePath(fileName), diffDir.filePath(fileName));
                 if (!ok)
                 {
                    qDebug() << "Error comparing :" << imgComparator.errorMessage() << "\n" << imgComparator.errorDetails();
                 }
             }

             closeProject(false);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::updateRegressionTest(const QString& testRootPath)
{
    // Find all sub folders

    QDir testDir(testRootPath); // If string is empty it will end up as cwd
    testDir.setFilter(QDir::Dirs);
    QStringList dirNameFilter;
    dirNameFilter.append(RegTestNames::testFolderFilter);
    testDir.setNameFilters(dirNameFilter);

    QFileInfoList folderList = testDir.entryInfoList();

    for (int i = 0; i < folderList.size(); ++i)
    {
        QDir testCaseFolder(folderList[i].filePath());

        QDir baseDir(testCaseFolder.filePath(RegTestNames::baseFolderName));
        removeDirectoryWithContent(baseDir);
        testCaseFolder.mkdir(RegTestNames::baseFolderName);

        QDir genDir(testCaseFolder.filePath(RegTestNames::generatedFolderName));

        QStringList imageFileNames = genDir.entryList();

        for (int fIdx = 0; fIdx < imageFileNames.size(); ++fIdx)
        {
            QString fileName = imageFileNames[fIdx];
            QFile::copy(genDir.filePath(fileName), baseDir.filePath(fileName));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Make sure changes in this functions is validated to RimIdenticalGridCaseGroup::initAfterRead()
//--------------------------------------------------------------------------------------------------
bool RiaApplication::addEclipseCases(const QStringList& fileNames)
{
    if (fileNames.size() == 0) return true;


    // First file is read completely including grid.
    // The main grid from the first case is reused directly in for the other cases. 
    // When reading active cell info, only the total cell count is tested for consistency
    RigCaseData* mainEclipseCase = NULL;

    {
        QString firstFileName = fileNames[0];
        QFileInfo gridFileName(firstFileName);

        QString caseName = gridFileName.completeBaseName();
        QString casePath = gridFileName.absolutePath();

        RimResultCase* rimResultReservoir = new RimResultCase();
        rimResultReservoir->caseName = caseName;
        rimResultReservoir->caseFileName = firstFileName;
        rimResultReservoir->caseDirectory = casePath;

        m_project->reservoirs.push_back(rimResultReservoir);

        if (!rimResultReservoir->openEclipseGridFile())
        {
            return false;
        }

        m_project->moveEclipseCaseIntoCaseGroup(rimResultReservoir);

        mainEclipseCase = rimResultReservoir->reservoirData();
    }

    caf::ProgressInfo info(fileNames.size(), "Reading Active Cell data");

    for (int i = 1; i < fileNames.size(); i++)
    {
        QString fileName = fileNames[i];
        QFileInfo gridFileName(fileName);

        QString caseName = gridFileName.completeBaseName();
        QString casePath = gridFileName.absolutePath();

        RimResultCase* rimResultReservoir = new RimResultCase();
        rimResultReservoir->caseName = caseName;
        rimResultReservoir->caseFileName = fileName;
        rimResultReservoir->caseDirectory = casePath;

        m_project->reservoirs.push_back(rimResultReservoir);

        if (!rimResultReservoir->openAndReadActiveCellData(mainEclipseCase))
        {
            return false;
        }

        m_project->moveEclipseCaseIntoCaseGroup(rimResultReservoir);

        info.setProgress(i);
    }

    onProjectOpenedOrClosed();

    return true;
}
