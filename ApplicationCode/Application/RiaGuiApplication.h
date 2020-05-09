////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfObject.h"

#include <QApplication>
#include <QMutex>
#include <QPointer>
#include <QProcess>
#include <QString>
#include <QTimer>

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
class FontHolderInterface;
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
    enum class RINavigationPolicy
    {
        NAVIGATION_POLICY_CEETRON,
        NAVIGATION_POLICY_CAD,
        NAVIGATION_POLICY_GEOQUEST,
        NAVIGATION_POLICY_RMS
    };

public:
    static bool               isRunning();
    static RiaGuiApplication* instance();

    RiaGuiApplication( int& argc, char** argv );
    ~RiaGuiApplication() override;

    bool    saveProject();
    QString promptForProjectSaveAsFileName() const;
    bool    askUserToSaveModifiedProject();
    bool    saveProjectAs( const QString& fileName );

    void runMultiCaseSnapshots( const QString&       templateProjectFileName,
                                std::vector<QString> gridFileNames,
                                const QString&       snapshotFolderName );
    bool useShaders() const;

    RINavigationPolicy navigationPolicy() const;

    RiuMainWindow* getOrCreateAndShowMainWindow();
    RiuMainWindow* mainWindow();
    RimViewWindow* activePlotWindow() const;

    RiuPlotMainWindow* getOrCreateMainPlotWindow();
    RiuPlotMainWindow* getOrCreateAndShowMainPlotWindow();
    RiuPlotMainWindow* mainPlotWindow();
    RiuMainWindowBase* mainWindowByID( int mainWindowID );

    static RimViewWindow* activeViewWindow();

    bool isMain3dWindowVisible() const;
    bool isMainPlotWindowVisible() const;

    void closeMainWindowIfOpenButHidden();
    void closeMainPlotWindowIfOpenButHidden();

    std::vector<QAction*> recentFileActions() const;

    static void clearAllSelections();
    void        applyGuiPreferences( const RiaPreferences*                         oldPreferences     = nullptr,
                                     const std::vector<caf::FontHolderInterface*>& defaultFontObjects = {} );
    void        updateGrpcServer();
    static int  applicationResolution();

    // Public RiaApplication overrides
    void              initialize() override;
    ApplicationStatus handleArguments( cvf::ProgramOptions* progOpt ) override;
    int               launchUnitTestsWithConsole() override;
    void              addToRecentFiles( const QString& fileName ) override;
    void              showFormattedTextInMessageBoxOrConsole( const QString& errMsg ) override;
    void              launchGrpcServer() override;
#ifdef ENABLE_GRPC
    RiaGrpcServer* grpcServer() const override;
#endif
protected:
    // Protected RiaApplication overrides
    void invokeProcessEvents( QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents ) override;
    void onFileSuccessfullyLoaded( const QString& fileName, RiaDefines::ImportFileType fileType ) override;

    void onProjectBeingOpened() override;
    void onProjectOpeningError( const QString& errMsg ) override;
    void onProjectOpened() override;
    void onProjectBeingClosed() override;
    void onProjectClosed() override;
    void onProjectBeingSaved() override;
    void onProjectSaved() override;

    void startMonitoringWorkProgress( caf::UiProcess* uiProcess ) override;
    void stopMonitoringWorkProgress() override;

private:
    void setWindowCaptionFromAppState();

    void createMainWindow();
    void deleteMainWindow();

    void createMainPlotWindow();
    void deleteMainPlotWindow();

    void storeTreeViewState();

    bool notify( QObject*, QEvent* ) override;

private slots:
    void slotWorkerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void runIdleProcessing();
    void onProgramExit();

private:
    RiuMainWindow*     m_mainWindow;
    RiuPlotMainWindow* m_mainPlotWindow;
#ifdef ENABLE_GRPC
    QPointer<QTimer> m_idleTimer;
#endif

    std::unique_ptr<RiuRecentFileActionProvider> m_recentFileActionProvider;

    std::unique_ptr<RiuMdiMaximizeWindowGuard> m_maximizeWindowGuard;
};
