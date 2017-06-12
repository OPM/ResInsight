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

#include "cafAppEnum.h"

class RimUnitSystem
{

public:
    enum UnitSystem
    {
        UNITS_METRIC,
        UNITS_FIELD,
        //UNITS_LAB, // Not yet
        UNITS_UNKNOWN,
    };

    typedef caf::AppEnum< RimUnitSystem::UnitSystem > UnitSystemType;

    static double feetPerMeter()                    { return 3.2808399; }
    static double meterPerFeet()                    { return 0.3048000; }

    static double meterToFeet(double meter)         { return meter*feetPerMeter(); }
    static double feetToMeter(double feet)          { return feet*meterPerFeet();}
    static double meterToInch(double meter)         { return meter*feetPerMeter()*12; }
    static double inchToMeter(double inch)          { return (inch / 12)*meterPerFeet(); }
    static double inchToFeet (double inch)          { return (inch / 12.0); } 

    static double darcysConstant(UnitSystem unitSystem);
};

