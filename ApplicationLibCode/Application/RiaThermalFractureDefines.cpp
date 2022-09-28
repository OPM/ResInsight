/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RiaThermalFractureDefines.h"
#include "RiaFractureDefines.h"

#include "cafAssert.h"

#include <map>

namespace RiaDefines
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString leakoffPressureDropResultName()
{
    return "LeakoffPressureDrop";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString filtratePressureDropResultName()
{
    return "FiltratePressureDrop";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString leakoffMobilityResultName()
{
    return "LeakoffMobility";
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString filterCakeMobilityResultName()
{
    return "FilterCakeMobility";
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString injectivityDeclineResultName()
{
    return "InjectivityDecline";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString viscosityResultName()
{
    return "Viscosity";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString getExpectedThermalFractureUnit( const QString& name, RiaDefines::EclipseUnitSystem unitSystem )
{
    CAF_ASSERT( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC ||
                unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD );

    // parameter name --> { metric unit, field unit }
    std::map<QString, std::pair<QString, QString>> mapping =
        { { "XCoord", { "m", "feet" } },
          { "YCoord", { "m", "feet" } },
          { "ZCoord", { "m", "feet" } },
          { "Width", { "cm", "inches" } },
          { "Pressure", { "BARa", "psia" } },
          { "Temperature", { "deg C", "deg F" } },
          { "Stress", { "BARa", "psia" } },
          { "Density", { "Kg/m3", "lb/ft3" } },
          { RiaDefines::viscosityResultName(), { "mPa.s", "centipoise" } },
          { RiaDefines::leakoffMobilityResultName(), { "m/day/bar", "ft/day/psi" } },
          { "Conductivity",
            { RiaDefines::unitStringConductivity( RiaDefines::EclipseUnitSystem::UNITS_METRIC ),
              RiaDefines::unitStringConductivity( RiaDefines::EclipseUnitSystem::UNITS_FIELD ) } },
          { "Velocity", { "m/sec", "ft/sec" } },
          { "ResPressure", { "BARa", "psia" } },
          { "ResTemperature", { "deg C", "deg F" } },
          { "FiltrateThickness", { "cm", "inches" } },
          { RiaDefines::filtratePressureDropResultName(), { "bar", "psi" } },
          { "EffectiveResStress", { "bar", "psi" } },
          { "EffectiveFracStress", { "bar", "psi" } },
          { RiaDefines::leakoffPressureDropResultName(), { "bar", "psi" } },
          { RiaDefines::injectivityDeclineResultName(), { "factor", "factor" } },
          { RiaDefines::filterCakeMobilityResultName(), { "m/day/bar", "ft/day/psi" } } };

    auto res = std::find_if( mapping.begin(), mapping.end(), [&]( const auto& val ) { return val.first == name; } );

    if ( res != mapping.end() )
    {
        if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
            return res->second.first;
        else
            return res->second.second;
    }

    return "";
}

}; // namespace RiaDefines
