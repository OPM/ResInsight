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
#include <QApplication>
#include <QProcess>
#include <QMutex>

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfFont.h"

#include <iostream>

class RIProcess;
class RigCaseData;
class RimCase;
class Drawable;
class RiaSocketServer;
class RiaPreferences;
class RimReservoirView;
class RimProject;
class RimCommandObject;
class RiaProjectModifier;

namespace caf
{
    class UiProcess;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RiaApplication : public QApplication
{
    Q_OBJECT

public:
    enum RINavigationPolicy
    {
        NAVIGATION_POLICY_CEETRON,
        NAVIGATION_POLICY_CAD
    };

public:
    RiaApplication(int& argc, char** argv);
    ~RiaApplication();
    static RiaApplication* instance();

    bool                    parseArguments();

    void                    executeRegressionTests(const QString& regressionTestPath);

    void                    setActiveReservoirView(RimReservoirView*);
    RimReservoirView*       activeReservoirView();
    const RimReservoirView* activeReservoirView() const;

    void                scheduleDisplayModelUpdateAndRedraw(RimReservoirView* resViewToUpdate);

    RimProject*         project(); 

    void                createMockModel();
    void                createResultsMockModel();
    void                createLargeResultsMockModel();
    void                createMockModelCustomized();
    void                createInputMockModel();

    QString             defaultFileDialogDirectory(const QString& dialogName);
    void                setDefaultFileDialogDirectory(const QString& dialogName, const QString& defaultDirectory);

    bool                openEclipseCaseFromFile(const QString& fileName);
    bool                openEclipseCase(const QString& caseName, const QString& caseFileName);
    bool                addEclipseCases(const QStringList& fileNames);
    bool                openInputEclipseCaseFromFileNames(const QStringList& fileNames);

    QString             currentProjectFileName() const;
    QString             createAbsolutePathFromProjectRelativePath(QString projectRelativePath);
    bool                loadProject(const QString& projectFileName);
    bool                saveProject();
    bool                saveProjectAs(const QString& fileName);
    bool                saveProjectPromptForFileName();
    bool                closeProject(bool askToSaveIfDirty);
    void                addWellPathsToModel(QList<QString> wellPathFilePaths);

    void                copySnapshotToClipboard();
    void                saveSnapshotPromtpForFilename();
    void                saveSnapshotAs(const QString& fileName);
    void                saveSnapshotForAllViews(const QString& snapshotFolderName);
    void                runMultiCaseSnapshots(const QString& templateProjectFileName, std::vector<QString> gridFileNames, const QString& snapshotFolderName);
    void                runRegressionTest(const QString& testRootPath);
    void                updateRegressionTest(const QString& testRootPath );
    void                regressionTestConfigureProject();

    void                processNonGuiEvents();

    static const char*  getVersionStringApp(bool includeCrtInfo);

    void                setUseShaders(bool enable);
    bool                useShaders() const;

    void                setShowPerformanceInfo(bool enable);
    bool                showPerformanceInfo() const;

    RINavigationPolicy  navigationPolicy() const;
    QString             scriptDirectories() const;
    QString             scriptEditorPath() const;
    
    QString             octavePath() const;
    QStringList         octaveArguments() const;

    bool                launchProcess(const QString& program, const QStringList& arguments);
    bool                launchProcessForMultipleCases(const QString& program, const QStringList& arguments, const std::vector<int>& caseIds);
    void                terminateProcess();
    
    RiaPreferences*     preferences();
    void                readFieldsFromApplicationStore(caf::PdmObject* object, const QString context = "");
    void                writeFieldsToApplicationStore(const caf::PdmObject* object, const QString context = "");
    void                applyPreferences();

    cvf::Font*          standardFont();

    QString             commandLineParameterHelp() const;
    void                showFormattedTextInMessageBox(const QString& text);

    void                setCacheDataObject(const QString& key, const QVariant& dataObject);
    QVariant            cacheDataObject(const QString& key) const;

    void                addCommandObject(RimCommandObject* commandObject);
    void                executeCommandObjects();

private:
    enum ProjectLoadAction
    {
        PLA_NONE = 0,
        PLA_CALCULATE_STATISTICS = 1
    };

    bool                    loadProject(const QString& projectFileName, ProjectLoadAction loadAction, RiaProjectModifier* projectModifier);
    void                    onProjectOpenedOrClosed();
    std::vector<QString>    readFileListFromTextFile(QString listFileName);
    void                    setWindowCaptionFromAppState();
    
    QImage                  grabFrameBufferImage();

private slots:
    void                slotWorkerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void                slotUpdateScheduledDisplayModels();

private:
    caf::PdmPointer<RimReservoirView>   m_activeReservoirView;
    caf::PdmPointer<RimProject>         m_project;

    std::vector<caf::PdmPointer<RimReservoirView> > m_resViewsToUpdate;
    QTimer*                             m_resViewUpdateTimer;

    RiaSocketServer*                    m_socketServer;

    caf::UiProcess*                     m_workerProcess;

    // Execute for all settings
    std::list<int>                      m_currentCaseIds;
    QString                             m_currentProgram;
    QStringList                         m_currentArguments;

    RiaPreferences*                     m_preferences;

    std::map<QString, QString>          m_fileDialogDefaultDirectories;
    QString                             m_startupDefaultDirectory;

    cvf::ref<cvf::Font>                 m_standardFont;

    QMap<QString, QVariant>             m_sessionCache;     // Session cache used to store username/passwords per session

    std::list<RimCommandObject*>       m_commandQueue;
    QMutex                             m_commandQueueLock;

    QString                            m_helpText;
};
