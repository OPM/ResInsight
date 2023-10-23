/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023  Equinor ASA
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

#include "RimFaultReactivationDataAccess.h"

#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFault.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccess::RimFaultReactivationDataAccess( RimEclipseCase* thecase, size_t timeStepIndex )
    : m_case( thecase )
    , m_caseData( nullptr )
    , m_mainGrid( nullptr )
    , m_timeStepIndex( timeStepIndex )
{
    if ( m_case )
    {
        m_caseData = m_case->eclipseCaseData();
        m_mainGrid = m_case->mainGrid();
    }
    if ( m_caseData )
    {
        RigEclipseResultAddress resVarAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" );

        m_case->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( resVarAddress );

        m_resultAccessor = RigResultAccessorFactory::createFromResultAddress( m_caseData,
                                                                              0,
                                                                              RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                              timeStepIndex,
                                                                              resVarAddress );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccess::~RimFaultReactivationDataAccess()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccess::useCellIndexAdjustment( std::map<size_t, size_t> adjustments )
{
    m_cellIndexAdjustment = adjustments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFaultReactivationDataAccess::findAdjustedCellIndex( const cvf::Vec3d&               position,
                                                              const RigMainGrid*              grid,
                                                              const std::map<size_t, size_t>& cellIndexAdjustmentMap )
{
    CAF_ASSERT( grid != nullptr );

    size_t cellIdx = grid->findReservoirCellIndexFromPoint( position );

    // adjust cell index if present in the map
    if ( auto search = cellIndexAdjustmentMap.find( cellIdx ); search != cellIndexAdjustmentMap.end() )
    {
        cellIdx = search->second;
    }

    return cellIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccess::porePressureAtPosition( const cvf::Vec3d& position, double defaultPorePressureGradient ) const
{
    if ( ( m_mainGrid != nullptr ) && m_resultAccessor.notNull() )
    {
        auto cellIdx = findAdjustedCellIndex( position, m_mainGrid, m_cellIndexAdjustment );

        if ( ( cellIdx != cvf::UNDEFINED_SIZE_T ) )
        {
            double value = m_resultAccessor->cellScalar( cellIdx );
            if ( !std::isinf( value ) )
            {
                return 100000.0 * m_resultAccessor->cellScalar( cellIdx ); // return in pascal, not bar
            }
        }
    }

    return calculatePorePressure( std::abs( position.z() ), defaultPorePressureGradient );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccess::calculatePorePressure( double depth, double gradient )
{
    return gradient * 9.81 * depth * 1000.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFaultReactivationDataAccess::timeStepIndex() const
{
    return m_timeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccess::elementHasValidData( std::vector<cvf::Vec3d> elementCorners ) const
{
    int nValid = 0;
    for ( auto& p : elementCorners )
    {
        auto cellIdx = findAdjustedCellIndex( p, m_mainGrid, m_cellIndexAdjustment );

        if ( ( cellIdx != cvf::UNDEFINED_SIZE_T ) )
        {
            double value = m_resultAccessor->cellScalar( cellIdx );
            if ( !std::isinf( value ) )
            {
                nValid++;
            }
        }
    }

    // if more than half of the nodes have valid data, we're ok
    return nValid > 4;
}
