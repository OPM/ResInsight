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

#include "RiuSummaryQtChartsPlot.h"

#include "RiaPreferences.h"

#include "RimEnsembleCurveInfoTextProvider.h"
#include "RimSummaryPlot.h"

#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"
#include "RiuQtChartsPlotTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQtChartsPlot::RiuSummaryQtChartsPlot( RimSummaryPlot* plot, QWidget* parent )
    : RiuSummaryPlot( plot, parent )
{
    m_plotWidget = new RiuQtChartsPlotWidget( plot, parent, new RimEnsembleCurveInfoTextProvider );
    m_plotWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_plotWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( showContextMenu( QPoint ) ) );

    setDefaults();

    RiuQtChartsPlotTools::setCommonPlotBehaviour( m_plotWidget );
    RiuQtChartsPlotTools::setDefaultAxes( m_plotWidget );

    m_plotWidget->insertLegend( RiuPlotWidget::Legend::BOTTOM );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQtChartsPlot::~RiuSummaryQtChartsPlot()
{
    delete m_plotWidget;
    m_plotWidget = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::useDateBasedTimeAxis( const QString&                   dateFormat,
                                                   const QString&                   timeFormat,
                                                   RiaDefines::DateFormatComponents dateComponents,
                                                   RiaDefines::TimeFormatComponents timeComponents )
{
    m_plotWidget->setAxisScaleType( RiuPlotAxis::defaultBottom(), RiuPlotWidget::AxisScaleType::DATE );
    m_plotWidget->setFormatStrings( dateFormat, timeFormat, dateComponents, timeComponents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::useTimeBasedTimeAxis()
{
    m_plotWidget->setAxisScaleType( RiuPlotAxis::defaultBottom(), RiuPlotWidget::AxisScaleType::DATE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::clearAnnotationObjects()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::setDefaults()
{
    QString dateFormat = RiaPreferences::current()->dateFormat();
    QString timeFormat = RiaPreferences::current()->timeFormat();

    useDateBasedTimeAxis( dateFormat, timeFormat );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RiuSummaryQtChartsPlot::plotWidget() const
{
    return m_plotWidget;
}
