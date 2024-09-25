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
#include "RigCaseToCaseCellMapperTools.h"
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
#include "RimFaultReactivationDataAccessorStressEclipse.h"
#include "RimFaultReactivationDataAccessorStressGeoMech.h"
#include "RimFaultReactivationDataAccessorTemperature.h"
#include "RimFaultReactivationDataAccessorVoidRatio.h"
#include "RimFaultReactivationEnums.h"
#include "RimFaultReactivationModel.h"
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccess::RimFaultReactivationDataAccess( const RimFaultReactivationModel&   model,
                                                                RimEclipseCase*                    eCase,
                                                                RimGeoMechCase*                    geoMechCase,
                                                                const std::vector<size_t>&         timeSteps,
                                                                RimFaultReactivation::StressSource stressSource )
    : m_timeSteps( timeSteps )
{
    double porePressureGradient = 1.0;
    double topTemperature       = model.seabedTemperature();
    double seabedDepth          = -model.seaBedDepth();
    m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorPorePressure>( eCase, porePressureGradient, seabedDepth ) );
    m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorVoidRatio>( eCase, 0.0001 ) );
    m_accessors.push_back(
        std::make_shared<RimFaultReactivationDataAccessorTemperature>( eCase, topTemperature, seabedDepth, timeSteps.front() ) );

    std::vector<RimFaultReactivation::Property> stressProperties = { RimFaultReactivation::Property::StressTop,
                                                                     RimFaultReactivation::Property::DepthTop,
                                                                     RimFaultReactivation::Property::StressBottom,
                                                                     RimFaultReactivation::Property::DepthBottom,
                                                                     RimFaultReactivation::Property::LateralStressComponentX,
                                                                     RimFaultReactivation::Property::LateralStressComponentY };

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

    if ( ( stressSource == RimFaultReactivation::StressSource::StressFromGeoMech ) && ( geoMechCase ) )
    {
        for ( auto property : stressProperties )
        {
            m_accessors.push_back(
                std::make_shared<RimFaultReactivationDataAccessorStressGeoMech>( geoMechCase, property, porePressureGradient, seabedDepth ) );
        }
    }
    else
    {
        auto extractDensities = []( const RimFaultReactivationModel& model )
        {
            std::map<RimFaultReactivation::ElementSets, double> densities;
            std::vector<RimFaultReactivation::ElementSets>      elementSets = { RimFaultReactivation::ElementSets::OverBurden,
                                                                                RimFaultReactivation::ElementSets::UnderBurden,
                                                                                RimFaultReactivation::ElementSets::Reservoir,
                                                                                RimFaultReactivation::ElementSets::IntraReservoir,
                                                                                RimFaultReactivation::ElementSets::FaultZone };
            for ( auto e : elementSets )
            {
                densities[e] = model.materialParameters( e )[2];
            }
            return densities;
        };

        std::map<RimFaultReactivation::ElementSets, double> densities = extractDensities( model );
        for ( auto property : stressProperties )
        {
            m_accessors.push_back( std::make_shared<RimFaultReactivationDataAccessorStressEclipse>( eCase,
                                                                                                    property,
                                                                                                    porePressureGradient,
                                                                                                    seabedDepth,
                                                                                                    model.waterDensity(),
                                                                                                    model.lateralStressCoefficient(),
                                                                                                    densities ) );
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
    std::set<RimFaultReactivation::Property> nodeProperties = { RimFaultReactivation::Property::PorePressure,
                                                                RimFaultReactivation::Property::VoidRatio,
                                                                RimFaultReactivation::Property::Temperature };

    auto computeAverageDepth = []( const std::vector<cvf::Vec3d>& positions, const std::vector<size_t>& indices )
    {
        double sum = 0.0;
        for ( size_t idx : indices )
            sum += positions[idx].z();

        return sum / indices.size();
    };

    std::shared_ptr<RimFaultReactivationDataAccessor> accessor = getAccessor( property );

    if ( accessor )
    {
        accessor->setModelAndTimeStep( model, timeStep );

        auto grid = model.grid( gridPart );

        const std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>>& borderSurfaceElements = grid->borderSurfaceElements();
        auto it = borderSurfaceElements.find( RimFaultReactivation::BorderSurface::Seabed );
        CAF_ASSERT( it != borderSurfaceElements.end() && "Sea bed border surface does not exist" );
        std::set<unsigned int> seabedElements( it->second.begin(), it->second.end() );

        if ( nodeProperties.contains( property ) )
        {
            int                 numNodes = static_cast<int>( grid->dataNodes().size() );
            std::vector<double> values( numNodes, std::numeric_limits<double>::infinity() );

            for ( int nodeIndex = 0; nodeIndex < numNodes; nodeIndex++ )
            {
                double value      = accessor->valueAtPosition( grid->dataNodes()[nodeIndex], model, gridPart );
                values[nodeIndex] = value;
            }
            return values;
        }
        else
        {
            int                 numElements = static_cast<int>( grid->elementIndices().size() );
            std::vector<double> values( numElements, std::numeric_limits<double>::infinity() );

            for ( int elementIndex = 0; elementIndex < numElements; elementIndex++ )
            {
                std::vector<cvf::Vec3d> corners = grid->elementDataCorners( elementIndex );

                // Move top of sea bed element down to end up inside top element
                bool   isTopElement   = seabedElements.contains( static_cast<unsigned int>( elementIndex ) );
                double topDepthAdjust = isTopElement ? 0.1 : 0.0;

                double topDepth    = computeAverageDepth( corners, { 0, 1, 2, 3 } ) - topDepthAdjust;
                double bottomDepth = computeAverageDepth( corners, { 4, 5, 6, 7 } );

                cvf::Vec3d position  = RigCaseToCaseCellMapperTools::calculateCellCenter( corners.data() );
                double     value     = accessor->valueAtPosition( position, model, gridPart, topDepth, bottomDepth, elementIndex );
                values[elementIndex] = value;
            }
            return values;
        }
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
                        RimFaultReactivation::Property::Density,
                        RimFaultReactivation::Property::StressTop,
                        RimFaultReactivation::Property::DepthTop,
                        RimFaultReactivation::Property::StressBottom,
                        RimFaultReactivation::Property::DepthBottom,
                        RimFaultReactivation::Property::LateralStressComponentX,
                        RimFaultReactivation::Property::LateralStressComponentY };

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
