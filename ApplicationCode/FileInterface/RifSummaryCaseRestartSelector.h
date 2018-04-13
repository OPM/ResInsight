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
#include <map>

class RifSummaryCaseFileInfo;


//==================================================================================================
//
//
//==================================================================================================
class RifSummaryCaseRestartSelector
{
public:
    RifSummaryCaseRestartSelector();
    ~RifSummaryCaseRestartSelector();

    std::vector<RifSummaryCaseFileInfo> getFilesToImportFromSummaryFiles(const QStringList& initialSummaryFiles);
    std::vector<RifSummaryCaseFileInfo> getFilesToImportFromGridFiles(const QStringList& initialGridFiles);

    void        showDialog(bool show)               { m_showDialog = show; }
    void        buildGridCaseFileList(bool build)   { m_buildGridCaseFileList = build; }
    QStringList gridCaseFiles() const               { return m_gridCaseFiles; }

    static QStringList                  getSummaryFilesFromGridFiles(const QStringList& gridFiles);

private:
    std::vector<RifSummaryCaseFileInfo> getFilesToImport(const QStringList& initialSummaryFiles);

    std::vector<RifSummaryCaseFileInfo> getFilesToImportByAskingUser(const QStringList& initialSummaryFiles,
                                                                     bool enableApplyToAllField);
    std::vector<RifSummaryCaseFileInfo> getFilesToImportUsingPrefs(const QStringList& initialSummaryFiles);

    bool                                                m_showDialog;
    RicSummaryCaseRestartDialog::ReadOptions            m_defaultRestartImportMode;
    bool                                                m_importRestartGridCaseFiles;

    bool                                                m_buildGridCaseFileList;
    QStringList                                         m_gridCaseFiles;
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
