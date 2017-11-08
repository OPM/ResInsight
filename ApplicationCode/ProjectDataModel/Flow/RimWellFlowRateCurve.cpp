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

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RigWellLogCurveData.h"

#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuLineSegmentQwtPlotCurve.h"

#include "qwt_plot.h"

#include "cvfMath.h"

#include <cmath>


//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellFlowRateCurve, "WellFlowRateCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellFlowRateCurve::RimWellFlowRateCurve()
{
    CAF_PDM_InitObject("Flow Rate Curve", "", "", "");
    m_groupId = 0;
    m_doFillCurve = true;
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
void RimWellFlowRateCurve::setGroupId(int groupId)
{
    m_groupId = groupId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellFlowRateCurve::groupId() const
{
    return m_groupId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::setDoFillCurve(bool doFill)
{
    m_doFillCurve = doFill;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellFlowRateCurve::doFillCurve() const
{
    return m_doFillCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellFlowRateCurve::createCurveAutoName()
{
    return m_curveAutoName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::onLoadDataAndUpdate(bool updateParentPlot)
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

    if ( isUsingConnectionNumberDepthType() )
    {
        m_qwtPlotCurve->setStyle(QwtPlotCurve::Steps);
    }

    if (m_doFillCurve)
    {
        QColor curveQColor = QColor (m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte());
        m_qwtPlotCurve->setBrush(QBrush( curveQColor));

        QLinearGradient gradient;
        gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
        gradient.setColorAt(0, curveQColor.darker(110));
        gradient.setColorAt(0.15, curveQColor);
        gradient.setColorAt(0.25, curveQColor);
        gradient.setColorAt(0.4, curveQColor.darker(110));
        gradient.setColorAt(0.6, curveQColor);
        gradient.setColorAt(0.8, curveQColor.darker(110));
        gradient.setColorAt(1, curveQColor);
        m_qwtPlotCurve->setBrush(gradient);

        QPen curvePen = m_qwtPlotCurve->pen();
        curvePen.setColor(curveQColor.darker());
        m_qwtPlotCurve->setPen(curvePen);
        m_qwtPlotCurve->setOrientation(Qt::Horizontal);
        m_qwtPlotCurve->setBaseline(0.0);
        m_qwtPlotCurve->setCurveAttribute(QwtPlotCurve::Inverted, true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_curveName);
    m_curveName.uiCapability()->setUiReadOnly(true);
    uiOrdering.add(&m_curveColor);
    m_curveColor.uiCapability()->setUiReadOnly(true);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellFlowRateCurve::updateStackedPlotData()
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfTypeAsserted(wellLogPlot);

    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfTypeAsserted(wellLogTrack);

    bool isFirstTrack =  (wellLogTrack == wellLogPlot->trackByIndex(0));

    RiaDefines::DepthUnitType displayUnit = RiaDefines::UNIT_NONE;

    std::vector<double> depthValues = m_curveData->measuredDepthPlotValues(displayUnit);
    std::vector< std::pair<size_t, size_t> > polyLineStartStopIndices = m_curveData->polylineStartStopIndices();
    std::vector<double> stackedValues(depthValues.size(), 0.0);
     
    std::map<int, std::vector<RimWellFlowRateCurve*>> stackedCurveGroups =  wellLogTrack->visibleStackedCurves();
    std::vector<RimWellFlowRateCurve*> stackedCurves;
    if (stackedCurveGroups.count(groupId()) > 0)
    {
        stackedCurves = stackedCurveGroups[groupId()];
    }

    double zPos = -0.1;
    for ( RimWellFlowRateCurve * stCurve: stackedCurves )
    {
        std::vector<double> values = stCurve->curveData()->xPlotValues();
        for ( size_t i = 0; i < values.size(); ++i )
        {
            stackedValues[i] += values[i]; 
        }

        if ( stCurve == this ) break;
        zPos -= 1.0;
    }

    // Insert the first depth position again, to add a <maxdepth, 0.0> value pair

    if ( depthValues.size() ) // Should we really do this for all curve variants ?
    {
        depthValues.insert(depthValues.begin(), depthValues[0]);
        stackedValues.insert(stackedValues.begin(), 0.0);
        polyLineStartStopIndices.front().second += 1;

        if (wellLogPlot->trackCount() > 1 && isFirstTrack)
        {
            // Add a dummy negative depth value to make the contribution
            // from other branches connected to well head visible

            double availableMinDepth;
            double availableMaxDepth;
            wellLogPlot->availableDepthRange(&availableMinDepth, &availableMaxDepth);

            double depthSpan = 0.1 * cvf::Math::abs(availableMinDepth - availableMaxDepth);

            // Round off value to floored decade
            double logDecValue = log10(depthSpan);
            logDecValue = cvf::Math::floor(logDecValue);
            depthSpan = pow(10.0, logDecValue);

            double dummyNegativeDepthValue = depthValues.back() - depthSpan;

            depthValues.push_back(dummyNegativeDepthValue);
            stackedValues.push_back(stackedValues.back());
            polyLineStartStopIndices.front().second += 1;
        }
    }

    // Add a dummy point for the zeroth connection to make the "end" distribution show better.

    if ( isFirstTrack  && isUsingConnectionNumberDepthType() )
    {
        stackedValues.push_back(stackedValues.back());
        depthValues.push_back(0.0);

        polyLineStartStopIndices.front().second += 1;
    }

    m_qwtPlotCurve->setSamples(stackedValues.data(), depthValues.data(), static_cast<int>(depthValues.size()));
    m_qwtPlotCurve->setLineSegmentStartStopIndices(polyLineStartStopIndices);

    m_qwtPlotCurve->setZ(doFillCurve() ? zPos : zPos + 10 /*?*/);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellFlowRateCurve::isUsingConnectionNumberDepthType() const
{
    RimWellLogPlot* wellLogPlot;
    firstAncestorOrThisOfType(wellLogPlot);
    if ( wellLogPlot && wellLogPlot->depthType() == RimWellLogPlot::CONNECTION_NUMBER )
    {
        return true;
    }

    return false;
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
void RimWellFlowRateCurve::setFlowValuesPrDepthValue(const QString& curveName, const std::vector<double>& depthValues, const std::vector<double>& flowRates)
{
    m_curveData = new RigWellLogCurveData;

    m_curveData->setValuesAndMD(flowRates, depthValues, RiaDefines::UNIT_NONE, false);

    m_curveAutoName = curveName;
}

