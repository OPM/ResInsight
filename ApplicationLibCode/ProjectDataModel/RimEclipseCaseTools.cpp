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

#include "RimEclipseCaseTools.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimProject.h"

#include "GeoMech/RimGeoMechCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimEclipseCaseTools::eclipseCases()
{
    if ( RimProject::current() )
    {
        return RimProject::current()->eclipseCases();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> RimEclipseCaseTools::eclipseResultCases()
{
    std::vector<RimEclipseResultCase*> resultCases;

    auto eclipseCases = RimEclipseCaseTools::eclipseCases();
    for ( auto ec : eclipseCases )
    {
        auto resultCase = dynamic_cast<RimEclipseResultCase*>( ec );
        if ( resultCase != nullptr )
        {
            resultCases.push_back( resultCase );
        }
    }

    return resultCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimEclipseCaseTools::nativeEclipseGridCases()
{
    // Find all Eclipse cases, including all single grid cases and source cases in a grid case group. Statistics cases are excluded.

    RimProject* proj = RimProject::current();
    if ( proj )
    {
        std::vector<RimEclipseCase*> eclipseCases;
        for ( auto c : proj->allGridCases() )
        {
            if ( dynamic_cast<RimEclipseStatisticsCase*>( c ) ) continue;

            if ( auto ec = dynamic_cast<RimEclipseCase*>( c ) )
            {
                eclipseCases.push_back( ec );
            }
        }

        return eclipseCases;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimEclipseCaseTools::allEclipseGridCases()
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        std::vector<RimEclipseCase*> eclipseCases;
        for ( auto c : proj->allGridCases() )
        {
            if ( auto ec = dynamic_cast<RimEclipseCase*>( c ) )
            {
                eclipseCases.push_back( ec );
            }
        }

        return eclipseCases;
    }

    return {};
}
