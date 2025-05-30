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
#include "RiaPlotDefines.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimPlot.h"
#include "RimPlotAxisAnnotation.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimRegularLegendConfig.h"

#include "RiuContextMenuLauncher.h"
#include "RiuGuiTheme.h"
#include "RiuPlotAnnotationTool.h"
#include "RiuPlotCurve.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuQwtSymbol.h"
#include "RiuWidgetDragger.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafFixedAtlasFont.h"
#include "cafSelectionManager.h"
#include "cafTitledOverlayFrame.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include "qwt_text.h"
#include "qwt_text_engine.h"

#include <QLabel>
#include <QMenu>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <memory>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGridCrossQwtPlot::RiuGridCrossQwtPlot( RimGridCrossPlot* plot, QWidget* parent /*= nullptr*/ )
    : RiuQwtPlotWidget( plot, parent )
{
    // LeftButton for the zooming
    m_plotZoomer = new RiuQwtPlotZoomer( qwtPlot()->canvas() );
    m_plotZoomer->setTrackerMode( QwtPicker::AlwaysOff );
    m_plotZoomer->initMousePattern( 1 );

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner( qwtPlot()->canvas() );
    panner->setMouseButton( Qt::MiddleButton );

    auto wheelZoomer = new RiuQwtPlotWheelZoomer( qwtPlot() );

    connect( wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    connect( m_plotZoomer, SIGNAL( zoomed() ), SLOT( onZoomedSlot() ) );
    connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );
    connect( this,
             SIGNAL( plotItemSelected( std::shared_ptr<RiuPlotItem>, bool, int ) ),
             SLOT( onPlotItemSelected( std::shared_ptr<RiuPlotItem>, bool, int ) ) );

    m_annotationTool      = std::make_unique<RiuPlotAnnotationTool>();
    m_selectedPointMarker = new QwtPlotMarker;

    // QwtPlotMarker takes ownership of the symbol, it is deleted in destructor of QwtPlotMarker
    auto       color    = RiuGuiTheme::getColorByVariableName( "markerColor" );
    QwtSymbol* mySymbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( QColor( 255, 255, 255, 50 ) ), QPen( color, 2.0 ), QSize( 10, 10 ) );

    m_selectedPointMarker->setSymbol( mySymbol );
    m_selectedPointMarker->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_selectedPointMarker->setSpacing( 3 );
    m_selectedPointMarker->setZ( 1000.0 ); // Make sure it ends up in front of highlighted curves.

    RiuQwtPlotTools::setCommonPlotBehaviour( qwtPlot() );
    RiuQwtPlotTools::setDefaultAxes( qwtPlot() );

    installEventFilter( this );
    qwtPlot()->canvas()->installEventFilter( this );

    setInternalQwtLegendVisible( true );

    {
        caf::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "RicSwapGridCrossPlotDataSetAxesFeature";
        menuBuilder << "Separator";
        menuBuilder << "RicShowPlotDataFeature";

        new RiuContextMenuLauncher( this, menuBuilder );
    }
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
void RiuGridCrossQwtPlot::updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties )
{
    m_annotationTool->detachAllAnnotations();

    for ( auto annotation : axisProperties->annotations() )
    {
        m_annotationTool->attachAnnotationLine( qwtPlot(),
                                                annotation->color(),
                                                annotation->name(),
                                                annotation->penStyle(),
                                                annotation->value(),
                                                RiaDefines::Orientation::HORIZONTAL,
                                                Qt::AlignRight );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::setLegendFontSize( int fontSize )
{
    if ( qwtPlot()->legend() )
    {
        QFont font = qwtPlot()->legend()->font();
        font.setPointSize( fontSize );
        qwtPlot()->legend()->setFont( font );
        // Set font size for all existing labels
        QList<QwtLegendLabel*> labels = qwtPlot()->legend()->findChildren<QwtLegendLabel*>();
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
        qwtPlot()->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        qwtPlot()->insertLegend( nullptr );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int pointNumber )
{
    if ( pointNumber == -1 )
        m_selectedPointMarker->detach();
    else
    {
        RiuQwtPlotItem* qwtPlotItem = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
        if ( !qwtPlotItem ) return;

        QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( qwtPlotItem->qwtPlotItem() );
        if ( curve )
        {
            QPointF sample = curve->sample( pointNumber );
            m_selectedPointMarker->setValue( sample );
            m_selectedPointMarker->setAxes( QwtAxis::XBottom, QwtAxis::YLeft );
            m_selectedPointMarker->attach( qwtPlot() );
            QString curveName, xAxisName, yAxisName;
            if ( curveText( curve, &curveName, &xAxisName, &yAxisName ) )
            {
                QString labelFormat( "<div style=\"margin: 4px;\"><b>%1:</b><br/>%2 = %3, %4 = %5</div>" );
                QString labelString = labelFormat.arg( curveName ).arg( xAxisName ).arg( sample.x() ).arg( yAxisName ).arg( sample.y() );
                QwtText curveLabel( labelString, QwtText::RichText );
                curveLabel.setBackgroundBrush( QBrush( QColor( 250, 250, 250, 220 ) ) );
                curveLabel.setPaintAttribute( QwtText::PaintBackground );

                auto color = RiuGuiTheme::getColorByVariableName( "markerColor" );
                curveLabel.setBorderPen( QPen( color, 1.0 ) );

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

    auto riuCurve = dynamic_cast<const RiuPlotCurve*>( curve );
    if ( riuCurve )
    {
        auto crossPlotCurve = dynamic_cast<const RimGridCrossPlotCurve*>( riuCurve->ownerRimCurve() );
        if ( crossPlotCurve )
        {
            *curveTitle = crossPlotCurve->curveName();

            auto dataSet = crossPlotCurve->firstAncestorOrThisOfType<RimGridCrossPlotDataSet>();
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
    return m_plotZoomer->isActiveAndValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::endZoomOperations()
{
    m_plotZoomer->endZoomOperation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::onZoomedSlot()
{
    emit plotZoomed();
}
