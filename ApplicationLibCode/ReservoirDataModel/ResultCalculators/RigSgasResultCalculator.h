/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <cstddef>

#include "RigEclipseResultCalculator.h"

class RigCaseCellResultsData;
class RigEclipseResultAddress;

//==================================================================================================
/// Compute SGAS for two-phase gas/water models where SGAS is not present in the simulator output.
/// SGAS is then computed as SGAS = 1 - SWAT
//==================================================================================================
class RigSgasResultCalculator : public RigEclipseResultCalculator
{
public:
    RigSgasResultCalculator( RigCaseCellResultsData& resultsData );
    ~RigSgasResultCalculator() override;

    void checkAndCreatePlaceholderEntry( const RigEclipseResultAddress& resVarAddr ) override;
    bool isMatching( const RigEclipseResultAddress& resVarAddr ) const override;
    void calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) override;

private:
    bool isTwoPhaseGasWater() const;
};
