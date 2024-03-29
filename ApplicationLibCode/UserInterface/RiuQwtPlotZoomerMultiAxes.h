/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "qwt_plot_picker.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuQwtPlotZoomerMultiAxes : public QwtPlotPicker
{
    Q_OBJECT
public:
    explicit RiuQwtPlotZoomerMultiAxes( QWidget*, bool doReplot = true );

Q_SIGNALS:
    void zoomed();

protected:
    bool end( bool ok = true ) override;

private:
    void zoomFromScreenCoords( const QRectF& screenCoords );
};
