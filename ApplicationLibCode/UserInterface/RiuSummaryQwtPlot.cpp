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

#include "RiuSummaryQwtPlot.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "Commands/CorrelationPlotCommands/RicNewCorrelationPlotFeature.h"

#include "RimEnsembleCurveInfoTextProvider.h"
#include "RimPlotAxisAnnotation.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"

#include "RiuPlotAnnotationTool.h"
#include "RiuPlotCurve.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWidgetDragger.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtScalePicker.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafIconProvider.h"
#include "cafSelectionManager.h"
#include "cafTitledOverlayFrame.h"

#include "qwt_date_scale_draw.h"
#include "qwt_date_scale_engine.h"
#include "qwt_interval.h"
#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"

#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QWheelEvent>

#include <limits>

static RimEnsembleCurveInfoTextProvider ensembleCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::RiuSummaryQwtPlot( RimSummaryPlot* plot, QWidget* parent /*= nullptr*/ )
    : RiuSummaryPlot( plot )
{
    m_plotWidget = new RiuQwtPlotWidget( plot, parent );
    m_plotWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_plotWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( showContextMenu( QPoint ) ) );

    // LeftButton for the zooming
    m_plotZoomer = new RiuQwtPlotZoomer( m_plotWidget->qwtPlot()->canvas() );
    m_plotZoomer->setTrackerMode( QwtPicker::AlwaysOff );
    m_plotZoomer->initMousePattern( 1 );

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner( m_plotWidget->qwtPlot()->canvas() );
    panner->setMouseButton( Qt::MiddleButton );

    m_wheelZoomer = new RiuQwtPlotWheelZoomer( m_plotWidget->qwtPlot() );

    connect( m_wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    connect( m_plotZoomer, SIGNAL( zoomed() ), SLOT( onZoomedSlot() ) );
    connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );

    setDefaults();
    new RiuQwtCurvePointTracker( m_plotWidget->qwtPlot(), true, &ensembleCurveInfoTextProvider );

    RiuQwtPlotTools::setCommonPlotBehaviour( m_plotWidget->qwtPlot() );
    RiuQwtPlotTools::setDefaultAxes( m_plotWidget->qwtPlot() );

    // PERFORMANCE NOTE
    // Do not set internal legends visible, as this will cause a performance hit.
    m_plotWidget->setInternalLegendVisible( false );

    m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>( new RiuPlotAnnotationTool() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::~RiuSummaryQwtPlot()
{
    delete m_plotWidget;
    m_plotWidget = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useDateBasedTimeAxis( const QString&                   dateFormat,
                                              const QString&                   timeFormat,
                                              RiaDefines::DateFormatComponents dateComponents,
                                              RiaDefines::TimeFormatComponents timeComponents )
{
    RiuQwtPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(), dateFormat, timeFormat, dateComponents, timeComponents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useTimeBasedTimeAxis()
{
    m_plotWidget->qwtPlot()->setAxisScaleEngine( QwtAxis::XBottom, new QwtLinearScaleEngine() );
    m_plotWidget->qwtPlot()->setAxisScaleDraw( QwtAxis::XBottom, new QwtScaleDraw() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties )
{
    RiaDefines::Orientation orientation = RiaDefines::Orientation::HORIZONTAL;
    if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
    {
        orientation = RiaDefines::Orientation::VERTICAL;
    }

    for ( auto annotation : axisProperties->annotations() )
    {
        if ( annotation->annotationType() == RimPlotAxisAnnotation::AnnotationType::LINE )
        {
            m_annotationTool->attachAnnotationLine( m_plotWidget->qwtPlot(),
                                                    annotation->color(),
                                                    annotation->name(),
                                                    annotation->penStyle(),
                                                    annotation->value(),
                                                    orientation );
        }
        else if ( annotation->annotationType() == RimPlotAxisAnnotation::AnnotationType::RANGE )
        {
            m_annotationTool->attachAnnotationRange( m_plotWidget->qwtPlot(),
                                                     annotation->color(),
                                                     annotation->name(),
                                                     annotation->rangeStart(),
                                                     annotation->rangeEnd(),
                                                     orientation );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setDefaults()
{
    QString dateFormat = RiaPreferences::current()->dateFormat();
    QString timeFormat = RiaPreferences::current()->timeFormat();

    useDateBasedTimeAxis( dateFormat, timeFormat );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuSummaryQwtPlot::isZoomerActive() const
{
    return m_plotZoomer->isActiveAndValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::endZoomOperations()
{
    m_plotZoomer->endZoomOperation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::onZoomedSlot()
{
    emit m_plotWidget->plotZoomed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RiuSummaryQwtPlot::plotWidget() const
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::clearAnnotationObjects()
{
    m_annotationTool->detachAllAnnotations();
}
