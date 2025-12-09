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

#pragma once

#include "RiaDefines.h"

#include <set>

class QString;

//==================================================================================================
///
/// Tool functions for analyzing phase systems in reservoir simulations
///
//==================================================================================================
class RiaPhaseTools
{
public:
    // Individual phase checks
    static bool hasOilPhase( const std::set<RiaDefines::PhaseType>& phases );
    static bool hasGasPhase( const std::set<RiaDefines::PhaseType>& phases );
    static bool hasWaterPhase( const std::set<RiaDefines::PhaseType>& phases );

    // Phase system analysis
    static bool isThreePhaseSystem( const std::set<RiaDefines::PhaseType>& phases );
    static bool isTwoPhaseGasWater( const std::set<RiaDefines::PhaseType>& phases );
    static bool isTwoPhaseOilWater( const std::set<RiaDefines::PhaseType>& phases );
    static bool isTwoPhaseOilGas( const std::set<RiaDefines::PhaseType>& phases );
    static bool isSinglePhase( const std::set<RiaDefines::PhaseType>& phases );
    static bool isSinglePhaseWater( const std::set<RiaDefines::PhaseType>& phases );

    // Specific utility functions
    static QString getPreferredPcogName( const std::set<RiaDefines::PhaseType>& phases );
    static QString getSystemDescription( const std::set<RiaDefines::PhaseType>& phases );
};