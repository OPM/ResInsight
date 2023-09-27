/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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
double RimFaultReactivationDataAccess::porePressureAtPosition( cvf::Vec3d position, double defaultPorePressureGradient )
{
    size_t cellIdx = cvf::UNDEFINED_SIZE_T;
    if ( ( m_mainGrid != nullptr ) && m_resultAccessor.notNull() )
    {
        cellIdx = m_mainGrid->findReservoirCellIndexFromPoint( position );

        // adjust cell index to be on correct side of fault
        if ( auto search = m_cellIndexAdjustment.find( cellIdx ); search != m_cellIndexAdjustment.end() )
        {
            cellIdx = search->second;
        }

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
    return gradient * 9.81 * depth * 1000;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFaultReactivationDataAccess::timeStepIndex() const
{
    return m_timeStepIndex;
}
