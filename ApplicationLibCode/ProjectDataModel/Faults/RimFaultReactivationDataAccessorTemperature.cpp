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
                                                                                          double          seabedDepth )
    : m_eclipseCase( eclipseCase )
    , m_caseData( nullptr )
    , m_mainGrid( nullptr )
    , m_seabedTemperature( seabedTemperature )
    , m_seabedDepth( seabedDepth )
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

    auto [wellPaths, extractors] =
        RimFaultReactivationDataAccessorWellLogExtraction::createEclipseWellPathExtractors( *m_model, *m_caseData, m_seabedDepth );
    m_wellPaths  = wellPaths;
    m_extractors = extractors;
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
    if ( ( m_mainGrid != nullptr ) && m_resultAccessor.notNull() )
    {
        CAF_ASSERT( m_extractors.find( gridPart ) != m_extractors.end() );
        auto extractor = m_extractors.find( gridPart )->second;

        CAF_ASSERT( m_wellPaths.find( gridPart ) != m_wellPaths.end() );
        auto wellPath = m_wellPaths.find( gridPart )->second;

        auto [values, intersections] =
            RimFaultReactivationDataAccessorWellLogExtraction::extractValuesAndIntersections( *m_resultAccessor.p(), *extractor.p(), *wellPath );

        auto [value, pos] =
            RimFaultReactivationDataAccessorWellLogExtraction::calculateTemperature( intersections, values, position, m_seabedTemperature );

        return value;
    }

    return std::numeric_limits<double>::infinity();
}
