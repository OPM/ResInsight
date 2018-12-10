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

class RifSummaryCaseFileImportInfo;
class RifSummaryCaseFileResultInfo;

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

    void                                      showDialog(bool show);
    void                                      setEnsembleOrGroupMode(bool eogMode);
    std::vector<RifSummaryCaseFileResultInfo> summaryFileInfos() const;
    QStringList                               gridCaseFiles() const;
    bool                                      foundErrors() const;
    QString                                   createCombinedErrorMessage() const;

    static QString     getSummaryFileFromGridFile(const QString& gridFile);

private:
    void determineFilesToImport(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles);
    void determineFilesToImportByAskingUser(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles, bool enableApplyToAllField);
    void determineFilesToImportUsingPrefs(const std::vector<RifSummaryCaseFileImportInfo>& initialFiles);

    bool                                       m_showDialog;
    bool                                       m_ensembleOrGroupMode;
    RicSummaryCaseRestartDialog::ImportOptions m_defaultSummaryImportMode;
    RicSummaryCaseRestartDialog::ImportOptions m_defaultGridImportMode;

    std::vector<RifSummaryCaseFileResultInfo> m_summaryFileInfos;
    QStringList                               m_gridFiles;
    QStringList                               m_summaryFileErrors;
};

//==================================================================================================
/// 
//==================================================================================================
class RifSummaryCaseFileImportInfo
{
public:
    RifSummaryCaseFileImportInfo(const QString& summaryFileName,
                                 const QString& gridFileName);

    const QString& summaryFileName() const;
    const QString& gridFileName() const;
    bool           failOnSummaryFileError() const;
    void           setFailOnSummaryFileError(bool failOnSummaryFileImportError);

private:
    QString m_summaryFileName;
    QString m_gridFileName;
    bool    m_failOnSummaryFileImportError;
};

//==================================================================================================
///
//==================================================================================================
class RifSummaryCaseFileResultInfo
{
public:
    RifSummaryCaseFileResultInfo(const QString& summaryFileName, 
                                 bool           includeRestartFiles);

    const QString& summaryFileName() const;
    bool           includeRestartFiles() const;

    bool operator<(const RifSummaryCaseFileResultInfo& other) const;
    bool operator==(const RifSummaryCaseFileResultInfo& other) const;

private:
    QString m_summaryFileName;
    bool    m_includeRestartFiles;
};

