/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025    Equinor ASA
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

#include "RicAddEclipseBorderResultFeature.h"

#include "RiaApplication.h"
#include "RiaDefines.h"

#include "RicImportGeneralDataFeature.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimRoffCase.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QStringList>

CAF_CMD_SOURCE_INIT( RicAddEclipseBorderResultFeature, "RicAddEclipseBorderResultFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddEclipseBorderResultFeature::onActionTriggered( bool isChecked )
{
    RimEclipseView* eclipseView = caf::SelectionManager::instance()->selectedItemOfType<RimEclipseView>();
    if ( !eclipseView ) return;
    if ( auto eCase = eclipseView->firstAncestorOrThisOfType<RimEclipseCase>() )
    {
        auto visibility = eclipseView->currentTotalCellVisibility();

        auto activeReservoirCellIdxs =
            eCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();
        int numActiveCells = static_cast<int>( activeReservoirCellIdxs.size() );

        std::vector<int> result;
        result.resize( numActiveCells, 0 );

        int i = 0;
        // go through all cells, only check those visible
        for ( auto cellIdx : activeReservoirCellIdxs )
        {
            if ( visibility->val( cellIdx ) )
            {
                result[i] = 1;
            }

            i++;
        }

        RigEclipseResultTools::createResultVector( *eCase, "BORDER", result );

        eCase->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddEclipseBorderResultFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Generate Border Cell Result" );
    actionToSetup->setIcon( QIcon( ":/CellResult.png" ) );
}
