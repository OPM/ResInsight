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

#include "RiaEnsembleNameTools.h"
#include "RiaSummaryDefines.h"

#include "RicRecursiveFileSearchDialog.h"
#include "cafCmdFeature.h"

#include <QString>
#include <vector>

class RicSummaryCaseRestartDialogResult;
class RimSummaryCase;

//==================================================================================================
///
//==================================================================================================
class RicImportSummaryCasesFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    RicImportSummaryCasesFeature() {}

    static std::pair<bool, std::vector<RimSummaryCase*>> createAndAddSummaryCasesFromFiles( const QStringList& fileName,
                                                                                            bool               doCreateDefaultPlot );

    struct CreateConfig
    {
        RiaDefines::FileType fileType;
        bool                 ensembleOrGroup;
        bool                 allowDialogs;
    };

    static std::pair<bool, std::vector<RimSummaryCase*>> createSummaryCasesFromFiles( const QStringList& fileName, CreateConfig createConfig );

    static void addSummaryCases( const std::vector<RimSummaryCase*>& cases );
    static void addCasesToGroupIfRelevant( const std::vector<RimSummaryCase*>& cases );

    static RicRecursiveFileSearchDialogResult runRecursiveSummaryCaseFileSearchDialog( const QString& dialogTitle,
                                                                                       const QString& pathCacheName );

    static RicRecursiveFileSearchDialogResult runRecursiveSummaryCaseFileSearchDialogWithGrouping( const QString& dialogTitle,
                                                                                                   const QString& pathCacheName );

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static QString m_pathFilter;
    static QString m_fileNameFilter;
};
