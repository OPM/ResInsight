/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicImportEnsembleFileSetFeature.h"

#include "RicImportSummaryCasesFeature.h"

#include "EnsembleFileSet/RimEnsembleFileSetTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportEnsembleFileSetFeature, "RicImportEnsembleFileSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleFileSetFeature::isCommandEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFileSetFeature::onActionTriggered( bool isChecked )
{
    const QString pathCacheName = "ENSEMBLE_FILES";
    auto result = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialogWithGrouping( "Import Ensemble", pathCacheName );
    if ( result.ok )
    {
        RimEnsembleFileSetTools::createEnsembleFileSets( result.files, result.groupingMode );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFileSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Cases16x16.png" ) );
    actionToSetup->setText( "Import Ensemble File Set" );
}
