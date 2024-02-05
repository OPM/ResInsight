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
                                                                                            double          porePressureGradient,
                                                                                            double          seabedDepth )
    : m_eclipseCase( eclipseCase )
    , m_defaultPorePressureGradient( porePressureGradient )
    , m_seabedDepth( seabedDepth )
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

    auto [wellPaths, extractors] =
        RimFaultReactivationDataAccessorWellLogExtraction::createEclipseWellPathExtractors( *m_model, *m_caseData, m_seabedDepth );
    m_wellPaths  = wellPaths;
    m_extractors = extractors;
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

        CAF_ASSERT( m_wellPaths.find( gridPart ) != m_wellPaths.end() );
        auto wellPath = m_wellPaths.find( gridPart )->second;

        auto [values, intersections] =
            RimFaultReactivationDataAccessorWellLogExtraction::extractValuesAndIntersections( *m_resultAccessor.p(), *extractor.p(), *wellPath );

        auto [value, pos] = RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( model,
                                                                                                gridPart,
                                                                                                intersections,
                                                                                                values,
                                                                                                position,
                                                                                                m_defaultPorePressureGradient );
        if ( pos.isUndefined() )
        {
            auto cellIdx = m_mainGrid->findReservoirCellIndexFromPoint( position );
            if ( cellIdx != cvf::UNDEFINED_SIZE_T )
            {
                double valueFromEclipse = m_resultAccessor->cellScalar( cellIdx );
                if ( !std::isinf( valueFromEclipse ) ) return RiaEclipseUnitTools::barToPascal( valueFromEclipse );
            }
        }

        return RiaEclipseUnitTools::barToPascal( value );
    }

    return std::numeric_limits<double>::infinity();
}
