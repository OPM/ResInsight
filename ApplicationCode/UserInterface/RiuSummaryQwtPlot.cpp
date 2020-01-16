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

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimPlot.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuCvfOverlayItemWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuRimQwtPlotCurve.h"
#include "RiuWidgetDragger.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtScalePicker.h"

#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"
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
        RiuRimQwtPlotCurve* riuCurve = dynamic_cast<RiuRimQwtPlotCurve*>( curve );
        RimSummaryCurve*    sumCurve = nullptr;
        if ( riuCurve )
        {
            sumCurve = dynamic_cast<RimSummaryCurve*>( riuCurve->ownerRimCurve() );
        }

        return sumCurve && sumCurve->summaryCaseY() ? sumCurve->summaryCaseY()->caseName() : "";
    }
};
static EnsembleCurveInfoTextProvider ensembleCurveInfoTextProvider;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::RiuSummaryQwtPlot( RimSummaryPlot* plot, QWidget* parent /*= nullptr*/ )
    : RiuQwtPlotWidget( plot, parent )
{
    // LeftButton for the zooming
    m_zoomerLeft = new RiuQwtPlotZoomer( canvas() );
    m_zoomerLeft->setRubberBandPen( QColor( Qt::black ) );
    m_zoomerLeft->setTrackerMode( QwtPicker::AlwaysOff );
    m_zoomerLeft->setTrackerPen( QColor( Qt::black ) );
    m_zoomerLeft->initMousePattern( 1 );

    // Attach a zoomer for the right axis
    m_zoomerRight = new RiuQwtPlotZoomer( canvas() );
    m_zoomerRight->setAxis( xTop, yRight );
    m_zoomerRight->setTrackerMode( QwtPicker::AlwaysOff );
    m_zoomerRight->initMousePattern( 1 );

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner( canvas() );
    panner->setMouseButton( Qt::MidButton );

    m_wheelZoomer = new RiuQwtPlotWheelZoomer( this );

    connect( m_wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerLeft, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerRight, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );

    setDefaults();
    new RiuQwtCurvePointTracker( this, true, &ensembleCurveInfoTextProvider );

    RiuQwtPlotTools::setCommonPlotBehaviour( this );
    RiuQwtPlotTools::setDefaultAxes( this );

    setLegendVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQwtPlot::~RiuSummaryQwtPlot() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useDateBasedTimeAxis( const QString&                          dateFormat,
                                              const QString&                          timeFormat,
                                              RiaQDateTimeTools::DateFormatComponents dateComponents,
                                              RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    RiuQwtPlotTools::enableDateBasedBottomXAxis( this, dateFormat, timeFormat, dateComponents, timeComponents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::useTimeBasedTimeAxis()
{
    setAxisScaleEngine( QwtPlot::xBottom, new QwtLinearScaleEngine() );
    setAxisScaleDraw( QwtPlot::xBottom, new QwtScaleDraw() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setLegendFontSize( int fontSize )
{
    if ( legend() )
    {
        QFont font = legend()->font();
        font.setPixelSize( RiaFontCache::pointSizeToPixelSize( fontSize ) );
        legend()->setFont( font );
        // Set font size for all existing labels
        QList<QwtLegendLabel*> labels = legend()->findChildren<QwtLegendLabel*>();
        for ( QwtLegendLabel* label : labels )
        {
            label->setFont( font );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setLegendVisible( bool visible )
{
    if ( visible )
    {
        QwtLegend* legend = new QwtLegend( this );
        this->insertLegend( legend, BottomLegend );
    }
    else
    {
        this->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setAxisIsLogarithmic( QwtPlot::Axis axis, bool logarithmic )
{
    if ( m_wheelZoomer ) m_wheelZoomer->setAxisIsLogarithmic( axis, logarithmic );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    emit plotSelected( false );

    menuBuilder << "RicShowPlotDataFeature";
    menuBuilder << "RicSavePlotTemplateFeature";

    menuBuilder.appendToMenu( &menu );

    if ( menu.actions().size() > 0 )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setDefaults()
{
    QString dateFormat = RiaApplication::instance()->preferences()->dateFormat();
    QString timeFormat = RiaApplication::instance()->preferences()->timeFormat();

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
    emit plotZoomed();
}
