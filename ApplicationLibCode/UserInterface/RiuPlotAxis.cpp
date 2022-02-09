/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RiuPlotAxis.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis::RiuPlotAxis()
    : m_axis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
    , m_index( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis::RiuPlotAxis( RiaDefines::PlotAxis axis )
    : m_axis( axis )
    , m_index( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis::RiuPlotAxis( RiaDefines::PlotAxis axis, int index )
    : m_axis( axis )
    , m_index( index )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis::~RiuPlotAxis()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuPlotAxis::defaultLeft()
{
    return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuPlotAxis::defaultRight()
{
    return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuPlotAxis::defaultTop()
{
    return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_TOP );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RiuPlotAxis::defaultBottom()
{
    return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RiuPlotAxis::axis() const
{
    return m_axis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuPlotAxis::index() const
{
    return m_index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotAxis::operator<( const RiuPlotAxis& rhs ) const
{
    if ( m_axis != rhs.m_axis )
        return m_axis < rhs.m_axis;
    else
        return m_index < rhs.m_index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotAxis::operator==( const RiuPlotAxis& rhs ) const
{
    return m_axis == rhs.m_axis && m_index == rhs.m_index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuPlotAxis::operator!=( const RiuPlotAxis& rhs ) const
{
    return !( *this == rhs );
}
