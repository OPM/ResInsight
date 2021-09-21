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

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RicImportEnsembleFractureStatisticsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    RicImportEnsembleFractureStatisticsFeature() {}

    static std::pair<QStringList, bool> runRecursiveFileSearchDialog( const QString& dialogTitle,
                                                                      const QString& pathCacheName );

protected:
    static void importSingleEnsembleFractureStatistics( const QStringList& fileNames );

    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static QString m_pathFilter;
    static QString m_fileNameFilter;
};
