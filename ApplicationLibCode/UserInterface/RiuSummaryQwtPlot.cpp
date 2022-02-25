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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class EnsembleCurveInfoTextProvider : public IPlotCurveInfoTextProvider
{
public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    QString curveInfoText( QwtPlotCurve* curve ) override
    {
        RiuPlotCurve*    riuCurve = dynamic_cast<RiuPlotCurve*>( curve );
        RimSummaryCurve* sumCurve = nullptr;
        if ( riuCurve )
        {
            sumCurve = dynamic_cast<RimSummaryCurve*>( riuCurve->ownerRimCurve() );
        }

        return sumCurve && sumCurve->summaryCaseY() ? sumCurve->summaryCaseY()->displayCaseName() : "";
    }
};
static EnsembleCurveInfoTextProvider ensembleCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::RiuSummaryQwtPlot( RimSummaryPlot* plot, QWidget* parent /*= nullptr*/ )
    : RiuSummaryPlot( plot, parent )
{
    m_plotWidget = new RiuQwtPlotWidget( plot, parent );
    m_plotWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_plotWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( showContextMenu( QPoint ) ) );

    // LeftButton for the zooming
    m_zoomerLeft = new RiuQwtPlotZoomer( m_plotWidget->qwtPlot()->canvas() );
    m_zoomerLeft->setTrackerMode( QwtPicker::AlwaysOff );
    m_zoomerLeft->initMousePattern( 1 );

    // Attach a zoomer for the right axis
    m_zoomerRight = new RiuQwtPlotZoomer( m_plotWidget->qwtPlot()->canvas() );
    m_zoomerRight->setAxis( QwtPlot::xTop, QwtPlot::yRight );
    m_zoomerRight->setTrackerMode( QwtPicker::AlwaysOff );
    m_zoomerRight->initMousePattern( 1 );

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner( m_plotWidget->qwtPlot()->canvas() );
    panner->setMouseButton( Qt::MiddleButton );

    m_wheelZoomer = new RiuQwtPlotWheelZoomer( m_plotWidget->qwtPlot() );

    connect( m_wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerLeft, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerRight, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );

    setDefaults();
    new RiuQwtCurvePointTracker( m_plotWidget->qwtPlot(), true, &ensembleCurveInfoTextProvider );

    RiuQwtPlotTools::setCommonPlotBehaviour( m_plotWidget->qwtPlot() );
    RiuQwtPlotTools::setDefaultAxes( m_plotWidget->qwtPlot() );

    m_plotWidget->setInternalLegendVisible( true );

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
void RiuSummaryQwtPlot::useDateBasedTimeAxis( const QString&                          dateFormat,
                                              const QString&                          timeFormat,
                                              RiaQDateTimeTools::DateFormatComponents dateComponents,
                                              RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    RiuQwtPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(), dateFormat, timeFormat, dateComponents, timeComponents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useTimeBasedTimeAxis()
{
    m_plotWidget->qwtPlot()->setAxisScaleEngine( QwtPlot::xBottom, new QwtLinearScaleEngine() );
    m_plotWidget->qwtPlot()->setAxisScaleDraw( QwtPlot::xBottom, new QwtScaleDraw() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties )
{
    RiuPlotAnnotationTool::Orientation orientation = RiuPlotAnnotationTool::Orientation::HORIZONTAL;
    if ( axisProperties->plotAxisType().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
    {
        orientation = RiuPlotAnnotationTool::Orientation::VERTICAL;
    }
    m_annotationTool->detachAllAnnotations( orientation );

    for ( auto annotation : axisProperties->annotations() )
    {
        if ( annotation->annotationType() == RimPlotAxisAnnotation::AnnotationType::LINE )
        {
            m_annotationTool->attachAnnotationLine( m_plotWidget->qwtPlot(),
                                                    annotation->color(),
                                                    annotation->name(),
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
    return m_zoomerLeft->isActiveAndValid() || m_zoomerRight->isActiveAndValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::endZoomOperations()
{
    m_zoomerLeft->endZoomOperation();
    m_zoomerRight->endZoomOperation();
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
