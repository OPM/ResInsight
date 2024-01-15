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

#include "RimFaultReactivationDataAccessorStressEclipse.h"

#include "RiaEclipseUnitTools.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaWellLogUnitTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFaultReactivationModel.h"
#include "RigGriddedPart3d.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimFaultReactivationDataAccessorStress.h"
#include "RimFaultReactivationDataAccessorWellLogExtraction.h"
#include "RimFaultReactivationEnums.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStressEclipse::RimFaultReactivationDataAccessorStressEclipse(
    RimEclipseCase*                                            eclipseCase,
    RimFaultReactivation::Property                             property,
    double                                                     gradient,
    double                                                     seabedDepth,
    double                                                     lateralStressComponentX,
    double                                                     lateralStressComponentY,
    const std::map<RimFaultReactivation::ElementSets, double>& densities )
    : RimFaultReactivationDataAccessorStress( property, gradient, seabedDepth )
    , m_eclipseCase( eclipseCase )
    , m_caseData( nullptr )
    , m_mainGrid( nullptr )
    , m_lateralStressComponentX( lateralStressComponentX )
    , m_lateralStressComponentY( lateralStressComponentY )
    , m_densities( densities )
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
RimFaultReactivationDataAccessorStressEclipse::~RimFaultReactivationDataAccessorStressEclipse()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorStressEclipse::updateResultAccessor()
{
    if ( !m_caseData ) return;

    RigEclipseResultAddress resVarAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" );
    m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( resVarAddress );

    m_resultAccessor =
        RigResultAccessorFactory::createFromResultAddress( m_caseData, 0, RiaDefines::PorosityModelType::MATRIX_MODEL, m_timeStep, resVarAddress );

    auto [wellPaths, extractors] =
        RimFaultReactivationDataAccessorWellLogExtraction::createEclipseWellPathExtractors( *m_model, *m_caseData, m_seabedDepth );
    m_wellPaths  = wellPaths;
    m_extractors = extractors;

    // TODO: get from model
    double waterDensity = 1030.0;

    for ( auto [gridPart, wellPath] : m_wellPaths )
    {
        auto                    extractor     = m_extractors[gridPart];
        std::vector<cvf::Vec3d> intersections = extractor->intersections();

        addOverburdenAndUnderburdenPoints( intersections, wellPath->wellPathPoints() );

        m_stressValues[gridPart] =
            integrateVerticalStress( *wellPath.p(), intersections, *m_model, gridPart, m_seabedDepth, waterDensity, m_densities );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorStressEclipse::addOverburdenAndUnderburdenPoints( std::vector<cvf::Vec3d>&       intersections,
                                                                                       const std::vector<cvf::Vec3d>& wellPathPoints )
{
    // Insert points at top of overburden and under underburden
    intersections.insert( intersections.begin(), wellPathPoints.front() );
    intersections.push_back( wellPathPoints.back() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStressEclipse::isDataAvailable() const
{
    return m_mainGrid != nullptr && m_resultAccessor.notNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStressEclipse::extractStressValue( StressType                     stressType,
                                                                          const cvf::Vec3d&              position,
                                                                          RimFaultReactivation::GridPart gridPart ) const
{
    CAF_ASSERT( m_extractors.find( gridPart ) != m_extractors.end() );
    auto extractor = m_extractors.find( gridPart )->second;

    CAF_ASSERT( m_stressValues.find( gridPart ) != m_stressValues.end() );
    auto stressValues = m_stressValues.find( gridPart )->second;

    CAF_ASSERT( m_wellPaths.find( gridPart ) != m_wellPaths.end() );
    auto wellPath = m_wellPaths.find( gridPart )->second;

    auto intersections = extractor->intersections();
    addOverburdenAndUnderburdenPoints( intersections, wellPath->wellPathPoints() );

    CAF_ASSERT( stressValues.size() == intersections.size() );

    auto [topIdx, bottomIdx] = RimFaultReactivationDataAccessorWellLogExtraction::findIntersectionsForTvd( intersections, position.z() );
    if ( topIdx != -1 && bottomIdx != -1 )
    {
        double topValue    = stressValues[topIdx];
        double bottomValue = stressValues[bottomIdx];

        if ( !std::isinf( topValue ) && !std::isinf( bottomValue ) )
        {
            // Interpolate value from the two closest points.
            std::vector<double> xs = { intersections[bottomIdx].z(), intersections[topIdx].z() };
            std::vector<double> ys = { stressValues[bottomIdx], stressValues[topIdx] };
            return RiaEclipseUnitTools::pascalToBar( RiaInterpolationTools::linear( xs, ys, position.z() ) );
        }
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStressEclipse::isPositionValid( const cvf::Vec3d&              position,
                                                                     const cvf::Vec3d&              topPosition,
                                                                     const cvf::Vec3d&              bottomPosition,
                                                                     RimFaultReactivation::GridPart gridPart ) const
{
    auto [porBar, extractionPosition] = calculatePorBar( position, m_gradient, gridPart );
    return !std::isinf( porBar ) && extractionPosition != cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorStressEclipse::calculatePorBar( const cvf::Vec3d&              position,
                                                                                              double                         gradient,
                                                                                              RimFaultReactivation::GridPart gridPart ) const
{
    if ( ( m_mainGrid != nullptr ) && m_resultAccessor.notNull() )
    {
        CAF_ASSERT( m_extractors.find( gridPart ) != m_extractors.end() );
        auto extractor = m_extractors.find( gridPart )->second;

        CAF_ASSERT( m_wellPaths.find( gridPart ) != m_wellPaths.end() );
        auto wellPath = m_wellPaths.find( gridPart )->second;

        auto [values, intersections] =
            RimFaultReactivationDataAccessorWellLogExtraction::extractValuesAndIntersections( *m_resultAccessor.p(), *extractor.p(), *wellPath );

        auto [value, pos] = RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( intersections, values, position, m_gradient );
        return { value, pos };
    }

    return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RimFaultReactivationDataAccessorStressEclipse::integrateVerticalStress( const RigWellPath&               wellPath,
                                                                            const std::vector<cvf::Vec3d>&   intersections,
                                                                            const RigFaultReactivationModel& model,
                                                                            RimFaultReactivation::GridPart   gridPart,
                                                                            double                           seabedDepth,
                                                                            double                           waterDensity,
                                                                            const std::map<RimFaultReactivation::ElementSets, double>& densities )
{
    double              gravity      = RiaWellLogUnitTools<double>::gravityAcceleration();
    double              seaWaterLoad = gravity * std::abs( seabedDepth ) * waterDensity;
    std::vector<double> values       = { seaWaterLoad };

    auto g           = model.grid( gridPart );
    auto elementSets = g->elementSets();

    auto getElementSet =
        []( auto part, const cvf::Vec3d& point, const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets )
        -> std::pair<bool, RimFaultReactivation::ElementSets>
    {
        for ( auto [elementSet, elements] : elementSets )
        {
            for ( unsigned int elementIndex : elements )
            {
                // Find nodes from element
                auto positions = part->elementCorners( elementIndex );

                std::array<cvf::Vec3d, 8> coordinates;
                for ( size_t i = 0; i < positions.size(); ++i )
                {
                    coordinates[i] = positions[i];
                }

                if ( RigHexIntersectionTools::isPointInCell( point, coordinates.data() ) )
                {
                    return { true, elementSet };
                }
            }
        }

        return { false, RimFaultReactivation::ElementSets::OverBurden };
    };

    for ( size_t i = 1; i < intersections.size(); i++ )
    {
        double previousValue = values[i - 1];
        double previousDepth = intersections[i - 1].z();
        double currentDepth  = intersections[i].z();

        double deltaDepth = previousDepth - currentDepth;

        auto [isOk, elementSet] = getElementSet( g, intersections[i], elementSets );
        if ( isOk )
        {
            // Unit: kg/m^3
            CAF_ASSERT( densities.find( elementSet ) != densities.end() );
            double density = densities.find( elementSet )->second;

            double value = previousValue + density * gravity * deltaDepth;
            values.push_back( value );
        }
        else
        {
            values.push_back( std::numeric_limits<double>::infinity() );
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStressEclipse::lateralStressComponentX( const cvf::Vec3d&              position,
                                                                               RimFaultReactivation::GridPart gridPart ) const
{
    return m_lateralStressComponentX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStressEclipse::lateralStressComponentY( const cvf::Vec3d&              position,
                                                                               RimFaultReactivation::GridPart gridPart ) const
{
    return m_lateralStressComponentY;
}
