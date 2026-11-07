/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicCalculateQcResultsFeature.h"

#include "RiaResultNames.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridQualityResultCalculator.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCalculateQcResultsFeature, "RicCalculateQcResultsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCalculateQcResultsFeature::onActionTriggered( bool isChecked )
{
    const auto selectedItems = caf::SelectionManager::instance()->objectsByType<RimEclipseCellColors>();
    if ( selectedItems.empty() ) return;

    for ( auto item : selectedItems )
    {
        if ( auto eclipseView = item->reservoirView() )
        {
            RimEclipseCase* eCase = eclipseView->eclipseCase();
            RicCalculateQcResultsFeature::calculateResults( eCase );
            eclipseView->cellResult()->setResultType( RiaDefines::ResultCatType::QUALITY_CONTROL );
            eclipseView->cellResult()->setResultVariable( RiaResultNames::qcMaxangleTopbase() );
            eclipseView->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCalculateQcResultsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/qualitycontrol.svg" ) );
    actionToSetup->setText( "Calculate Grid Quality Results" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCalculateQcResultsFeature::calculateResults( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return;
    auto* resultsData = eclipseCase->eclipseCaseData();
    if ( !resultsData ) return;

    if ( auto results = resultsData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
    {
        RigGridQualityResultCalculator::calculateResults( results );
    }
}