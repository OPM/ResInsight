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
#include "RiaLogging.h"

#include "RicSummaryCaseRestartDialog.h"

#include "RifEclipseSummaryTools.h"

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
RicSummaryCaseRestartDialog::ImportOptions mapReadOption(RiaPreferences::SummaryRestartFilesImportMode mode)
{
    return
        mode == RiaPreferences::SummaryRestartFilesImportMode::NOT_IMPORT ? RicSummaryCaseRestartDialog::ImportOptions::NOT_IMPORT :
        mode == RiaPreferences::SummaryRestartFilesImportMode::SEPARATE_CASES ? RicSummaryCaseRestartDialog::ImportOptions::SEPARATE_CASES :
        RicSummaryCaseRestartDialog::ImportOptions::IMPORT_ALL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryCaseRestartSelector::RifSummaryCaseRestartSelector()
{
    RiaPreferences* prefs = RiaApplication::instance()->preferences();
    m_showDialog = prefs->summaryRestartFilesShowImportDialog();
    m_defaultSummaryImportMode = mapReadOption(prefs->summaryImportMode());
    m_defaultGridImportMode = mapReadOption(prefs->gridImportMode());

    m_buildGridFileList = false;
    m_gridFiles.clear();
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
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportFromSummaryFiles(const QStringList& initialSummaryFiles)
{
    return getFilesToImport(initialSummaryFiles);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportFromGridFiles(const QStringList& initialGridFiles)
{
    QStringList summaryFiles = getSummaryFilesFromGridFiles(initialGridFiles);
    return getFilesToImport(summaryFiles);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImport(const QStringList& initialSummaryFiles)
{
    std::vector<RifSummaryCaseFileInfo> fileInfos;
    if (m_showDialog)
    {
        bool enableApplyToAllField = initialSummaryFiles.size() > 1;
        fileInfos = getFilesToImportByAskingUser(initialSummaryFiles, enableApplyToAllField);
    }
    else
    {
        fileInfos = getFilesToImportUsingPrefs(initialSummaryFiles);
    }
    return fileInfos;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportByAskingUser(const QStringList& initialSummaryFiles,
                                                                                                bool enableApplyToAllField)
{
    std::vector<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    for (const QString& summaryFile : initialSummaryFiles)
    {
        RicSummaryCaseRestartDialogResult result = RicSummaryCaseRestartDialog::openDialog(summaryFile,
                                                                                           enableApplyToAllField,
                                                                                           m_buildGridFileList,
                                                                                           m_defaultSummaryImportMode,
                                                                                           m_defaultGridImportMode,
                                                                                           &lastResult);
        if (result.ok)
        {
            for (const QString& file : result.summaryFiles)
            {
                RifSummaryCaseFileInfo fi(file, result.summaryImportOption == RicSummaryCaseRestartDialog::IMPORT_ALL);
                if (!vectorContains(filesToImport, fi))
                {
                    filesToImport.push_back(fi);
                }
            }
            lastResult = result;

            for (const QString& gridFile : result.gridFiles)
            {
                m_gridFiles.push_back(gridFile);
            }
        }
        else
        {
            // Cancel pressed, cancel everything
            m_gridFiles.clear();
            return std::vector<RifSummaryCaseFileInfo>();
        }
    }
    return std::vector<RifSummaryCaseFileInfo>(filesToImport.begin(), filesToImport.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RifSummaryCaseFileInfo> RifSummaryCaseRestartSelector::getFilesToImportUsingPrefs(const QStringList& initialSummaryFiles)
{
    std::vector<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    m_gridFiles.clear();

    for (const QString& summaryFile : initialSummaryFiles)
    {
        QString file = RiaFilePathTools::toInternalSeparator(summaryFile);

        if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::IMPORT_ALL)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(file, true));
        }
        else if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::NOT_IMPORT)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(file, false));
        }
        else if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::SEPARATE_CASES)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(file, false));

            RifReaderEclipseSummary reader;
            bool hasWarnings = false;
            std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(file, &hasWarnings);
            for (const auto& rfi : restartFileInfos)
            {
                RifSummaryCaseFileInfo fi(rfi.fileName, false);
                if (!vectorContains(filesToImport, fi))
                {
                    filesToImport.push_back(fi);
                }
            }
        }

        if (m_buildGridFileList)
        {
            m_gridFiles.push_back(RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(file));

            if (m_defaultGridImportMode == RicSummaryCaseRestartDialog::SEPARATE_CASES)
            {
                RifReaderEclipseSummary reader;
                bool hasWarnings = false;
                std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(file, &hasWarnings);
                for (const auto& rfi : restartFileInfos)
                {
                    RifSummaryCaseFileInfo fi(RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile(rfi.fileName), false);
                    if (!m_gridFiles.contains(fi.fileName) && QFileInfo(fi.fileName).exists())
                    {
                        m_gridFiles.push_back(fi.fileName);
                    }
                }

                if (hasWarnings)
                {
                    for (const QString& warning : reader.warnings()) RiaLogging::error(warning);
                }
            }
        }
    }
    return filesToImport;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RifSummaryCaseRestartSelector::getSummaryFilesFromGridFiles(const QStringList& gridFiles)
{
    QStringList summaryFiles;

    // Find summary header file names from eclipse case file names
    for (const auto& gridFile : gridFiles)
    {
        if (!gridFile.isEmpty())
        {
            QString summaryHeaderFile;
            bool    formatted;

            RifEclipseSummaryTools::findSummaryHeaderFile(gridFile, &summaryHeaderFile, &formatted);

            if (!summaryHeaderFile.isEmpty())
            {
                summaryFiles.push_back(summaryHeaderFile);
            }
        }
    }
    return summaryFiles;
}
