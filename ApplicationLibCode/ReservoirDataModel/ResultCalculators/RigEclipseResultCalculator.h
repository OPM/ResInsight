/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

class RigCaseCellResultsData;
class RigEclipseResultAddress;

//==================================================================================================
///
//==================================================================================================
class RigEclipseResultCalculator
{
public:
    RigEclipseResultCalculator( RigCaseCellResultsData& resultsData );
    virtual ~RigEclipseResultCalculator();

    virtual void checkAndCreatePlaceholderEntry( const RigEclipseResultAddress& resVarAddr );
    virtual bool isMatching( const RigEclipseResultAddress& resVarAddr ) const                = 0;
    virtual void calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) = 0;

protected:
    RigCaseCellResultsData* m_resultsData;
};
