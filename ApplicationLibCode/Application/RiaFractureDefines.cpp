/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaFractureDefines.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::conductivityResultName()
{
    return "CONDUCTIVITY";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::unitStringConductivity( RiaDefines::EclipseUnitSystem unitSystem )
{
    switch ( unitSystem )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            return "md-m";
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            return "md-ft";
        default:
            return "";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaDefines::nonDarcyFlowAlpha( RiaDefines::EclipseUnitSystem unitSystem )
{
    switch ( unitSystem )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            return 2.24460e-10;
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            return 6.83352e-8;
        case RiaDefines::EclipseUnitSystem::UNITS_LAB:
            return 5.41375E-11;
            // case RiaEclipseUnitTools::PVT_METRIC:  return 2.25533E-10;

        default:
            return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::faciesColorLegendName()
{
    return "Facies colors";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaDefines::rockTypeColorLegendName()
{
    return "Rock Types";
}
