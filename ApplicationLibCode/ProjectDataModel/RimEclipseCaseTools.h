/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include <vector>

class RimEclipseCase;
class RimEclipseResultCase;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimEclipseCaseTools
{
public:
    // Single grid cases not part of a grid case group
    static std::vector<RimEclipseCase*> eclipseCases();

    // Result cases based on RimEclipseCaseTools::elipseCases()
    static std::vector<RimEclipseResultCase*> eclipseResultCases();

    // All native Eclipse cases including grid case group source cases, excluding statistics cases
    static std::vector<RimEclipseCase*> nativeEclipseGridCases();

    // All Eclipse cases including statistics cases
    static std::vector<RimEclipseCase*> allEclipseGridCases();
};
