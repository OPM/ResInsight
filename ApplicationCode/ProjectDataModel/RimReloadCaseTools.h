/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

class RimEclipseCase;
class RigEclipseCaseData;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimReloadCaseTools
{
public:
    // Reload all eclipse data, both grid and summary
    static void reloadAllEclipseData( RimEclipseCase* eclipseCase );

    // Reload grid data, but not summary
    static void reloadAllEclipseGridData( RimEclipseCase* eclipseCase );

private:
    static void reloadAllEclipseData( RimEclipseCase* eclipseCase, bool reloadSummaryData );
    static void clearAllGridData( RigEclipseCaseData* eclipseCaseData );
    static void updateAll3dViews( RimEclipseCase* eclipseCase );
    static void updateAllPlots();
};
