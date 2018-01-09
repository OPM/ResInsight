/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaApplication.h"

#include "RiaBaseDefs.h"
#include "RiaImageCompareReporter.h"
#include "RiaImageFileCompare.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaProjectModifier.h"
#include "RiaSocketServer.h"
#include "RiaVersionInfo.h"
#include "RiaArgumentParser.h"

#include "RigGridManager.h"
#include "RigEclipseCaseData.h"


#include "Rim3dOverlayInfoConfig.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCommandObject.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimRftPlotCollection.h"
#include "RimPltPlotCollection.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimScriptCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellRftPlot.h"
#include "RimWellPltPlot.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"
#include "RiuProcessMonitor.h"
#include "RiuRecentFileActionProvider.h"
#include "RiuSelectionManager.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuViewer.h"
#include "RiuWellLogPlot.h"
#include "RiuWellAllocationPlot.h"
#include "RiuFlowCharacteristicsPlot.h"

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
#include "RimFractureTemplateCollection.h"
#include "RimWellPathFracture.h"
#endif // USE_PROTOTYPE_FEATURE_FRACTURES


#include "RicImportInputEclipseCaseFeature.h"
#include "RicImportSummaryCaseFeature.h"
#include "ExportCommands/RicSnapshotViewToFileFeature.h"
#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include "RicfCommandFileExecutor.h"

#include "cafFixedAtlasFont.h"

#include "cafAppEnum.h"
#include "cafCeetronPlusNavigation.h"
#include "cafEffectCache.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmSettings.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafPdmUiTreeView.h"
#include "cafProgressInfo.h"
#include "cafQTreeViewStateSerializer.h"
#include "cafUiProcess.h"
#include "cafUtils.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QTimer>
#include <QUrl>
#include <QTreeView>

#include "gtest/gtest.h"

#ifdef WIN32
#include <fcntl.h>
#endif


namespace caf
{
template<>
void AppEnum< RiaApplication::RINavigationPolicy >::setUp()
{
    addItem(RiaApplication::NAVIGATION_POLICY_CEETRON,  "NAVIGATION_POLICY_CEETRON",    "Ceetron");
    addItem(RiaApplication::NAVIGATION_POLICY_CAD,      "NAVIGATION_POLICY_CAD",        "CAD");
    addItem(RiaApplication::NAVIGATION_POLICY_GEOQUEST, "NAVIGATION_POLICY_GEOQUEST",   "GEOQUEST");
    addItem(RiaApplication::NAVIGATION_POLICY_RMS,      "NAVIGATION_POLICY_RMS",        "RMS");
    setDefault(RiaApplication::NAVIGATION_POLICY_RMS);
}
}

namespace RegTestNames
{
    const QString generatedFolderName   = "RegTestGeneratedImages";
    const QString diffFolderName        = "RegTestDiffImages";
    const QString baseFolderName        = "RegTestBaseImages";
    const QString testProjectName       = "RegressionTest";
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
    caf::PdmSettings::readFieldsFromApplicationStore(m_preferences);
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

#ifdef WIN32
    m_startupDefaultDirectory = QDir::homePath();
#else
    m_startupDefaultDirectory = QDir::currentPath();
#endif

    setLastUsedDialogDirectory("MULTICASEIMPORT", "/");

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font
    m_standardFont = new caf::FixedAtlasFont(caf::FixedAtlasFont::POINT_SIZE_8);

    m_resViewUpdateTimer = nullptr;
    m_recalculateCompletionTypeTimer = nullptr;

    m_runningRegressionTests = false;

    m_runningWorkerProcess = false;

    m_mainPlotWindow = nullptr;

