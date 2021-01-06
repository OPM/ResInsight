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
//////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RiaDefines.h"
#ifdef ENABLE_GRPC
#include "RiaGrpcServer.h"
#endif

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfFont.h"
#include "cvfObject.h"

#include <QMutex>
#include <QPointer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>

#include <gsl/gsl>

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
class RimWellPath;
class RimGridView;
class RimProject;
class RimSummaryPlot;
class Rim3dView;
class RimViewWindow;
class RimWellLogFile;
class RimWellLogPlot;
class RimWellAllocationPlot;

class RiuMainWindow;
class RiuMainWindowBase;
class RiuPlotMainWindow;
class RiuRecentFileActionProvider;
class RiaArgumentParser;

namespace caf
{
class UiProcess;
}

namespace cvf
{
class ProgramOptions;
}

//==================================================================================================
/// Base class for all ResInsight applications. I.e. console and GUI
///
//==================================================================================================
class RiaApplication
{
public:
    enum class ProjectLoadAction
    {
        PLA_NONE                 = 0,
        PLA_CALCULATE_STATISTICS = 1
    };

    enum class ApplicationStatus
    {
        KEEP_GOING = 0,
        EXIT_COMPLETED,
        EXIT_WITH_ERROR
    };

public:
    static RiaApplication* instance();
    RiaApplication();
    virtual ~RiaApplication();

    static const char* getVersionStringApp( bool includeCrtInfo );
    static bool        enableDevelopmentFeatures();

    void             setActiveReservoirView( Rim3dView* );
    Rim3dView*       activeReservoirView();
    const Rim3dView* activeReservoirView() const;
    RimGridView*     activeGridView();
    RimGridView*     activeMainOrComparisonGridView();
    int              currentScriptCaseId() const;

    RimProject* project();

    void createMockModel();
    void createResultsMockModel();
    void createLargeResultsMockModel();
    void createMockModelCustomized();
    void createInputMockModel();

    bool openFile( const QString& fileName );

    bool    isProjectSavedToDisc() const;
    QString currentProjectPath() const;
    QString createAbsolutePathFromProjectRelativePath( QString projectRelativePath );

    bool loadProject( const QString& projectFileName );
    bool loadProject( const QString& projectFileName, ProjectLoadAction loadAction, RiaProjectModifier* projectModifier );

    bool saveProject( gsl::not_null<QString*> errorMessage );
    bool saveProjectAs( const QString& fileName, gsl::not_null<QString*> errorMessage );

    static bool hasValidProjectFileExtension( const QString& fileName );
    void        closeProject();

    QString lastUsedDialogDirectory( const QString& dialogName );
    QString lastUsedDialogDirectoryWithFallbackToProjectFolder( const QString& dialogName );
    QString lastUsedDialogDirectoryWithFallback( const QString& dialogName, const QString& fallbackDirectory );
    void    setLastUsedDialogDirectory( const QString& dialogName, const QString& directory );

    bool openOdbCaseFromFile( const QString& fileName, bool applyTimeStepFilter = false );

    std::vector<RimWellPath*>    addWellPathsToModel( QList<QString>              wellPathFilePaths,
                                                      bool                        importGrouped,
                                                      gsl::not_null<QStringList*> errorMessages );
    void                         addWellPathFormationsToModel( QList<QString> wellPathFilePaths );
    std::vector<RimWellLogFile*> addWellLogsToModel( const QList<QString>&       wellLogFilePaths,
                                                     gsl::not_null<QStringList*> errorMessages );

    QString scriptDirectories() const;
    QString scriptEditorPath() const;

    QString             octavePath() const;
    QStringList         octaveArguments() const;
    QProcessEnvironment octaveProcessEnvironment() const;

    QString             pythonPath() const;
    QProcessEnvironment pythonProcessEnvironment() const;

    bool launchProcess( const QString& program, const QStringList& arguments, const QProcessEnvironment& processEnvironment );
    bool launchProcessForMultipleCases( const QString&             program,
                                        const QStringList&         arguments,
                                        const std::vector<int>&    caseIds,
                                        const QProcessEnvironment& processEnvironment );
    void terminateProcess();
    void waitForProcess() const;

