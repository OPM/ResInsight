/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimEclipseViewTools.h"

#include "RiaModelExportDefines.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
#include "RigMainGrid.h"
#include "Well/RigSimWellData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"

namespace RimEclipseViewTools
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigSimWellData*> getVisibleSimulationWells( RimEclipseView* view )
{
    std::vector<const RigSimWellData*> visibleWells;

    if ( !view ) return visibleWells;

    // Get well collection from view
    RimSimWellInViewCollection* wellCollection = view->wellCollection();
    if ( !wellCollection ) return visibleWells;

    // Iterate through visible wells in the collection
    for ( RimSimWellInView* rimWell : wellCollection->wells() )
    {
        if ( rimWell && rimWell->showWell() && rimWell->simWellData() )
        {
            visibleWells.push_back( rimWell->simWellData() );
        }
    }

    return visibleWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<caf::VecIjk0, caf::VecIjk0> computeVisibleWellCells( RimEclipseView* view, RigEclipseCaseData* caseData, int visibleWellsPadding )
{
    if ( view )
    {
        // Get visible simulation wells from the view
        std::vector<const RigSimWellData*> visibleWells = RimEclipseViewTools::getVisibleSimulationWells( view );

        if ( !visibleWells.empty() )
        {
            // Get current time step
            int currentTimeStep = view->currentTimeStep();

            // Calculate wells bounding box IJK
            auto [minIjk, maxIjk] = RigEclipseCaseDataTools::wellsBoundingBoxIjk( caseData, visibleWells, currentTimeStep, true, true );
            if ( !minIjk.isUndefined() && !maxIjk.isUndefined() )
            {
                // Apply user-defined padding
                size_t padding                  = static_cast<size_t>( std::max( 0, visibleWellsPadding ) );
                auto [expandedMin, expandedMax] = RigEclipseCaseDataTools::expandBoundingBoxIjk( caseData, minIjk, maxIjk, padding );

                if ( !expandedMin.isUndefined() && !expandedMax.isUndefined() )
                {
                    // Use 0-based indexing as expected by setMin/setMax
                    return { expandedMin, expandedMax };
                }
                else
                {
                    // Fallback without padding
                    return { minIjk, maxIjk };
                }
            }
        }
    }

    // No view available, fallback to full grid
    const RigMainGrid* mainGrid  = caseData->mainGrid();
    cvf::Vec3st        maxCounts = mainGrid->cellCounts() - cvf::Vec3st( 1, 1, 1 );
    return { caf::VecIjk0::ZERO, caf::VecIjk0( maxCounts.x(), maxCounts.y(), maxCounts.z() ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<caf::VecIjk0, caf::VecIjk0> getVisibleCellRange( RimEclipseView* view, const cvf::UByteArray& cellVisibillity )
{
    const RigMainGrid* mainGrid = view->eclipseCase()->mainGrid();
    caf::VecIjk0       max      = caf::VecIjk0::ZERO;
    caf::VecIjk0       min( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );

    size_t cellCount = mainGrid->cellCount();
    for ( size_t index = 0; index < cellCount; ++index )
    {
        if ( cellVisibillity[index] )
        {
            auto ijk = mainGrid->ijkFromCellIndex( index );
            if ( ijk.has_value() )
            {
                for ( int n = 0; n < 3; ++n )
                {
                    min[n] = std::min( min[n], ijk.value()[n] );
                    max[n] = std::max( max[n], ijk.value()[n] );
                }
            }
        }
    }
    return std::make_pair( min, max );
}

} // namespace RimEclipseViewTools
