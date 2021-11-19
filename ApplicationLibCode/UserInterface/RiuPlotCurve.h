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

#include "RiaCurveDataTools.h"

#include "RiuPlotWidget.h"
#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtSymbol.h"

#include <QBrush>
#include <QColor>
#include <QDateTime>
#include <QString>

class RimPlotCurve;

//==================================================================================================
//
// If infinite data is present in the curve data, Qwt is not able to draw a nice curve.
// This class assumes that inf data is removed, and segments to be draw are indicated by start/stop indices into curve
// data.
//
// Single values in the curve are drawn using a CrossX symbol
//
//  Here you can see the curve segments visualized. Curve segments are drawn between vector indices.
//
//  0 - 1
//  5 - 7
//  9 -10
//
//                 *                    *
//                *                   *   *
//  Curve        *                   *     *       -----        X
//
//  Values     1.0|2.0|inf|inf|inf|1.0|2.0|1.0|inf|1.0|1.0|inf|1.0|inf
//  Vec index   0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13
//==================================================================================================
class RiuPlotCurve
{
public:
    explicit RiuPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title = QString() );
    explicit RiuPlotCurve();
    virtual ~RiuPlotCurve();

    virtual void setTitle( const QString& title ) = 0;

    virtual void setSamplesValues( const std::vector<double>& xValues, const std::vector<double>& yValues );

    void setSamplesFromXValuesAndYValues( const std::vector<double>& xValues,
                                          const std::vector<double>& yValues,
                                          bool                       keepOnlyPositiveValues );

    void setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                        const std::vector<double>&    yValues,
                                        bool                          keepOnlyPositiveValues );

    void setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                        const std::vector<double>& yValues,
                                        bool                       keepOnlyPositiveValues );

    virtual void setSamplesFromXYErrorValues(
        const std::vector<double>&   xValues,
        const std::vector<double>&   yValues,
        const std::vector<double>&   errorValues,
        bool                         keepOnlyPositiveValues,
        RiaCurveDataTools::ErrorAxis errorAxis = RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS );

    void setLineSegmentStartStopIndices( const std::vector<std::pair<size_t, size_t>>& lineSegmentStartStopIndices );

    void setSymbolSkipPixelDistance( float distance );
    void setPerPointLabels( const std::vector<QString>& labels );

    virtual void setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum          lineStyle,
                                RiuQwtPlotCurveDefines::CurveInterpolationEnum interpolationType,
                                int                                            curveThickness,
                                const QColor&                                  curveColor,
                                const QBrush&                                  fillBrush = QBrush( Qt::NoBrush ) ) = 0;

    virtual void setSymbolAppearance( RiuQwtSymbol::PointSymbolEnum, int size, const QColor& color ) = 0;

    virtual void setBrush( const QBrush& brush ) = 0;

    void setBlackAndWhiteLegendIcon( bool blackAndWhite );
    //    QwtGraphic legendIcon( int index, const QSizeF& size ) const override;

    virtual void attachToPlot( RiuPlotWidget* plotWidget ) = 0;
    virtual void showInPlot()                              = 0;
    virtual void detach()                                  = 0;

    static std::vector<double> fromQDateTime( const std::vector<QDateTime>& dateTimes );
    static std::vector<double> fromTime_t( const std::vector<time_t>& timeSteps );

    virtual void setZ( int z ) = 0;

    virtual void clearErrorBars() = 0;

    virtual int                       numSamples() const        = 0;
    virtual std::pair<double, double> sample( int index ) const = 0;

    RimPlotCurve*       ownerRimCurve();
    const RimPlotCurve* ownerRimCurve() const;

    virtual std::pair<double, double> xDataRange() const = 0;
    virtual std::pair<double, double> yDataRange() const = 0;

protected:
    virtual void
        setSamplesInPlot( const std::vector<double>& xValues, const std::vector<double>& yValues, int numSamples ) = 0;

private:
    void computeValidIntervalsAndSetCurveData( const std::vector<double>& xValues,
                                               const std::vector<double>& yValues,
                                               bool                       keepOnlyPositiveValues );

protected:
    float m_symbolSkipPixelDistance;
    bool  m_blackAndWhiteLegendIcon;

    std::vector<QString> m_perPointLabels;

    std::vector<std::pair<size_t, size_t>> m_polyLineStartStopIndices;

    caf::PdmPointer<RimPlotCurve> m_ownerRimCurve;
};
