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

    //m_buildGridFileList = false;
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
bool RifSummaryCaseRestartSelector::getFilesToImportFromSummaryFiles(const QStringList& initialSummaryFiles)
{
    std::vector<std::pair<QString, QString>> files;
    for (QString f : initialSummaryFiles)
    {
        files.push_back(std::make_pair(f, ""));
    }
    return getFilesToImport(files);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryCaseRestartSelector::getFilesToImportFromGridFiles(const QStringList& initialGridFiles)
{
    std::vector<std::pair<QString, QString>>    files;
    for (QString f : initialGridFiles)
    {
        files.push_back(std::make_pair(getSummaryFileFromGridFile(f), f));
    }
    return getFilesToImport(files);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryCaseRestartSelector::getFilesToImport(const std::vector<std::pair<QString /*sum*/, QString /*grid*/>>& initialFiles)
{
    std::vector<RifSummaryCaseFileInfo> fileInfos;
    if (m_showDialog)
    {
        bool enableApplyToAllField = initialFiles.size() > 1;
        return getFilesToImportByAskingUser(initialFiles, enableApplyToAllField);
    }
    else
    {
        return getFilesToImportUsingPrefs(initialFiles);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryCaseRestartSelector::getFilesToImportByAskingUser(const std::vector<std::pair<QString /*sum*/, QString /*grid*/>>& initialFiles,
                                                                                                bool enableApplyToAllField)
{
    RicSummaryCaseRestartDialogResult lastResult;

    for (const std::pair<QString, QString>& initialFile : initialFiles)
    {
        RicSummaryCaseRestartDialogResult result = RicSummaryCaseRestartDialog::openDialog(initialFile,
                                                                                           enableApplyToAllField,
                                                                                           m_defaultSummaryImportMode,
                                                                                           m_defaultGridImportMode,
                                                                                           &lastResult);
        if (result.ok)
        {
            for (const QString& file : result.summaryFiles)
            {
                RifSummaryCaseFileInfo fi(file, result.summaryImportOption == RicSummaryCaseRestartDialog::IMPORT_ALL);
                if (!vectorContains(m_summaryFileInfos, fi))
                {
                    m_summaryFileInfos.push_back(fi);
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
            m_summaryFileInfos.clear();
            m_gridFiles.clear();
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryCaseRestartSelector::getFilesToImportUsingPrefs(const std::vector<std::pair<QString /*sum*/, QString /*grid*/>>& initialFiles)
{
    std::vector<RifSummaryCaseFileInfo> filesToImport;
    RicSummaryCaseRestartDialogResult lastResult;

    m_gridFiles.clear();

    for (const std::pair<QString, QString>& initialFile : initialFiles)
    {
        QString initialSummaryFile = RiaFilePathTools::toInternalSeparator(initialFile.first);
        QString initialGridFile = RiaFilePathTools::toInternalSeparator(initialFile.second);
        bool handleGridFile = !initialGridFile.isEmpty();

        if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::IMPORT_ALL)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(initialSummaryFile, true));
        }
        else if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::NOT_IMPORT)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(initialSummaryFile, false));
        }
        else if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::SEPARATE_CASES)
        {
            filesToImport.push_back(RifSummaryCaseFileInfo(initialSummaryFile, false));

            RifReaderEclipseSummary reader;
            bool hasWarnings = false;
            std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(initialSummaryFile, &hasWarnings);
            for (const auto& rfi : restartFileInfos)
            {
                RifSummaryCaseFileInfo fi(rfi.fileName, false);
                if (!vectorContains(filesToImport, fi))
                {
                    filesToImport.push_back(fi);
                }
            }
        }

        if (handleGridFile)
        {
            m_gridFiles.push_back(initialGridFile);

            if (m_defaultGridImportMode == RicSummaryCaseRestartDialog::SEPARATE_CASES)
            {
                RifReaderEclipseSummary reader;
                bool hasWarnings = false;
                std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(initialSummaryFile, &hasWarnings);
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
    return true;
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
        summaryFiles.push_back(getSummaryFileFromGridFile(gridFile));
    }
    return summaryFiles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifSummaryCaseRestartSelector::getSummaryFileFromGridFile(const QString& gridFile)
{
    // Find summary header file names from eclipse case file names
    if (!gridFile.isEmpty())
    {
        QString summaryHeaderFile;
        bool    formatted;

        RifEclipseSummaryTools::findSummaryHeaderFile(gridFile, &summaryHeaderFile, &formatted);

        if (!summaryHeaderFile.isEmpty())
        {
            return summaryHeaderFile;
        }
    }
    return "";
}
