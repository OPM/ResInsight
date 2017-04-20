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

#include "RiuFlowCharacteristicsPlot.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RiuResultQwtPlot.h"

#include "qwt_plot.h"
#include "cvfBase.h"
#include "cvfColor3.h"

#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include "RiuLineSegmentQwtPlotCurve.h"
#include <QDateTime>
#include "RiuSummaryQwtPlot.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "qwt_plot_zoomer.h"
#include "RiaColorTables.h"
#include "qwt_plot_zoneitem.h"
#include "qwt_date.h"
#include "RiuQwtPlotZoomer.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuFlowCharacteristicsPlot::RiuFlowCharacteristicsPlot(RimFlowCharacteristicsPlot* plotDefinition, QWidget* parent) 
    :   m_plotDefinition(plotDefinition),
        QFrame(parent)
{
    Q_ASSERT(m_plotDefinition);
    
    QGridLayout* mainLayout = new QGridLayout();
    this->setLayout(mainLayout);
    this->layout()->setMargin(3);
    this->layout()->setSpacing(3);

    // White background
    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_lorenzPlot = new QwtPlot(this);
    m_flowCapVsStorageCapPlot = new QwtPlot(this);
    m_sweepEffPlot = new QwtPlot(this);

    mainLayout->addWidget(m_lorenzPlot, 0 ,0, 1, 2);
    mainLayout->addWidget(m_flowCapVsStorageCapPlot, 1, 0);
    mainLayout->addWidget(m_sweepEffPlot, 1, 1);

    RiuSummaryQwtPlot::setCommonPlotBehaviour(m_lorenzPlot);
    new RiuQwtPlotWheelZoomer(m_lorenzPlot);
    addWindowZoom(m_lorenzPlot);
    RiuSummaryQwtPlot::enableDateBasedBottomXAxis(m_lorenzPlot);
    m_lorenzPlot->setTitle("Lorenz Coefficient");

    RiuSummaryQwtPlot::setCommonPlotBehaviour(m_sweepEffPlot);
    new RiuQwtPlotWheelZoomer(m_sweepEffPlot);
    addWindowZoom(m_sweepEffPlot);
    m_sweepEffPlot->setTitle("Sweep Efficiency");

    RiuSummaryQwtPlot::setCommonPlotBehaviour(m_flowCapVsStorageCapPlot);
    new RiuQwtPlotWheelZoomer(m_flowCapVsStorageCapPlot);
    addWindowZoom(m_flowCapVsStorageCapPlot);
    m_flowCapVsStorageCapPlot->setTitle("Flow Capacity vs Storage Capacity");
}

