/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiaEnsembleNameTools.h"

#include <QString>

class RimEnsembleWellLogs;

//==================================================================================================
///
//==================================================================================================
class RicImportEnsembleWellLogsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    RicImportEnsembleWellLogsFeature();

    static RimEnsembleWellLogs*
        createSingleEnsembleWellLogsFromFiles( const QStringList&                         fileNames,
                                               RiaEnsembleNameTools::EnsembleGroupingMode groupingMode );

    static std::vector<RimEnsembleWellLogs*>
        createEnsembleWellLogsFromFiles( const QStringList&                         fileNames,
                                         RiaEnsembleNameTools::EnsembleGroupingMode groupingMode );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    std::pair<QStringList, RiaEnsembleNameTools::EnsembleGroupingMode>
        runRecursiveFileSearchDialog( const QString& dialogTitle, const QString& pathCacheName );

private:
    QString m_pathFilter;
    QString m_fileNameFilter;
};
