/////////////////////////////////////////////////////////////////////////////////
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

public:
    static RiaApplication*    instance();
    RiaApplication();
    virtual ~RiaApplication();

    int                parseArgumentsAndRunUnitTestsIfRequested();
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

    QString commandLineParameterHelp() const;

    void     setCacheDataObject(const QString& key, const QVariant& dataObject);
    QVariant cacheDataObject(const QString& key) const;

    void executeCommandFile(const QString& commandFile);
    void addCommandObject(RimCommandObject* commandObject);
    void executeCommandObjects();
    void waitUntilCommandObjectsHasBeenProcessed();

    int launchUnitTests();

    void setStartDir(const QString& startDir);

    static std::vector<QString> readFileListFromTextFile(QString listFileName);

    // Public implementation specific overrides
    virtual void initialize();
    virtual bool parseArguments() = 0;
    virtual int  launchUnitTestsWithConsole() = 0;
    virtual void addToRecentFiles(const QString& fileName) {}
    virtual void showInformationMessage(const QString& infoText) = 0;
    virtual void showErrorMessage(const QString& errMsg) = 0;
protected:
    // Protected implementation specific overrides
    virtual void handleEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents) = 0;
    virtual void onChangedActiveReservoirView() = 0;
    virtual void onFileSuccessfullyLoaded(const QString& fileName, RiaDefines::ImportFileType fileType) = 0;
    virtual void onProjectBeingOpened() = 0;
    virtual void onProjectOpened() = 0;
    virtual void onProjectOpeningError(const QString& errMsg) = 0;
    virtual void onProjectBeingClosed() = 0;
    virtual void onProjectClosed() = 0;
    virtual void startMonitoringWorkProgress(caf::UiProcess* uiProcess) = 0;
    virtual void stopMonitoringWorkProgress() = 0;

protected:
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

    QString m_helpText;

    bool m_runningWorkerProcess;

private:
    static RiaApplication* s_riaApplication;
};



