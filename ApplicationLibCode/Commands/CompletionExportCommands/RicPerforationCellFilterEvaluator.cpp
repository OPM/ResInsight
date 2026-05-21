/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RicPerforationCellFilterEvaluator.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"

#include "RimCellFilter.h"
#include "RimEclipseCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPerforationCellFilterEvaluator::RicPerforationCellFilterEvaluator( RimCellFilter* filter, const RimEclipseCase* eclipseCase )
    : m_filter( filter )
    , m_eclipseCase( eclipseCase )
    , m_enabled( false )
    , m_rejectedCellCount( 0 )
{
    if ( m_filter && m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->mainGrid() )
    {
        m_enabled = m_filter->isActive() && m_filter->isFilterEnabled();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPerforationCellFilterEvaluator::isEnabled() const
{
    return m_enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPerforationCellFilterEvaluator::includesGlobalCell( size_t globalCellIndex ) const
{
    if ( !m_enabled ) return true;

    const RigMainGrid* mainGrid = m_eclipseCase->eclipseCaseData()->mainGrid();

    if ( globalCellIndex >= mainGrid->totalCellCount() )
    {
        ++m_rejectedCellCount;
        return false;
    }

    size_t             localCellIndex = 0;
    const RigGridBase* localGrid      = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( globalCellIndex, &localCellIndex );
    if ( !localGrid )
    {
        ++m_rejectedCellCount;
        return false;
    }

    const cvf::UByteArray* mask = maskForGrid( static_cast<int>( localGrid->gridIndex() ) );
    if ( !mask || localCellIndex >= mask->size() )
    {
        ++m_rejectedCellCount;
        return false;
    }

    const bool included = ( ( *mask )[localCellIndex] != 0 );
    if ( !included ) ++m_rejectedCellCount;
    return included;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicPerforationCellFilterEvaluator::rejectedCellCount() const
{
    return m_rejectedCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::UByteArray* RicPerforationCellFilterEvaluator::maskForGrid( int gridIndex ) const
{
    auto it = m_visibilityByGridIndex.find( gridIndex );
    if ( it != m_visibilityByGridIndex.end() ) return it->second.p();

    const RigEclipseCaseData* caseData = m_eclipseCase->eclipseCaseData();
    if ( !caseData || gridIndex < 0 || static_cast<size_t>( gridIndex ) >= caseData->gridCount() ) return nullptr;

    const RigGridBase* grid = caseData->grid( static_cast<size_t>( gridIndex ) );
    if ( !grid ) return nullptr;

    cvf::ref<cvf::UByteArray> mask = new cvf::UByteArray( grid->cellCount() );
    mask->setAll( 1 );
    m_filter->applyToCellVisibility( mask.p(), grid, 0 );
    auto inserted = m_visibilityByGridIndex.emplace( gridIndex, mask );
    return inserted.first->second.p();
}
