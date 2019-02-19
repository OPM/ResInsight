/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RimGridCrossPlotCurve.h"

#include <QPointF>
#include <QVector>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <random>

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurve, "GridCrossPlotCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCurve::RimGridCrossPlotCurve()
{
    CAF_PDM_InitObject("GridCrossPlotCurve", ":/WellLogCurve16x16.png", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::updateZoomInParentPlot()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::updateLegendsInPlot() 
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurve::createCurveAutoName()
{
    return "Cross Plot";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    QVector<QPointF> samples;

    const int N = 10000000;
    const double minValueX = 0.0;
    const double maxValueX = 1.0;
    const double yValueStdDev = 5000.0;

    std::random_device                     rd;
    std::mt19937                           gen(rd());
    std::uniform_real_distribution<double> distributionX(minValueX, maxValueX);

    for (size_t i = 0; i < N; ++i)
    {
        double xValue = distributionX(gen);
        double yValueMean = 2000 + 4000 * std::pow(xValue, 3.0);
        std::normal_distribution<double> distributionY(yValueMean, yValueStdDev);
        double yValueRandom = distributionY(gen);
        QPointF point(xValue, yValueRandom);
        samples.push_back(point);
    }
    m_qwtPlotCurve->setSamples(samples);    
}
