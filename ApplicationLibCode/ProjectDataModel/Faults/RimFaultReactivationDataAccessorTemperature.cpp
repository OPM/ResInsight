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
#include "RigFaultReactivationModel.h"
#include "RimFaultReactivationEnums.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimFaultReactivationDataAccessorWellLogExtraction.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorTemperature::RimFaultReactivationDataAccessorTemperature( RimEclipseCase* eclipseCase,
                                                                                          double          seabedTemperature,
                                                                                          double          seabedDepth,
                                                                                          size_t          firstTimeStep )
    : m_eclipseCase( eclipseCase )
    , m_caseData( nullptr )
    , m_mainGrid( nullptr )
    , m_seabedTemperature( seabedTemperature )
    , m_seabedDepth( seabedDepth )
    , m_firstTimeStep( firstTimeStep )
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
    if ( !m_caseData ) return;

    RigEclipseResultAddress resVarAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "TEMP" );
    m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( resVarAddress );
    m_resultAccessor =
        RigResultAccessorFactory::createFromResultAddress( m_caseData, 0, RiaDefines::PorosityModelType::MATRIX_MODEL, m_timeStep, resVarAddress );

    // Only create the first time step accessor once: it is always the same.
    if ( m_resultAccessor0.isNull() )
    {
        m_resultAccessor0 = RigResultAccessorFactory::createFromResultAddress( m_caseData,
                                                                               0,
                                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                               m_firstTimeStep,
                                                                               resVarAddress );
        if ( m_resultAccessor0.notNull() )
        {
            auto [wellPaths, extractors] =
                RimFaultReactivationDataAccessorWellLogExtraction::createEclipseWellPathExtractors( *m_model, *m_caseData, m_seabedDepth );
            m_wellPaths  = wellPaths;
            m_extractors = extractors;

            m_gradient = computeGradient();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Find the top encounter with reservoir (of the two well paths), and create gradient from that point
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorTemperature::computeGradient() const
{
    double gradient = std::numeric_limits<double>::infinity();
    double minDepth = -std::numeric_limits<double>::max();
    for ( auto gridPart : m_model->allGridParts() )
    {
        auto extractor = m_extractors.find( gridPart )->second;
        auto wellPath  = m_wellPaths.find( gridPart )->second;

        auto [values, intersections] =
            RimFaultReactivationDataAccessorWellLogExtraction::extractValuesAndIntersections( *m_resultAccessor0.p(), *extractor.p(), *wellPath );

        int lastOverburdenIndex = RimFaultReactivationDataAccessorWellLogExtraction::findLastOverburdenIndex( values );
        if ( lastOverburdenIndex != -1 )
        {
            double depth = intersections[lastOverburdenIndex].z();
            double value = values[lastOverburdenIndex];

            if ( !std::isinf( value ) )
            {
                double currentGradient =
                    RimFaultReactivationDataAccessorWellLogExtraction::computeGradient( intersections[0].z(),
                                                                                        m_seabedTemperature,
                                                                                        intersections[lastOverburdenIndex].z(),
                                                                                        values[lastOverburdenIndex] );
                if ( !std::isinf( value ) && !std::isnan( currentGradient ) && depth > minDepth )
                {
                    gradient = currentGradient;
                    minDepth = depth;
                }
            }
        }
    }

    return gradient;
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
double RimFaultReactivationDataAccessorTemperature::valueAtPosition( const cvf::Vec3d&                position,
                                                                     const RigFaultReactivationModel& model,
                                                                     RimFaultReactivation::GridPart   gridPart,
                                                                     double                           topDepth,
                                                                     double                           bottomDepth,
                                                                     size_t                           elementIndex ) const
{
    if ( ( m_mainGrid != nullptr ) && m_resultAccessor.notNull() && m_resultAccessor0.notNull() )
    {
        auto cellIdx = m_mainGrid->findReservoirCellIndexFromPoint( position );
        if ( cellIdx != cvf::UNDEFINED_SIZE_T )
        {
            double tempFromEclipse = m_resultAccessor->cellScalar( cellIdx );
            if ( !std::isinf( tempFromEclipse ) ) return tempFromEclipse;
        }

        CAF_ASSERT( m_extractors.find( gridPart ) != m_extractors.end() );
        auto extractor = m_extractors.find( gridPart )->second;

        CAF_ASSERT( m_wellPaths.find( gridPart ) != m_wellPaths.end() );
        auto wellPath = m_wellPaths.find( gridPart )->second;

        // Use data from first time step when not in reservoir. This ensures that the only difference between the
        // timesteps in the fault-reactivation model is in the reservoir.
        auto [values, intersections] =
            RimFaultReactivationDataAccessorWellLogExtraction::extractValuesAndIntersections( *m_resultAccessor0.p(), *extractor.p(), *wellPath );

        auto [value, pos] =
            RimFaultReactivationDataAccessorWellLogExtraction::calculateTemperature( intersections, position, m_seabedTemperature, m_gradient );

        return value;
    }

    return std::numeric_limits<double>::infinity();
}
