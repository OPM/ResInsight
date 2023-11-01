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

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFault.h"
#include "RigFaultReactivationModel.h"
#include "RigGriddedPart3d.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimFaultReactivationDataAccessor.h"
#include "RimFaultReactivationDataAccessorGeoMech.h"
#include "RimFaultReactivationDataAccessorPorePressure.h"
#include "RimFaultReactivationDataAccessorTemperature.h"
#include "RimFaultReactivationDataAccessorVoidRatio.h"
#include "RimFaultReactivationEnums.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccess::RimFaultReactivationDataAccess( RimEclipseCase*            thecase,
                                                                RimGeoMechCase*            geoMechCase,
                                                                const std::vector<size_t>& timeSteps )
    : m_timeSteps( timeSteps )
{
    // TODO: correct default pore pressure gradient?
    m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorPorePressure>( thecase, 1.0 ) );
    m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorVoidRatio>( thecase, 0.0001 ) );
    m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorTemperature>( thecase ) );
    if ( geoMechCase )
    {
        std::vector<RimFaultReactivation::Property> properties = { RimFaultReactivation::Property::YoungsModulus,
                                                                   RimFaultReactivation::Property::PoissonsRatio,
                                                                   RimFaultReactivation::Property::Density };

        for ( auto property : properties )
        {
            m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorGeoMech>( geoMechCase, property ) );
        }
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
std::vector<double> RimFaultReactivationDataAccess::propertyValues( RimFaultReactivation::GridPart gridPart,
                                                                    RimFaultReactivation::Property property,
                                                                    size_t                         outputTimeStep ) const
{
    const auto it = m_propertyValues.find( { gridPart, property } );
    if ( it == m_propertyValues.end() || outputTimeStep >= it->second.size() ) return {};
    return it->second[outputTimeStep];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccess::clearModelData()
{
    m_accessors.clear();
    m_propertyValues.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFaultReactivationDataAccess::extractModelData( const RigFaultReactivationModel& model,
                                                                      RimFaultReactivation::GridPart   gridPart,
                                                                      RimFaultReactivation::Property   property,
                                                                      size_t                           timeStep )
{
    std::shared_ptr<RimFaultReactivationDataAccessor> accessor = getAccessor( property );
    if ( accessor )
    {
        accessor->setTimeStep( timeStep );

        auto grid = model.grid( gridPart );

        std::vector<double> values;
        for ( auto& node : grid->globalNodes() )
        {
            double value = accessor->valueAtPosition( node );
            values.push_back( value );
        }
        return values;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccess::extractModelData( const RigFaultReactivationModel& model )
{
    auto properties = { RimFaultReactivation::Property::PorePressure,
                        RimFaultReactivation::Property::VoidRatio,
                        RimFaultReactivation::Property::Temperature,
                        RimFaultReactivation::Property::YoungsModulus,
                        RimFaultReactivation::Property::PoissonsRatio,
                        RimFaultReactivation::Property::Density };

    for ( auto property : properties )
    {
        for ( auto part : model.allGridParts() )
        {
            std::vector<std::vector<double>> dataForAllTimeSteps;
            for ( size_t timeStep : m_timeSteps )
            {
                dataForAllTimeSteps.push_back( extractModelData( model, part, property, timeStep ) );
            }
            m_propertyValues[{ part, property }] = dataForAllTimeSteps;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RimFaultReactivationDataAccessor> RimFaultReactivationDataAccess::getAccessor( RimFaultReactivation::Property property ) const
{
    for ( auto accessor : m_accessors )
        if ( accessor->isMatching( property ) ) return accessor;

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccess::elementHasValidData( std::vector<cvf::Vec3d> elementCorners ) const
{
    auto accessor = getAccessor( RimFaultReactivation::Property::PorePressure );
    if ( !accessor ) return false;

    accessor->setTimeStep( 0 );

    int nValid = 0;

    for ( auto& p : elementCorners )
    {
        if ( accessor->hasValidDataAtPosition( p ) ) nValid++;
    }

    // if more than half of the nodes have valid data, we're ok
    return nValid > 4;
}
