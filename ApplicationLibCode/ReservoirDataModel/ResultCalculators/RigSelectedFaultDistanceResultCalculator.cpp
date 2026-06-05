/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigSelectedFaultDistanceResultCalculator.h"

#include "RiaDefines.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFaultDistanceCalculator.h"
#include "RigMainGrid.h"

#include "cafProgressInfo.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSelectedFaultDistanceResultCalculator::compute( RigEclipseCaseData*                 caseData,
                                                        const QString&                      resultName,
                                                        const std::vector<const RigFault*>& selectedFaults )
{
    if ( !caseData || resultName.isEmpty() ) return;

    RigCaseCellResultsData* resultsData = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    const RigActiveCellInfo* activeCellInfo = resultsData->activeCellInfo();
    if ( !activeCellInfo ) return;

    const size_t activeCellCount = activeCellInfo->reservoirActiveCellCount();
    if ( activeCellCount == 0 ) return;

    caf::ProgressInfo progressInfo( 3, "Computing fault distance" );

    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );

    progressInfo.setProgressDescription( "Preparing result storage" );
    if ( !resultsData->hasResultEntry( resultAddress ) )
    {
        resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, activeCellCount );
    }

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    if ( !resultVector ) return;

    resultVector->assign( activeCellCount, std::numeric_limits<double>::infinity() );
    progressInfo.incrementProgress();

    progressInfo.setProgressDescription( "Computing distances to faults" );
    RigFaultDistanceCalculator::computeFaultDistances( caseData->mainGrid(), activeCellInfo, selectedFaults, *resultVector );
    progressInfo.incrementProgress();

    progressInfo.setProgressDescription( "Updating statistics" );
    resultsData->recalculateStatistics( resultAddress );
    progressInfo.incrementProgress();
}
