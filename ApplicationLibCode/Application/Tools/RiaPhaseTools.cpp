/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaPhaseTools.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// Check if oil phase is present
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::hasOilPhase( const std::set<RiaDefines::PhaseType>& phases )
{
    return phases.contains( RiaDefines::PhaseType::OIL_PHASE );
}

//--------------------------------------------------------------------------------------------------
/// Check if gas phase is present
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::hasGasPhase( const std::set<RiaDefines::PhaseType>& phases )
{
    return phases.contains( RiaDefines::PhaseType::GAS_PHASE );
}

//--------------------------------------------------------------------------------------------------
/// Check if water phase is present
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::hasWaterPhase( const std::set<RiaDefines::PhaseType>& phases )
{
    return phases.contains( RiaDefines::PhaseType::WATER_PHASE );
}

//--------------------------------------------------------------------------------------------------
/// Check if this is a three-phase system (oil, gas, and water)
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::isThreePhaseSystem( const std::set<RiaDefines::PhaseType>& phases )
{
    return hasOilPhase( phases ) && hasGasPhase( phases ) && hasWaterPhase( phases );
}

//--------------------------------------------------------------------------------------------------
/// Check if this is a two-phase gas-water system (gas and water, no oil)
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::isTwoPhaseGasWater( const std::set<RiaDefines::PhaseType>& phases )
{
    return hasGasPhase( phases ) && hasWaterPhase( phases ) && !hasOilPhase( phases );
}

//--------------------------------------------------------------------------------------------------
/// Check if this is a two-phase oil-water system (oil and water, no gas)
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::isTwoPhaseOilWater( const std::set<RiaDefines::PhaseType>& phases )
{
    return hasOilPhase( phases ) && hasWaterPhase( phases ) && !hasGasPhase( phases );
}

//--------------------------------------------------------------------------------------------------
/// Check if this is a two-phase oil-gas system (oil and gas, no water)
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::isTwoPhaseOilGas( const std::set<RiaDefines::PhaseType>& phases )
{
    return hasOilPhase( phases ) && hasGasPhase( phases ) && !hasWaterPhase( phases );
}

//--------------------------------------------------------------------------------------------------
/// Check if this is a single-phase system
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::isSinglePhase( const std::set<RiaDefines::PhaseType>& phases )
{
    return phases.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// Check if this is a single-phase water system
//--------------------------------------------------------------------------------------------------
bool RiaPhaseTools::isSinglePhaseWater( const std::set<RiaDefines::PhaseType>& phases )
{
    return phases.size() == 1 && hasWaterPhase( phases );
}

//--------------------------------------------------------------------------------------------------
/// Get the preferred name for PCOG curves based on phase system
/// Returns "PCGW" for gas-water systems, "PCOG" otherwise
//--------------------------------------------------------------------------------------------------
QString RiaPhaseTools::getPreferredPcogName( const std::set<RiaDefines::PhaseType>& phases )
{
    return isTwoPhaseGasWater( phases ) ? "PCGW" : "PCOG";
}

//--------------------------------------------------------------------------------------------------
/// Get a human-readable description of the phase system
//--------------------------------------------------------------------------------------------------
QString RiaPhaseTools::getSystemDescription( const std::set<RiaDefines::PhaseType>& phases )
{
    if ( isThreePhaseSystem( phases ) )
    {
        return "Three-phase (Oil/Gas/Water)";
    }
    else if ( isTwoPhaseGasWater( phases ) )
    {
        return "Two-phase (Gas/Water)";
    }
    else if ( isTwoPhaseOilWater( phases ) )
    {
        return "Two-phase (Oil/Water)";
    }
    else if ( isTwoPhaseOilGas( phases ) )
    {
        return "Two-phase (Oil/Gas)";
    }
    else if ( isSinglePhase( phases ) )
    {
        if ( hasOilPhase( phases ) )
            return "Single-phase (Oil)";
        else if ( hasGasPhase( phases ) )
            return "Single-phase (Gas)";
        else if ( hasWaterPhase( phases ) )
            return "Single-phase (Water)";
    }

    return "Unknown phase system";
}