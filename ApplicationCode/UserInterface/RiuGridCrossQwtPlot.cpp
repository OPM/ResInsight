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
#include "RiuGridCrossQwtPlot.h"

#include "RiaFontCache.h"

#include "RiuCvfOverlayItemWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuRimQwtPlotCurve.h"
#include "RiuWidgetDragger.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimPlot.h"
#include "RimRegularLegendConfig.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafFixedAtlasFont.h"
#include "cafSelectionManager.h"
#include "cafTitledOverlayFrame.h"

#include "RimPlotAxisAnnotation.h"
#include "RimPlotAxisProperties.h"
#include "RiuPlotAnnotationTool.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include "qwt_text.h"
#include "qwt_text_engine.h"

#include <QLabel>
#include <QMenu>
#include <QResizeEvent>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGridCrossQwtPlot::RiuGridCrossQwtPlot( RimGridCrossPlot* plot, QWidget* parent /*= nullptr*/ )
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

    auto wheelZoomer = new RiuQwtPlotWheelZoomer( this );

    connect( wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerLeft, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerRight, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );
    connect( this,
             SIGNAL( plotItemSelected( QwtPlotItem*, bool, int ) ),
             SLOT( onPlotItemSelected( QwtPlotItem*, bool, int ) ) );

    m_annotationTool      = std::unique_ptr<RiuPlotAnnotationTool>( new RiuPlotAnnotationTool() );
    m_selectedPointMarker = new QwtPlotMarker;

    // QwtPlotMarker takes ownership of the symbol, it is deleted in destructor of QwtPlotMarker
    QwtSymbol* mySymbol =
        new QwtSymbol( QwtSymbol::Ellipse, QBrush( QColor( 255, 255, 255, 50 ) ), QPen( Qt::black, 2.0 ), QSize( 10, 10 ) );
    m_selectedPointMarker->setSymbol( mySymbol );
    m_selectedPointMarker->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_selectedPointMarker->setSpacing( 3 );
    m_selectedPointMarker->setZ( 1000.0 ); // Make sure it ends up in front of highlighted curves.

    RiuQwtPlotTools::setCommonPlotBehaviour( this );
    RiuQwtPlotTools::setDefaultAxes( this );

    this->installEventFilter( this );
    this->canvas()->installEventFilter( this );

    setInternalQwtLegendVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGridCrossQwtPlot::~RiuGridCrossQwtPlot()
{
    if ( m_selectedPointMarker->plot() )
    {
        m_selectedPointMarker->detach();
    }
    delete m_selectedPointMarker;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::updateAnnotationObjects( RimPlotAxisProperties* axisProperties )
{
    m_annotationTool->detachAllAnnotations();

    for ( auto annotation : axisProperties->annotations() )
    {
        m_annotationTool->attachAnnotationLine( this, annotation->color(), annotation->name(), annotation->value() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::setLegendFontSize( int fontSize )
{
    if ( legend() )
    {
        QFont font = legend()->font();
        font.setPixelSize( caf::FontTools::pointSizeToPixelSize( fontSize ) );
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
/// The internal qwt legend is not used in multi plot windows
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::setInternalQwtLegendVisible( bool visible )
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
void RiuGridCrossQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    emit plotSelected( false );

    menuBuilder << "RicSwapGridCrossPlotDataSetAxesFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicShowPlotDataFeature";

    menuBuilder.appendToMenu( &menu );

    if ( menu.actions().size() > 0 )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int pointNumber )
{
    if ( pointNumber == -1 )
        m_selectedPointMarker->detach();
    else
    {
        QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( plotItem );
        if ( curve )
        {
            QPointF sample = curve->sample( pointNumber );
            m_selectedPointMarker->setValue( sample );
            m_selectedPointMarker->setAxes( QwtPlot::xBottom, QwtPlot::yLeft );
            m_selectedPointMarker->attach( this );
            QString curveName, xAxisName, yAxisName;
            if ( curveText( curve, &curveName, &xAxisName, &yAxisName ) )
            {
                QString labelFormat( "<div style=\"margin: 4px;\"><b>%1:</b><br/>%2 = %3, %4 = %5</div>" );
                QString labelString =
                    labelFormat.arg( curveName ).arg( xAxisName ).arg( sample.x() ).arg( yAxisName ).arg( sample.y() );
                QwtText curveLabel( labelString, QwtText::RichText );
                curveLabel.setBackgroundBrush( QBrush( QColor( 250, 250, 250, 220 ) ) );
                curveLabel.setPaintAttribute( QwtText::PaintBackground );
                curveLabel.setBorderPen( QPen( Qt::black, 1.0 ) );
                curveLabel.setBorderRadius( 2.0 );
                m_selectedPointMarker->setLabel( curveLabel );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGridCrossQwtPlot::curveText( const QwtPlotCurve* curve, QString* curveTitle, QString* xParamName, QString* yParamName ) const
{
    CVF_ASSERT( curveTitle && xParamName && yParamName );

    auto riuCurve = dynamic_cast<const RiuRimQwtPlotCurve*>( curve );
    if ( riuCurve )
    {
        auto crossPlotCurve = dynamic_cast<const RimGridCrossPlotCurve*>( riuCurve->ownerRimCurve() );
        if ( crossPlotCurve )
        {
            *curveTitle = crossPlotCurve->curveName();

            RimGridCrossPlotDataSet* dataSet = nullptr;
            crossPlotCurve->firstAncestorOrThisOfType( dataSet );
            if ( dataSet )
            {
                *xParamName = dataSet->xAxisName();
                *yParamName = dataSet->yAxisName();
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGridCrossQwtPlot::isZoomerActive() const
{
    return m_zoomerLeft->isActiveAndValid() || m_zoomerRight->isActiveAndValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::endZoomOperations()
{
    m_zoomerLeft->endZoomOperation();
    m_zoomerRight->endZoomOperation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::onZoomedSlot()
{
    emit plotZoomed();
}
