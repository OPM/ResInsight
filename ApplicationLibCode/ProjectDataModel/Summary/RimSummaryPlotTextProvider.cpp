/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimSummaryPlotTextProvider.h"

#include "RiaDateTimeDefines.h"
#include "RiaPreferencesSummary.h"
#include "RiaQDateTimeTools.h"

#include "RimSummaryPlot.h"

#include "cafAssert.h"

#include <vector>

namespace
{
static std::vector<RiaDefines::DateTimePeriod> supportedTabs()
{
    return { RiaDefines::DateTimePeriod::NONE,
             RiaDefines::DateTimePeriod::YEAR,
             RiaDefines::DateTimePeriod::HALFYEAR,
             RiaDefines::DateTimePeriod::QUARTER,
             RiaDefines::DateTimePeriod::MONTH,
             RiaDefines::DateTimePeriod::WEEK,
             RiaDefines::DateTimePeriod::DAY,
             RiaDefines::DateTimePeriod::HOUR,
             RiaDefines::DateTimePeriod::MINUTE };
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotTextProvider::RimSummaryPlotTextProvider( const RimSummaryPlot* summaryPlot )
    : m_summaryPlot( summaryPlot )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotTextProvider::isValid() const
{
    return m_summaryPlot.notNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotTextProvider::description() const
{
    CAF_ASSERT( m_summaryPlot.notNull() && "Need to check that provider is valid" );
    return m_summaryPlot->description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotTextProvider::tabTitle( int tabIndex ) const
{
    auto allTabs = supportedTabs();
    CAF_ASSERT( tabIndex < static_cast<int>( allTabs.size() ) );
    RiaDefines::DateTimePeriod timePeriod = allTabs[tabIndex];
    if ( timePeriod == RiaDefines::DateTimePeriod::NONE )
    {
        return "No Resampling";
    }
    return QString( "%1" ).arg( RiaQDateTimeTools::dateTimePeriodName( timePeriod ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotTextProvider::tabText( int tabIndex ) const
{
    CAF_ASSERT( m_summaryPlot.notNull() && "Need to check that provider is valid" );

    auto allTabs = supportedTabs();
    CAF_ASSERT( tabIndex < static_cast<int>( allTabs.size() ) );
    RiaDefines::DateTimePeriod timePeriod = allTabs[tabIndex];
    RiaPreferencesSummary*     prefs      = RiaPreferencesSummary::current();

    return m_summaryPlot->asciiDataForSummaryPlotExport( timePeriod, prefs->showSummaryTimeAsLongString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryPlotTextProvider::tabCount() const
{
    return static_cast<int>( supportedTabs().size() );
}
