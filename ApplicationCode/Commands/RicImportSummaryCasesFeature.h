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

#pragma once

#include "cafCmdFeature.h"

#include <QString>

class RicSummaryCaseRestartDialogResult;


//==================================================================================================
/// 
//==================================================================================================
class RicSummaryCaseFileInfo
{
public:
    RicSummaryCaseFileInfo(const QString _fileName, bool _includeRestartFiles) : 
        fileName(_fileName), includeRestartFiles(_includeRestartFiles) {}

    QString fileName;
    bool    includeRestartFiles;

    bool operator<(const RicSummaryCaseFileInfo& other) const { return fileName < other.fileName; }
    bool operator==(const RicSummaryCaseFileInfo& other) const { return fileName == other.fileName; }
};

//==================================================================================================
/// 
//==================================================================================================
class RicImportSummaryCasesFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    RicImportSummaryCasesFeature() : m_pathFilter("*"), m_fileNameFilter("*") { }

    static std::vector<RicSummaryCaseFileInfo> getFilesToImportWithDialog(const QStringList& initialFiles,
                                                                          bool enableApplyToAllField);

    static bool createAndAddSummaryCaseFromFileInfo(const std::vector<RicSummaryCaseFileInfo>& fileInfos);
    static bool createAndAddSummaryCaseFromFileWithDialog(const QString& fileName);

private:
    static bool createAndAddSummaryCaseFromFile(const QString& fileName, bool includeRestartFiles);

protected:
    // Overrides
    virtual bool isCommandEnabled() override;
    virtual void onActionTriggered( bool isChecked ) override;
    virtual void setupActionLook( QAction* actionToSetup ) override;

private:
    QString m_pathFilter;
    QString m_fileNameFilter;
};


