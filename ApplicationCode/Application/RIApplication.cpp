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

#include "RIStdInclude.h"

#include "cafLog.h"
#include "cafEffectCache.h"
#include "cafUtils.h"

#include "cvfPart.h"

#include "cvfStructGridGeometryGenerator.h"

#include "RIVersionInfo.h"
#include "RIBaseDefs.h"

#include "RIApplication.h"
#include "RIMainWindow.h"
#include "RIViewer.h"
#include "RIProcessMonitor.h"
#include "RIPreferences.h"

#include "RimResultReservoir.h"
#include "RimInputReservoir.h"
#include "RimReservoirView.h"

#include "RigReservoir.h"
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

namespace caf
{
template<>
void AppEnum< RIApplication::RINavigationPolicy >::setUp()
{
    addItem(RIApplication::NAVIGATION_POLICY_CEETRON,  "NAVIGATION_POLICY_CEETRON",    "Ceetron");
    addItem(RIApplication::NAVIGATION_POLICY_CAD,      "NAVIGATION_POLICY_CAD",        "CAD");
    setDefault(RIApplication::NAVIGATION_POLICY_CAD);
}
}


//==================================================================================================
///
/// \class RIApplication
///
/// Application class
///
//==================================================================================================

 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIApplication::RIApplication(int& argc, char** argv)
