/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifSummaryCaseRestartSelector.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaFilePathTools.h"

#include "RicSummaryCaseRestartDialog.h"

#include <string>
#include <assert.h>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDir>


//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
template<typename T>
bool vectorContains(const std::vector<T>& vector, T item)
{
    for (const auto& i : vector)
    {
        if (i == item) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// INternal function
//--------------------------------------------------------------------------------------------------
RicSummaryCaseRestartDialog::ReadOptions mapReadOption(RiaPreferences::SummaryRestartFilesImportMode mode)
{
    return
        mode == RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT ? RicSummaryCaseRestartDialog::ReadOptions::NOT_IMPORT :
        mode == RiaPreferences::SummaryRestartFilesImportMode::SEPARATE_CASES ? RicSummaryCaseRestartDialog::ReadOptions::SEPARATE_CASES :
        RicSummaryCaseRestartDialog::ReadOptions::IMPORT_ALL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryCaseRestartSelector::RifSummaryCaseRestartSelector()
{
	
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryCaseRestartSelector::~RifSummaryCaseRestartSelector()
{
	
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImport(const QStringList& selectedFiles)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* prefs = app->preferences();

    std::vector<RifSummaryCaseFileInfo> fileInfos;
    if (prefs->summaryRestartFilesShowImportDialog)
    {
        bool enableApplyToAllField = selectedFiles.size() > 1;
        fileInfos = getFilesToImportByAskingUser(selectedFiles, enableApplyToAllField, prefs->summaryRestartFilesImportMode);
    }
    else
    {
        fileInfos = getFilesToImportUsingPrefs(selectedFiles, prefs->summaryRestartFilesImportMode);
    }

    return fileInfos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportByAskingUser(const QStringList& initialFiles,
                                                                                                bool enableApplyToAllField,
                                                                                                RiaPreferences::SummaryRestartFilesImportModeType defaultSummaryRestartMode)
{
    std::vector<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& file : initialFiles)
    {
        RicSummaryCaseRestartDialogResult result = RicSummaryCaseRestartDialog::openDialog(file, enableApplyToAllField, mapReadOption(defaultSummaryRestartMode), &lastResult);
        if (result.ok)
        {
            for (const QString& file : result.files)
            {
                RifSummaryCaseFileInfo fi(file, result.option == RicSummaryCaseRestartDialog::IMPORT_ALL);
                if (!vectorContains(filesToImport, fi))
                {
                    filesToImport.push_back(fi);
                }
            }
            lastResult = result;
        }
    }
    return std::vector<RifSummaryCaseFileInfo>(filesToImport.begin(), filesToImport.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportUsingPrefs(const QStringList& initialFiles,
                                                                                              RiaPreferences::SummaryRestartFilesImportModeType summaryRestartMode)
{
    std::vector<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& initialFile : initialFiles)
    {
        QString file = RiaFilePathTools::toInternalSeparator(initialFile);

        if (summaryRestartMode == RiaPreferences::IMPORT)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(file, true));
        }
        else if (summaryRestartMode == RiaPreferences::NOT_IMPORT)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(file, false));
        }
        else if (summaryRestartMode == RiaPreferences::SEPARATE_CASES)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(file, false));

            RifReaderEclipseSummary reader;
            std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(file);
            for (const auto& rfi : restartFileInfos)
            {
                RifSummaryCaseFileInfo fi(rfi.fileName, false);
                if (!vectorContains(filesToImport, fi))
                {
                    filesToImport.push_back(fi);
                }
            }
        }
    }
    return filesToImport;
}
