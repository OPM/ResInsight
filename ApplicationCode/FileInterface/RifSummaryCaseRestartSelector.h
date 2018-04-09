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
#include <QString>

#include <string>
#include <vector>
#include <map>

class QStringList;
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

    std::vector<RifSummaryCaseFileInfo> getFilesToImport(const QStringList& selectedFiles);

private:
    std::vector<RifSummaryCaseFileInfo> getFilesToImportByAskingUser(const QStringList& initialFiles,
                                                                     bool enableApplyToAllField,
                                                                     RiaPreferences::SummaryRestartFilesImportModeType defaultSummaryRestartMode);
    std::vector<RifSummaryCaseFileInfo> getFilesToImportUsingPrefs(const QStringList& initialFiles,
                                                                   RiaPreferences::SummaryRestartFilesImportModeType summaryRestartMode);


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
