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

#include "RiaApplication.h"

#include "cafEffectCache.h"
#include "cafUtils.h"
#include "cafAppEnum.h"

#include "RiaVersionInfo.h"
#include "RiaBaseDefs.h"
//
#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuProcessMonitor.h"
#include "RiaPreferences.h"
//
#include "RimResultCase.h"
#include "RimInputCase.h"
#include "RimReservoirView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimOilField.h"
#include "RimAnalysisModels.h"

#include "cafCeetronNavigation.h"
#include "cafCadNavigation.h"
#include "RiaSocketServer.h"
#include "cafUiProcess.h"
//
#include "RimUiTreeModelPdm.h"
#include "RiaImageCompareReporter.h"
#include "RiaImageFileCompare.h"
#include "cafProgressInfo.h"
#include "RigGridManager.h"

#include "RimProject.h"

#include "RimResultSlot.h"

#include "RimIdenticalGridCaseGroup.h"
#include "RimInputPropertyCollection.h"

#include "RimDefines.h"
#include "RimScriptCollection.h"
#include "RimCaseCollection.h"

////////////

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimWellCollection.h"

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

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font
    m_standardFont = new cvf::FixedAtlasFont(cvf::FixedAtlasFont::STANDARD);
    m_resViewUpdateTimer = NULL;
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

    // Propagate possible new location of project

    m_project->setProjectFileNameAndUpdateDependencies(projectFileName);

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
    
    // VL check regarding specific order mentioned in comment above...

    m_preferences->lastUsedProjectFileName = projectFileName;
    writePreferences();

    for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = m_project->oilFields[oilFieldIdx];
        RimAnalysisModels* analysisModels = oilField ? oilField->analysisModels() : NULL;
        if (analysisModels == NULL) continue;

        for (size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); ++cgIdx)
        {
            // Load the Main case of each IdenticalGridCaseGroup
            RimIdenticalGridCaseGroup* igcg = analysisModels->caseGroups[cgIdx];
            igcg->loadMainCaseAndActiveCellInfo(); // VL is this supposed to be done for each RimOilField?
        }
    }

    // Add well paths for each oil field
    for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = m_project->oilFields[oilFieldIdx];
        if (oilField == NULL) continue;        
        if (oilField->wellPathCollection == NULL)
        {
            //printf("Create well path collection for oil field %i in loadProject.\n", oilFieldIdx);
            oilField->wellPathCollection = new RimWellPathCollection();
            oilField->wellPathCollection->setProject(m_project);
        }

        if (oilField->wellPathCollection) oilField->wellPathCollection->readWellPathFiles();
    }

    // Now load the ReservoirViews for the cases
    // Add all "native" cases in the project
    std::vector<RimCase*> casesToLoad;
    m_project->allCases(casesToLoad);

    caf::ProgressInfo caseProgress(casesToLoad.size() , "Reading Cases");
    
    for (size_t cIdx = 0; cIdx < casesToLoad.size(); ++cIdx)
    {
        RimCase* ri = casesToLoad[cIdx];
        CVF_ASSERT(ri);

        caseProgress.setProgressDescription(ri->caseUserDescription());

        caf::ProgressInfo viewProgress(ri->reservoirViews().size() , "Creating Views");

        size_t j;
        for (j = 0; j < ri->reservoirViews().size(); j++)
        {
            RimReservoirView* riv = ri->reservoirViews[j];
            CVF_ASSERT(riv);

            viewProgress.setProgressDescription(riv->name());

            riv->loadDataAndUpdate();
            this->setActiveReservoirView(riv);
            viewProgress.incrementProgress();
        }

        caseProgress.incrementProgress();
    }

    // NB! This function must be called before executing command objects, 
    // because the tree view state is restored from project file and sets
    // current active view ( see restoreTreeViewState() )
    // Default behavior for scripts is to use current active view for data read/write
    onProjectOpenedOrClosed();
    processEvents();

    // Loop over command objects and execute them
    for (size_t i = 0; i < m_project->commandObjects.size(); i++)
    {
        m_commandQueue.push_back(m_project->commandObjects[i]);
    }

    // Lock the command queue
    m_commandQueueLock.lock();

    // Execute command objects, and release the mutex when the queue is empty
    executeCommandObjects();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Add a list of well path file paths (JSON files) to the well path collection
