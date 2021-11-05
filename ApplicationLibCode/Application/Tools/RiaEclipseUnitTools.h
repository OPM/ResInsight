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

#pragma once

#include "RiaDefines.h"
#include "cafAppEnum.h"

class RiaEclipseUnitTools
{
public:
    static double feetPerMeter() { return 3.2808399; }
    static double meterPerFeet() { return 0.3048000; }

    static double meterToFeet( double meter ) { return meter * feetPerMeter(); }
    static double feetToMeter( double feet ) { return feet * meterPerFeet(); }
    static double meterToInch( double meter ) { return meter * feetPerMeter() * 12.0; }
    static double inchToMeter( double inch ) { return ( inch / 12.0 ) * meterPerFeet(); }
    static double inchToFeet( double inch ) { return ( inch / 12.0 ); }
    static double mmToMeter( double mm ) { return mm / 1000.0; }
    static double meterToMm( double meter ) { return 1000.0 * meter; }

    static double barToPascal( double bar ) { return bar * 100000.0; }
    static double pascalToBar( double pas ) { return pas / 100000.0; }

    static double barToPsi( double bar ) { return bar * 14.5038; }
    static double psiToBar( double psi ) { return psi / 14.5038; }
    static double barPerMeterToPsiPerFeet( double barPerMeter ) { return barPerMeter * 4.42075; }
    static double gigaPascalToPascal( double gigaPascal ) { return gigaPascal * 1.0e9; }

    static double darcysConstant( RiaDefines::EclipseUnitSystem unitSystem );

    static double convertSurfaceGasFlowRateToOilEquivalents( RiaDefines::EclipseUnitSystem, double eclGasFlowRate );

    static QString unitStringPressure( RiaDefines::EclipseUnitSystem unitSystem );

    static double convertToMeter( double sourceValue, const QString& unitText );
    static double convertToFeet( double sourceValue, const QString& unitText );
};
