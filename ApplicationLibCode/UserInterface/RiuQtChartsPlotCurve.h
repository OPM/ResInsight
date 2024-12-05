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

#include <QAreaSeries>
#include <QLineSeries>
#include <QScatterSeries>

class RiuQtChartsPlotWidget;
class RiuPlotCurveSymbol;

class QLineSeries;
class QScatterSeries;
class QAreaSeries;
//==================================================================================================
//
//==================================================================================================
class RiuQtChartsPlotCurve : public QObject, public RiuPlotCurve
{
    Q_OBJECT
public:
    explicit RiuQtChartsPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title = QString() );
    ~RiuQtChartsPlotCurve() override;

    void setTitle( const QString& title ) override;

    void setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                        RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                        int                                            curveThickness,
                        const QColor&                                  curveColor,
                        const QBrush&                                  fillBrush = QBrush( Qt::NoBrush ) ) override;

    void setBrush( const QBrush& brush ) override;
    void setColor( const QColor& color ) override;

    void    setLegendIconSize( const QSize& iconSize ) override;
    QSize   legendIconSize() const override;
    QPixmap legendIcon( const QSizeF& size ) const override;

    void attachToPlot( RiuPlotWidget* plotWidget ) override;
    void showInPlot() override;

    void setZ( int z ) override;

    void updateErrorBarsAppearance( bool showErrorBars, const QColor& curveColor ) override;
    void clearErrorBars() override;

    int                       numSamples() const override;
    std::pair<double, double> sample( int index ) const override;

    std::pair<double, double> xDataRange() const override;
    std::pair<double, double> yDataRange() const override;

    void detach() override;

    void setXAxis( RiuPlotAxis axis ) override;
    void setYAxis( RiuPlotAxis axis ) override;

    void setVisibleInLegend( bool isVisibleInLegend ) override;

    void setSymbol( RiuPlotCurveSymbol* symbol ) override;

    void updateLineAndAreaSeries();

    RiuPlotCurveSymbol* createSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbol ) const override;
    void                setCurveFittingTolerance( double tolerance ) override;

    void updateScatterSeries();

public slots:
    void axisRangeChanged();

private:
    void setSamplesInPlot( const std::vector<double>&, const std::vector<double>& ) override;

    bool            isQtChartObjectsPresent() const;
    QLineSeries*    lineSeries() const;
    QScatterSeries* scatterSeries() const;
    QAreaSeries*    areaSeries() const;

    cvf::BoundingBox computeBoundingBox() const;

private:
    QLineSeries*                        m_lineSeries;
    QScatterSeries*                     m_scatterSeries;
    QAreaSeries*                        m_areaSeries;
    std::shared_ptr<RiuPlotCurveSymbol> m_symbol;
    QPointer<RiuQtChartsPlotWidget>     m_plotWidget;
    RiuPlotAxis                         m_axisX;
    RiuPlotAxis                         m_axisY;
    bool                                m_isVisibleInLegend;
};
