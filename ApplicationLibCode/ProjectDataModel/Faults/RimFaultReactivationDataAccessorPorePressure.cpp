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

#include "RimFaultReactivationDataAccessorPorePressure.h"
#include "RimEclipseCase.h"
#include "RimFaciesProperties.h"
#include "RimFaultReactivationDataAccessorWellLogExtraction.h"
#include "RimFaultReactivationEnums.h"

#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaPorosityModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFaultReactivationModel.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorPorePressure::RimFaultReactivationDataAccessorPorePressure( RimEclipseCase* eclipseCase,
                                                                                            double          porePressureGradient )
    : m_eclipseCase( eclipseCase )
    , m_defaultPorePressureGradient( porePressureGradient )
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
RimFaultReactivationDataAccessorPorePressure::~RimFaultReactivationDataAccessorPorePressure()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorPorePressure::updateResultAccessor()
{
    if ( !m_caseData ) return;

    RigEclipseResultAddress resVarAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" );
    m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->ensureKnownResultLoaded( resVarAddress );

    m_resultAccessor =
        RigResultAccessorFactory::createFromResultAddress( m_caseData, 0, RiaDefines::PorosityModelType::MATRIX_MODEL, m_timeStep, resVarAddress );

    auto [wellPaths, extractors] = RimFaultReactivationDataAccessorWellLogExtraction::createEclipseWellPathExtractors( *m_model, *m_caseData );
    m_wellPaths                  = wellPaths;
    m_extractors                 = extractors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorPorePressure::isMatching( RimFaultReactivation::Property property ) const
{
    return property == RimFaultReactivation::Property::PorePressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorPorePressure::valueAtPosition( const cvf::Vec3d&                position,
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

        // Extract values along well path
        std::vector<double> values;
        extractor->curveData( m_resultAccessor.p(), &values );

        auto intersections = extractor->intersections();

        CAF_ASSERT( m_wellPaths.find( gridPart ) != m_wellPaths.end() );
        auto wellPath = m_wellPaths.find( gridPart )->second;

        // Insert top of overburden point
        intersections.insert( intersections.begin(), wellPath->wellPathPoints().front() );
        values.insert( values.begin(), std::numeric_limits<double>::infinity() );

        // Insert bottom of underburden point
        intersections.push_back( wellPath->wellPathPoints().back() );
        values.push_back( std::numeric_limits<double>::infinity() );

        auto [value, pos] =
            RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( intersections, values, position, m_defaultPorePressureGradient );
        return RiaEclipseUnitTools::barToPascal( value );
    }

    return std::numeric_limits<double>::infinity();
}
