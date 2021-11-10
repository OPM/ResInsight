/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuPlotCurve.h"
#include "RiuQwtPlotCurveDefines.h"

#include "qwt_plot_curve.h"
#include "qwt_plot_intervalcurve.h"
#include "qwt_symbol.h"

//==================================================================================================
//
//==================================================================================================
class RiuQwtPlotCurve : public RiuPlotCurve, public QwtPlotCurve
{
public:
    explicit RiuQwtPlotCurve( const QString& title = QString() );
    ~RiuQwtPlotCurve() override;

    void setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                        RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                        int                                            curveThickness,
                        const QColor&                                  curveColor,
                        const QBrush&                                  fillBrush = QBrush( Qt::NoBrush ) ) override;

    QwtGraphic legendIcon( int index, const QSizeF& size ) const override;

    void attachToPlot( RiuPlotWidget* plotWidget ) override;
    void showInPlot() override;

protected:
    void drawCurve( QPainter*          p,
                    int                style,
                    const QwtScaleMap& xMap,
                    const QwtScaleMap& yMap,
                    const QRectF&      canvasRect,
                    int                from,
                    int                to ) const override;

    void drawSymbols( QPainter*          p,
                      const QwtSymbol&   symbol,
                      const QwtScaleMap& xMap,
                      const QwtScaleMap& yMap,
                      const QRectF&      canvasRect,
                      int                from,
                      int                to ) const override;

    void setSamplesInPlot( const std::vector<double>&, const std::vector<double>&, int ) override;
};