void RiuFlowCharacteristicsPlot::addWindowZoom(QwtPlot* plot)
{
    auto zoomer = new RiuQwtPlotZoomer(plot->canvas());
    zoomer->setRubberBandPen(QColor(Qt::black));
    zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    zoomer->setTrackerPen(QColor(Qt::black));
    zoomer->initMousePattern(1);
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuFlowCharacteristicsPlot::~RiuFlowCharacteristicsPlot()
{
    if (m_plotDefinition)
    {
        m_plotDefinition->handleMdiWindowClosed();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::setLorenzCurve(const std::vector<QDateTime>& dateTimes, const std::vector<double>& timeHistoryValues)
{
    initializeColors(dateTimes);

    m_lorenzPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true); 
    auto curve = createEmptyCurve(m_lorenzPlot, "Lorenz Coefficient", QColor(0, 0, 0));
    curve->setSamplesFromDateAndValues(dateTimes, timeHistoryValues, false);
    //curve->setSymbol(QwtSymbol )

    //size_t tsIdx = 0;
    //for ( const QDateTime& dateTime: dateTimes )
    //{
    //    auto curve = createEmptyCurve(m_lorenzPlot, dateTime.toString(), m_dateToColorMap[dateTime]);
    //    std::vector<QDateTime> timeStep;
    //    timeStep.push_back(dateTime);
    //    std::vector<double> lorCoeff;
    //    lorCoeff.push_back(timeHistoryValues[tsIdx]);
    //    
    //    curve->setSamplesFromDateAndValues(timeStep, lorCoeff, false);
    //
    //    ++tsIdx;
    //}
    //double milliSecSinceEpoch = QwtDate::toDouble(filteredDateTimes[i]);

    for ( size_t tsIdx = 0; tsIdx < dateTimes.size(); ++tsIdx )
    {
        double currentTsValue = QwtDate::toDouble(dateTimes[tsIdx]);

        double minTsValue = currentTsValue;
        if ( tsIdx > 0 ) minTsValue = 0.5 * (currentTsValue + QwtDate::toDouble(dateTimes[tsIdx-1]));
        
        double maxTsValue = currentTsValue;
        if ( tsIdx < dateTimes.size()-1 ) maxTsValue = 0.5 * (currentTsValue + QwtDate::toDouble(dateTimes[tsIdx+1]));

        auto plotZone = new QwtPlotZoneItem();
        plotZone->setOrientation(Qt::Vertical);
        plotZone->setInterval(minTsValue, maxTsValue);
        plotZone->setBrush(QBrush(m_dateToColorMap[dateTimes[tsIdx]]));
        plotZone->attach(m_lorenzPlot);
    }

    m_lorenzPlot->replot();
}

RiuLineSegmentQwtPlotCurve* RiuFlowCharacteristicsPlot::createEmptyCurve(QwtPlot* plot, const QString& curveName, const QColor& curveColor )
{
    RiuLineSegmentQwtPlotCurve* plotCurve = new RiuLineSegmentQwtPlotCurve(curveName);
   
    plotCurve->setTitle(curveName);
    plotCurve->setPen(QPen(curveColor));
    plotCurve->attach(plot);
    return plotCurve;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addFlowCapStorageCapCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals)
{
    CVF_ASSERT(!m_dateToColorMap.empty());

    RiuLineSegmentQwtPlotCurve* plotCurve = createEmptyCurve(m_flowCapVsStorageCapPlot, dateTime.toString(), m_dateToColorMap[dateTime]);
    plotCurve->setSamplesFromTimeAndValues(xVals, yVals, false);
    m_flowCapVsStorageCapPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addSweepEfficiencyCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals)
{
    CVF_ASSERT(!m_dateToColorMap.empty());

    RiuLineSegmentQwtPlotCurve* plotCurve = createEmptyCurve(m_sweepEffPlot, dateTime.toString(),  m_dateToColorMap[dateTime]);
    plotCurve->setSamplesFromTimeAndValues(xVals, yVals, false);

    m_sweepEffPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::removeAllCurves()
{
    m_lorenzPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true); 
    m_lorenzPlot->replot();
    m_sweepEffPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true); 
    m_sweepEffPlot->replot();
    m_flowCapVsStorageCapPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true); 
    m_flowCapVsStorageCapPlot->replot();
    m_dateToColorMap.clear();
}

void zoomAllInPlot(QwtPlot * plot)
{
    plot->setAxisAutoScale(QwtPlot::xBottom, true);
    plot->setAxisAutoScale(QwtPlot::yLeft, true);
    plot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::zoomAll()
{
    zoomAllInPlot(m_lorenzPlot);
    zoomAllInPlot(m_sweepEffPlot);
    zoomAllInPlot(m_flowCapVsStorageCapPlot);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot* RiuFlowCharacteristicsPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuFlowCharacteristicsPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuFlowCharacteristicsPlot::minimumSizeHint() const
{
    return QSize(0, 100);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuFlowCharacteristicsPlot::sizeHint() const
{
    return QSize(0, 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::setDefaults()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::initializeColors(const std::vector<QDateTime>& dateTimes)
{
    CVF_ASSERT(m_dateToColorMap.empty());

    const caf::ColorTable& palette = RiaColorTables::timestepsPaletteColors();
    cvf::Color3ubArray colorArray = caf::ColorTable::interpolateColorArray(palette.color3ubArray(), dateTimes.size());

    for (size_t tsIdx = 0; tsIdx < dateTimes.size(); ++tsIdx)
    {
        m_dateToColorMap[dateTimes[tsIdx]] = QColor( colorArray[tsIdx].r(), colorArray[tsIdx].g(), colorArray[tsIdx].b());
    }
}

