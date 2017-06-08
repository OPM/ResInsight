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

#include "RimUnitSystem.h"

#include "cafAppEnum.h"

#include "cvfAssert.h"

namespace caf
{
    template<>
    void RimUnitSystem::UnitSystemType::setUp()
    {
        addItem(RimUnitSystem::UNITS_METRIC,  "UNITS_METRIC",  "Metric");
        addItem(RimUnitSystem::UNITS_FIELD,   "UNITS_FIELD",   "Field");
        addItem(RimUnitSystem::UNITS_UNKNOWN, "UNITS_UNKNOWN", "Unknown");

        setDefault(RimUnitSystem::UNITS_METRIC);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimUnitSystem::darcysConstant(UnitSystem unitSystem)
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
