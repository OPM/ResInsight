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

#include <set>
#include <vector>

#include <QString>

class RimHistogramDataSource;
class RimHistogramPlot;
class RimHistogramMultiPlot;
class RimHistogramMultiPlotCollection;
class RimEnsembleParameterHistogramDataSource;
class RimHistogramCurve;
class RimSummaryEnsemble;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RicHistogramPlotTools
{
public:
    enum class DataSourceType
    {
        ENSEMBLE_PARAMETER,
        GRID_STATISTICS,
        SUMMARY_VECTOR,
        ENSEMBLE_FRACTURE_STATISTICS
    };

    static std::vector<DataSourceType> allDataSourceTypes();

    static void createDefaultHistogramCurve( RimHistogramPlot* plot, DataSourceType dataSourceType );
    static void createHistogramCurve( RimHistogramPlot* plot, RimHistogramDataSource* dataSource );
    static void appendEnsembleParameterHistogramCurve( RimHistogramPlot* plot, RimEnsembleParameterHistogramDataSource* dataSource );
    static void addHistogramCurveToPlot( RimHistogramPlot* plot, RimHistogramCurve* curve, bool resolveRefs = false );

    static RimHistogramMultiPlot* addNewHistogramMultiplot();
    static RimHistogramMultiPlot* addNewHistogramMultiplot( RimHistogramMultiPlotCollection* collection );
    static RimHistogramPlot*      addNewHistogramPlot( RimHistogramMultiPlot* histogramMultiPlot );

    static void appendEnsembleToHistogram( RimHistogramPlot* plot, RimSummaryEnsemble* ensemble );

    static std::set<QString> existingEnsembleParameters( RimHistogramPlot* plot );

private:
    static std::vector<RimHistogramDataSource*> existingDataSources( RimHistogramPlot* plot );
};
