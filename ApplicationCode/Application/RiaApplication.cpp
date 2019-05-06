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

#include "RiaArgumentParser.h"
#include "RiaBaseDefs.h"
#include "RiaColorTables.h"
#include "RiaFilePathTools.h"
#include "RiaFontCache.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaProjectModifier.h"
#include "RiaSocketServer.h"
#include "RiaVersionInfo.h"
#include "RiaViewRedrawScheduler.h"

#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "HoloLensCommands/RicHoloLensSessionManager.h"
#include "RicImportGeneralDataFeature.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationTextAppearance.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCommandObject.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNamesCollection.h"
#include "RimFractureTemplateCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechModels.h"
#include "RimGeoMechView.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimPltPlotCollection.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSaturationPressurePlot.h"
#include "RimSaturationPressurePlotCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCrossPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimTextAnnotation.h"
#include "RimTextAnnotationInView.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "Riu3dSelectionManager.h"
#include "RiuDockWidgetTools.h"
#include "RiuMainWindow.h"
#include "RiuMessagePanel.h"
#include "RiuPlotMainWindow.h"
#include "RiuProcessMonitor.h"
#include "RiuRecentFileActionProvider.h"
#include "RiuViewer.h"

#include "cafAppEnum.h"
#include "cafEffectGenerator.h"
#include "cafFixedAtlasFont.h"
#include "cafPdmSettings.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafPdmUiTreeView.h"
#include "cafProgressInfo.h"
#include "cafQTreeViewStateSerializer.h"
#include "cafSelectionManager.h"
#include "cafUiProcess.h"
#include "cafUtils.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include <QDir>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QTreeView>

#ifndef WIN32
#include <unistd.h> // for usleep
#endif // WIN32

#ifdef USE_UNIT_TESTS
#include "gtest/gtest.h"
#endif // USE_UNIT_TESTS

