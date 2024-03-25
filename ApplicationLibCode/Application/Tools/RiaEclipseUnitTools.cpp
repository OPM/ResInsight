/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiaEclipseUnitTools.h"

#include "cafAppEnum.h"

#include "cvfAssert.h"
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::darcysConstant( RiaDefines::EclipseUnitSystem unitSystem )
{
    // See "Cartesian transmissibility calculations" in the "Eclipse Technical Description"
    //     CDARCY Darcys constant
    //         = 0.00852702 (E300); 0.008527 (ECLIPSE 100) (METRIC)
    //         = 0.00112712 (E300); 0.001127 (ECLIPSE 100) (FIELD)
    //         = 3.6 (LAB)
    //         = 0.00864 (PVT - M)
    switch ( unitSystem )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            return 0.001127;
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            return 0.008527;
        default:
            CVF_ASSERT( false );
            return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// Convert Gas to oil equivalents
/// If field unit, the Gas is in Mft^3(=1000ft^3) while the others are in [stb] (barrel)
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( RiaDefines::EclipseUnitSystem caseUnitSystem, double eclGasFlowRate )
{
    /// Unused Gas to Barrel conversion :
    /// we convert gas to stb as well. Based on
    /// 1 [stb] = 0.15898729492800007 [m^3]
    /// 1 [ft]  = 0.3048 [m]
    ///
    /// NB Mft^3 = 1000 ft^3 - can wrongly be interpreted as M for Mega in metric units

    if ( caseUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        const double fieldGasToOilEquivalent = 1000.0 / 5614.63;

        return fieldGasToOilEquivalent * eclGasFlowRate;
    }

    if ( caseUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        double metricGasToOilEquivalent = 1.0 / 1000.0; // Sm^3 Gas to Sm^3 oe

        return metricGasToOilEquivalent * eclGasFlowRate;
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaEclipseUnitTools::unitStringPressure( RiaDefines::EclipseUnitSystem unitSystem )
{
    switch ( unitSystem )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            return "barsa";
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            return "psia";
        case RiaDefines::EclipseUnitSystem::UNITS_LAB:
            return "atma";
        case RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN:
            return "";
        default:
            return "";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::convertToMeter( double sourceValue, const QString& sourceValueUnitText, bool replaceUnmatched )
{
    QString timmed = sourceValueUnitText.trimmed();

    if ( timmed.compare( "m", Qt::CaseInsensitive ) == 0 || timmed.compare( "md-m", Qt::CaseInsensitive ) == 0 )
    {
        return sourceValue;
    }
    else if ( timmed.compare( "cm", Qt::CaseInsensitive ) == 0 )
    {
        return sourceValue / 100.0;
    }
    else if ( timmed.compare( "mm", Qt::CaseInsensitive ) == 0 )
    {
        return sourceValue / 1000.0;
    }
    else if ( timmed.compare( "in", Qt::CaseInsensitive ) == 0 || timmed.compare( "inches", Qt::CaseInsensitive ) == 0 )
    {
        return RiaEclipseUnitTools::inchToMeter( sourceValue );
    }
    else if ( timmed.compare( "ft", Qt::CaseInsensitive ) == 0 || timmed.compare( "feet", Qt::CaseInsensitive ) == 0 ||
              timmed.compare( "md-ft", Qt::CaseInsensitive ) == 0 )
    {
        return RiaEclipseUnitTools::feetToMeter( sourceValue );
    }

    if ( replaceUnmatched )
        return HUGE_VAL;
    else
        return sourceValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::convertToFeet( double sourceValue, const QString& sourceValueUnitText, bool replaceUnmatched )
{
    QString timmed = sourceValueUnitText.trimmed();

    if ( timmed.compare( "ft", Qt::CaseInsensitive ) == 0 || timmed.compare( "feet", Qt::CaseInsensitive ) == 0 ||
         timmed.compare( "md-ft", Qt::CaseInsensitive ) == 0 )
    {
        return sourceValue;
    }
    else if ( timmed.compare( "in", Qt::CaseInsensitive ) == 0 || timmed.compare( "inches", Qt::CaseInsensitive ) == 0 )
    {
        return RiaEclipseUnitTools::inchToFeet( sourceValue );
    }
    else if ( timmed.compare( "cm", Qt::CaseInsensitive ) == 0 )
    {
        double meter = sourceValue / 100.0;
        return RiaEclipseUnitTools::meterToFeet( meter );
    }
    else if ( timmed.compare( "mm", Qt::CaseInsensitive ) == 0 )
    {
        double meter = sourceValue / 1000.0;
        return RiaEclipseUnitTools::meterToFeet( meter );
    }
    else if ( timmed.compare( "m", Qt::CaseInsensitive ) == 0 || timmed.compare( "md-m", Qt::CaseInsensitive ) == 0 )
    {
        return RiaEclipseUnitTools::meterToFeet( sourceValue );
    }

    if ( replaceUnmatched )
        return HUGE_VAL;
    else
        return sourceValue;
}
