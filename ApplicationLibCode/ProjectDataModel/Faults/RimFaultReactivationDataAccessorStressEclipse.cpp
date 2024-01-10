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
RimFaultReactivationDataAccessorStressEclipse::RimFaultReactivationDataAccessorStressEclipse( RimEclipseCase*                eclipseCase,
                                                                                              RimFaultReactivation::Property property,
                                                                                              double                         gradient,
                                                                                              double                         seabedDepth )
    : RimFaultReactivationDataAccessorStress( property, gradient, seabedDepth )
    , m_eclipseCase( eclipseCase )
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
    double waterDensity = 1.030;

    for ( auto [gridPart, wellPaths] : m_wellPaths )
    {
        auto                    extractor     = m_extractors[gridPart];
        std::vector<cvf::Vec3d> intersections = extractor->intersections();
        m_stressValues[gridPart]              = integrateVerticalStress( *wellPaths.p(), intersections, m_seabedDepth, waterDensity );
    }
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
double RimFaultReactivationDataAccessorStressEclipse::extractStressValue( StressType stressType, const cvf::Vec3d& position ) const
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStressEclipse::isPositionValid( const cvf::Vec3d& position,
                                                                     const cvf::Vec3d& topPosition,
                                                                     const cvf::Vec3d& bottomPosition ) const
{
    // TODO: get grid part from somewhere!!!!!!!!!!!!!!!!!!!
    auto [porBar, extractionPosition] = calculatePorBar( position, m_gradient, RimFaultReactivation::GridPart::FW );
    return !std::isinf( porBar ) && extractionPosition != cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorStressEclipse::calculatePorBar( const cvf::Vec3d& position,
                                                                                              double            gradient,
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
        return { RiaEclipseUnitTools::barToPascal( value ), pos };
    }

    return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFaultReactivationDataAccessorStressEclipse::integrateVerticalStress( const RigWellPath&             wellPath,
                                                                                            const std::vector<cvf::Vec3d>& intersections,
                                                                                            double                         seabedDepth,
                                                                                            double                         waterDensity )
{
    double              gravity      = RiaWellLogUnitTools<double>::gravityAcceleration();
    double              seaWaterLoad = gravity * seabedDepth * waterDensity;
    std::vector<double> values       = { seaWaterLoad };

    for ( size_t i = 1; i < intersections.size(); i++ )
    {
        double previousValue = values[i - 1];
        double previousDepth = intersections[i - 1].z();
        double currentDepth  = intersections[i].z();

        double deltaDepth = currentDepth - previousDepth;

        double density = 999.0;
        double value   = previousValue + density * gravity * deltaDepth;

        values.push_back( value );
    }

    return values;
}
