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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfBase.h"
#include "cvfFont.h"
#include "cvfObject.h"

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
    enum ProjectLoadAction
    {
        PLA_NONE                 = 0,
        PLA_CALCULATE_STATISTICS = 1
    };

    enum ApplicationStatus
    {
        KEEP_GOING = 0,
        EXIT_COMPLETED,
        EXIT_WITH_ERROR
    };

public:
    static RiaApplication*    instance();
    RiaApplication();
    virtual ~RiaApplication();

    static const char* getVersionStringApp(bool includeCrtInfo);
    static bool        enableDevelopmentFeatures();
    
    void             setActiveReservoirView(Rim3dView*);
    Rim3dView*       activeReservoirView();
    const Rim3dView* activeReservoirView() const;
    RimGridView*     activeGridView();

    RimProject* project();

    void createMockModel();
    void createResultsMockModel();
    void createLargeResultsMockModel();
    void createMockModelCustomized();
    void createInputMockModel();

    bool openFile(const QString& fileName);

    QString     currentProjectPath() const;
    QString     createAbsolutePathFromProjectRelativePath(QString projectRelativePath);
    bool        loadProject(const QString& projectFileName);
    bool        loadProject(const QString& projectFileName, ProjectLoadAction loadAction, RiaProjectModifier* projectModifier);
    bool        saveProjectAs(const QString& fileName, QString* errorMessage);
    static bool hasValidProjectFileExtension(const QString& fileName);
    void        closeProject();

    QString lastUsedDialogDirectory(const QString& dialogName);
    QString lastUsedDialogDirectoryWithFallbackToProjectFolder(const QString& dialogName);
    QString lastUsedDialogDirectoryWithFallback(const QString& dialogName, const QString& fallbackDirectory);
    void    setLastUsedDialogDirectory(const QString& dialogName, const QString& directory);

    bool openOdbCaseFromFile(const QString& fileName, bool applyTimeStepFilter = false);

    void addWellPathsToModel(QList<QString> wellPathFilePaths);
    void addWellPathFormationsToModel(QList<QString> wellPathFilePaths);
    void addWellLogsToModel(const QList<QString>& wellLogFilePaths);

    QString scriptDirectories() const;
    QString scriptEditorPath() const;

    QString     octavePath() const;
    QStringList octaveArguments() const;

    bool launchProcess(const QString& program, const QStringList& arguments);
    bool launchProcessForMultipleCases(const QString& program, const QStringList& arguments, const std::vector<int>& caseIds);
    void terminateProcess();
    void waitForProcess() const;

    RiaPreferences* preferences();
    void            applyPreferences(const RiaPreferences* oldPreferences = nullptr);

    static QString commandLineParameterHelp();

    void     setCacheDataObject(const QString& key, const QVariant& dataObject);
    QVariant cacheDataObject(const QString& key) const;

    void executeCommandFile(const QString& commandFile);
    void addCommandObject(RimCommandObject* commandObject);
    void executeCommandObjects();
    void waitUntilCommandObjectsHasBeenProcessed();

    int launchUnitTests();

    const QString startDir() const;
    void setStartDir(const QString& startDir);

    static std::vector<QString> readFileListFromTextFile(QString listFileName);

    cvf::Font* defaultSceneFont();
    cvf::Font* defaultAnnotationFont();
    cvf::Font* defaultWellLabelFont();

    // Public implementation specific overrides
    virtual void initialize();
    virtual ApplicationStatus handleArguments(cvf::ProgramOptions* progOpt) = 0;
    virtual int  launchUnitTestsWithConsole();
    virtual void addToRecentFiles(const QString& fileName) {}
    virtual void showInformationMessage(const QString& infoText) = 0;
    virtual void showErrorMessage(const QString& errMsg) = 0;
    virtual void cleanupBeforeProgramExit() {}

protected:
    // Protected implementation specific overrides
    virtual void handleEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents) = 0;
    virtual void onChangedActiveReservoirView() {}
    virtual void onFileSuccessfullyLoaded(const QString& fileName, RiaDefines::ImportFileType fileType) {}
    virtual void onProjectBeingOpened() {}
    virtual void onProjectOpened() = 0;
    virtual void onProjectOpeningError(const QString& errMsg) = 0;
    virtual void onProjectBeingClosed() {}
    virtual void onProjectClosed() = 0;
    virtual void startMonitoringWorkProgress(caf::UiProcess* uiProcess) {}
    virtual void stopMonitoringWorkProgress() {}

protected:
    cvf::ref<cvf::Font> m_defaultSceneFont;
    cvf::ref<cvf::Font> m_defaultAnnotationFont;
    cvf::ref<cvf::Font> m_defaultWellLabelFont;

    caf::PdmPointer<Rim3dView>  m_activeReservoirView;
    caf::PdmPointer<RimProject> m_project;

    RiaSocketServer* m_socketServer;
    caf::UiProcess*  m_workerProcess;

    // Execute for all settings
    std::list<int> m_currentCaseIds;
    QString        m_currentProgram;
    QStringList    m_currentArguments;

    RiaPreferences* m_preferences;

    std::map<QString, QString> m_fileDialogDefaultDirectories;
    QString                    m_startupDefaultDirectory;

    QMap<QString, QVariant> m_sessionCache; // Session cache used to store username/passwords per session

    std::list<RimCommandObject*> m_commandQueue;
    QMutex                       m_commandQueueLock;

    bool m_runningWorkerProcess;

private:
    static RiaApplication* s_riaApplication;
};