    RiaPreferences* preferences();
    void            applyPreferences();

    QString commandLineParameterHelp();
    void    setCommandLineHelpText( const QString& commandLineHelpText );

    void     setCacheDataObject( const QString& key, const QVariant& dataObject );
    QVariant cacheDataObject( const QString& key ) const;

    void executeCommandFile( const QString& commandFile );
    void addCommandObject( RimCommandObject* commandObject );
    void executeCommandObjects();
    void waitUntilCommandObjectsHasBeenProcessed();

    int launchUnitTests();

    const QString startDir() const;
    void          setStartDir( const QString& startDir );

    static std::vector<QString> readFileListFromTextFile( QString listFileName );

    cvf::Font* defaultSceneFont();
    cvf::Font* sceneFont( int fontSize );
    cvf::Font* defaultAnnotationFont();
    cvf::Font* defaultWellLabelFont();

    bool initializeGrpcServer( const cvf::ProgramOptions& progOpt );

    // Public implementation specific overrides
    virtual void              initialize();
    virtual ApplicationStatus handleArguments( gsl::not_null<cvf::ProgramOptions*> progOpt ) = 0;
    virtual int               launchUnitTestsWithConsole();
    virtual void              addToRecentFiles( const QString& fileName ) {}
    virtual void              showFormattedTextInMessageBoxOrConsole( const QString& errMsg ) = 0;

    virtual void launchGrpcServer() = 0;

#ifdef ENABLE_GRPC
    virtual RiaGrpcServer* grpcServer() const = 0;
#endif
protected:
    // Protected implementation specific overrides
    virtual void invokeProcessEvents( QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents ) = 0;
    virtual void onFileSuccessfullyLoaded( const QString& fileName, RiaDefines::ImportFileType fileType ) {}

    virtual void onProjectBeingOpened() {}
    virtual void onProjectOpened()                              = 0;
    virtual void onProjectOpeningError( const QString& errMsg ) = 0;
    virtual void onProjectBeingClosed() {}
    virtual void onProjectClosed() = 0;
    virtual void onProjectBeingSaved() {}
    virtual void onProjectSaved() {}

    virtual void startMonitoringWorkProgress( caf::UiProcess* uiProcess ) {}
    virtual void stopMonitoringWorkProgress() {}

    void loadAndUpdatePlotData();

    friend class RiaRegressionTestRunner;
    void resetProject();

    bool generateCode( const QString& outputPath, gsl::not_null<QString*> errMsg );

protected:
    cvf::ref<cvf::Font> m_defaultSceneFont;
    cvf::ref<cvf::Font> m_defaultAnnotationFont;
    cvf::ref<cvf::Font> m_defaultWellLabelFont;

    caf::PdmPointer<Rim3dView>  m_activeReservoirView;
    std::unique_ptr<RimProject> m_project;

    QPointer<RiaSocketServer>       m_socketServer;
    std::unique_ptr<caf::UiProcess> m_workerProcess;

#ifdef ENABLE_GRPC
    std::unique_ptr<RiaGrpcServer> m_grpcServer;
#endif

    // Execute for all settings
    std::list<int>                  m_scriptCaseIds;
    int                             m_currentScriptCaseId;
    QString                         m_currentProgram;
    QStringList                     m_currentArguments;
    std::unique_ptr<RiaPreferences> m_preferences;

    std::map<QString, QString> m_fileDialogDefaultDirectories;
    QString                    m_startupDefaultDirectory;
    QString                    m_commandLineHelpText;
    QMap<QString, QVariant>    m_sessionCache; // Session cache used to store username/passwords per session

    std::list<caf::PdmPointer<RimCommandObject>> m_commandQueue;
    QMutex                                       m_commandQueueLock;

    bool m_runningWorkerProcess;

private:
    static RiaApplication* s_riaApplication;
};
