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
#include "RifReaderEclipseSummary.h"

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
void RifSummaryCaseRestartSelector::determineFilesToImportFromSummaryFiles(const QStringList& initialSummaryFiles)
{
    std::vector<RifSummaryCaseFileImportInfo> files;
    for (QString f : initialSummaryFiles)
    {
        files.push_back(RifSummaryCaseFileImportInfo(f, "", true));
    }
    determineFilesToImport(files);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifSummaryCaseRestartSelector::determineFilesToImportFromGridFiles(const QStringList& initialGridFiles)
{
    std::vector<RifSummaryCaseFileImportInfo> files;
    for (QString f : initialGridFiles)
    {
        files.push_back(RifSummaryCaseFileImportInfo(getSummaryFileFromGridFile(f), f));
    }
    determineFilesToImport(files);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifSummaryCaseRestartSelector::determineFilesToImport(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles)
{
    std::vector<RifSummaryCaseFileInfo> fileInfos;
    if (m_showDialog)
    {
        bool enableApplyToAllField = initialFiles.size() > 1;
        determineFilesToImportByAskingUser(initialFiles, enableApplyToAllField);
    }
    else
    {
        determineFilesToImportUsingPrefs(initialFiles);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifSummaryCaseRestartSelector::determineFilesToImportByAskingUser(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles,
                                                                       bool enableApplyToAllField)
{
    RicSummaryCaseRestartDialogResult lastResult;

    m_summaryFileInfos.clear();
    m_gridFiles.clear();
    m_summaryFileErrors.clear();

    for (const RifSummaryCaseFileImportInfo& initialFile : initialFiles)
    {
        RicSummaryCaseRestartDialogResult result = RicSummaryCaseRestartDialog::openDialog(initialFile.summaryFileName(),
                                                                                           initialFile.gridFileName(),
                                                                                           initialFile.failOnSummaryFileError(),
                                                                                           enableApplyToAllField,
                                                                                           m_defaultSummaryImportMode,
                                                                                           m_defaultGridImportMode,
                                                                                           &lastResult);

        lastResult = result;

        if (result.status == RicSummaryCaseRestartDialogResult::CANCELLED)
        {
            // Cancel pressed, cancel everything and return early
            m_summaryFileInfos.clear();
            m_gridFiles.clear();
            m_summaryFileErrors.clear();
            return;
        }        
        
        if (result.status == RicSummaryCaseRestartDialogResult::ERROR ||
            result.status == RicSummaryCaseRestartDialogResult::SUMMARY_FILE_WARNING)
        {
            // A summary import failure occurred with one of the files. The others may still have worked.
            m_summaryFileErrors.push_back(initialFile.summaryFileName());
        }
        else
        {
            for (const QString& file : result.summaryFiles)
            {
                RifSummaryCaseFileInfo fi(file, result.summaryImportOption == RicSummaryCaseRestartDialog::IMPORT_ALL);
                if (!vectorContains(m_summaryFileInfos, fi))
                {
                    m_summaryFileInfos.push_back(fi);
                }
            }
        }

        if (result.status == RicSummaryCaseRestartDialogResult::OK ||
            result.status == RicSummaryCaseRestartDialogResult::SUMMARY_FILE_WARNING)
        {
            for (const QString& gridFile : result.gridFiles)
            {
                m_gridFiles.push_back(gridFile);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifSummaryCaseRestartSelector::determineFilesToImportUsingPrefs(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles)
{
    RicSummaryCaseRestartDialogResult lastResult;

    m_summaryFileInfos.clear();
    m_gridFiles.clear();
    m_summaryFileErrors.clear();

    for (const RifSummaryCaseFileImportInfo& initialFile : initialFiles)
    {
        QString initialSummaryFile = RiaFilePathTools::toInternalSeparator(initialFile.summaryFileName());
        QString initialGridFile = RiaFilePathTools::toInternalSeparator(initialFile.gridFileName());
        bool handleSummaryFile = false;
        bool handleGridFile = !initialGridFile.isEmpty();        

        RifReaderEclipseSummary reader;
        if (!initialSummaryFile.isEmpty())
        {            
            RifRestartFileInfo fileInfo = reader.getFileInfo(initialSummaryFile);
            if (!fileInfo.valid())
            {
                m_summaryFileErrors.push_back(initialSummaryFile);
                if (initialFile.failOnSummaryFileError())
                {
                    handleGridFile = false;
                }
            }
            else
            {
                handleSummaryFile = true;
            }
        }
        
        if (handleSummaryFile)
        {
            if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::IMPORT_ALL)
            {
                m_summaryFileInfos.push_back(RifSummaryCaseFileInfo(initialSummaryFile, true));
            }
            else if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::NOT_IMPORT)
            {
                m_summaryFileInfos.push_back(RifSummaryCaseFileInfo(initialSummaryFile, false));
            }
            else if (m_defaultSummaryImportMode == RicSummaryCaseRestartDialog::SEPARATE_CASES)
            {
                m_summaryFileInfos.push_back(RifSummaryCaseFileInfo(initialSummaryFile, false));
                bool hasWarnings = false;
                std::vector<RifRestartFileInfo> restartFileInfos = reader.getRestartFiles(initialSummaryFile, &hasWarnings);
                for (const auto& rfi : restartFileInfos)
                {
                    RifSummaryCaseFileInfo fi(RiaFilePathTools::toInternalSeparator(rfi.fileName), false);
                    if (!vectorContains(m_summaryFileInfos, fi))
                    {
                        m_summaryFileInfos.push_back(fi);
                    }
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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryCaseRestartSelector::foundErrors() const
{
    return !m_summaryFileErrors.empty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QStringList& RifSummaryCaseRestartSelector::summaryFilesWithErrors() const
{
    return m_summaryFileErrors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifSummaryCaseRestartSelector::createCombinedErrorMessage() const
{
    QString errorMessage;
    if (!m_summaryFileErrors.empty())
    {
        errorMessage = QString("Failed to import the following summary file");
        if (m_summaryFileErrors.size() > 1)
        {
            errorMessage += QString("s");
        }
        errorMessage += QString(":\n");
        for (const QString& fileWarning : m_summaryFileErrors)
        {
            errorMessage += fileWarning + QString("\n");
        }
    }
    return errorMessage;
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
        QString sumFile = getSummaryFileFromGridFile(gridFile);
        if (!sumFile.isEmpty())
        {
            summaryFiles.push_back(sumFile);
        }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryCaseFileImportInfo::RifSummaryCaseFileImportInfo(const QString& summaryFileName,
    const QString& gridFileName,
    bool           failOnSummaryFileImportError /*= false*/)
    : m_summaryFileName(summaryFileName)
    , m_gridFileName(gridFileName)
    , m_failOnSummaryFileImportError(failOnSummaryFileImportError)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifSummaryCaseFileImportInfo::failOnSummaryFileError() const
{
    return m_failOnSummaryFileImportError;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RifSummaryCaseFileImportInfo::summaryFileName() const
{
    return m_summaryFileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RifSummaryCaseFileImportInfo::gridFileName() const
{
    return m_gridFileName;
}