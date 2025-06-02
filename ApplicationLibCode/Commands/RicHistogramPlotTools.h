/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

class RimHistogramDataSource;
class RimHistogramPlot;
class RimHistogramMultiPlot;
class RimHistogramMultiPlotCollection;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RicHistogramPlotTools
{
public:
    static void createHistogramCurve( RimHistogramPlot* plot, RimHistogramDataSource* dataSource );

    static RimHistogramMultiPlot* addNewHistogramMultiplot();
    static RimHistogramMultiPlot* addNewHistogramMultiplot( RimHistogramMultiPlotCollection* collection );
    static RimHistogramPlot*      addNewHistogramPlot( RimHistogramMultiPlot* histogramMultiPlot );
};
