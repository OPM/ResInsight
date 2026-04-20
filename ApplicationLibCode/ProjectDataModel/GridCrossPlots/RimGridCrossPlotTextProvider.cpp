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

#include "RimGridCrossPlotTextProvider.h"

#include "RimGridCrossPlot.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotTextProvider::RimGridCrossPlotTextProvider( RimGridCrossPlot* crossPlot )
    : m_crossPlot( crossPlot )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotTextProvider::isValid() const
{
    return m_crossPlot.notNull();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotTextProvider::description() const
{
    CAF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
    return m_crossPlot->createAutoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotTextProvider::tabTitle( int tabIndex ) const
{
    CAF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
    return m_crossPlot->asciiTitleForPlotExport( tabIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotTextProvider::tabText( int tabIndex ) const
{
    CAF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
    return m_crossPlot->asciiDataForGridCrossPlotExport( tabIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlotTextProvider::tabCount() const
{
    CAF_ASSERT( m_crossPlot.notNull() && "Need to check that provider is valid" );
    return static_cast<int>( m_crossPlot->dataSets().size() );
}
