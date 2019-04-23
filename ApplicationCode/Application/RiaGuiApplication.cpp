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

#include "RiaGuiApplication.h"

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
#include "RiuMdiMaximizeWindowGuard.h"
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

#include <iostream>

#ifndef WIN32
#include <unistd.h> // for usleep
#endif // WIN32

#ifdef USE_UNIT_TESTS
#include "gtest/gtest.h"
#endif // USE_UNIT_TESTS

namespace caf
{
template<>
void AppEnum<RiaGuiApplication::RINavigationPolicy>::setUp()
{
    addItem(RiaGuiApplication::NAVIGATION_POLICY_CEETRON, "NAVIGATION_POLICY_CEETRON", "Ceetron");
    addItem(RiaGuiApplication::NAVIGATION_POLICY_CAD, "NAVIGATION_POLICY_CAD", "CAD");
    addItem(RiaGuiApplication::NAVIGATION_POLICY_GEOQUEST, "NAVIGATION_POLICY_GEOQUEST", "GEOQUEST");
    addItem(RiaGuiApplication::NAVIGATION_POLICY_RMS, "NAVIGATION_POLICY_RMS", "RMS");
    setDefault(RiaGuiApplication::NAVIGATION_POLICY_RMS);
}
} // namespace caf

//==================================================================================================
///
/// \class RiaGuiApplication
///
/// Application class
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication* RiaGuiApplication::instance()
{
    return dynamic_cast<RiaGuiApplication*>(RiaApplication::instance());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication::RiaGuiApplication(int& argc, char** argv)
    : QApplication(argc, argv)   
    , m_mainWindow(nullptr)
    , m_mainPlotWindow(nullptr)
{

    // For idle processing
    //    m_idleTimerStarted = false;
    installEventFilter(this);

    // cvf::Trace::enable(false);


    if (useShaders())
    {
        caf::EffectGenerator::setRenderingMode(caf::EffectGenerator::SHADER_BASED);
    }
    else
    {
        caf::EffectGenerator::setRenderingMode(caf::EffectGenerator::FIXED_FUNCTION);
    }


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
RiaGuiApplication::~RiaGuiApplication()
{
    RiuDockWidgetTools::instance()->saveDockWidgetsState();

    deleteMainPlotWindow();
    deleteMainWindow();  

    RiaLogging::deleteLoggerInstance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::parseArguments()
{
    bool result = RiaArgumentParser::parseArguments();
    if (!result)
    {
        closeAllWindows();
        handleEvents();
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::saveProject()
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
bool RiaGuiApplication::saveProjectPromptForFileName()
{
    // if (m_project.isNull()) return true;

    RiaGuiApplication* app = RiaGuiApplication::instance();

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
bool RiaGuiApplication::askUserToSaveModifiedProject()
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
bool RiaGuiApplication::saveProjectAs(const QString& fileName)
{
    storeTreeViewState();
    QString errMsg;
    if (!RiaApplication::saveProjectAs(fileName, &errMsg))
    {
        QMessageBox::warning(nullptr, "Error when saving project file", errMsg);
        return false;
    }

    m_recentFileActionProvider->addFileName(fileName);
    caf::PdmUiModelChangeDetector::instance()->reset();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::loadAndUpdatePlotData()
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
void RiaGuiApplication::storeTreeViewState()
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
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::setWindowCaptionFromAppState()
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
RimViewWindow* RiaGuiApplication::activePlotWindow() const
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
bool RiaGuiApplication::useShaders() const
{
    if (!m_preferences->useShaders) return false;

    bool isShadersSupported = caf::Viewer::isShadersSupported();
    if (!isShadersSupported) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGuiApplication::RINavigationPolicy RiaGuiApplication::navigationPolicy() const
{
    return m_preferences->navigationPolicy();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::showPerformanceInfo() const
{
    return m_preferences->showHud;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaGuiApplication::launchUnitTestsWithConsole()
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
RiuMainWindow* RiaGuiApplication::getOrCreateAndShowMainWindow()
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
RiuMainWindow* RiaGuiApplication::mainWindow()
{
    return m_mainWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotMainWindow* RiaGuiApplication::getOrCreateMainPlotWindow()
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
void RiaGuiApplication::createMainWindow()
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
void RiaGuiApplication::deleteMainWindow()
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
void RiaGuiApplication::createMainPlotWindow()
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
void RiaGuiApplication::deleteMainPlotWindow()
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
RiuPlotMainWindow* RiaGuiApplication::getOrCreateAndShowMainPlotWindow()
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
RiuPlotMainWindow* RiaGuiApplication::mainPlotWindow()
{
    return m_mainPlotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RiaGuiApplication::mainWindowByID(int mainWindowID)
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
RimViewWindow* RiaGuiApplication::activeViewWindow()
{
    RimViewWindow* viewWindow = nullptr;

    QWidget* mainWindowWidget = RiaGuiApplication::activeWindow();

    if (dynamic_cast<RiuMainWindow*>(mainWindowWidget))
    {
        viewWindow = RiaGuiApplication::instance()->activeReservoirView();
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
bool RiaGuiApplication::isMain3dWindowVisible() const
{
    return m_mainWindow && m_mainWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaGuiApplication::isMainPlotWindowVisible() const
{
    return m_mainPlotWindow && m_mainPlotWindow->isVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::closeMainWindowIfOpenButHidden()
{
    if (m_mainWindow && !m_mainWindow->isVisible())
    {
        m_mainWindow->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::closeMainPlotWindowIfOpenButHidden()
{
    if (m_mainPlotWindow && !m_mainPlotWindow->isVisible())
    {
        m_mainPlotWindow->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::addToRecentFiles(const QString& fileName)
{
    CVF_ASSERT(m_recentFileActionProvider &&
               "The provider needs to be created before any attempts to use the recent file actions");
    m_recentFileActionProvider->addFileName(fileName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QAction*> RiaGuiApplication::recentFileActions() const
{
    CVF_ASSERT(m_recentFileActionProvider &&
               "The provider needs to be created before any attempts to use the recent file actions");
    return m_recentFileActionProvider->actions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::saveMainWinGeoAndDockToolBarLayout()
{
    if (isMain3dWindowVisible())
    {
        m_mainWindow->saveWinGeoAndDockToolBarLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::savePlotWinGeoAndDockToolBarLayout()
{
    if (isMainPlotWindowVisible())
    {
        m_mainPlotWindow->saveWinGeoAndDockToolBarLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::clearAllSelections()
{
    Riu3dSelectionManager::instance()->deleteAllItems(Riu3dSelectionManager::RUI_APPLICATION_GLOBAL);
    Riu3dSelectionManager::instance()->deleteAllItems(Riu3dSelectionManager::RUI_TEMPORARY);
    caf::SelectionManager::instance()->clearAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::handleEvents(QEventLoop::ProcessEventsFlags flags)
{
    processEvents(flags);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onChangedActiveReservoirView()
{
    RiuDockWidgetTools::instance()->changeDockWidgetVisibilityBasedOnView(activeReservoirView());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onFileSuccessfullyLoaded(const QString& fileName, RiaDefines::ImportFileType fileType)
{
    if (fileType & RiaDefines::ANY_ECLIPSE_FILE)
    {
        getOrCreateAndShowMainPlotWindow();
    }
    
    if (!RiaGuiApplication::hasValidProjectFileExtension(fileName))
    {
        caf::PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectBeingOpened()
{
    // When importing a project, do not maximize the first MDI window to be created
    m_maximizeWindowGuard.reset(new RiuMdiMaximizeWindowGuard);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectOpeningError(const QString& errMsg)
{
    QMessageBox::warning(nullptr, "Error when opening project file", errMsg);
    m_mainWindow->setPdmRoot(nullptr);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectOpened()
{
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

    loadAndUpdatePlotData();

    if (m_mainWindow)
    {
        m_mainWindow->initializeGuiNewProjectLoaded();
    }
    if (m_mainPlotWindow)
    {
        m_mainPlotWindow->initializeGuiNewProjectLoaded();
    }

    setWindowCaptionFromAppState();

    m_maximizeWindowGuard.reset();

    processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectBeingClosed()
{
    RicHoloLensSessionManager::instance()->terminateSession();
    RicHoloLensSessionManager::refreshToolbarState();

    RiaViewRedrawScheduler::instance()->clearViewsScheduledForUpdate();

    RiaGuiApplication::clearAllSelections();

    m_mainWindow->cleanupGuiBeforeProjectClose();

    if (m_mainPlotWindow)
    {
        m_mainPlotWindow->cleanupGuiBeforeProjectClose();
    }

    caf::EffectGenerator::clearEffectCache();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onProjectClosed()
{
    // Always creating new project when closing old one
    onProjectOpened();

    // Make sure all project windows are closed down properly before returning
    processEvents();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::onPreferencesChanged(const RiaPreferences* oldPreferences)
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

    m_defaultSceneFont      = RiaFontCache::getFont(fontSizes[RiaDefines::SCENE_FONT]);
    m_defaultAnnotationFont = RiaFontCache::getFont(fontSizes[RiaDefines::ANNOTATION_FONT]);
    m_defaultWellLabelFont  = RiaFontCache::getFont(fontSizes[RiaDefines::WELL_LABEL_FONT]);

    if (this->project())
    {
        std::vector<RimViewWindow*> allViewWindows;
        project()->descendantsIncludingThisOfType(allViewWindows);

        RimWellPathCollection* wellPathCollection = this->project()->activeOilField()->wellPathCollection();

        bool existingViewsWithDifferentMeshLines = false;
        bool existingViewsWithCustomColors       = false;
        bool existingViewsWithCustomZScale       = false;
        bool existingObjectsWithCustomFonts      = false;
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
                        existingObjectsWithCustomFonts =
                            gridView->annotationCollection()->hasTextAnnotationsWithCustomFontSize(oldFontSize);
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
        if (existingViewsWithCustomColors || existingViewsWithCustomZScale || existingViewsWithDifferentMeshLines ||
            existingObjectsWithCustomFonts)
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
            reply = QMessageBox::question(
                m_mainWindow,
                QString("Apply %1 to Existing Views or Plots?").arg(listString),
                QString("You have changed default %1 and have existing views or plots with different settings.\n")
                        .arg(listString) +
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

                if (oldPreferences &&
                    (applySettingsToAllViews || rim3dView->backgroundColor() == oldPreferences->defaultViewerBackgroundColor()))
                {
                    rim3dView->setBackgroundColor(m_preferences->defaultViewerBackgroundColor());
                    rim3dView->applyBackgroundColorAndFontChanges();
                }

                if (oldPreferences &&
                    (applySettingsToAllViews || rim3dView->scaleZ == static_cast<double>(oldPreferences->defaultScaleFactorZ())))
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
                    if (oldPreferences && (applySettingsToAllViews || eclipseView->wellCollection()->wellLabelColor() ==
                                                                          oldPreferences->defaultWellLabelColor()))
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
void RiaGuiApplication::startMonitoringWorkProgress(caf::UiProcess* uiProcess)
{
    m_mainWindow->processMonitor()->startMonitorWorkProcess(m_workerProcess);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::stopMonitoringWorkProgress()
{
    m_mainWindow->processMonitor()->stopMonitorWorkProcess();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::slotWorkerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
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
void RiaGuiApplication::runMultiCaseSnapshots(const QString&       templateProjectFileName,
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
cvf::Font* RiaGuiApplication::defaultSceneFont()
{
    CVF_ASSERT(m_defaultSceneFont.notNull());

    // The creation of a font is time consuming, so make sure you really need your own font
    // instead of using the application font

    return m_defaultSceneFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaGuiApplication::defaultAnnotationFont()
{
    CVF_ASSERT(m_defaultAnnotationFont.notNull());

    return m_defaultAnnotationFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Font* RiaGuiApplication::defaultWellLabelFont()
{
    CVF_ASSERT(m_defaultWellLabelFont.notNull());

    return m_defaultWellLabelFont.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGuiApplication::showFormattedTextInMessageBox(const QString& text)
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
bool RiaGuiApplication::notify(QObject* receiver, QEvent* event)
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
