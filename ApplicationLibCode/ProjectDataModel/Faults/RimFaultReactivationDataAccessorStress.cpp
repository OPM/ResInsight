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

#include "RimFaultReactivationDataAccessorStress.h"

#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"

#include "RigFaultReactivationModel.h"
#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigGriddedPart3d.h"
#include "RigWellPath.h"

#include "RimFaultReactivationDataAccessorWellLogExtraction.h"
#include "RimFaultReactivationEnums.h"
#include "RimGeoMechCase.h"
#include "RimWellIADataAccess.h"

#include "cvfVector3.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStress::RimFaultReactivationDataAccessorStress( RimFaultReactivation::Property property,
                                                                                double                         gradient,
                                                                                double                         seabedDepth )
    : m_property( property )
    , m_gradient( gradient )
    , m_seabedDepth( seabedDepth )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStress::~RimFaultReactivationDataAccessorStress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStress::isMatching( RimFaultReactivation::Property property ) const
{
    return property == m_property;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStress::valueAtPosition( const cvf::Vec3d&                position,
                                                                const RigFaultReactivationModel& model,
                                                                RimFaultReactivation::GridPart   gridPart,
                                                                double                           topDepth,
                                                                double                           bottomDepth,
                                                                size_t                           elementIndex ) const
{
    if ( !isDataAvailable() ) return std::numeric_limits<double>::infinity();

    cvf::Vec3d topPosition( position.x(), position.y(), topDepth );
    cvf::Vec3d bottomPosition( position.x(), position.y(), bottomDepth );

    if ( isPositionValid( position, topPosition, bottomPosition ) )
    {
        if ( m_property == RimFaultReactivation::Property::StressTop )
        {
            auto [porBar, extractionPos] = calculatePorBar( topPosition, m_gradient );
            if ( std::isinf( porBar ) ) return porBar;
            double s33 = extractStressValue( StressType::S33, extractionPos );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::StressBottom )
        {
            auto [porBar, extractionPos] = calculatePorBar( bottomPosition, m_gradient );
            if ( std::isinf( porBar ) ) return porBar;
            double s33 = extractStressValue( StressType::S33, extractionPos );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::DepthTop )
        {
            return topDepth;
        }
        else if ( m_property == RimFaultReactivation::Property::DepthBottom )
        {
            return bottomDepth;
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentX )
        {
            auto [porBar, extractionPos] = calculatePorBar( position, m_gradient );
            if ( std::isinf( porBar ) ) return porBar;
            double s11 = extractStressValue( StressType::S11, extractionPos );
            double s33 = extractStressValue( StressType::S33, extractionPos );
            return ( s11 - porBar ) / ( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentY )
        {
            auto [porBar, extractionPos] = calculatePorBar( position, m_gradient );
            if ( std::isinf( porBar ) ) return porBar;
            double s22 = extractStressValue( StressType::S22, extractionPos );
            double s33 = extractStressValue( StressType::S33, extractionPos );
            return ( s22 - porBar ) / ( s33 - porBar );
        }
    }

    return std::numeric_limits<double>::infinity();
}
