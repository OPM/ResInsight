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



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuFlowCharacteristicsPlot::RiuFlowCharacteristicsPlot(RimFlowCharacteristicsPlot* plotDefinition, QWidget* parent) 
    :   m_plotDefinition(plotDefinition),
        QFrame(parent)
{
    Q_ASSERT(m_plotDefinition);
    
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    this->layout()->setMargin(0);
    this->layout()->setSpacing(2);

    // White background
    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    m_lorenzPlot = new RiuResultQwtPlot(this);
    m_flowCapVsStorageCapPlot = new RiuResultQwtPlot(this);
    m_sweepEffPlot = new RiuResultQwtPlot(this);
    mainLayout->addWidget(m_lorenzPlot);
    mainLayout->addWidget(m_flowCapVsStorageCapPlot);
    mainLayout->addWidget(m_sweepEffPlot);
    
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
    m_lorenzPlot->deleteAllCurves();
    m_lorenzPlot->addCurve("Lorenz Coefficient", cvf::Color3f::BLUE, dateTimes, timeHistoryValues);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addFlowCapStorageCapCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals)
{
    RiuLineSegmentQwtPlotCurve* plotCurve = new RiuLineSegmentQwtPlotCurve(dateTime.toString());
    plotCurve->setSamplesFromTimeAndValues(xVals, yVals, false);
    plotCurve->setTitle(dateTime.toString());

    plotCurve->setPen(QPen(QColor(180, 0, 20)));

    plotCurve->attach(m_flowCapVsStorageCapPlot);

    m_flowCapVsStorageCapPlot->setAxisScale( QwtPlot::xBottom, 0.0, 1.0);

    m_flowCapVsStorageCapPlot->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFlowCharacteristicsPlot::addSweepEfficiencyCurve(const QDateTime& dateTime, const std::vector<double>& xVals, const std::vector<double>& yVals)
{
    RiuLineSegmentQwtPlotCurve* plotCurve = new RiuLineSegmentQwtPlotCurve(dateTime.toString());
    plotCurve->setSamplesFromTimeAndValues(xVals, yVals, false);
    plotCurve->setTitle(dateTime.toString());

    plotCurve->setPen(QPen(QColor(180, 0, 20)));

    plotCurve->attach(m_sweepEffPlot);

    //m_sweepEffPlot->setAxisScale( QwtPlot::xBottom, 0.0, 1.0);

    m_sweepEffPlot->replot();
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

