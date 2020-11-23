/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigEclipseResultAddress.h"

#include "RimHistogramData.h"

#include "RigStatisticsDataCache.h"

#include "cvfObject.h"

class RimGeoMechContourMapView;
class RimEclipseContourMapView;
class RimEclipseView;
class RimGeoMechView;

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramCalculator
{
public:
    enum class StatisticsTimeRangeType
    {
        ALL_TIMESTEPS,
        CURRENT_TIMESTEP
    };

    enum class StatisticsCellRangeType
    {
        ALL_CELLS,
        VISIBLE_CELLS
    };

    RimHistogramCalculator();

    RimHistogramData histogramData( RimEclipseContourMapView* contourMap );
    RimHistogramData histogramData( RimGeoMechContourMapView* contourMap );
    RimHistogramData
        histogramData( RimEclipseView* eclipseView, StatisticsCellRangeType cellRange, StatisticsTimeRangeType timeRange );

    RimHistogramData
        histogramData( RimGeoMechView* geoMechView, StatisticsCellRangeType cellRange, StatisticsTimeRangeType timeRange );

    void invalidateVisibleCellsCache();

private:
    void updateVisCellStatsIfNeeded( RimEclipseView* eclipseView );
    void updateVisCellStatsIfNeeded( RimGeoMechView* geoMechView );

    std::vector<RigEclipseResultAddress> sourcesForMultiPropertyResults( const QString& resultName );

    bool                             m_isVisCellStatUpToDate;
    cvf::ref<RigStatisticsDataCache> m_visibleCellStatistics;
};