//--------------------------------------------------------------------------------------------------
void RiaApplication::addWellPathsToModel(QList<QString> wellPathFilePaths)
{
    if (m_project == NULL || m_project->oilFields.size() < 1) return;

    RimOilField* oilField = m_project->activeOilField();
    if (oilField == NULL) return;

    if (oilField->wellPathCollection == NULL)
    {
        //printf("Create well path collection.\n");
        oilField->wellPathCollection = new RimWellPathCollection();
        oilField->wellPathCollection->setProject(m_project);
        RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(m_project);
    }

    if (oilField->wellPathCollection) oilField->wellPathCollection->addWellPaths(wellPathFilePaths);
    
    RiuMainWindow::instance()->uiPdmModel()->updateUiSubTree(oilField->wellPathCollection);
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

    m_commandQueue.clear();

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
    RimResultCase* rimResultReservoir = new RimResultCase();
    rimResultReservoir->setCaseInfo(caseName, caseFileName);

    RimAnalysisModels* analysisModels = m_project->activeOilField() ? m_project->activeOilField()->analysisModels() : NULL;
    if (analysisModels == NULL) return false;

    analysisModels->cases.push_back(rimResultReservoir);

    RimReservoirView* riv = rimResultReservoir->createAndAddReservoirView();

    // Select SOIL as default result variable
    riv->cellResult()->setResultType(RimDefines::DYNAMIC_NATIVE);
    riv->cellResult()->setResultVariable("SOIL");
    riv->animationMode = true;

    riv->loadDataAndUpdate();

    if (!riv->cellResult()->hasResult())
    {
        riv->cellResult()->setResultVariable(RimDefines::undefinedResultName());
    }

    RimUiTreeModelPdm* uiModel = RiuMainWindow::instance()->uiPdmModel();

    uiModel->updateUiSubTree(analysisModels);

    RiuMainWindow::instance()->setCurrentObjectInTreeView(riv->cellResult());

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openInputEclipseCase(const QString& caseName, const QStringList& caseFileNames)
{
    RimInputCase* rimInputReservoir = new RimInputCase();
    m_project->assignCaseIdToCase(rimInputReservoir);

    rimInputReservoir->caseUserDescription = caseName;
    rimInputReservoir->openDataFileSet(caseFileNames);

    RimAnalysisModels* analysisModels = m_project->activeOilField() ? m_project->activeOilField()->analysisModels() : NULL;
    if (analysisModels == NULL) return false;

    analysisModels->cases.push_back(rimInputReservoir);

    RimReservoirView* riv = rimInputReservoir->createAndAddReservoirView();

    riv->cellResult()->setResultType(RimDefines::INPUT_PROPERTY);
    riv->animationMode = true;

    riv->loadDataAndUpdate();

    if (!riv->cellResult()->hasResult())
    {
        riv->cellResult()->setResultVariable(RimDefines::undefinedResultName());
    }

    RimUiTreeModelPdm* uiModel = RiuMainWindow::instance()->uiPdmModel();
    uiModel->updateUiSubTree(analysisModels);

    RiuMainWindow::instance()->setCurrentObjectInTreeView(riv->cellResult());

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
        QString helpText = commandLineParameterHelp();

#if defined(_MSC_VER) && defined(_WIN32)
        showFormattedTextInMessageBox(helpText);
#else
        fprintf(stdout, "%s\n", helpText.toAscii().data());
        fflush(stdout);
#endif
        return false;
    }

    if (isRunRegressionTest)
    {
        RiuMainWindow* mainWnd = RiuMainWindow::instance();
        if (mainWnd)
        {
            mainWnd->hideAllDockWindows();

            runRegressionTest(regressionTestPath);

            mainWnd->loadWinGeoAndDockToolBarLayout();
        }

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
QString RiaApplication::scriptDirectories() const
{
    return m_preferences->scriptDirectories();
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


    executeCommandObjects();


    // Exit code != 0 means we have an error
    if (exitCode != 0)
    {
      //  MFLog::error(QString("Simulation execution failed (exit code %1).").arg(exitCode));
        return;
    }

    // If multiple cases are present, invoke launchProcess() which will set next current case, and run script on this case 
    if (m_currentCaseIds.size() > 0)
    {
        launchProcess(m_currentProgram, m_currentArguments);
    }
    else
    {
        // Disable concept of current case
        m_socketServer->setCurrentCaseId(-1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcess(const QString& program, const QStringList& arguments)
{
    if (m_workerProcess == NULL)
    {
        // If multiple cases are present, pop the first case ID from the list and set as current case
        if (m_currentCaseIds.size() > 0)
        {
            int nextCaseId = m_currentCaseIds.front();
            m_currentCaseIds.pop_front();

            m_socketServer->setCurrentCaseId(nextCaseId);
        }
        else
        {
            // Disable current case concept
            m_socketServer->setCurrentCaseId(-1);
        }

        m_workerProcess = new caf::UiProcess(this);

        // Set the LD_LIBRARY_PATH to make the octave plugins find the embedded Qt

        QProcessEnvironment penv = m_workerProcess->processEnvironment();
        QString ldPath = penv.value("LD_LIBRARY_PATH", "");

        if (ldPath == "") ldPath = QApplication::applicationDirPath();
        else  ldPath = QApplication::applicationDirPath() + ":" + ldPath;

        penv.insert("LD_LIBRARY_PATH", ldPath);
        m_workerProcess->setProcessEnvironment(penv);

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
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcessForMultipleCases(const QString& program, const QStringList& arguments, const std::vector<int>& caseIds)
{
    m_currentCaseIds.clear();
    std::copy( caseIds.begin(), caseIds.end(), std::back_inserter( m_currentCaseIds ) );

    m_currentProgram = program;
    m_currentArguments = arguments;

    return launchProcess(m_currentProgram, m_currentArguments);
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
        this->project()->setScriptDirectories(m_preferences->scriptDirectories());
        RimUiTreeModelPdm* treeModel = RiuMainWindow::instance()->uiPdmModel();
        if (treeModel) treeModel->updateUiSubTree(this->project()->scriptCollection());
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
    QImage image = grabFrameBufferImage();
    if (!image.isNull())
    {
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
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
        QImage image = grabFrameBufferImage();
        if (!image.isNull())
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

    std::vector<RimCase*> projectCases;
    m_project->allCases(projectCases);

    for (size_t i = 0; i < projectCases.size(); i++)
    {
        RimCase* ri = projectCases[i];
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

                QString fileName = ri->caseUserDescription() + "-" + riv->name();

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
    this->preferences()->resetToDefaults();

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());
        if (testCaseFolder.exists(regTestProjectName))
        {
             loadProject(testCaseFolder.filePath(regTestProjectName));

             // Wait until all command objects have completed
             while (!m_commandQueueLock.tryLock())
             {
                 processEvents();
             }
             m_commandQueueLock.unlock();

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
    RimResultCase* mainResultCase = NULL;
    std::vector< std::vector<int> > mainCaseGridDimensions;
    RimIdenticalGridCaseGroup* gridCaseGroup = NULL;

    {
        QString firstFileName = fileNames[0];
        QFileInfo gridFileName(firstFileName);

        QString caseName = gridFileName.completeBaseName();

        RimResultCase* rimResultReservoir = new RimResultCase();
        rimResultReservoir->setCaseInfo(caseName, firstFileName);
        if (!rimResultReservoir->openEclipseGridFile())
        {
            delete rimResultReservoir;

            return false;
        }

        rimResultReservoir->readGridDimensions(mainCaseGridDimensions);

        mainResultCase = rimResultReservoir;
        RimOilField* oilField = m_project->activeOilField();
        if (oilField && oilField->analysisModels())
        {
            gridCaseGroup = oilField->analysisModels->createIdenticalCaseGroupFromMainCase(mainResultCase);
        }
    }

    caf::ProgressInfo info(fileNames.size(), "Reading Active Cell data");

    for (int i = 1; i < fileNames.size(); i++)
    {
        QString caseFileName = fileNames[i];
        QFileInfo gridFileName(caseFileName);

        QString caseName = gridFileName.completeBaseName();

        RimResultCase* rimResultReservoir = new RimResultCase();
        rimResultReservoir->setCaseInfo(caseName, caseFileName);

        std::vector< std::vector<int> > caseGridDimensions;
        rimResultReservoir->readGridDimensions(caseGridDimensions);

        bool identicalGrid = RigGridManager::isGridDimensionsEqual(mainCaseGridDimensions, caseGridDimensions);
        if (identicalGrid)
        {
            if (rimResultReservoir->openAndReadActiveCellData(mainResultCase->reservoirData()))
            {
                RimOilField* oilField = m_project->activeOilField();
                if (oilField && oilField->analysisModels())
                {
                    oilField->analysisModels()->insertCaseInCaseGroup(gridCaseGroup, rimResultReservoir);
                }
            }
            else
            {
                delete rimResultReservoir;
            }
        }
        else
        {
            delete rimResultReservoir;
        }

        info.setProgress(i);
    }

    RimUiTreeModelPdm* uiModel = RiuMainWindow::instance()->uiPdmModel();
 
    uiModel->updateUiSubTree( m_project->activeOilField()->analysisModels());

    if (gridCaseGroup->statisticsCaseCollection()->reservoirs.size() > 0)
    {
        RiuMainWindow::instance()->setCurrentObjectInTreeView(gridCaseGroup->statisticsCaseCollection()->reservoirs[0]);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::standardFont()
{
    CVF_ASSERT(m_standardFont.notNull());

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font

    return m_standardFont.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RiaApplication::grabFrameBufferImage()
{
    QImage image;
    if (m_activeReservoirView && m_activeReservoirView->viewer())
    {
        m_activeReservoirView->viewer()->repaint();

        GLint currentReadBuffer;
        glGetIntegerv(GL_READ_BUFFER, &currentReadBuffer);

        glReadBuffer(GL_FRONT);
        image = m_activeReservoirView->viewer()->grabFrameBuffer();

        glReadBuffer(currentReadBuffer);
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimProject* RiaApplication::project()
{
    return m_project;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::showFormattedTextInMessageBox(const QString& text)
{
    QString helpText = text;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("ResInsight");

    helpText.replace("&", "&amp;");
    helpText.replace("<", "&lt;");
    helpText.replace(">", "&gt;");

    helpText = QString("<pre>%1</pre>").arg(helpText);
    msgBox.setText(helpText);

    msgBox.exec();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::commandLineParameterHelp() const
{
    QString text = QString("\n%1 v. %2\n").arg(RI_APPLICATION_NAME).arg(getVersionStringApp(false));
    text += "Copyright Statoil ASA, Ceetron AS 2011, 2012\n\n";

    text +=
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

    return text;
}

//--------------------------------------------------------------------------------------------------
/// Schedule a creation of the Display model and redraw of the reservoir view
/// The redraw will happen as soon as the event loop is entered
//--------------------------------------------------------------------------------------------------
void RiaApplication::scheduleDisplayModelUpdateAndRedraw(RimReservoirView* resViewToUpdate)
{
    m_resViewsToUpdate.push_back(resViewToUpdate);

    if (!m_resViewUpdateTimer) 
    {
        m_resViewUpdateTimer = new QTimer(this);
        connect(m_resViewUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateScheduledDisplayModels()));
    }

    if (!m_resViewUpdateTimer->isActive())
    {
        m_resViewUpdateTimer->setSingleShot(true);
        m_resViewUpdateTimer->start(0);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::slotUpdateScheduledDisplayModels()
{
    // Compress to remove duplicates
    std::set<RimReservoirView*> resViewsToUpdate;
    for (size_t i = 0; i < m_resViewsToUpdate.size(); ++i)
    {
        resViewsToUpdate.insert(m_resViewsToUpdate[i]);
    }

    for (std::set<RimReservoirView*>::iterator it = resViewsToUpdate.begin(); it != resViewsToUpdate.end(); ++it )
    {
        if (*it)
        {
            (*it)->createDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setCacheDataObject(const QString& key, const QVariant& dataObject)
{
    m_sessionCache[key] = dataObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVariant RiaApplication::cacheDataObject(const QString& key) const
{
    QMap<QString, QVariant>::const_iterator it = m_sessionCache.find(key);

    if (it != m_sessionCache.end())
    {
        return it.value();
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::addCommandObject(RimCommandObject* commandObject)
{
    m_commandQueue.push_back(commandObject);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::executeCommandObjects()
{
    std::list< RimCommandObject* >::iterator it = m_commandQueue.begin();
    while (it != m_commandQueue.end())
    {
        RimCommandObject* toBeRemoved = *it;
        if (!toBeRemoved->isAsyncronous())
        {
            toBeRemoved->redo();

            it++;
            m_commandQueue.remove(toBeRemoved);
        }
        else
        {
            it++;
        }
    }

    if (m_commandQueue.size() > 0)
    {
        std::list< RimCommandObject* >::iterator it = m_commandQueue.begin();

        RimCommandObject* first = *it;
        first->redo();

        m_commandQueue.pop_front();
    }
    else
    {
        // Unlock the command queue lock when the command queue is empty
        m_commandQueueLock.unlock();
    }
}