:   QApplication(argc, argv)
{
    // USed to get registry settings in the right place
    QCoreApplication::setOrganizationName(RI_COMPANY_NAME);
    QCoreApplication::setApplicationName(RI_APPLICATION_NAME);

    // For idle processing
//    m_idleTimerStarted = false;
    installEventFilter(this);

    //cvf::Trace::enable(false);

    m_preferences = new RIPreferences;
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

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIApplication::~RIApplication()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIApplication* RIApplication::instance()
{
    return static_cast<RIApplication*>qApp;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::setWindowCaptionFromAppState()
{
    RIMainWindow* mainWnd = RIMainWindow::instance();
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
void RIApplication::processNonGuiEvents()
{
    processEvents(QEventLoop::ExcludeUserInputEvents);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const char* RIApplication::getVersionStringApp(bool includeCrtInfo)
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
bool RIApplication::loadProject(const QString& projectFileName)
{
    if (!closeProject(true)) return false;

    if (!QFile::exists(projectFileName)) return false;

    m_project->fileName = projectFileName;
    m_project->readFile();

    if (m_project->projectFileVersionString().isEmpty())
    {
        closeProject(false);

        QString tmp = QString("Unknown project file version detected in file \n%1\n\nCould not open project.").arg(projectFileName);
        QMessageBox::warning(NULL, "Error when opening project file", tmp);

        RIMainWindow* mainWnd = RIMainWindow::instance();
        mainWnd->setPdmRoot(NULL);

        // Delete all object possibly generated by readFile()
        delete m_project;
        m_project = new RimProject;
    }
    else
    {
        m_preferences->lastUsedProjectFileName = projectFileName;
        writePreferences();

        size_t i;
        for (i = 0; i < m_project->reservoirs().size(); ++i)
        {
            RimReservoir* ri = m_project->reservoirs()[i];
            CVF_ASSERT(ri);

            size_t j;
            for (j = 0; j < ri->reservoirViews().size(); j++)
            {
                RimReservoirView* riv = ri->reservoirViews()[j];
                CVF_ASSERT(riv);

                riv->loadDataAndUpdate();
            }
        }
    }

    onProjectOpenedOrClosed();
    
    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::loadLastUsedProject()
{
    return loadProject(m_preferences->lastUsedProjectFileName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::saveProject()
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
bool RIApplication::saveProjectPromptForFileName()
{
    //if (m_project.isNull()) return true;

    RIApplication* app = RIApplication::instance();

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
bool RIApplication::saveProjectAs(const QString& fileName)
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
bool RIApplication::closeProject(bool askToSaveIfDirty)
{
    RIMainWindow* mainWnd = RIMainWindow::instance();

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
void RIApplication::onProjectOpenedOrClosed()
{
    RIMainWindow* mainWnd = RIMainWindow::instance();
    if (!mainWnd) return;

    mainWnd->initializeGuiNewProjectLoaded();
    //mainWnd->redrawAllViews();

    setWindowCaptionFromAppState();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RIApplication::currentProjectFileName() const
{
    return m_project->fileName();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::openEclipseCaseFromFile(const QString& fileName)
{
    if (!QFile::exists(fileName)) return false;

    QFileInfo gridFileName(fileName);
    QString caseName = gridFileName.completeBaseName();

    return openEclipseCase(caseName, fileName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::openEclipseCase(const QString& caseName, const QString& caseFileName)
{
    QFileInfo gridFileName(caseFileName);
    QString casePath = gridFileName.absolutePath();

    RimResultReservoir* rimResultReservoir = new RimResultReservoir();
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
bool RIApplication::openInputEclipseCase(const QString& caseName, const QStringList& caseFileNames)
{
    RimInputReservoir* rimInputReservoir = new RimInputReservoir();
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
void RIApplication::createMockModel()
{
    openEclipseCase("Result Mock Debug Model Simple", "Result Mock Debug Model Simple");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::createResultsMockModel()
{
    openEclipseCase("Result Mock Debug Model With Results", "Result Mock Debug Model With Results");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::createLargeResultsMockModel()
{
    openEclipseCase("Result Mock Debug Model Large With Results", "Result Mock Debug Model Large With Results");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::createInputMockModel()
{
    openInputEclipseCase("Input Mock Debug Model Simple", QStringList("Input Mock Debug Model Simple"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimReservoirView* RIApplication::activeReservoirView() const
{
    return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirView* RIApplication::activeReservoirView()
{
   return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::setActiveReservoirView(RimReservoirView* rv)
{
    m_activeReservoirView = rv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::setUseShaders(bool enable)
{
    m_preferences->useShaders = enable;
    writePreferences();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::useShaders() const
{
    if (!m_preferences->useShaders) return false;

    bool isShadersSupported = caf::Viewer::isShadersSupported();
    if (!isShadersSupported) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIApplication::RINavigationPolicy RIApplication::navigationPolicy() const
{
    return m_preferences->navigationPolicy();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::setShowPerformanceInfo(bool enable)
{
    m_preferences->showHud = enable;
    writePreferences();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::showPerformanceInfo() const
{
    return m_preferences->showHud;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RIApplication::parseArguments()
{
    QStringList arguments = QCoreApplication::arguments();

    bool openLatestProject = false;
    QString projectFilename;
    QStringList caseNames;

    bool isParsingProjectFile = false;
    bool isParsingCaseNames = false;
    bool isParsingStartDir = false;
    bool showHelp = false;
    
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
            isParsingCaseNames = false;
            isParsingProjectFile = true;
            isParsingStartDir = false;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-case")
        {
            isParsingCaseNames = true;
            isParsingProjectFile = false;
            isParsingStartDir = false;

            foundKnownOption = true;
        }
        else if (arg.toLower() == "-startdir")
        {
            isParsingCaseNames = false;
            isParsingProjectFile = false;
            isParsingStartDir = true;

            foundKnownOption = true;
        }

        if (!foundKnownOption)
        {
            if (isParsingProjectFile && QFile::exists(arg))
            {
                projectFilename = arg;
            }

            if (isParsingCaseNames)
            {
                caseNames.append(arg);
            }

            if (isParsingStartDir)
            {
                m_startupDefaultDirectory = arg;
            }
        }
    }

    if (showHelp)
    {
        QString helpText = QString("\n%1 v. %2\n").arg(RI_APPLICATION_NAME).arg(getVersionStringApp(false));
        helpText += "Copyright Statoil ASA, Ceetron AS 2011, 2012\n\n";
        
        helpText +=
        "\nParameter            Description\n"
        "-----------------------------------------------------------------\n"
        "-last                Open last used project\n"
        "-project <filename>  Open project file <filename>\n"
        "-case <casename>     Open Eclipse case <casename>\n"
        "                     (do not include .GRID/.EGRID)\n"
        "-startdir            The default directory for open/save commands\n"

        "-help                \n"
        "-?                   Displays help text\n"
        "-----------------------------------------------------------------";

        fprintf(stdout, "%s\n", helpText.toAscii().data());
        fflush(stdout);

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

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RIApplication::scriptDirectory() const
{
    return m_preferences->scriptDirectory();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RIApplication::scriptEditorPath() const
{
    return m_preferences->scriptEditorExecutable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RIApplication::octavePath() const
{
    return m_preferences->octaveExecutable();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::slotWorkerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    RIMainWindow::instance()->processMonitor()->stopMonitorWorkProcess();

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
bool RIApplication::launchProcess(const QString& program, const QStringList& arguments)
{
    if (m_workerProcess == NULL)
    {
        m_workerProcess = new caf::UiProcess(this);
        connect(m_workerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotWorkerProcessFinished(int, QProcess::ExitStatus)));

        RIMainWindow::instance()->processMonitor()->startMonitorWorkProcess(m_workerProcess);

        m_workerProcess->start(program, arguments);
        if (!m_workerProcess->waitForStarted(1000))
        {
            m_workerProcess->close();
            m_workerProcess = NULL;

            RIMainWindow::instance()->processMonitor()->stopMonitorWorkProcess();

            QMessageBox::warning(RIMainWindow::instance(), "Script execution", "Failed to start script executable located at\n" + program);

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
void RIApplication::readPreferences()
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
void RIApplication::writePreferences()
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
RIPreferences* RIApplication::preferences()
{
    return m_preferences;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::applyPreferences()
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
        RimUiTreeModelPdm* treeModel = RIMainWindow::instance()->uiPdmModel();
        if (treeModel) treeModel->rebuildUiSubTree(this->project()->scriptCollection());
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIApplication::terminateProcess()
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
QString RIApplication::defaultFileDialogDirectory(const QString& dialogName)
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
void RIApplication::setDefaultFileDialogDirectory(const QString& dialogName, const QString& defaultDirectory)
{
    m_fileDialogDefaultDirectories[dialogName] = defaultDirectory;
}
