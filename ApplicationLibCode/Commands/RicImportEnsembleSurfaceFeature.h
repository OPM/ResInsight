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

//==================================================================================================
///
//==================================================================================================
class RicImportEnsembleSurfaceFeature : public caf::CmdFeature
{
public:
    CAF_CMD_HEADER_INIT;

    RicImportEnsembleSurfaceFeature();

    static void importEnsembleSurfaceFromFiles( const QStringList& fileNames, RiaDefines::EnsembleGroupingMode groupingMode );

protected:
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    std::pair<QStringList, RiaDefines::EnsembleGroupingMode> runRecursiveFileSearchDialog( const QString& dialogTitle,
                                                                                           const QString& pathCacheName );

    static void importSingleEnsembleSurfaceFromFiles( const QStringList& fileNames, RiaDefines::EnsembleGroupingMode groupingMode );

private:
    QString m_pathFilter;
    QString m_fileNameFilter;
};
