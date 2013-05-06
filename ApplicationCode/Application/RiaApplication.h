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

#pragma once
#include <QApplication>
#include <QProcess>
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

    void                    setActiveReservoirView(RimReservoirView*);
    RimReservoirView*       activeReservoirView();
    const RimReservoirView* activeReservoirView() const;

    RimProject*         project(); 

    void                createMockModel();
    void                createResultsMockModel();
    void                createLargeResultsMockModel();
    void                createInputMockModel();

    QString             defaultFileDialogDirectory(const QString& dialogName);
    void                setDefaultFileDialogDirectory(const QString& dialogName, const QString& defaultDirectory);

    bool                openEclipseCaseFromFile(const QString& fileName);
    bool                openEclipseCase(const QString& caseName, const QString& caseFileName);
    bool                addEclipseCases(const QStringList& fileNames);
    bool                openInputEclipseCase(const QString& caseName, const QStringList& caseFileNames);

    bool                loadLastUsedProject();
    QString             currentProjectFileName() const;
    bool                loadProject(const QString& fileName);
    bool                saveProject();
    bool                saveProjectAs(const QString& fileName);
    bool                saveProjectPromptForFileName();
    bool                closeProject(bool askToSaveIfDirty);
    
    void                copySnapshotToClipboard();
    void                saveSnapshotPromtpForFilename();
    void                saveSnapshotAs(const QString& fileName);
    void                saveSnapshotForAllViews(const QString& snapshotFolderName);
    void                runRegressionTest(const QString& testRootPath);
    void                updateRegressionTest(const QString& testRootPath );

    void                processNonGuiEvents();

    static const char*	getVersionStringApp(bool includeCrtInfo);

    void                setUseShaders(bool enable);
    bool                useShaders() const;

    void                setShowPerformanceInfo(bool enable);
    bool                showPerformanceInfo() const;

    RINavigationPolicy  navigationPolicy() const;
    QString             scriptDirectories() const;
    QString             scriptEditorPath() const;
    QString             octavePath() const;

    bool                launchProcess(const QString& program, const QStringList& arguments);
    void                terminateProcess();
    
    RiaPreferences*     preferences();
    void                readPreferences();
    void                writePreferences();
    void                applyPreferences();

    cvf::Font*          standardFont();

private:
    void		        onProjectOpenedOrClosed();
    void		        setWindowCaptionFromAppState();
    
    QImage              grabFrameBufferImage();

private slots:
    void                slotWorkerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);


private:
    caf::PdmPointer<RimReservoirView>   m_activeReservoirView;
    caf::PdmPointer<RimProject>         m_project;

    RiaSocketServer*                    m_socketServer;

    caf::UiProcess*                     m_workerProcess;

    RiaPreferences*                     m_preferences;

    std::map<QString, QString>          m_fileDialogDefaultDirectories;
    QString                             m_startupDefaultDirectory;

    cvf::ref<cvf::Font>                 m_standardFont;
};