    m_recentFileActionProvider = std::unique_ptr<RiuRecentFileActionProvider>(new RiuRecentFileActionProvider);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaApplication::~RiaApplication()
{
    deleteMainPlotWindow();

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
/// Return -1 if unit test is not executed, returns 0 if test passed, returns 1 if tests failed
//--------------------------------------------------------------------------------------------------
int RiaApplication::parseArgumentsAndRunUnitTestsIfRequested()
{
    cvf::ProgramOptions progOpt;
    progOpt.registerOption("unittest", "", "Execute unit tests");
    progOpt.setOptionPrefix(cvf::ProgramOptions::DOUBLE_DASH);

    QStringList arguments = QCoreApplication::arguments();

    bool parseOk = progOpt.parse(cvfqt::Utils::toStringVector(arguments));
    if (!parseOk)
    {
        return -1;
    }

    // Unit testing
    // --------------------------------------------------------
    if (cvf::Option o = progOpt.option("unittest"))
    {
        int testReturnValue = launchUnitTestsWithConsole();

        return testReturnValue;
    }

    return -1;
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
bool RiaApplication::loadProject(const QString& projectFileName, ProjectLoadAction loadAction, RiaProjectModifier* projectModifier)
{
    // First Close the current project

    closeProject();

    RiaLogging::info(QString("Starting to open project file : '%1'").arg(projectFileName));

    // Open the project file and read the serialized data. 
    // Will initialize itself.

    if (!caf::Utils::fileExists(projectFileName))
    {
        RiaLogging::info(QString("File does not exist : '%1'").arg(projectFileName));
        return false;
    }

    m_project->fileName = projectFileName;
    m_project->readFile();

    // Apply any modifications to the loaded project before we go ahead and load actual data
    if (projectModifier)
    {
        projectModifier->applyModificationsToProject(m_project);
    }

    // Propagate possible new location of project

    m_project->setProjectFileNameAndUpdateDependencies(projectFileName);

    // On error, delete everything, and bail out.

    if (m_project->projectFileVersionString().isEmpty())
    {
        closeProject();

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

    if (m_project->show3DWindow())
    {
        RiuMainWindow::instance()->show();
    }
    else
    {
        RiuMainWindow::instance()->hide();
    }

    if (m_project->showPlotWindow())
    {
        if (!m_mainPlotWindow)
        {
            createMainPlotWindow();
        }
        else
        {
            m_mainPlotWindow->show();
            m_mainPlotWindow->raise();
        }
    }
    else if (mainPlotWindow())
    {
        mainPlotWindow()->hide();
    }


    ///////
    // Load the external data, and initialize stuff that needs specific ordering
    
    // VL check regarding specific order mentioned in comment above...

    m_preferences->lastUsedProjectFileName = projectFileName;
    caf::PdmSettings::writeFieldsToApplicationStore(m_preferences);

    for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = m_project->oilFields[oilFieldIdx];
        RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : NULL;
        if (analysisModels == NULL) continue;

        for (size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); ++cgIdx)
        {
            // Load the Main case of each IdenticalGridCaseGroup
            RimIdenticalGridCaseGroup* igcg = analysisModels->caseGroups[cgIdx];
            igcg->loadMainCaseAndActiveCellInfo(); // VL is this supposed to be done for each RimOilField?
        }
    }

    // Load the formation names

    for(RimOilField* oilField: m_project->oilFields)
    {
        if (oilField == NULL) continue; 
        if(oilField->formationNamesCollection() != NULL)
        {
            oilField->formationNamesCollection()->readAllFormationNames();
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
        }

        if (oilField->wellPathCollection)
        {
            oilField->wellPathCollection->readWellPathFiles();
            oilField->wellPathCollection->readWellPathFormationFiles();
        }
    }

    for (RimOilField* oilField:  m_project->oilFields)
    {
        if (oilField == NULL) continue; 
        // Temporary
        if(!oilField->summaryCaseMainCollection())
        {
            oilField->summaryCaseMainCollection = new RimSummaryCaseMainCollection();
        }
        oilField->summaryCaseMainCollection()->createSummaryCasesFromRelevantEclipseResultCases();
        oilField->summaryCaseMainCollection()->loadAllSummaryCaseData();

        if (!oilField->observedDataCollection())
        {
            oilField->observedDataCollection = new RimObservedDataCollection();
        }
        for (auto observedCases : oilField->observedDataCollection()->allObservedData())
        {
            observedCases->createSummaryReaderInterface();

            RimObservedData* rimObservedData = dynamic_cast<RimObservedData*>(observedCases);
            if (rimObservedData)
            {
                rimObservedData->updateMetaData();
            }
        }

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
        oilField->fractureDefinitionCollection()->loadAndUpdateData();

        {
            std::vector<RimWellPathFracture*> wellPathFractures;
            oilField->wellPathCollection->descendantsIncludingThisOfType(wellPathFractures);

            for (auto fracture : wellPathFractures)
            {
                fracture->loadDataAndUpdate();
            }
        }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    }

    // If load action is specified to recalculate statistics, do it now.
    // Apparently this needs to be done before the views are loaded, lest the number of time steps for statistics will be clamped
    if (loadAction & PLA_CALCULATE_STATISTICS)
    {
        for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
        {
            RimOilField* oilField = m_project->oilFields[oilFieldIdx];
            RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : NULL;
            if (analysisModels)
            {
                analysisModels->recomputeStatisticsForAllCaseGroups();
            }
        }
    }


    // Now load the ReservoirViews for the cases
    // Add all "native" cases in the project
    std::vector<RimCase*> casesToLoad;
    m_project->allCases(casesToLoad);
    {
        caf::ProgressInfo caseProgress(casesToLoad.size(), "Reading Cases");

        for (size_t cIdx = 0; cIdx < casesToLoad.size(); ++cIdx)
        {
            RimCase* cas = casesToLoad[cIdx];
            CVF_ASSERT(cas);

            caseProgress.setProgressDescription(cas->caseUserDescription());
            std::vector<Rim3dView*> views = cas->views();
            { // To delete the view progress before incrementing the caseProgress
                caf::ProgressInfo viewProgress(views.size(), "Creating Views");

                size_t j;
                for ( j = 0; j < views.size(); j++ )
                {
                    Rim3dView* riv = views[j];
                    CVF_ASSERT(riv);

                    viewProgress.setProgressDescription(riv->name());

                    riv->loadDataAndUpdate();
                    this->setActiveReservoirView(riv);

                    riv->rangeFilterCollection()->updateIconState();

                    viewProgress.incrementProgress();
                }
            }
            caseProgress.incrementProgress();
        }
    }

    if (m_project->viewLinkerCollection() && m_project->viewLinkerCollection()->viewLinker())
    {
        m_project->viewLinkerCollection()->viewLinker()->updateOverrides();
    }

    loadAndUpdatePlotData();

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

    RiaLogging::info(QString("Completed open of project file : '%1'").arg(projectFileName));

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadProject(const QString& projectFileName)
{
    return loadProject(projectFileName, PLA_NONE, NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::loadAndUpdatePlotData()
{
    RimWellLogPlotCollection* wlpColl = nullptr;
    RimSummaryPlotCollection* spColl = nullptr;
    RimSummaryCrossPlotCollection* scpColl = nullptr;
    RimFlowPlotCollection* flowColl = nullptr;
    RimRftPlotCollection* rftColl = nullptr;
    RimPltPlotCollection* pltColl = nullptr;

    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->wellLogPlotCollection())
    {
        wlpColl = m_project->mainPlotCollection()->wellLogPlotCollection();
    }
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->summaryPlotCollection())
    {
        spColl = m_project->mainPlotCollection()->summaryPlotCollection();
    }
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->summaryCrossPlotCollection())
    {
        scpColl = m_project->mainPlotCollection()->summaryCrossPlotCollection();
    }
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->flowPlotCollection())
    {
        flowColl = m_project->mainPlotCollection()->flowPlotCollection();
    }
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->rftPlotCollection())
    {
        rftColl = m_project->mainPlotCollection()->rftPlotCollection();
    }
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->pltPlotCollection())
    {
        pltColl = m_project->mainPlotCollection()->pltPlotCollection();
    }

    size_t plotCount = 0;
    plotCount += wlpColl ? wlpColl->wellLogPlots().size() : 0;
    plotCount += spColl ? spColl->summaryPlots().size() : 0;
    plotCount += scpColl ? scpColl->summaryPlots().size() : 0;
    plotCount += flowColl ? flowColl->plotCount() : 0;
    plotCount += rftColl ? rftColl->rftPlots().size() : 0;
    plotCount += pltColl ? pltColl->pltPlots().size() : 0;

    caf::ProgressInfo plotProgress(plotCount, "Loading Plot Data");
    if (wlpColl)
    {
        for (size_t wlpIdx = 0; wlpIdx < wlpColl->wellLogPlots().size(); ++wlpIdx)
        {
            wlpColl->wellLogPlots[wlpIdx]->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }
    }

    if (spColl)
    {
        for (size_t wlpIdx = 0; wlpIdx < spColl->summaryPlots().size(); ++wlpIdx)
        {
            spColl->summaryPlots[wlpIdx]->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }
    }
    
    if (scpColl)
    {
        for (auto plot : scpColl->summaryPlots())
        {
            plot->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }
    }

    if (flowColl)
    {
        plotProgress.setNextProgressIncrement(flowColl->plotCount());
        flowColl->loadDataAndUpdate();
        plotProgress.incrementProgress();
    }

    if (rftColl)
    {
        for (const auto& rftPlot : rftColl->rftPlots())
        {
            rftPlot->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }
    }

    if (pltColl)
    {
        for (const auto& pltPlot : pltColl->pltPlots())
        {
            pltPlot->loadDataAndUpdate();
            plotProgress.incrementProgress();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::storeTreeViewState()
{
    {
        if (mainPlotWindow() && mainPlotWindow()->projectTreeView())
        {
            caf::PdmUiTreeView* projectTreeView = mainPlotWindow()->projectTreeView();

            QString treeViewState;
            caf::QTreeViewStateSerializer::storeTreeViewStateToString(projectTreeView->treeView(), treeViewState);

            QModelIndex mi = projectTreeView->treeView()->currentIndex();

            QString encodedModelIndexString;
            caf::QTreeViewStateSerializer::encodeStringFromModelIndex(mi, encodedModelIndexString);

            project()->plotWindowTreeViewState = treeViewState;
            project()->plotWindowCurrentModelIndexPath = encodedModelIndexString;
        }
    }

    {
        caf::PdmUiTreeView* projectTreeView = RiuMainWindow::instance()->projectTreeView();
        if (projectTreeView)
        {
            QString treeViewState;
            caf::QTreeViewStateSerializer::storeTreeViewStateToString(projectTreeView->treeView(), treeViewState);

            QModelIndex mi = projectTreeView->treeView()->currentIndex();

            QString encodedModelIndexString;
            caf::QTreeViewStateSerializer::encodeStringFromModelIndex(mi, encodedModelIndexString);

            project()->mainWindowTreeViewState = treeViewState;
            project()->mainWindowCurrentModelIndexPath = encodedModelIndexString;
        }
    }
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

        m_project->updateConnectedEditors();
    }

    if (oilField->wellPathCollection) oilField->wellPathCollection->addWellPaths(wellPathFilePaths);
    
    oilField->wellPathCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::addWellPathFormationsToModel(QList<QString> wellPathFormationsFilePaths)
{
    if (m_project == nullptr || m_project->oilFields.size() < 1) return;

    RimOilField* oilField = m_project->activeOilField();
    if (oilField == nullptr) return;

    if (oilField->wellPathCollection == nullptr)
    {
        oilField->wellPathCollection = new RimWellPathCollection();

        m_project->updateConnectedEditors();
    }

    if (oilField->wellPathCollection)
    {
        oilField->wellPathCollection->addWellPathFormations(wellPathFormationsFilePaths);
    }

    oilField->wellPathCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Add a list of well log file paths (LAS files) to the well path collection
//--------------------------------------------------------------------------------------------------
void RiaApplication::addWellLogsToModel(const QList<QString>& wellLogFilePaths)
{
    if (m_project == nullptr || m_project->oilFields.size() < 1) return;

    RimOilField* oilField = m_project->activeOilField();
    if (oilField == nullptr) return;

    if (oilField->wellPathCollection == nullptr)
    {
        oilField->wellPathCollection = new RimWellPathCollection();

        m_project->updateConnectedEditors();
    }

    RimWellLogFile* wellLogFile = oilField->wellPathCollection->addWellLogs(wellLogFilePaths);

    oilField->wellPathCollection->updateConnectedEditors();
    
    RiuMainWindow::instance()->selectAsCurrentItem(wellLogFile);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProject()
{
    CVF_ASSERT(m_project.notNull());

    if (!caf::Utils::fileExists(m_project->fileName()))
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
        startPath = m_project->fileName();
    }
    else
    {
        startPath = app->lastUsedDialogDirectory("BINARY_GRID");
        startPath += "/ResInsightProject.rsp";
    }

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"), startPath, tr("Project Files (*.rsp);;All files(*.*)"));
    if (fileName.isEmpty())
    {
        return false;
    }

    // Remember the directory to next time
    app->setLastUsedDialogDirectory("BINARY_GRID", QFileInfo(fileName).absolutePath());

    bool bSaveOk = saveProjectAs(fileName);

    setWindowCaptionFromAppState();

    return bSaveOk;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::hasValidProjectFileExtension(const QString& fileName)
{
    if (fileName.contains(".rsp", Qt::CaseInsensitive) || fileName.contains(".rip", Qt::CaseInsensitive))
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::askUserToSaveModifiedProject()
{
    if (caf::PdmUiModelChangeDetector::instance()->isModelChanged())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);

        QString questionText;
        questionText = QString("The current project is modified.\n\nDo you want to save the changes?");

        msgBox.setText(questionText);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::Yes)
        {
            if (!saveProject())
            {
                return false;
            }
        }
        else
        {
            caf::PdmUiModelChangeDetector::instance()->reset();
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::saveProjectAs(const QString& fileName)
{
    m_project->fileName = fileName;

    storeTreeViewState();

    if (!m_project->writeFile())
    {
        QMessageBox::warning(NULL, "Error when saving project file", QString("Not possible to save project file. Make sure you have sufficient access rights.\n\nProject file location : %1").arg(fileName));

        return false;
    }

    m_preferences->lastUsedProjectFileName = fileName;
    caf::PdmSettings::writeFieldsToApplicationStore(m_preferences);

    m_recentFileActionProvider->addFileName(fileName);

    caf::PdmUiModelChangeDetector::instance()->reset();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::closeProject()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();

    clearViewsScheduledForUpdate();

    terminateProcess();

    RiuSelectionManager::instance()->deleteAllItems();

    mainWnd->cleanupGuiBeforeProjectClose();

    if (m_mainPlotWindow)
    {
        m_mainPlotWindow->cleanupGuiBeforeProjectClose();
    }

    caf::EffectGenerator::clearEffectCache();
    m_project->close();

    m_commandQueue.clear();

    onProjectOpenedOrClosed();

    // Make sure all project windows are closed down properly before returning
    processEvents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::onProjectOpenedOrClosed()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (mainWnd)
    {
        mainWnd->initializeGuiNewProjectLoaded();
    }
    if (m_mainPlotWindow)
    {
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
    }

    setWindowCaptionFromAppState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::currentProjectPath() const
{
    QString projectFolder;
    if (m_project)
    {
        QString projectFileName = m_project->fileName();

        if (!projectFileName.isEmpty())
        {
            QFileInfo fi(projectFileName);
            projectFolder = fi.absolutePath();
        }
    }

    return projectFolder;
}

//--------------------------------------------------------------------------------------------------
/// Create an absolute path from a path that is specified relative to the project directory
/// 
/// If the path specified in \a projectRelativePath is already absolute, no changes will be made
//--------------------------------------------------------------------------------------------------
QString RiaApplication::createAbsolutePathFromProjectRelativePath(QString projectRelativePath)
{
    // Check if path is already absolute
    if (QDir::isAbsolutePath(projectRelativePath))
    {
        return projectRelativePath;
    }

    QString absolutePath;
    if (m_project && !m_project->fileName().isEmpty())
    {
        absolutePath = QFileInfo(m_project->fileName()).absolutePath();
    }
    else
    {
        absolutePath = this->lastUsedDialogDirectory("BINARY_GRID");
    }

    QDir projectDir(absolutePath);

    return projectDir.absoluteFilePath(projectRelativePath);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openOdbCaseFromFile(const QString& fileName)
{
    if (!caf::Utils::fileExists(fileName)) return false;

    QFileInfo gridFileName(fileName);
    QString caseName = gridFileName.completeBaseName();

    RimGeoMechCase* geoMechCase = new RimGeoMechCase();
    geoMechCase->setFileName(fileName);
    geoMechCase->caseUserDescription = caseName;

    RimGeoMechModels* geoMechModelCollection = m_project->activeOilField() ? m_project->activeOilField()->geoMechModels() : NULL;

    // Create the geoMech model container if it is not there already
    if (geoMechModelCollection == NULL)
    {
        geoMechModelCollection = new RimGeoMechModels();
        m_project->activeOilField()->geoMechModels = geoMechModelCollection;
    }

    geoMechModelCollection->cases.push_back(geoMechCase);

    RimGeoMechView* riv = geoMechCase->createAndAddReservoirView();
    caf::ProgressInfo progress(11, "Loading Case");
    progress.setNextProgressIncrement(10);

    riv->loadDataAndUpdate();

    //if (!riv->cellResult()->hasResult())
    //{
    //    riv->cellResult()->setResultVariable(RiaDefines::undefinedResultName());
    //}
    progress.incrementProgress();
    progress.setProgressDescription("Loading results information");

    m_project->updateConnectedEditors();

    RiuMainWindow::instance()->selectAsCurrentItem(riv->cellResult());
    
    return true;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMockModel()
{
    RiaImportEclipseCaseTools::openMockModel(RiaDefines::mockModelBasic());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createResultsMockModel()
{
    RiaImportEclipseCaseTools::openMockModel(RiaDefines::mockModelBasicWithResults());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createLargeResultsMockModel()
{
    RiaImportEclipseCaseTools::openMockModel(RiaDefines::mockModelLargeWithResults());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMockModelCustomized()
{
    RiaImportEclipseCaseTools::openMockModel(RiaDefines::mockModelCustomized());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createInputMockModel()
{
    RicImportInputEclipseCaseFeature::openInputEclipseCaseFromFileNames(QStringList(RiaDefines::mockModelBasicInputCase()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const Rim3dView* RiaApplication::activeReservoirView() const
{
    return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RiaApplication::activeReservoirView()
{
   return m_activeReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiaApplication::activePlotWindow() const
{
    RimViewWindow* viewWindow = nullptr;

    if ( m_mainPlotWindow )
    {
        QList<QMdiSubWindow*> subwindows = m_mainPlotWindow->subWindowList(QMdiArea::StackingOrder);
        if ( subwindows.size() > 0 )
        {
            viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget(subwindows.back()->widget());
        }
    }

    return viewWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setActiveReservoirView(Rim3dView* rv)
{
    m_activeReservoirView = rv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setUseShaders(bool enable)
{
    m_preferences->useShaders = enable;
    caf::PdmSettings::writeFieldsToApplicationStore(m_preferences);
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
    caf::PdmSettings::writeFieldsToApplicationStore(m_preferences);
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
    bool result = RiaArgumentParser::parseArguments();
    if (!result)
    {
        closeAllWindows();
        processEvents();
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiaApplication::launchUnitTests()
{
#ifdef USE_UNIT_TESTS
    cvf::Assert::setReportMode(cvf::Assert::CONSOLE);

    int argc = QCoreApplication::argc();
    testing::InitGoogleTest(&argc, QCoreApplication::argv());

    // Use this macro in main() to run all tests.  It returns 0 if all
    // tests are successful, or 1 otherwise.
    //
    // RUN_ALL_TESTS() should be invoked after the command line has been
    // parsed by InitGoogleTest().

    return RUN_ALL_TESTS();
#else
    return -1;
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiaApplication::launchUnitTestsWithConsole()
{
    // Following code is taken from cvfAssert.cpp
#ifdef WIN32
    {
        // Allocate a new console for this app
        // Only one console can be associated with an app, so should fail if a console is already present.
        AllocConsole();

        FILE* consoleFilePointer;

        freopen_s(&consoleFilePointer, "CONOUT$", "w", stdout);
        freopen_s(&consoleFilePointer, "CONOUT$", "w", stderr);

        // Make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
        std::ios::sync_with_stdio();
    }
#endif

    return launchUnitTests();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMainPlotWindow()
{
    CVF_ASSERT(m_mainPlotWindow == NULL);

    m_mainPlotWindow = new RiuMainPlotWindow;

    m_mainPlotWindow->setWindowTitle("Plots - ResInsight");
    m_mainPlotWindow->setDefaultWindowSize();
    m_mainPlotWindow->loadWinGeoAndDockToolBarLayout();
    m_mainPlotWindow->showWindow();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::deleteMainPlotWindow()
{
    if (m_mainPlotWindow)
    {
        m_mainPlotWindow->deleteLater();
        m_mainPlotWindow = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainPlotWindow* RiaApplication::getOrCreateAndShowMainPlotWindow()
{
    if (!m_mainPlotWindow)
    {
        createMainPlotWindow();
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
        loadAndUpdatePlotData();
    }

    if (m_mainPlotWindow->isMinimized())
    {
        m_mainPlotWindow->showNormal();
        m_mainPlotWindow->update();
    }
    else
    {
        m_mainPlotWindow->show();
    }

    m_mainPlotWindow->raise();

    return m_mainPlotWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainPlotWindow* RiaApplication::mainPlotWindow()
{
    return m_mainPlotWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RiaApplication::mainWindowByID(int mainWindowID)
{
    if (mainWindowID == 0) return RiuMainWindow::instance();
    else if (mainWindowID == 1) return m_mainPlotWindow;
    else return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiaApplication::activeViewWindow()
{
    RimViewWindow* viewWindow = NULL;

    QWidget* mainWindowWidget = RiaApplication::activeWindow();

    if (dynamic_cast<RiuMainWindow*>(mainWindowWidget))
    {
        viewWindow = RiaApplication::instance()->activeReservoirView();
    }
    else if (dynamic_cast<RiuMainPlotWindow*>(mainWindowWidget))
    {
        RiuMainPlotWindow* mainPlotWindow = dynamic_cast<RiuMainPlotWindow*>(mainWindowWidget);

        QList<QMdiSubWindow*> subwindows = mainPlotWindow->subWindowList(QMdiArea::StackingOrder);
        if (subwindows.size() > 0)
        {
            viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget(subwindows.back()->widget());
        }
    }

    return viewWindow;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::isMain3dWindowVisible() const
{
    return RiuMainWindow::instance()->isVisible();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::isMainPlotWindowVisible() const
{
    if (!m_mainPlotWindow) return false;

    return m_mainPlotWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::tryCloseMainWindow()
{
    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if (mainWindow && !mainWindow->isVisible())
    {
        mainWindow->close();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::tryClosePlotWindow()
{
    if (!m_mainPlotWindow)
    {
        return true;
    }

    if (m_mainPlotWindow && !m_mainPlotWindow->isVisible())
    {
        m_mainPlotWindow->close();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::addToRecentFiles(const QString& fileName)
{
    m_recentFileActionProvider->addFileName(fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QAction*> RiaApplication::recentFileActions() const
{
    return m_recentFileActionProvider->actions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setStartDir(const QString& startDir)
{
    m_startupDefaultDirectory = startDir;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiaApplication::readFileListFromTextFile(QString listFileName)
{
    std::vector<QString> fileList;

    QFile file(listFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return fileList;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull())
    {
        line = line.trimmed();
        if (!line.isEmpty())
        {
            fileList.push_back(line);
        }

        line = in.readLine();
    }

    return fileList;
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
QStringList RiaApplication::octaveArguments() const
{
    // http://www.gnu.org/software/octave/doc/interpreter/Command-Line-Options.html#Command-Line-Options

    // -p path
    // Add path to the head of the search path for function files. The value of path specified on the command line
    // will override any value of OCTAVE_PATH found in the environment, but not any commands in the system or
    // user startup files that set the internal load path through one of the path functions.


    QStringList arguments;
    arguments.append("--path");
    arguments << QApplication::applicationDirPath();

    if (!m_preferences->octaveShowHeaderInfoWhenExecutingScripts)
    {
        // -q
        // Don't print the usual greeting and version message at startup.

        arguments.append("-q");
    }

    return arguments;
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
        m_runningWorkerProcess = false;
        return;
    }


    executeCommandObjects();


    // Exit code != 0 means we have an error
    if (exitCode != 0)
    {
      //  MFLog::error(QString("Simulation execution failed (exit code %1).").arg(exitCode));
        m_runningWorkerProcess = false;
        return;
    }

    // If multiple cases are present, invoke launchProcess() which will set next current case, and run script on this case 
    if (!m_currentCaseIds.empty())
    {
        launchProcess(m_currentProgram, m_currentArguments);
    }
    else
    {
        // Disable concept of current case
        m_socketServer->setCurrentCaseId(-1);
        m_runningWorkerProcess = false;
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
        if (!m_currentCaseIds.empty())
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

        m_runningWorkerProcess = true;
        m_workerProcess = new caf::UiProcess(this);

        QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();

#ifdef WIN32
        // Octave plugins compiled by ResInsight are dependent on Qt (currently Qt 32-bit only)
        // Some Octave installations for Windows have included Qt, and some don't. To make sure these plugins always can be executed, 
        // the path to octave_plugin_dependencies is added to global path
        
        QString pathString = penv.value("PATH", "");

        if (pathString == "") pathString = QApplication::applicationDirPath() + "\\octave_plugin_dependencies";
        else pathString = QApplication::applicationDirPath() + "\\octave_plugin_dependencies" + ";" + pathString;

        penv.insert("PATH", pathString);
#else
            // Set the LD_LIBRARY_PATH to make the octave plugins find the embedded Qt
            QString ldPath = penv.value("LD_LIBRARY_PATH", "");

            if (ldPath == "") ldPath = QApplication::applicationDirPath();
            else ldPath = QApplication::applicationDirPath() + ":" + ldPath;

            penv.insert("LD_LIBRARY_PATH", ldPath);
#endif

        m_workerProcess->setProcessEnvironment(penv);

        connect(m_workerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotWorkerProcessFinished(int, QProcess::ExitStatus)));

        RiuMainWindow::instance()->processMonitor()->startMonitorWorkProcess(m_workerProcess);

        m_workerProcess->start(program, arguments);
        if (!m_workerProcess->waitForStarted(1000))
        {
            m_workerProcess->close();
            m_workerProcess = NULL;
            m_runningWorkerProcess = false;

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
        m_activeReservoirView->viewer()->updateNavigationPolicy();
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

    if (RiuMainWindow::instance() && RiuMainWindow::instance()->projectTreeView())
    {
        RiuMainWindow::instance()->projectTreeView()->enableAppendOfClassNameToUiItemText(m_preferences->appendClassNameToUiText());
        if (mainPlotWindow()) mainPlotWindow()->projectTreeView()->enableAppendOfClassNameToUiItemText(m_preferences->appendClassNameToUiText());
    }

    caf::FixedAtlasFont::FontSize fontSizeType = caf::FixedAtlasFont::POINT_SIZE_16;
    if (m_preferences->fontSizeInScene() == "8")
    {
        fontSizeType = caf::FixedAtlasFont::POINT_SIZE_8;
    }
    else if (m_preferences->fontSizeInScene() == "12")
    {
        fontSizeType = caf::FixedAtlasFont::POINT_SIZE_12;
    }
    else if (m_preferences->fontSizeInScene() == "16")
    {
        fontSizeType = caf::FixedAtlasFont::POINT_SIZE_16;
    }
    else if (m_preferences->fontSizeInScene() == "24")
    {
        fontSizeType = caf::FixedAtlasFont::POINT_SIZE_24;
    }
    else if (m_preferences->fontSizeInScene() == "32")
    {
        fontSizeType = caf::FixedAtlasFont::POINT_SIZE_32;
    }
    
    m_customFont = new caf::FixedAtlasFont(fontSizeType);

    if (this->project())
    {
        this->project()->setScriptDirectories(m_preferences->scriptDirectories());
        this->project()->updateConnectedEditors();

        std::vector<Rim3dView*> visibleViews;
        this->project()->allVisibleViews(visibleViews);

        for (auto view : visibleViews)
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(view);
            if (eclipseView)
            {
                eclipseView->scheduleReservoirGridGeometryRegen();
            }
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    caf::PdmUiItem::enableExtraDebugText(m_preferences->appendFieldKeywordToToolTipText());
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

    m_runningWorkerProcess = false;
    m_workerProcess = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::waitForProcess() const
{
    while (m_runningWorkerProcess)
    {
#ifdef WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
        processEvents();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::lastUsedDialogDirectory(const QString& dialogName)
{
    QString lastUsedDirectory = m_startupDefaultDirectory;

    auto it = m_fileDialogDefaultDirectories.find(dialogName);
    if (it != m_fileDialogDefaultDirectories.end())
    {
        lastUsedDirectory = it->second;
    }

    return lastUsedDirectory;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaApplication::lastUsedDialogDirectoryWithFallback(const QString& dialogName, const QString& fallbackDirectory)
{
    QString lastUsedDirectory = m_startupDefaultDirectory;
    if (!fallbackDirectory.isEmpty())
    {
        lastUsedDirectory = fallbackDirectory;
    }

    auto it = m_fileDialogDefaultDirectories.find(dialogName);
    if (it != m_fileDialogDefaultDirectories.end())
    {
        lastUsedDirectory = it->second;
    }

    return lastUsedDirectory;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setLastUsedDialogDirectory(const QString& dialogName, const QString& directory)
{
    m_fileDialogDefaultDirectories[dialogName] = directory;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::openFile(const QString& fileName)
{
    if (!caf::Utils::fileExists(fileName)) return false;

    bool loadingSucceded = false;

    if (RiaApplication::hasValidProjectFileExtension(fileName))
    {
        loadingSucceded = loadProject(fileName);
    }
    else if (fileName.contains(".egrid", Qt::CaseInsensitive) || fileName.contains(".grid", Qt::CaseInsensitive))
    {
        loadingSucceded = RiaImportEclipseCaseTools::openEclipseCaseFromFile(fileName);
    }
    else if (fileName.contains(".grdecl", Qt::CaseInsensitive))
    {
        loadingSucceded = RicImportInputEclipseCaseFeature::openInputEclipseCaseFromFileNames(QStringList(fileName));
    }
    else if (fileName.contains(".odb", Qt::CaseInsensitive))
    {
        loadingSucceded = openOdbCaseFromFile(fileName);
    }
    else if (fileName.contains(".smspec", Qt::CaseInsensitive))
    {
        loadingSucceded = RicImportSummaryCaseFeature::createAndAddSummaryCaseFromFile(fileName);
        if (loadingSucceded)
        {
            getOrCreateAndShowMainPlotWindow();

            std::vector<RimCase*> cases;
            m_project->allCases(cases);

            if (cases.size() == 0)
            {
                RiuMainWindow::instance()->close();
            }

            m_project->updateConnectedEditors();
        }
    }

    if (loadingSucceded && !RiaApplication::hasValidProjectFileExtension(fileName))
    {
        caf::PdmUiModelChangeDetector::instance()->setModelChanged();
    }

    return loadingSucceded;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::runMultiCaseSnapshots(const QString& templateProjectFileName, std::vector<QString> gridFileNames, const QString& snapshotFolderName)
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    mainWnd->hideAllDockWindows();

    const size_t numGridFiles = gridFileNames.size();
    for (size_t i = 0; i < numGridFiles; i++)
    {
        QString gridFn = gridFileNames[i];

        RiaProjectModifier modifier;
        modifier.setReplaceCaseFirstOccurrence(gridFn);

        bool loadOk = loadProject(templateProjectFileName, PLA_NONE, &modifier);
        if (loadOk)
        {
            RicSnapshotAllViewsToFileFeature::exportSnapshotOfAllViewsIntoFolder(snapshotFolderName);
        }
    }

    mainWnd->loadWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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
void logInfoTextWithTimeInSeconds(const QTime& time, const QString& msg)
{
    double timeRunning = time.elapsed() / 1000.0;

    QString timeText = QString("(%1 s) ").arg(timeRunning, 0, 'f', 1);

    RiaLogging::info(timeText + msg);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::runRegressionTest(const QString& testRootPath, QStringList* testFilter)
{
    m_runningRegressionTests = true;

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

    if (testFilter && testFilter->size() > 0)
    {
        QFileInfoList subset;

        for (auto fi : folderList)
        {
            QString path = fi.path();
            QString baseName = fi.baseName();

            for (auto s : *testFilter)
            {
                QString trimmed = s.trimmed();
                if (baseName.contains(trimmed))
                {
                    subset.push_back(fi);
                }
            }
        }

        folderList = subset;
    }

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

    // Minor workaround
    // Use registry to define if interactive diff images should be created
    // Defined by user in RiaRegressionTest
    {
        QSettings settings;

        bool useInteractiveDiff = settings.value("showInteractiveDiffImages").toBool();
        if (useInteractiveDiff)
        {
            imageCompareReporter.showInteractiveOnly();
        }
    }

    QTime timeStamp;
    timeStamp.start();

    logInfoTextWithTimeInSeconds(timeStamp, "Starting regression tests\n");

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());

        QString testFolderName = testCaseFolder.dirName();
        QString reportBaseFolderName       = testCaseFolder.filePath(baseFolderName);
        QString reportGeneratedFolderName  = testCaseFolder.filePath(generatedFolderName);
        QString reportDiffFolderName       = testCaseFolder.filePath(diffFolderName);

        imageCompareReporter.addImageDirectoryComparisonSet(testFolderName.toStdString(), reportBaseFolderName.toStdString(), reportGeneratedFolderName.toStdString(), reportDiffFolderName.toStdString());
    }

    QString htmlReportFileName = testDir.filePath(RegTestNames::reportFileName);
    imageCompareReporter.generateHTMLReport(htmlReportFileName.toStdString());

    // Open HTML report
    QDesktopServices::openUrl(htmlReportFileName);

    for (int dirIdx = 0; dirIdx < folderList.size(); ++dirIdx)
    {
        QDir testCaseFolder(folderList[dirIdx].filePath());

        QString projectFileName;
        
        if (testCaseFolder.exists(regTestProjectName + ".rip"))
        {
            projectFileName = regTestProjectName + ".rip";
        }

        if (testCaseFolder.exists(regTestProjectName + ".rsp"))
        {
            projectFileName = regTestProjectName + ".rsp";
        }

        if (!projectFileName.isEmpty())
        {
            logInfoTextWithTimeInSeconds(timeStamp, "Initializing test :" + testCaseFolder.absolutePath());

            loadProject(testCaseFolder.filePath(projectFileName));

            // Wait until all command objects have completed
            while (!m_commandQueueLock.tryLock())
            {
                processEvents();
            }
            m_commandQueueLock.unlock();

            regressionTestConfigureProject();

            resizeMaximizedPlotWindows();

            QString fullPathGeneratedFolder = testCaseFolder.absoluteFilePath(generatedFolderName);
            RicSnapshotAllViewsToFileFeature::exportSnapshotOfAllViewsIntoFolder(fullPathGeneratedFolder);

            RicSnapshotAllPlotsToFileFeature::exportSnapshotOfAllPlotsIntoFolder(fullPathGeneratedFolder);

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

            closeProject();
        
            logInfoTextWithTimeInSeconds(timeStamp, "Completed test :" + testCaseFolder.absolutePath());
        }
    }

    RiaLogging::info("\n");
    logInfoTextWithTimeInSeconds(timeStamp, "Completed regression tests");

    m_runningRegressionTests = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::resizeMaximizedPlotWindows()
{
    std::vector<RimViewWindow*> viewWindows;
    m_project->mainPlotCollection()->descendantsIncludingThisOfType(viewWindows);

    for (auto viewWindow : viewWindows)
    {
        if (viewWindow->isMdiWindow())
        {
            RimMdiWindowGeometry wndGeo = viewWindow->mdiWindowGeometry();
            if (wndGeo.isMaximized)
            {
                QWidget* viewWidget = viewWindow->viewWidget();

                if (viewWidget)
                {
                    QMdiSubWindow* mdiWindow = m_mainPlotWindow->findMdiSubWindow(viewWidget);
                    if (mdiWindow)
                    {
                        mdiWindow->showNormal();

                        viewWidget->resize(RiaApplication::regressionDefaultImageSize());
                    }
                }
            }
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
cvf::Font* RiaApplication::customFont()
{
    CVF_ASSERT(m_customFont.notNull());

    return m_customFont.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::clearViewsScheduledForUpdate()
{
    if (m_resViewUpdateTimer)
    {
        while (m_resViewUpdateTimer->isActive())
        {
            QCoreApplication::processEvents();
        }
    }
    m_resViewsToUpdate.clear();
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
    return m_helpText;

    /*
    QString text = QString("\n%1 v. %2\n").arg(RI_APPLICATION_NAME).arg(getVersionStringApp(false));
    text += "Copyright Statoil ASA, Ceetron AS 2011, 2012\n\n";

    text +=
        "\nParameter              Description\n"
        "-----------------------------------------------------------------\n"
        "-last                    Open last used project\n"
        "\n"
        "-project <filename>      Open project file <filename>\n"
        "\n"
        "-case <casename>         Import Eclipse case <casename>\n"
        "                         (do not include .GRID/.EGRID)\n"
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
    */
}

//--------------------------------------------------------------------------------------------------
/// Schedule a creation of the Display model and redraw of the reservoir view
/// The redraw will happen as soon as the event loop is entered
//--------------------------------------------------------------------------------------------------
void RiaApplication::scheduleDisplayModelUpdateAndRedraw(Rim3dView* resViewToUpdate)
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
void RiaApplication::scheduleRecalculateCompletionTypeAndRedrawAllViews()
{
    for (RimEclipseCase* eclipseCase : project()->activeOilField()->analysisModels->cases())
    {
        m_eclipseCasesToRecalculate.push_back(eclipseCase);
    }

    if (!m_recalculateCompletionTypeTimer)
    {
        m_recalculateCompletionTypeTimer = new QTimer(this);
        m_recalculateCompletionTypeTimer->setSingleShot(true);
        connect(m_recalculateCompletionTypeTimer, SIGNAL(timeout()), this, SLOT(slotRecalculateCompletionType()));
    }

    m_recalculateCompletionTypeTimer->start(1500);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::scheduleRecalculateCompletionTypeAndRedrawEclipseCase(RimEclipseCase* eclipseCase)
{
    m_eclipseCasesToRecalculate.push_back(eclipseCase);


    if (!m_recalculateCompletionTypeTimer)
    {
        m_recalculateCompletionTypeTimer = new QTimer(this);
        m_recalculateCompletionTypeTimer->setSingleShot(true);
        connect(m_recalculateCompletionTypeTimer, SIGNAL(timeout()), this, SLOT(slotRecalculateCompletionType()));
    }

    m_recalculateCompletionTypeTimer->start(500);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::slotUpdateScheduledDisplayModels()
{
    // Compress to remove duplicates
    // and update dependent views after independent views

    std::set<Rim3dView*> independent3DViewsToUpdate;
    std::set<Rim3dView*> dependent3DViewsToUpdate;

    for (size_t i = 0; i < m_resViewsToUpdate.size(); ++i)
    {
        if (!m_resViewsToUpdate[i]) continue;

        if (m_resViewsToUpdate[i]->viewController())
            dependent3DViewsToUpdate.insert(m_resViewsToUpdate[i]);
        else
            independent3DViewsToUpdate.insert(m_resViewsToUpdate[i]);
    }
   
    for (std::set<Rim3dView*>::iterator it = independent3DViewsToUpdate.begin(); it != independent3DViewsToUpdate.end(); ++it )
    {
        if (*it)
        {
            (*it)->createDisplayModelAndRedraw();
        }
    }

    for (std::set<Rim3dView*>::iterator it = dependent3DViewsToUpdate.begin(); it != dependent3DViewsToUpdate.end(); ++it)
    {
        if (*it)
        {
            (*it)->createDisplayModelAndRedraw();
        }
    }

    m_resViewsToUpdate.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::slotRecalculateCompletionType()
{
    std::set<RimEclipseCase*> uniqueCases(m_eclipseCasesToRecalculate.begin(), m_eclipseCasesToRecalculate.end());

    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    QModelIndex mi = RiuMainWindow::instance()->projectTreeView()->treeView()->currentIndex();

    for (RimEclipseCase* eclipseCase : uniqueCases)
    {
        eclipseCase->recalculateCompletionTypeAndRedrawAllViews();
    }

    m_eclipseCasesToRecalculate.clear();

    // Recalculation of completion type causes active view to be set to potentially a different view
    // Also current index in project tree is changed. Restore both to initial state.

    if (activeView && activeView->viewer())
    {
        RiaApplication::instance()->setActiveReservoirView(activeView);
        RiuMainWindow::instance()->setActiveViewer(activeView->viewer()->layoutWidget());
    }

    if (mi.isValid())
    {
        RiuMainWindow::instance()->projectTreeView()->treeView()->setCurrentIndex(mi);
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

            ++it;
            m_commandQueue.remove(toBeRemoved);
        }
        else
        {
            ++it;
        }
    }

    if (!m_commandQueue.empty())
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaApplication::isRunningRegressionTests() const
{
    return m_runningRegressionTests;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::executeRegressionTests(const QString& regressionTestPath, QStringList* testFilter)
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (mainWnd)
    {
        mainWnd->hideAllDockWindows();
 
        mainWnd->setDefaultWindowSize();
        runRegressionTest(regressionTestPath, testFilter);

        mainWnd->loadWinGeoAndDockToolBarLayout();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::regressionTestConfigureProject()
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    if (!mainWnd) return;

    if (m_project.isNull()) return;

    std::vector<RimCase*> projectCases;
    m_project->allCases(projectCases);

    for (size_t i = 0; i < projectCases.size(); i++)
    {
        RimCase* cas = projectCases[i];
        if (!cas) continue;

        std::vector<Rim3dView*> views = cas->views();

        for (size_t j = 0; j < views.size(); j++)
        {
            Rim3dView* riv = views[j];

            if (riv && riv->viewer())
            {
                // Make sure all views are maximized for snapshotting
                QMdiSubWindow* subWnd = mainWnd->findMdiSubWindow(riv->viewer()->layoutWidget());
                if (subWnd)
                {
                    subWnd->showMaximized();
                }

                // This size is set to match the regression test reference images
                riv->viewer()->setFixedSize(RiaApplication::regressionDefaultImageSize());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiaApplication::regressionDefaultImageSize()
{
    return QSize(1000, 745);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaApplication::setHelpText(const QString& helpText)
{
    m_helpText = helpText;
}
