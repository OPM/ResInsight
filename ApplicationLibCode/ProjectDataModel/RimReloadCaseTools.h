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
class RimSummaryCase;
class RimEclipseResultCase;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimReloadCaseTools
{
public:
    // Reload all eclipse data, both grid and summary
    static void reloadEclipseGridAndSummary( RimEclipseCase* eclipseCase );

    // Reload grid data, but not summary
    static void reloadEclipseGrid( RimEclipseCase* eclipseCase );

    static void updateAll3dViews( RimEclipseCase* eclipseCase );

    static RimEclipseCase* gridModelFromSummaryCase( const RimSummaryCase* summaryCase );
    static RimSummaryCase* findSummaryCaseFromEclipseResultCase( const RimEclipseResultCase* eclResCase );
    static bool            openOrImportGridModelFromSummaryCase( const RimSummaryCase* summaryCase );

private:
    static void reloadEclipseData( RimEclipseCase* eclipseCase, bool reloadSummaryData );
    static void clearAllGridData( RigEclipseCaseData* eclipseCaseData );
    static void updateAllPlots();

    static bool findAndActivateFirstView( const RimSummaryCase* summaryCase );
};
