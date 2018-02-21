/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RiuMohrsCirclePlot.h"

#include "qwt_round_scale_draw.h"
#include "qwt_symbol.h"

#include "cvfAssert.h"

//==================================================================================================
///
/// \class RiuMohrsCirclePlot
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot::RiuMohrsCirclePlot(QWidget* parent)
:   QwtPlot(parent)
{
    setDefaults();
    setPrincipalsAndRedrawCircles(320, 200, 150);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMohrsCirclePlot::~RiuMohrsCirclePlot()
{
    deleteCircles();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setPrincipals(double p1, double p2, double p3)
{
    CVF_ASSERT(p1 > p2);
    CVF_ASSERT(p2 > p3);

    m_principal1 = p1;
    m_principal2 = p2;
    m_principal3 = p3;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setPrincipalsAndRedrawCircles(double p1, double p2, double p3)
{
    setPrincipals(p1, p2, p3);

    redrawCircles();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuMohrsCirclePlot::sizeHint() const
{
    return QSize(100, 100);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuMohrsCirclePlot::minimumSizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::redrawCircles()
{
    deleteCircles();
    createMohrCircles();

    for (MohrCircle circle : m_mohrCircles)
    {
        QwtSymbol* circleSymbol = new QwtSymbol(QwtSymbol::Ellipse);
        circleSymbol->setSize(2 * circle.radius, 2 * circle.radius);
        
        QwtPlotMarker* circlePlotItem = new QwtPlotMarker("Circle");
        circlePlotItem->setSymbol(circleSymbol);
        circlePlotItem->setXValue(circle.centerX);
        circlePlotItem->setYValue(0);
        
        m_mohrCirclesMarkers.push_back(circlePlotItem);
        circlePlotItem->attach(this);
    }

    this->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::deleteCircles()
{
    for (size_t i = 0; i < m_mohrCirclesMarkers.size(); i++)
    {
        m_mohrCirclesMarkers[i]->detach();
        delete m_mohrCirclesMarkers[i];
    }

    m_mohrCirclesMarkers.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::setDefaults()
{
    enableAxis(QwtPlot::xBottom, true);
    enableAxis(QwtPlot::yLeft, true);
    enableAxis(QwtPlot::xTop, false);
    enableAxis(QwtPlot::yRight, false);

    this->setAxisScale(QwtPlot::yLeft, -400, 400);
    this->setAxisScale(QwtPlot::xBottom, 0, 400);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMohrsCirclePlot::createMohrCircles()
{
    m_mohrCircles[0].component = 2;
    m_mohrCircles[0].radius = (m_principal1 - m_principal3) / 2;
    m_mohrCircles[0].centerX = (m_principal1 + m_principal3) / 2;

    m_mohrCircles[1].component = 1;
    m_mohrCircles[1].radius = (m_principal2 - m_principal3) / 2;
    m_mohrCircles[1].centerX = (m_principal2 + m_principal3) / 2;

    m_mohrCircles[2].component = 3;
    m_mohrCircles[2].radius = (m_principal1 - m_principal2) / 2;
    m_mohrCircles[2].centerX = (m_principal1 + m_principal2) / 2;
}
