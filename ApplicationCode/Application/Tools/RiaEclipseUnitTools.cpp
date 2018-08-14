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

namespace caf
{
    template<>
    void RiaEclipseUnitTools::UnitSystemType::setUp()
    {
        addItem(RiaEclipseUnitTools::UNITS_METRIC,  "UNITS_METRIC",  "Metric");
        addItem(RiaEclipseUnitTools::UNITS_FIELD,   "UNITS_FIELD",   "Field");
        addItem(RiaEclipseUnitTools::UNITS_UNKNOWN, "UNITS_UNKNOWN", "Unknown");

        setDefault(RiaEclipseUnitTools::UNITS_METRIC);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::darcysConstant(UnitSystem unitSystem)
{
    // See "Cartesian transmissibility calculations" in the "Eclipse Technical Description"
    //     CDARCY Darcys constant
    //         = 0.00852702 (E300); 0.008527 (ECLIPSE 100) (METRIC)
    //         = 0.00112712 (E300); 0.001127 (ECLIPSE 100) (FIELD)
    //         = 3.6 (LAB)
    //         = 0.00864 (PVT - M)
    switch (unitSystem)
    {
    case UNITS_FIELD:
        return 0.001127;
    case UNITS_METRIC:
        return 0.008527;
    default:
        CVF_ASSERT(false);
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RiaEclipseUnitTools::depthUnit(UnitSystem unit)
{
    switch (unit)
    {
    case RiaEclipseUnitTools::UNITS_METRIC:
        return RiaDefines::UNIT_METER;
        break;
    case RiaEclipseUnitTools::UNITS_FIELD:
        return RiaDefines::UNIT_FEET;
        break;
    case RiaEclipseUnitTools::UNITS_LAB:
        return RiaDefines::UNIT_NONE;
        break;
    case RiaEclipseUnitTools::UNITS_UNKNOWN:
        return RiaDefines::UNIT_NONE;
        break;
    default:
        return RiaDefines::UNIT_NONE;
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/// Convert Gas to oil equivalents
/// If field unit, the Gas is in Mega ft^3 while the others are in [stb] (barrel) 
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents(UnitSystem caseUnitSystem, double eclGasFlowRate)
{
    /// Unused Gas to Barrel conversion :
    /// we convert gas to stb as well. Based on 
    /// 1 [stb] = 0.15898729492800007 [m^3]
    /// 1 [ft]  = 0.3048 [m]
    /// megaFt3ToStbFactor = 1.0 / (1.0e-6 * 0.15898729492800007 * ( 1.0 / 0.3048 )^3 )
    /// double megaFt3ToStbFactor = 178107.60668;

    double fieldGasToOilEquivalent  = 1.0e6/5800; // Mega ft^3 to BOE
    double metricGasToOilEquivalent = 1.0/1.0e3; // Sm^3 Gas to Sm^3 oe  
    
    double oilEquivalentGasRate = HUGE_VAL;

    if (caseUnitSystem == RiaEclipseUnitTools::UNITS_FIELD)  oilEquivalentGasRate = fieldGasToOilEquivalent * eclGasFlowRate; 
    if (caseUnitSystem == RiaEclipseUnitTools::UNITS_METRIC) oilEquivalentGasRate = metricGasToOilEquivalent * eclGasFlowRate; 

    return oilEquivalentGasRate;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiaEclipseUnitTools::unitStringPressure(UnitSystem unitSystem)
{
    switch (unitSystem)
    {
        case RiaEclipseUnitTools::UNITS_METRIC:     return "barsa";
        case RiaEclipseUnitTools::UNITS_FIELD:      return "psia";
        case RiaEclipseUnitTools::UNITS_LAB:        return "atma";
        case RiaEclipseUnitTools::UNITS_UNKNOWN:    return "";
        default:                                    return "";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::convertToMeter(double sourceValue, const QString& sourceValueUnitText)
{
    QString timmed = sourceValueUnitText.trimmed();

    if (timmed.compare("m", Qt::CaseInsensitive) == 0 || timmed.compare("md-m", Qt::CaseInsensitive) == 0)
    {
        return sourceValue;
    }
    else if (timmed.compare("cm", Qt::CaseInsensitive) == 0)
    {
        return sourceValue / 100.0;
    }
    else if (timmed.compare("in", Qt::CaseInsensitive) == 0)
    {
        return RiaEclipseUnitTools::inchToMeter(sourceValue);
    }
    else if (timmed.compare("ft", Qt::CaseInsensitive) == 0 || timmed.compare("md-ft", Qt::CaseInsensitive) == 0)
    {
        return RiaEclipseUnitTools::feetToMeter(sourceValue);
    }

    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaEclipseUnitTools::convertToFeet(double sourceValue, const QString& sourceValueUnitText)
{
    QString timmed = sourceValueUnitText.trimmed();

    if (timmed.compare("ft", Qt::CaseInsensitive) == 0 || timmed.compare("md-ft", Qt::CaseInsensitive) == 0)
    {
        return sourceValue;
    }
    else if (timmed.compare("in", Qt::CaseInsensitive) == 0)
    {
        return RiaEclipseUnitTools::inchToFeet(sourceValue);
    }
    else if (timmed.compare("cm", Qt::CaseInsensitive) == 0)
    {
        double meter = sourceValue / 100.0;
        return RiaEclipseUnitTools::meterToFeet(meter);
    }
    else if (timmed.compare("m", Qt::CaseInsensitive) == 0 || timmed.compare("md-m", Qt::CaseInsensitive) == 0)
    {
        return RiaEclipseUnitTools::meterToFeet(sourceValue);
    }

    return HUGE_VAL;
}
