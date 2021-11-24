/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "cvfBoundingBox.h"

#include <QLineSeries>

class RiuQtChartsPlotWidget;

//==================================================================================================
//
//==================================================================================================
class RiuQtChartsPlotCurve : public RiuPlotCurve
{
public:
    explicit RiuQtChartsPlotCurve( const QString& title = QString() );
    ~RiuQtChartsPlotCurve() override;

    void setTitle( const QString& title ) override;

    void setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                        RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                        int                                            curveThickness,
                        const QColor&                                  curveColor,
                        const QBrush&                                  fillBrush = QBrush( Qt::NoBrush ) ) override;

    void setSymbolAppearance( RiuQwtSymbol::PointSymbolEnum, int size, const QColor& color ) override;

    void setBrush( const QBrush& brush ) override;

    // QtChartsGraphic legendIcon( int index, const QSizeF& size ) const override;

    void attachToPlot( RiuPlotWidget* plotWidget ) override;
    void showInPlot() override;

    void setZ( int z ) override;

    void clearErrorBars() override;

    int                       numSamples() const override;
    std::pair<double, double> sample( int index ) const override;

    std::pair<double, double> xDataRange() const override;
    std::pair<double, double> yDataRange() const override;

    void detach() override;

    void setXAxis( RiaDefines::PlotAxis axis ) override;
    void setYAxis( RiaDefines::PlotAxis axis ) override;

protected:
    void setSamplesInPlot( const std::vector<double>&, const std::vector<double>&, int ) override;

    cvf::BoundingBox computeBoundingBox() const;

    QtCharts::QLineSeries* m_lineSeries;
    RiuQtChartsPlotWidget* m_plotWidget;
    RiaDefines::PlotAxis   m_axisX;
    RiaDefines::PlotAxis   m_axisY;
};
