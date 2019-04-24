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

#pragma once

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaFontCache.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfFont.h"

#include <QApplication>
#include <QMutex>
#include <QProcess>
#include <QString>

#include <iostream>
#include <memory>

class QAction;

class Drawable;

class RIProcess;

class RiaPreferences;
class RiaProjectModifier;
class RiaSocketServer;

class RigEclipseCaseData;

class RimCommandObject;
class RimEclipseCase;
class RimEclipseView;
class RimGridView;
class RimProject;
class RimSummaryPlot;
class Rim3dView;
class RimViewWindow;
class RimWellLogPlot;
class RimWellAllocationPlot;

class RiuMainWindow;
class RiuMainWindowBase;
class RiuMdiMaximizeWindowGuard;
class RiuPlotMainWindow;
class RiuRecentFileActionProvider;
class RiaArgumentParser;

namespace caf
{
    class UiProcess;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RiaGuiApplication : public QApplication, public RiaApplication
{
    Q_OBJECT

public:
    enum RINavigationPolicy
    {
        NAVIGATION_POLICY_CEETRON,
        NAVIGATION_POLICY_CAD,
        NAVIGATION_POLICY_GEOQUEST,
        NAVIGATION_POLICY_RMS
    };


    typedef RiaFontCache::FontSize FontSize;

public:
    static RiaGuiApplication* instance();

    RiaGuiApplication(int& argc, char** argv);
    ~RiaGuiApplication() override;
    
    bool                saveProject();
    bool                saveProjectPromptForFileName();
    bool                askUserToSaveModifiedProject();
    bool                saveProjectAs(const QString& fileName);

    void                runMultiCaseSnapshots(const QString& templateProjectFileName, std::vector<QString> gridFileNames, const QString& snapshotFolderName);
    bool                useShaders() const;
    bool                showPerformanceInfo() const;

    RINavigationPolicy  navigationPolicy() const;

    cvf::Font*          defaultSceneFont();
    cvf::Font*          defaultAnnotationFont();
    cvf::Font*          defaultWellLabelFont();
    
    RiuMainWindow*      getOrCreateAndShowMainWindow();
    RiuMainWindow*      mainWindow();
    RimViewWindow*      activePlotWindow() const;

    RiuPlotMainWindow*  getOrCreateMainPlotWindow();
    RiuPlotMainWindow*  getOrCreateAndShowMainPlotWindow();
    RiuPlotMainWindow*  mainPlotWindow();
    RiuMainWindowBase*  mainWindowByID(int mainWindowID);

    static RimViewWindow* activeViewWindow();

    bool                isMain3dWindowVisible() const;
    bool                isMainPlotWindowVisible() const;

    void                closeMainWindowIfOpenButHidden();
    void                closeMainPlotWindowIfOpenButHidden();

    std::vector<QAction*> recentFileActions() const;

    void                saveMainWinGeoAndDockToolBarLayout();
    void                savePlotWinGeoAndDockToolBarLayout();

    static void         clearAllSelections();
    void                applyGuiPreferences(const RiaPreferences* oldPreferences = nullptr);

    // Public RiaApplication overrides
    void                initialize() override;
    bool                parseArguments() override;
    int                 launchUnitTestsWithConsole() override;
    void                addToRecentFiles(const QString& fileName) override;
    void                showInformationMessage(const QString& text) override;
    void                showErrorMessage(const QString& errMsg) override;
protected:
    // Protected RiaApplication overrides
    void                handleEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents) override;    
    void                onChangedActiveReservoirView() override;
    void                onFileSuccessfullyLoaded(const QString& fileName, RiaDefines::ImportFileType fileType) override;
    void                onProjectBeingOpened() override;
    void                onProjectOpeningError(const QString& errMsg);
    void                onProjectOpened() override;
    void                onProjectBeingClosed() override;
    void                onProjectClosed() override;
    void                startMonitoringWorkProgress(caf::UiProcess* uiProcess) override;
    void                stopMonitoringWorkProgress() override;

private:
    void                setWindowCaptionFromAppState();

    void                createMainWindow();
    void                deleteMainWindow();

    void                createMainPlotWindow();
    void                deleteMainPlotWindow();
    
    void                loadAndUpdatePlotData();
    
    void                storeTreeViewState();

    bool                notify(QObject *, QEvent *) override;

private slots:
    void                slotWorkerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    cvf::ref<cvf::Font>                 m_defaultSceneFont;
    cvf::ref<cvf::Font>                 m_defaultAnnotationFont;
    cvf::ref<cvf::Font>                 m_defaultWellLabelFont;

    RiuMainWindow*                      m_mainWindow;
    RiuPlotMainWindow*                  m_mainPlotWindow;
    
    std::unique_ptr<RiuRecentFileActionProvider> m_recentFileActionProvider;

    std::unique_ptr<RiuMdiMaximizeWindowGuard> m_maximizeWindowGuard;
};
