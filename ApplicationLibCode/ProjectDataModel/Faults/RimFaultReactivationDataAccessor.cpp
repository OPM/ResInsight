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

#include "RimFaultReactivationDataAccessor.h"

#include "RigFaultReactivationModel.h"
#include "RigMainGrid.h"

#include "RimFaultReactivationEnums.h"
#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessor::RimFaultReactivationDataAccessor()
{
    m_timeStep = -1;
    m_model    = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessor::~RimFaultReactivationDataAccessor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessor::setModelAndTimeStep( const RigFaultReactivationModel& model, size_t timeStep )
{
    m_model    = &model;
    m_timeStep = timeStep;
    updateResultAccessor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, RimFaultReactivation::ElementSets> RimFaultReactivationDataAccessor::findElementSetForElementIndex(
    const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets,
    int                                                                           elementIndex )
{
    for ( auto [s, indexes] : elementSets )
    {
        if ( std::find( indexes.begin(), indexes.end(), elementIndex ) != indexes.end() )
        {
            return std::pair<bool, RimFaultReactivation::ElementSets>( true, s );
        }
    }

    return std::pair<bool, RimFaultReactivation::ElementSets>( false, RimFaultReactivation::ElementSets::OverBurden );
}
