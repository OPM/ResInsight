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

#pragma once

#include "RiaPlotDefines.h"

class RiuPlotAxis
{
public:
    RiuPlotAxis()
        : m_axis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        , m_index( 0 ){};

    RiuPlotAxis( RiaDefines::PlotAxis axis )
        : m_axis( axis )
        , m_index( 0 ){};

    RiuPlotAxis( RiaDefines::PlotAxis axis, int index )
        : m_axis( axis )
        , m_index( index ){};

    virtual ~RiuPlotAxis(){};

    static RiuPlotAxis defaultLeft() { return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT ); }
    static RiuPlotAxis defaultRight() { return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT ); }
    static RiuPlotAxis defaultTop() { return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_TOP ); }
    static RiuPlotAxis defaultBottom() { return RiuPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM ); }

    RiaDefines::PlotAxis axis() const { return m_axis; }

    int index() { return m_index; }

    bool operator<( const RiuPlotAxis& rhs ) const
    {
        return ( ( m_axis < rhs.m_axis ) || ( ( m_axis == rhs.m_axis ) && m_index < rhs.m_index ) );
    }

private:
    RiaDefines::PlotAxis m_axis;
    int                  m_index;
};
