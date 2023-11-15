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

#include "RimFaultReactivationDataAccessorTemperature.h"
#include "RimFaultReactivationEnums.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorTemperature::RimFaultReactivationDataAccessorTemperature( RimEclipseCase* eclipseCase )
    : m_eclipseCase( eclipseCase )
    , m_caseData( nullptr )
    , m_mainGrid( nullptr )
{
    if ( m_eclipseCase )
    {
        m_caseData = m_eclipseCase->eclipseCaseData();
        m_mainGrid = m_eclipseCase->mainGrid();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorTemperature::~RimFaultReactivationDataAccessorTemperature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorTemperature::updateResultAccessor()
{
    if ( m_caseData )
    {
        RigEclipseResultAddress resVarAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "TEMP" );
        m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( resVarAddress );
        m_resultAccessor = RigResultAccessorFactory::createFromResultAddress( m_caseData,
                                                                              0,
                                                                              RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                              m_timeStep,
                                                                              resVarAddress );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorTemperature::isMatching( RimFaultReactivation::Property property ) const
{
    return property == RimFaultReactivation::Property::Temperature;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorTemperature::valueAtPosition( const cvf::Vec3d& position, double topDepth, double bottomDepth ) const
{
    if ( ( m_mainGrid != nullptr ) && m_resultAccessor.notNull() )
    {
        auto cellIdx = m_mainGrid->findReservoirCellIndexFromPoint( position );
        if ( cellIdx != cvf::UNDEFINED_SIZE_T )
        {
            return m_resultAccessor->cellScalar( cellIdx );
        }
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorTemperature::hasValidDataAtPosition( const cvf::Vec3d& position ) const
{
    auto cellIdx = m_mainGrid->findReservoirCellIndexFromPoint( position );
    return ( cellIdx != cvf::UNDEFINED_SIZE_T );
}
