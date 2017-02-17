/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimWellFlowRateCurve.h"

#include "RimWellAllocationPlot.h"

#include "RiuLineSegmentQwtPlotCurve.h"

#include "qwt_plot.h"
#include "RimWellLogPlot.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"
#include "RimWellLogTrack.h"



//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellFlowRateCurve, "RimWellFlowRateCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellFlowRateCurve::RimWellFlowRateCurve()
{
    CAF_PDM_InitObject("Flow Rate Curve", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellFlowRateCurve::~RimWellFlowRateCurve()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::wellName() const
{
    QString name;

    RimWellAllocationPlot* wap = wellAllocationPlot();

    if (wap)
    {
        name = wap->wellName();
    }
    else
    {
        name = "Undefined";
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::wellLogChannelName() const
{
    return "AccumulatedFlowRate";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::createCurveAutoName()
{
    return m_tracerName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::onLoadDataAndUpdate()
{
    RimWellLogCurve::updateCurvePresentation();

    if (isCurveVisible())
    {
        m_qwtPlotCurve->setTitle(createCurveAutoName());

        updateStackedPlotData();

        updateZoomInParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::updateCurveAppearance()
{
    RimWellLogCurve::updateCurveAppearance();
   
    m_qwtPlotCurve->setStyle(QwtPlotCurve::Steps);
    QColor curveQColor = QColor (m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte());
    m_qwtPlotCurve->setBrush(QBrush( curveQColor));

    QLinearGradient gradient; 
    gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    gradient.setColorAt(0,curveQColor.darker(110));
    gradient.setColorAt(0.15,curveQColor);
    gradient.setColorAt(0.25,curveQColor);
    gradient.setColorAt(0.4,curveQColor.darker(110));
    gradient.setColorAt(0.6,curveQColor);
    gradient.setColorAt(0.8,curveQColor.darker(110));
    gradient.setColorAt(1,curveQColor);
    m_qwtPlotCurve->setBrush(gradient);

    QPen curvePen = m_qwtPlotCurve->pen();
    curvePen.setColor(curveQColor.darker());
    m_qwtPlotCurve->setPen(curvePen);
    m_qwtPlotCurve->setOrientation(Qt::Horizontal);
    m_qwtPlotCurve->setBaseline(0.0);
    m_qwtPlotCurve->setCurveAttribute(QwtPlotCurve::Inverted, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::updateStackedPlotData()
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType(wellLogPlot);

    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfType(wellLogTrack);

    bool isFirstTrack =  (wellLogTrack == wellLogPlot->trackByIndex(0));

    RimDefines::DepthUnitType displayUnit = RimDefines::UNIT_NONE;

    std::vector<double> depthValues = m_curveData->measuredDepthPlotValues(displayUnit);
    if (depthValues.size()) depthValues.insert(depthValues.begin(), depthValues[0]); // Insert the first depth position again, to make room for a real 0 value
    std::vector<double> stackedValues(depthValues.size(), 0.0);
     
    std::vector<RimWellFlowRateCurve*> stackedCurves =  wellLogTrack->visibleStackedCurves();
    double zPos = -0.1;
    for ( RimWellFlowRateCurve * stCurve: stackedCurves )
    {
        std::vector<double> values = stCurve->curveData()->xPlotValues();
        for ( size_t i = 0; i < values.size(); ++i )
        {
            stackedValues[i+1] += values[i];
        }

        if ( stCurve == this ) break;
        zPos -= 1.0;
    }
    
    // Add a dummy point for the zeroth connection to make the "end" distribution show better.

    stackedValues.push_back(stackedValues.back());
    depthValues.push_back(0.0);
    std::vector< std::pair<size_t, size_t> > polyLineStartStopIndices = m_curveData->polylineStartStopIndices();

    if ( isFirstTrack ) polyLineStartStopIndices.front().second += 2;
    else                polyLineStartStopIndices.front().second += 1;


    m_qwtPlotCurve->setSamples(stackedValues.data(), depthValues.data(), static_cast<int>(depthValues.size()));
    m_qwtPlotCurve->setLineSegmentStartStopIndices(polyLineStartStopIndices);
    
    m_qwtPlotCurve->setZ(zPos);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot* RimWellFlowRateCurve::wellAllocationPlot() const
{
    RimWellAllocationPlot* wap = nullptr;
    this->firstAncestorOrThisOfType(wap);
    
    return wap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::setFlowValuesPrConnection(const QString& tracerName, const std::vector<double>& connectionNumbers, const std::vector<double>& flowRates)
{
    m_curveData = new RigWellLogCurveData;
    m_curveData->setValuesAndMD(flowRates, connectionNumbers, RimDefines::UNIT_NONE, false);

    m_tracerName = tracerName;
}

