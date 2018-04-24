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

#pragma once

#include "RiaPreferences.h"
#include "RicSummaryCaseRestartDialog.h"

#include <QString>
#include <QStringList>

#include <string>
#include <vector>

class RifSummaryCaseFileInfo;
class RifSummaryCaseFileImportInfo;

//==================================================================================================
//
//
//==================================================================================================
class RifSummaryCaseRestartSelector
{
public:
    RifSummaryCaseRestartSelector();
    ~RifSummaryCaseRestartSelector();

    void determineFilesToImportFromSummaryFiles(const QStringList& initialSummaryFiles);
    void determineFilesToImportFromGridFiles(const QStringList& initialGridFiles);

    void                                showDialog(bool show)               { m_showDialog = show; }
    std::vector<RifSummaryCaseFileInfo> summaryFileInfos() const            { return m_summaryFileInfos; }
    QStringList                         gridCaseFiles() const               { return m_gridFiles; }
    bool                                foundErrors() const;
    const QStringList&                  summaryFilesWithErrors() const;
    QString                             createCombinedErrorMessage() const;


    static QStringList  getSummaryFilesFromGridFiles(const QStringList& gridFiles);
    static QString      getSummaryFileFromGridFile(const QString& gridFile);

private:
    void determineFilesToImport(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles);
    void determineFilesToImportByAskingUser(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles, bool enableApplyToAllField);
    void determineFilesToImportUsingPrefs(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles);

    bool                                       m_showDialog;
    RicSummaryCaseRestartDialog::ImportOptions m_defaultSummaryImportMode;
    RicSummaryCaseRestartDialog::ImportOptions m_defaultGridImportMode;

    std::vector<RifSummaryCaseFileInfo>        m_summaryFileInfos;
    QStringList                                m_gridFiles;
    QStringList                                m_summaryFileErrors;
};

//==================================================================================================
/// 
//==================================================================================================
class RifSummaryCaseFileInfo
{
public:
    RifSummaryCaseFileInfo(const QString _fileName, bool _includeRestartFiles) :
        fileName(_fileName), includeRestartFiles(_includeRestartFiles) {}

    QString fileName;
    bool    includeRestartFiles;

    bool operator<(const RifSummaryCaseFileInfo& other) const { return fileName < other.fileName; }
    bool operator==(const RifSummaryCaseFileInfo& other) const { return fileName == other.fileName; }
};

//==================================================================================================
///  
//==================================================================================================
class RifSummaryCaseFileImportInfo
{
public:
    RifSummaryCaseFileImportInfo(const QString& summaryFileName, const QString& gridFileName, bool failOnSummaryFileImportError = false);

    const QString& summaryFileName() const;
    const QString& gridFileName() const;
    bool           failOnSummaryFileError() const;

private:
    QString m_summaryFileName;
    QString m_gridFileName;
    bool    m_failOnSummaryFileImportError;
};