namespace caf
{
template<>
void AppEnum<RiaApplication::RINavigationPolicy>::setUp()
{
    addItem(RiaApplication::NAVIGATION_POLICY_CEETRON, "NAVIGATION_POLICY_CEETRON", "Ceetron");
    addItem(RiaApplication::NAVIGATION_POLICY_CAD, "NAVIGATION_POLICY_CAD", "CAD");
    addItem(RiaApplication::NAVIGATION_POLICY_GEOQUEST, "NAVIGATION_POLICY_GEOQUEST", "GEOQUEST");
    addItem(RiaApplication::NAVIGATION_POLICY_RMS, "NAVIGATION_POLICY_RMS", "RMS");
    setDefault(RiaApplication::NAVIGATION_POLICY_RMS);
}
} // namespace caf

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
bool RiaApplication::notify(QObject* receiver, QEvent* event)
{
    // Pre-allocating a memory exhaustion message
    // Doing som e trickery to avoid deadlock, as creating a messagebox actually triggers a call to this notify method.

    static QMessageBox* memoryExhaustedBox   = nullptr;
    static bool         allocatingMessageBox = false;
    if (!memoryExhaustedBox && !allocatingMessageBox)
    {
        allocatingMessageBox = true;
        memoryExhaustedBox   = new QMessageBox(QMessageBox::Critical,
                                             "ResInsight Exhausted Memory",
                                             "Memory is Exhausted!\n ResInsight could not allocate the memory needed, and is now "
                                             "unstable and will probably crash soon.");
    }

    bool done = true;
    try
    {
        done = QApplication::notify(receiver, event);
    }
    catch (const std::bad_alloc&)
    {
        if (memoryExhaustedBox) memoryExhaustedBox->exec();
        std::cout << "ResInsight: Memory is Exhausted!\n ResInsight could not allocate the memory needed, and is now unstable "
                     "and will probably crash soon."
                  << std::endl;
        // If we really want to crash instead of limping forward:
        // throw;
    }

    return done;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::RiaApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_socketServer(nullptr)
    , m_workerProcess(nullptr)
    , m_preferences(nullptr)
    , m_runningWorkerProcess(false)
    , m_mainWindow(nullptr)
    , m_mainPlotWindow(nullptr)
{
    // USed to get registry settings in the right place
    QCoreApplication::setOrganizationName(RI_COMPANY_NAME);
    QCoreApplication::setApplicationName(RI_APPLICATION_NAME);

    // For idle processing
    //    m_idleTimerStarted = false;
    installEventFilter(this);

    // cvf::Trace::enable(false);

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

    m_socketServer = new RiaSocketServer(this);

#ifdef WIN32
    m_startupDefaultDirectory = QDir::homePath();
#else
    m_startupDefaultDirectory = QDir::currentPath();
#endif

    setLastUsedDialogDirectory("MULTICASEIMPORT", "/");

    m_recentFileActionProvider = std::unique_ptr<RiuRecentFileActionProvider>(new RiuRecentFileActionProvider);

    // Create main windows
    // The plot window is created to be able to set expanded state on created objects, but hidden by default
    getOrCreateAndShowMainWindow();
    getOrCreateMainPlotWindow();

    RiaLogging::setLoggerInstance(new RiuMessagePanelLogger(m_mainWindow->messagePanel()));
    RiaLogging::loggerInstance()->setLevel(RI_LL_DEBUG);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::~RiaApplication()
{
    RiuDockWidgetTools::instance()->saveDockWidgetsState();

    deleteMainPlotWindow();
    deleteMainWindow();

    delete m_preferences;

    RiaLogging::deleteLoggerInstance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication* RiaApplication::instance()
{
    return static_cast<RiaApplication*> qApp;
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
    if (!m_mainWindow) return;

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

    m_mainWindow->setWindowTitle(capt);
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
class RiaMdiMaximizeWindowGuard
{
public:
    RiaMdiMaximizeWindowGuard()
    {
        {
            RiuMainWindow* mainWindow = RiaApplication::instance()->mainWindow();
            if (mainWindow)
            {
                mainWindow->enableShowFirstVisibleMdiWindowMaximized(false);
            }
        }

        {
            RiuPlotMainWindow* plotMainWindow = RiaApplication::instance()->mainPlotWindow();
            if (plotMainWindow)
            {
                plotMainWindow->enableShowFirstVisibleMdiWindowMaximized(false);
            }
        }
    }

    ~RiaMdiMaximizeWindowGuard()
    {
        {
            RiuMainWindow* mainWindow = RiaApplication::instance()->mainWindow();
            if (mainWindow)
            {
                mainWindow->enableShowFirstVisibleMdiWindowMaximized(true);
            }
        }

        {
            RiuPlotMainWindow* plotMainWindow = RiaApplication::instance()->mainPlotWindow();
            if (plotMainWindow)
            {
                plotMainWindow->enableShowFirstVisibleMdiWindowMaximized(true);
            }
        }
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::loadProject(const QString&      projectFileName,
                                 ProjectLoadAction   loadAction,
                                 RiaProjectModifier* projectModifier)
{
    // First Close the current project

    closeProject();

    // When importing a project, do not maximize the first MDI window to be created
    RiaMdiMaximizeWindowGuard maximizeWindowGuard;

    RiaLogging::info(QString("Starting to open project file : '%1'").arg(projectFileName));

    // Create a absolute path file name, as this is required for update of file references in the project modifier object
    QString fullPathProjectFileName = caf::Utils::absoluteFileName(projectFileName);
    if (!caf::Utils::fileExists(fullPathProjectFileName))
    {
        RiaLogging::info(QString("File does not exist : '%1'").arg(fullPathProjectFileName));
        return false;
    }

    m_project->fileName = fullPathProjectFileName;
    m_project->readFile();

    // Apply any modifications to the loaded project before we go ahead and load actual data
    if (projectModifier)
    {
        projectModifier->applyModificationsToProject(m_project);
    }

    // Propagate possible new location of project

    m_project->setProjectFileNameAndUpdateDependencies(fullPathProjectFileName);

    // On error, delete everything, and bail out.

    if (m_project->projectFileVersionString().isEmpty())
    {
        closeProject();

        QString tmp =
            QString("Unknown project file version detected in file \n%1\n\nCould not open project.").arg(fullPathProjectFileName);
        QMessageBox::warning(nullptr, "Error when opening project file", tmp);

        m_mainWindow->setPdmRoot(nullptr);

        // Delete all object possibly generated by readFile()
        delete m_project;
        m_project = new RimProject;

        onProjectOpenedOrClosed();

        return true;
    }

    if (m_project->show3DWindow())
    {
        m_mainWindow->show();
    }
    else
    {
        m_mainWindow->hide();
    }

    if (m_project->showPlotWindow())
    {
        if (!m_mainPlotWindow)
        {
            createMainPlotWindow();
            m_mainPlotWindow->show();
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

    m_preferences->lastUsedProjectFileName = fullPathProjectFileName;
    caf::PdmSettings::writeFieldsToApplicationStore(m_preferences);

    for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
    {
        RimOilField*              oilField       = m_project->oilFields[oilFieldIdx];
        RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : nullptr;
        if (analysisModels == nullptr) continue;

        for (size_t cgIdx = 0; cgIdx < analysisModels->caseGroups.size(); ++cgIdx)
        {
            // Load the Main case of each IdenticalGridCaseGroup
            RimIdenticalGridCaseGroup* igcg = analysisModels->caseGroups[cgIdx];
            igcg->loadMainCaseAndActiveCellInfo(); // VL is this supposed to be done for each RimOilField?
        }
    }

    // Load the formation names

    for (RimOilField* oilField : m_project->oilFields)
    {
        if (oilField == nullptr) continue;
        if (oilField->formationNamesCollection() != nullptr)
        {
            oilField->formationNamesCollection()->readAllFormationNames();
        }
    }

    // Add well paths for each oil field
    for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
    {
        RimOilField* oilField = m_project->oilFields[oilFieldIdx];
        if (oilField == nullptr) continue;
        if (oilField->wellPathCollection == nullptr)
        {
            // printf("Create well path collection for oil field %i in loadProject.\n", oilFieldIdx);
            oilField->wellPathCollection = new RimWellPathCollection();
        }

        if (oilField->wellPathCollection)
        {
            oilField->wellPathCollection->loadDataAndUpdate();
            oilField->wellPathCollection->readWellPathFormationFiles();
        }
    }

    for (RimOilField* oilField : m_project->oilFields)
    {
        if (oilField == nullptr) continue;
        // Temporary
        if (!oilField->summaryCaseMainCollection())
        {
            oilField->summaryCaseMainCollection = new RimSummaryCaseMainCollection();
        }
        oilField->summaryCaseMainCollection()->loadAllSummaryCaseData();

        if (!oilField->observedDataCollection())
        {
            oilField->observedDataCollection = new RimObservedDataCollection();
        }
        for (RimObservedData* observedData : oilField->observedDataCollection()->allObservedData())
        {
            observedData->createSummaryReaderInterface();
            observedData->updateMetaData();
        }

        oilField->fractureDefinitionCollection()->loadAndUpdateData();
        oilField->fractureDefinitionCollection()->createAndAssignTemplateCopyForNonMatchingUnit();

        {
            std::vector<RimWellPathFracture*> wellPathFractures;
            oilField->wellPathCollection->descendantsIncludingThisOfType(wellPathFractures);

            for (auto fracture : wellPathFractures)
            {
                fracture->loadDataAndUpdate();
            }
        }
    }

    // If load action is specified to recalculate statistics, do it now.
    // Apparently this needs to be done before the views are loaded, lest the number of time steps for statistics will be clamped
    if (loadAction & PLA_CALCULATE_STATISTICS)
    {
        for (size_t oilFieldIdx = 0; oilFieldIdx < m_project->oilFields().size(); oilFieldIdx++)
        {
            RimOilField*              oilField       = m_project->oilFields[oilFieldIdx];
            RimEclipseCaseCollection* analysisModels = oilField ? oilField->analysisModels() : nullptr;
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
                for (j = 0; j < views.size(); j++)
                {
                    Rim3dView* riv = views[j];
                    CVF_ASSERT(riv);

                    viewProgress.setProgressDescription(riv->name());

                    if (m_project->isProjectFileVersionEqualOrOlderThan("2018.1.0.103"))
                    {
                        std::vector<RimStimPlanColors*> stimPlanColors;
                        riv->descendantsIncludingThisOfType(stimPlanColors);
                        if (stimPlanColors.size() == 1)
                        {
                            stimPlanColors[0]->updateConductivityResultName();
                        }
                    }

                    riv->loadDataAndUpdate();

                    if (m_project->isProjectFileVersionEqualOrOlderThan("2018.1.1.110"))
                    {
                        auto* geoView = dynamic_cast<RimGeoMechView*>(riv);
                        if (geoView)
                        {
                            geoView->convertCameraPositionFromOldProjectFiles();
                        }
                    }

                    this->setActiveReservoirView(riv);

                    RimGridView* rigv = dynamic_cast<RimGridView*>(riv);
                    if (rigv) rigv->rangeFilterCollection()->updateIconState();

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

    // Intersection Views: Sync from intersections in the case.

    for (RimCase* cas : casesToLoad)
    {
        cas->intersectionViewCollection()->syncFromExistingIntersections(false);
    }

    // Init summary case groups
    for (RimOilField* oilField : m_project->oilFields)
    {
        auto sumMainCollection = oilField->summaryCaseMainCollection();
        if (!sumMainCollection) continue;

        for (auto sumCaseGroup : sumMainCollection->summaryCaseCollections())
        {
            sumCaseGroup->loadDataAndUpdate();
        }

        oilField->annotationCollection()->loadDataAndUpdate();
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
    return loadProject(projectFileName, PLA_NONE, nullptr);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::loadAndUpdatePlotData()
{
    RimWellLogPlotCollection*            wlpColl  = nullptr;
    RimSummaryPlotCollection*            spColl   = nullptr;
    RimSummaryCrossPlotCollection*       scpColl  = nullptr;
    RimFlowPlotCollection*               flowColl = nullptr;
    RimRftPlotCollection*                rftColl  = nullptr;
    RimPltPlotCollection*                pltColl  = nullptr;
    RimGridCrossPlotCollection*          gcpColl  = nullptr;
    RimSaturationPressurePlotCollection* sppColl  = nullptr;

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
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->gridCrossPlotCollection())
    {
        gcpColl = m_project->mainPlotCollection()->gridCrossPlotCollection();
    }
    if (m_project->mainPlotCollection() && m_project->mainPlotCollection()->saturationPressurePlotCollection())
    {
        sppColl = m_project->mainPlotCollection()->saturationPressurePlotCollection();
    }

    size_t plotCount = 0;
    plotCount += wlpColl ? wlpColl->wellLogPlots().size() : 0;
    plotCount += spColl ? spColl->summaryPlots().size() : 0;
    plotCount += scpColl ? scpColl->summaryPlots().size() : 0;
    plotCount += flowColl ? flowColl->plotCount() : 0;
    plotCount += rftColl ? rftColl->rftPlots().size() : 0;
    plotCount += pltColl ? pltColl->pltPlots().size() : 0;
    plotCount += gcpColl ? gcpColl->gridCrossPlots().size() : 0;
    plotCount += sppColl ? sppColl->plots().size() : 0;

    if (plotCount > 0)
    {
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

        if (gcpColl)
        {
            for (const auto& gcpPlot : gcpColl->gridCrossPlots())
            {
                gcpPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
        }

        if (sppColl)
        {
            for (const auto& sppPlot : sppColl->plots())
            {
                sppPlot->loadDataAndUpdate();
                plotProgress.incrementProgress();
            }
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

            project()->plotWindowTreeViewState         = treeViewState;
            project()->plotWindowCurrentModelIndexPath = encodedModelIndexString;
        }
    }

    {
        caf::PdmUiTreeView* projectTreeView = m_mainWindow->projectTreeView();
        if (projectTreeView)
        {
            QString treeViewState;
            caf::QTreeViewStateSerializer::storeTreeViewStateToString(projectTreeView->treeView(), treeViewState);

            QModelIndex mi = projectTreeView->treeView()->currentIndex();

            QString encodedModelIndexString;
            caf::QTreeViewStateSerializer::encodeStringFromModelIndex(mi, encodedModelIndexString);

            project()->mainWindowTreeViewState         = treeViewState;
            project()->mainWindowCurrentModelIndexPath = encodedModelIndexString;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Add a list of well path file paths (JSON files) to the well path collection
//--------------------------------------------------------------------------------------------------
void RiaApplication::addWellPathsToModel(QList<QString> wellPathFilePaths)
{
    if (m_project == nullptr || m_project->oilFields.size() < 1) return;

    RimOilField* oilField = m_project->activeOilField();
    if (oilField == nullptr) return;

    if (oilField->wellPathCollection == nullptr)
    {
        // printf("Create well path collection.\n");
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

    m_mainWindow->selectAsCurrentItem(wellLogFile);
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
    // if (m_project.isNull()) return true;

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

    QString fileName =
        QFileDialog::getSaveFileName(nullptr, tr("Save File"), startPath, tr("Project Files (*.rsp);;All files(*.*)"));
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
    if (m_preferences->showProjectChangedDialog() && caf::PdmUiModelChangeDetector::instance()->isModelChanged())
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
    // Make sure we always store path with forward slash to avoid issues when opening the project file on Linux
    m_project->fileName = RiaFilePathTools::toInternalSeparator(fileName);

    storeTreeViewState();

    if (!m_project->writeFile())
    {
        QMessageBox::warning(
            nullptr,
            "Error when saving project file",
            QString(
                "Not possible to save project file. Make sure you have sufficient access rights.\n\nProject file location : %1")
                .arg(fileName));

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
    RicHoloLensSessionManager::instance()->terminateSession();
    RicHoloLensSessionManager::refreshToolbarState();

    RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();

    terminateProcess();

    RiaApplication::clearAllSelections();

    m_mainWindow->cleanupGuiBeforeProjectClose();

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
    if (m_mainWindow)
    {
        m_mainWindow->initializeGuiNewProjectLoaded();
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
bool RiaApplication::openOdbCaseFromFile(const QString& fileName, bool applyTimeStepFilter)
{
    if (!caf::Utils::fileExists(fileName)) return false;

    QFileInfo gridFileName(fileName);
    QString   caseName = gridFileName.completeBaseName();

    RimGeoMechCase* geoMechCase = new RimGeoMechCase();
    geoMechCase->setFileName(fileName);
    geoMechCase->caseUserDescription = caseName;
    geoMechCase->setApplyTimeFilter(applyTimeStepFilter);

    RimGeoMechModels* geoMechModelCollection =
        m_project->activeOilField() ? m_project->activeOilField()->geoMechModels() : nullptr;

    // Create the geoMech model container if it is not there already
    if (geoMechModelCollection == nullptr)
    {
        geoMechModelCollection                     = new RimGeoMechModels();
        m_project->activeOilField()->geoMechModels = geoMechModelCollection;
    }

    RimGeoMechView*   riv = geoMechCase->createAndAddReservoirView();
    caf::ProgressInfo progress(11, "Loading Case");
    progress.setNextProgressIncrement(10);

    riv->loadDataAndUpdate();

    if (!riv->geoMechCase())
    {
        delete geoMechCase;
        return false;
    }
    geoMechModelCollection->cases.push_back(geoMechCase);

    // if (!riv->cellResult()->hasResult())
    //{
    //    riv->cellResult()->setResultVariable(RiaDefines::undefinedResultName());
    //}
    progress.incrementProgress();
    progress.setProgressDescription("Loading results information");

    m_project->updateConnectedEditors();

    m_mainWindow->selectAsCurrentItem(riv->cellResult());

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
    RiaImportEclipseCaseTools::openEclipseInputCaseFromFileNames(QStringList(RiaDefines::mockModelBasicInputCase()));
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
RimGridView* RiaApplication::activeGridView()
{
    return dynamic_cast<RimGridView*>(m_activeReservoirView.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiaApplication::activePlotWindow() const
{
    RimViewWindow* viewWindow = nullptr;

    if (m_mainPlotWindow)
    {
        QList<QMdiSubWindow*> subwindows = m_mainPlotWindow->subWindowList(QMdiArea::StackingOrder);
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
void RiaApplication::setActiveReservoirView(Rim3dView* rv)
{
    m_activeReservoirView = rv;

    RiuDockWidgetTools::instance()->changeDockWidgetVisibilityBasedOnView(rv);
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

    caf::ProgressInfoBlocker progressBlocker;
    cvf::Assert::setReportMode(cvf::Assert::CONSOLE);

#if QT_VERSION < 0x050000
    int    argc = QCoreApplication::argc();
    char** argv = QCoreApplication::argv();
#else
    int argc = QCoreApplication::arguments().size();
    QStringList arguments = QCoreApplication::arguments();
    std::vector<std::string> argumentsStd;
    for (QString qstring : arguments)
    {
        argumentsStd.push_back(qstring.toStdString());
    }
    std::vector<char*> argVector;
    for (std::string& string : argumentsStd)
    {
        argVector.push_back(&string.front());
    }
    char** argv = argVector.data();
#endif

    testing::InitGoogleTest(&argc, argv);

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
RiuMainWindow* RiaApplication::getOrCreateAndShowMainWindow()
{
    if (!m_mainWindow)
    {
        createMainWindow();
    }
    return m_mainWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindow* RiaApplication::mainWindow()
{
    return m_mainWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaApplication::getOrCreateMainPlotWindow()
{
    if (!m_mainPlotWindow)
    {
        createMainPlotWindow();
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
        loadAndUpdatePlotData();
    }
    return m_mainPlotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMainWindow()
{
    CVF_ASSERT(m_mainWindow == nullptr);
    m_mainWindow     = new RiuMainWindow;
    QString platform = cvf::System::is64Bit() ? "(64bit)" : "(32bit)";
    m_mainWindow->setWindowTitle("ResInsight " + platform);
    m_mainWindow->setDefaultWindowSize();
    m_mainWindow->setDefaultToolbarVisibility();
    m_mainWindow->loadWinGeoAndDockToolBarLayout();
    m_mainWindow->showWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::deleteMainWindow()
{
    if (m_mainWindow)
    {
        delete m_mainWindow;
        m_mainWindow = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::createMainPlotWindow()
{
    CVF_ASSERT(m_mainPlotWindow == nullptr);

    m_mainPlotWindow = new RiuPlotMainWindow;

    m_mainPlotWindow->setWindowTitle("Plots - ResInsight");
    m_mainPlotWindow->setDefaultWindowSize();
    m_mainPlotWindow->loadWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::deleteMainPlotWindow()
{
    if (m_mainPlotWindow)
    {
        m_mainPlotWindow->deleteLater();
        m_mainPlotWindow = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaApplication::getOrCreateAndShowMainPlotWindow()
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
    m_mainPlotWindow->activateWindow();
    return m_mainPlotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaApplication::mainPlotWindow()
{
    return m_mainPlotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RiaApplication::mainWindowByID(int mainWindowID)
{
    if (mainWindowID == 0)
        return m_mainWindow;
    else if (mainWindowID == 1)
        return m_mainPlotWindow;
    else
        return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiaApplication::activeViewWindow()
{
    RimViewWindow* viewWindow = nullptr;

    QWidget* mainWindowWidget = RiaApplication::activeWindow();

    if (dynamic_cast<RiuMainWindow*>(mainWindowWidget))
    {
        viewWindow = RiaApplication::instance()->activeReservoirView();
    }
    else if (dynamic_cast<RiuPlotMainWindow*>(mainWindowWidget))
    {
        RiuPlotMainWindow* mainPlotWindow = dynamic_cast<RiuPlotMainWindow*>(mainWindowWidget);

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
    return m_mainWindow && m_mainWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::isMainPlotWindowVisible() const
{
    return m_mainPlotWindow && m_mainPlotWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::closeMainWindowIfOpenButHidden()
{
    if (m_mainWindow && !m_mainWindow->isVisible())
    {
        m_mainWindow->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::closeMainPlotWindowIfOpenButHidden()
{
    if (m_mainPlotWindow && !m_mainPlotWindow->isVisible())
    {
        m_mainPlotWindow->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::addToRecentFiles(const QString& fileName)
{
    CVF_ASSERT(m_recentFileActionProvider &&
               "The provider needs to be created before any attempts to use the recent file actions");
    m_recentFileActionProvider->addFileName(fileName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QAction*> RiaApplication::recentFileActions() const
{
    CVF_ASSERT(m_recentFileActionProvider &&
               "The provider needs to be created before any attempts to use the recent file actions");
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
    QString     line = in.readLine();
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
void RiaApplication::waitUntilCommandObjectsHasBeenProcessed()
{
    // Wait until all command objects have completed
    bool mutexLockedSuccessfully = m_commandQueueLock.tryLock();

    while (!mutexLockedSuccessfully)
    {
        processEvents();

        mutexLockedSuccessfully = m_commandQueueLock.tryLock();
    }
    m_commandQueueLock.unlock();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::saveMainWinGeoAndDockToolBarLayout()
{
    if (isMain3dWindowVisible())
    {
        m_mainWindow->saveWinGeoAndDockToolBarLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::savePlotWinGeoAndDockToolBarLayout()
{
    if (isMainPlotWindowVisible())
    {
        m_mainPlotWindow->saveWinGeoAndDockToolBarLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::enableDevelopmentFeatures()
{
    QString environmentVar = QProcessEnvironment::systemEnvironment().value("RESINSIGHT_DEVEL", QString("0"));
    return environmentVar.toInt() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::clearAllSelections()
{
    Riu3dSelectionManager::instance()->deleteAllItems(Riu3dSelectionManager::RUI_APPLICATION_GLOBAL);
    Riu3dSelectionManager::instance()->deleteAllItems(Riu3dSelectionManager::RUI_TEMPORARY);
    caf::SelectionManager::instance()->clearAll();
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
    m_mainWindow->processMonitor()->stopMonitorWorkProcess();

    // Execute delete later so that other slots that are hooked up
    // get a chance to run before we delete the object
    if (m_workerProcess)
    {
        m_workerProcess->close();
    }
    m_workerProcess = nullptr;

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
    if (m_workerProcess == nullptr)
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
        m_workerProcess        = new caf::UiProcess(this);

        QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();

#ifdef WIN32
        // Octave plugins compiled by ResInsight are dependent on Qt (currently Qt 32-bit only)
        // Some Octave installations for Windows have included Qt, and some don't. To make sure these plugins always can be
        // executed, the path to octave_plugin_dependencies is added to global path

        QString pathString = penv.value("PATH", "");

        if (pathString == "")
            pathString = QApplication::applicationDirPath() + "\\octave_plugin_dependencies";
        else
            pathString = QApplication::applicationDirPath() + "\\octave_plugin_dependencies" + ";" + pathString;

        penv.insert("PATH", pathString);
#else
        // Set the LD_LIBRARY_PATH to make the octave plugins find the embedded Qt
        QString ldPath = penv.value("LD_LIBRARY_PATH", "");

        if (ldPath == "")
            ldPath = QApplication::applicationDirPath();
        else
            ldPath = QApplication::applicationDirPath() + ":" + ldPath;

        penv.insert("LD_LIBRARY_PATH", ldPath);
#endif

        m_workerProcess->setProcessEnvironment(penv);

        connect(m_workerProcess,
                SIGNAL(finished(int, QProcess::ExitStatus)),
                SLOT(slotWorkerProcessFinished(int, QProcess::ExitStatus)));

        m_mainWindow->processMonitor()->startMonitorWorkProcess(m_workerProcess);

        m_workerProcess->start(program, arguments);

        // The wait time is a compromise between large wait time when processing many octave runs after each other and short wait
        // time when starting octave processes interactively
        int waitTimeMilliseconds = 7 * 1000;
        if (!m_workerProcess->waitForStarted(waitTimeMilliseconds))
        {
            m_workerProcess->close();
            m_workerProcess        = nullptr;
            m_runningWorkerProcess = false;

            m_mainWindow->processMonitor()->stopMonitorWorkProcess();

            QMessageBox::warning(m_mainWindow, "Script execution", "Failed to start script executable located at\n" + program);

            return false;
        }

        return true;
    }
    else
    {
        QMessageBox::warning(nullptr,
                             "Script execution",
                             "An Octave process is still running. Please stop this process before executing a new script.");
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaApplication::launchProcessForMultipleCases(const QString&          program,
                                                   const QStringList&      arguments,
                                                   const std::vector<int>& caseIds)
{
    m_currentCaseIds.clear();
    std::copy(caseIds.begin(), caseIds.end(), std::back_inserter(m_currentCaseIds));

    m_currentProgram   = program;
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
void RiaApplication::applyPreferences(const RiaPreferences* oldPreferences)
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

    if (m_mainWindow && m_mainWindow->projectTreeView())
    {
        m_mainWindow->projectTreeView()->enableAppendOfClassNameToUiItemText(m_preferences->appendClassNameToUiText());
        if (mainPlotWindow())
            mainPlotWindow()->projectTreeView()->enableAppendOfClassNameToUiItemText(m_preferences->appendClassNameToUiText());
    }

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font
    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> fontSizes = m_preferences->defaultFontSizes();

    m_defaultSceneFont                        = RiaFontCache::getFont(fontSizes[RiaDefines::SCENE_FONT]);    
    m_defaultAnnotationFont                   = RiaFontCache::getFont(fontSizes[RiaDefines::ANNOTATION_FONT]);
    m_defaultWellLabelFont                    = RiaFontCache::getFont(fontSizes[RiaDefines::WELL_LABEL_FONT]);

    if (this->project())
    {
        this->project()->setScriptDirectories(m_preferences->scriptDirectories());
        this->project()->updateConnectedEditors();

        RimWellPathCollection* wellPathCollection = this->project()->activeOilField()->wellPathCollection();

        std::vector<RimViewWindow*> allViewWindows;
        project()->descendantsIncludingThisOfType(allViewWindows);

        bool existingViewsWithDifferentMeshLines   = false;
        bool existingViewsWithCustomColors         = false;
        bool existingViewsWithCustomZScale         = false;
        bool existingObjectsWithCustomFonts        = false;
        if (oldPreferences)
        {
            for (auto viewWindow : allViewWindows)
            {
                auto rim3dView = dynamic_cast<Rim3dView*>(viewWindow);
                if (rim3dView)
                {
                    if (rim3dView->meshMode() != oldPreferences->defaultMeshModeType())
                    {
                        existingViewsWithDifferentMeshLines = true;
                    }
                    if (rim3dView->backgroundColor() != oldPreferences->defaultViewerBackgroundColor())
                    {
                        existingViewsWithCustomColors = true;
                    }
                    if (rim3dView->scaleZ() != static_cast<double>(oldPreferences->defaultScaleFactorZ))
                    {
                        existingViewsWithCustomZScale = true;
                    }

                    RimGridView* gridView = dynamic_cast<RimGridView*>(rim3dView);
                    if (gridView)
                    {
                        RiaFontCache::FontSize oldFontSize = oldPreferences->defaultAnnotationFontSize();
                        existingObjectsWithCustomFonts = gridView->annotationCollection()->hasTextAnnotationsWithCustomFontSize(oldFontSize);
                    }
                    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(rim3dView);
                    if (eclipseView)
                    {
                        if (eclipseView->wellCollection()->wellLabelColor() != oldPreferences->defaultWellLabelColor())
                        {
                            existingViewsWithCustomColors = true;
                        }
                    }
                }

                for (auto fontTypeSizePair : fontSizes)
                {
                    RiaFontCache::FontSize oldFontSizeEnum = oldPreferences->defaultFontSizes()[fontTypeSizePair.first];
                    if (oldFontSizeEnum != fontTypeSizePair.second)
                    {
                        int oldFontSize = RiaFontCache::pointSizeFromFontSizeEnum(oldFontSizeEnum);
                        if (viewWindow->hasCustomFontSizes(fontTypeSizePair.first, oldFontSize))
                        {
                            existingObjectsWithCustomFonts = true;
                        }
                    }
                }
            }

            if (oldPreferences->defaultWellLabelColor() != wellPathCollection->wellPathLabelColor())
            {
                existingViewsWithCustomColors = true;
            }
        }        

        bool applySettingsToAllViews = false;
        if (existingViewsWithCustomColors || existingViewsWithCustomZScale ||
            existingViewsWithDifferentMeshLines || existingObjectsWithCustomFonts)
        {
            QStringList changedData;
            if (existingViewsWithDifferentMeshLines) changedData << "Mesh Visibility";
            if (existingViewsWithCustomColors) changedData << "Colors";
            if (existingViewsWithCustomZScale) changedData << "Z-Scale";
            if (existingObjectsWithCustomFonts) changedData << "Fonts Sizes";

            QString listString = changedData.takeLast();
            if (!changedData.empty())
            {
                listString = changedData.join(", ") + " and " + listString;
            }

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(m_mainWindow,
                QString("Apply %1 to Existing Views or Plots?").arg(listString),
                QString("You have changed default %1 and have existing views or plots with different settings.\n").arg(listString) +
                QString("Do you want to apply the new default settings to all existing views?"),
                QMessageBox::Ok | QMessageBox::Cancel);
            applySettingsToAllViews = (reply == QMessageBox::Ok);
        }

        for (auto viewWindow : allViewWindows)
        {
            for (auto fontTypeSizePair : fontSizes)
            {
                RiaFontCache::FontSize oldFontSizeEnum = oldPreferences->defaultFontSizes()[fontTypeSizePair.first];
                if (oldFontSizeEnum != fontTypeSizePair.second)
                {
                    int oldFontSize = RiaFontCache::pointSizeFromFontSizeEnum(oldFontSizeEnum);
                    int newFontSize = RiaFontCache::pointSizeFromFontSizeEnum(fontTypeSizePair.second);
                    viewWindow->applyFontSize(fontTypeSizePair.first, oldFontSize, newFontSize, applySettingsToAllViews);
                }

            }

            auto rim3dView = dynamic_cast<Rim3dView*>(viewWindow);
            if (rim3dView)
            {
                if (oldPreferences && (applySettingsToAllViews || rim3dView->meshMode() == oldPreferences->defaultMeshModeType()))
                {
                    rim3dView->meshMode = m_preferences->defaultMeshModeType();
                }

                if (oldPreferences && (applySettingsToAllViews || rim3dView->backgroundColor() == oldPreferences->defaultViewerBackgroundColor()))
                {
                    rim3dView->setBackgroundColor(m_preferences->defaultViewerBackgroundColor());
                    rim3dView->applyBackgroundColorAndFontChanges();
                }

                if (oldPreferences && (applySettingsToAllViews || rim3dView->scaleZ == static_cast<double>(oldPreferences->defaultScaleFactorZ())))
                {
                    rim3dView->scaleZ = static_cast<double>(m_preferences->defaultScaleFactorZ());
                    rim3dView->updateScaling();
                    if (rim3dView == activeViewWindow())
                    {
                        RiuMainWindow::instance()->updateScaleValue();
                    }
                }

                RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(rim3dView);
                if (eclipseView)
                {
                    if (oldPreferences && (applySettingsToAllViews || eclipseView->wellCollection()->wellLabelColor() == oldPreferences->defaultWellLabelColor()))
                    {
                        eclipseView->wellCollection()->wellLabelColor = m_preferences->defaultWellLabelColor();
                    }
                    eclipseView->scheduleReservoirGridGeometryRegen();
                }
                rim3dView->scheduleCreateDisplayModelAndRedraw();
            }
        }

        if (oldPreferences)
        {            
            bool matchingColor = wellPathCollection->wellPathLabelColor() == oldPreferences->defaultWellLabelColor();
            if (applySettingsToAllViews || matchingColor)
            {
                wellPathCollection->wellPathLabelColor = oldPreferences->defaultWellLabelColor();
            }

            if (oldPreferences->defaultPlotFontSize() != m_preferences->defaultPlotFontSize())
            {
                m_mainWindow->applyFontSizesToDockedPlots();
            }
        }

        std::vector<caf::PdmUiItem*> uiEditorsToUpdate;
        caf::SelectionManager::instance()->selectedItems(uiEditorsToUpdate);

        for (caf::PdmUiItem* uiItem : uiEditorsToUpdate)
        {
            uiItem->updateConnectedEditors();
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
    m_workerProcess        = nullptr;
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
QString RiaApplication::lastUsedDialogDirectoryWithFallbackToProjectFolder(const QString& dialogName)
{
    return lastUsedDialogDirectoryWithFallback(dialogName, currentProjectPath());
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

    QString lastUsedDialogTag;

    RiaDefines::ImportFileType fileType = RiaDefines::obtainFileTypeFromFileName(fileName);

    if (fileType == RiaDefines::RESINSIGHT_PROJECT_FILE)
    {
        loadingSucceded = loadProject(fileName);
    }
    else if (fileType == RiaDefines::GEOMECH_ODB_FILE)
    {
        loadingSucceded = openOdbCaseFromFile(fileName);
        if (loadingSucceded) lastUsedDialogTag = "GEOMECH_MODEL";
    }
    else if ( fileType & RiaDefines::ANY_ECLIPSE_FILE)
    {
        loadingSucceded = RicImportGeneralDataFeature::openEclipseFilesFromFileNames(QStringList{ fileName });
        if (loadingSucceded)
        {
            lastUsedDialogTag = RiaDefines::defaultDirectoryLabel(fileType);

            if (fileType & RiaDefines::ECLIPSE_SUMMARY_FILE)
            {
                getOrCreateAndShowMainPlotWindow();
            }
        }
    }

    if (loadingSucceded)
    {
        if (!lastUsedDialogTag.isEmpty())
        {
            RiaApplication::instance()->setLastUsedDialogDirectory(lastUsedDialogTag, QFileInfo(fileName).absolutePath());
        }
        if (!RiaApplication::hasValidProjectFileExtension(fileName))
        {
            caf::PdmUiModelChangeDetector::instance()->setModelChanged();
        }
    }

    return loadingSucceded;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::runMultiCaseSnapshots(const QString&       templateProjectFileName,
                                           std::vector<QString> gridFileNames,
                                           const QString&       snapshotFolderName)
{
    if (!m_mainWindow) return;

    m_mainWindow->hideAllDockWindows();

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

    m_mainWindow->loadWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::defaultSceneFont()
{
    CVF_ASSERT(m_defaultSceneFont.notNull());

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font

    return m_defaultSceneFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::defaultAnnotationFont()
{
    CVF_ASSERT(m_defaultAnnotationFont.notNull());

    return m_defaultAnnotationFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaApplication::defaultWellLabelFont()
{
    CVF_ASSERT(m_defaultWellLabelFont.notNull());

    return m_defaultWellLabelFont.p();
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
    {
        std::list<RimCommandObject*>::iterator it = m_commandQueue.begin();
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
    }

    if (!m_commandQueue.empty())
    {
        std::list<RimCommandObject*>::iterator it = m_commandQueue.begin();

        RimCommandObject* first = *it;
        first->redo();

        m_commandQueue.pop_front();
    }
    else
    {
        // Unlock the command queue lock when the command queue is empty
        // Required to lock the mutex before unlocking to avoid undefined behavior
        m_commandQueueLock.tryLock();
        m_commandQueueLock.unlock();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaApplication::setHelpText(const QString& helpText)
{
    m_helpText = helpText;
}
