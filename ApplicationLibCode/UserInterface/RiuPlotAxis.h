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
    explicit RiuPlotAxis();
    explicit RiuPlotAxis( RiaDefines::PlotAxis axis );
    explicit RiuPlotAxis( RiaDefines::PlotAxis axis, int index );
    virtual ~RiuPlotAxis();

    static RiuPlotAxis defaultLeft();
    static RiuPlotAxis defaultRight();
    static RiuPlotAxis defaultTop();
    static RiuPlotAxis defaultBottom();

    RiaDefines::PlotAxis axis() const;

    int index() const;

    bool operator<( const RiuPlotAxis& rhs ) const;
    bool operator==( const RiuPlotAxis& rhs );

private:
    RiaDefines::PlotAxis m_axis;
    int                  m_index;
};
