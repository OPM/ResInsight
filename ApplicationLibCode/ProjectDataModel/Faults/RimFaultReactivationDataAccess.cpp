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
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccess::RimFaultReactivationDataAccess( RimEclipseCase* thecase, size_t timeStepIndex )
    : m_case( thecase )
    , m_caseData( nullptr )
    , m_timeStepIndex( timeStepIndex )
{
    if ( m_case ) m_caseData = m_case->eclipseCaseData();
    if ( m_caseData )
    {
        RigEclipseResultAddress resVarAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" );
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
double RimFaultReactivationDataAccess::porePressureAtPosition( cvf::Vec3d position, double defaultPorePressureGradient )
{
    double retValue = 0.0;

    size_t cellIdx = cvf::UNDEFINED_SIZE_T;
    if ( ( m_case != nullptr ) && ( m_caseData != nullptr ) )
    {
        auto grid = m_case->mainGrid();
        if ( grid != nullptr ) cellIdx = grid->findReservoirCellIndexFromPoint( position );

        if ( ( cellIdx != cvf::UNDEFINED_SIZE_T ) && m_resultAccessor.notNull() )
        {
            return m_resultAccessor->cellScalar( cellIdx );
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
