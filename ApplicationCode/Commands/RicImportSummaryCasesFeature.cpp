/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportSummaryCasesFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicFileHierarchyDialog.h"
#include "RicSummaryCaseRestartDialog.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportSummaryCasesFeature, "RicImportSummaryCasesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* prefs = app->preferences();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");

    RicFileHierarchyDialogResult result = RicFileHierarchyDialog::getOpenFileNames(nullptr, "Import Summary Cases", defaultDir, m_pathFilter, m_fileNameFilter, QStringList(".SMSPEC"));

    // Remember filters
    m_pathFilter = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if (!result.ok) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(result.rootDir).absoluteFilePath());

    QStringList fileNames = result.files;
    if (fileNames.isEmpty()) return;

    std::vector<RicSummaryCaseFileInfo> fileInfos;
    if (prefs->summaryRestartFilesImportMode == RiaPreferences::ASK_USER)
    {
        fileInfos = getFilesToImportWithDialog(fileNames, true);
    }
    else
    {
        fileInfos = getFilesToImportFromPrefs(fileNames, prefs->summaryRestartFilesImportMode);
    }

    createAndAddSummaryCaseFromFileInfo(fileInfos);

    if (fileInfos.size() > 0) RiaApplication::instance()->addToRecentFiles(fileInfos.front().fileName);

    std::vector<RimCase*> cases;
    app->project()->allCases(cases);

    if (cases.size() == 0)
    {
        RiuMainWindow::instance()->close();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryCase48x48.png"));
    actionToSetup->setText("Import Summary Cases Recursively");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RicSummaryCaseFileInfo> RicImportSummaryCasesFeature::getFilesToImportWithDialog(const QStringList& initialFiles,
                                                                                             bool enableApplyToAllField)
{
    std::set<RicSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& file : initialFiles)
    {
        RicSummaryCaseRestartDialogResult result;
        if (lastResult.applyToAll)  result = lastResult;
        else                        result = RicSummaryCaseRestartDialog::openDialog(file, enableApplyToAllField);

        if (result.ok)
        {
            for (const QString& file : result.files)
            {
                RicSummaryCaseFileInfo fi(file, result.option == RicSummaryCaseRestartDialog::READ_ALL);
                if (filesToImport.count(fi) == 0)
                {
                    filesToImport.insert(fi);
                }
            }
        }
        lastResult = result;
    }
    return std::vector<RicSummaryCaseFileInfo>(filesToImport.begin(), filesToImport.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RicSummaryCaseFileInfo> RicImportSummaryCasesFeature::getFilesToImportFromPrefs(const QStringList& initialFiles,
                                                                                            RiaPreferences::SummaryRestartFilesImportModeType summaryRestartMode)
{
    std::set<RicSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& file : initialFiles)
    {
        if (summaryRestartMode == RiaPreferences::IMPORT)
        {
            filesToImport.insert(RicSummaryCaseFileInfo(file, true));
        }
        else if (summaryRestartMode == RiaPreferences::NOT_IMPORT)
        {
            filesToImport.insert(RicSummaryCaseFileInfo(file, false));
        }
        else if (summaryRestartMode == RiaPreferences::SEPARATE_CASES)
        {
            filesToImport.insert(RicSummaryCaseFileInfo(file, false));

            RifReaderEclipseSummary reader;
            std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(file);
            for (const auto& fi : restartFileInfos)
                filesToImport.insert(RicSummaryCaseFileInfo(fi.fileName, false));
        }
    }
    return std::vector<RicSummaryCaseFileInfo>(filesToImport.begin(), filesToImport.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::createAndAddSummaryCaseFromFileInfo(const std::vector<RicSummaryCaseFileInfo>& fileInfos)
{
    RiaApplication*                 app = RiaApplication::instance();
    RimProject*                     proj = app->project();
    RimSummaryCaseMainCollection*   sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return false;

    for (const auto& fi : fileInfos)
    {
        createAndAddSummaryCaseFromFile(fi.fileName, fi.includeRestartFiles);
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::createAndAddSummaryCaseFromFileWithDialog(const QString& fileName)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* prefs = app->preferences();

    QStringList fileNames({ fileName });
    std::vector<RicSummaryCaseFileInfo> fileInfos;
    if (prefs->summaryRestartFilesImportMode == RiaPreferences::ASK_USER)
    {
        fileInfos = getFilesToImportWithDialog(fileNames, false);
    }
    else
    {
        fileInfos = getFilesToImportFromPrefs(fileNames, prefs->summaryRestartFilesImportMode);
    }


    bool res = createAndAddSummaryCaseFromFileInfo(fileInfos);
    RiaApplication::instance()->addToRecentFiles(fileName);
    return res;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::createAndAddSummaryCaseFromFile(const QString& fileName, bool includeRestartFiles)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return false;

    RimSummaryCase* sumCase = sumCaseColl->createAndAddSummaryCaseFromFileName(fileName, includeRestartFiles);
    sumCaseColl->updateAllRequiredEditors();

    RiuMainPlotWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow)
    {
        mainPlotWindow->selectAsCurrentItem(sumCase);

        mainPlotWindow->updateSummaryPlotToolBar();
    }
    return true;
}

