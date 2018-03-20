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
    if (prefs->summaryRestartFilesImportMode == RiaPreferences::ASK_USER)
    {
        bool enableApplyToAllField = selectedFiles.size() > 1;
        fileInfos = getFilesToImportByAskingUser(selectedFiles, enableApplyToAllField);
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
                                                                                                bool enableApplyToAllField)
{
    std::vector<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& file : initialFiles)
    {
        RicSummaryCaseRestartDialogResult result = RicSummaryCaseRestartDialog::openDialog(file, enableApplyToAllField, &lastResult);
        if (result.ok)
        {
            for (const QString& file : result.files)
            {
                RifSummaryCaseFileInfo fi(file, result.option == RicSummaryCaseRestartDialog::READ_ALL);
                if (!vectorContains(filesToImport, fi))
                {
                    filesToImport.push_back(fi);
                }
            }
        }
        lastResult = result;
    }
    return std::vector<RifSummaryCaseFileInfo>(filesToImport.begin(), filesToImport.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportUsingPrefs(const QStringList& initialFiles,
                                                                                              RiaPreferences::SummaryRestartFilesImportModeType summaryRestartMode)
{
    std::set<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& file : initialFiles)
    {
        if (summaryRestartMode == RiaPreferences::IMPORT)
        {
            filesToImport.insert(RifSummaryCaseFileInfo(file, true));
        }
        else if (summaryRestartMode == RiaPreferences::NOT_IMPORT)
        {
            filesToImport.insert(RifSummaryCaseFileInfo(file, false));
        }
        else if (summaryRestartMode == RiaPreferences::SEPARATE_CASES)
        {
            filesToImport.insert(RifSummaryCaseFileInfo(file, false));

            RifReaderEclipseSummary reader;
            std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(file);
            for (const auto& fi : restartFileInfos)
                filesToImport.insert(RifSummaryCaseFileInfo(fi.fileName, false));
        }
    }
    return std::vector<RifSummaryCaseFileInfo>(filesToImport.begin(), filesToImport.end());
}
