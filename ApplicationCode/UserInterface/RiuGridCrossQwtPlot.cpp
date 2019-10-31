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
#include "RimPlotInterface.h"
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
RiuGridCrossQwtPlot::RiuGridCrossQwtPlot( RimPlotInterface* plotDefinition, QWidget* parent /*= nullptr*/ )
    : RiuQwtPlotWidget( plotDefinition, parent )
{
    // LeftButton for the zooming
    m_zoomerLeft = new RiuQwtPlotZoomer( canvas() );
    m_zoomerLeft->setRubberBandPen( QColor( Qt::black ) );
    m_zoomerLeft->setTrackerMode( QwtPicker::AlwaysOff );
    m_zoomerLeft->setTrackerPen( QColor( Qt::black ) );
    m_zoomerLeft->initMousePattern( 1 );
    m_zoomerLeft->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier );

    // Attach a zoomer for the right axis
    m_zoomerRight = new RiuQwtPlotZoomer( canvas() );
    m_zoomerRight->setAxis( xTop, yRight );
    m_zoomerRight->setTrackerMode( QwtPicker::AlwaysOff );
    m_zoomerRight->initMousePattern( 1 );
    m_zoomerRight->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier );

    // MidButton for the panning
    QwtPlotPanner* panner = new QwtPlotPanner( canvas() );
    panner->setMouseButton( Qt::MidButton );

    auto wheelZoomer = new RiuQwtPlotWheelZoomer( this );

    connect( wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerLeft, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( m_zoomerRight, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );

    m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>( new RiuPlotAnnotationTool() );
    m_infoBox        = new RiuDraggableOverlayFrame( this, canvas() );

    m_selectedPointMarker = new QwtPlotMarker;

    // QwtPlotMarker takes ownership of the symbol, it is deleted in destructor of QwtPlotMarker
    QwtSymbol* mySymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                         QBrush( QColor( 255, 255, 255, 50 ) ),
                                         QPen( Qt::black, 2.0 ),
                                         QSize( 10, 10 ) );
    m_selectedPointMarker->setSymbol( mySymbol );
    m_selectedPointMarker->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_selectedPointMarker->setSpacing( 3 );

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
void RiuGridCrossQwtPlot::addOrUpdateDataSetLegend( RimGridCrossPlotDataSet* dataSet )
{
    RiuCvfOverlayItemWidget* overlayWidget = nullptr;

    auto it = m_legendWidgets.find( dataSet );
    if ( it == m_legendWidgets.end() || it->second == nullptr )
    {
        overlayWidget            = new RiuCvfOverlayItemWidget( this, canvas() );
        m_legendWidgets[dataSet] = overlayWidget;
    }
    else
    {
        overlayWidget = it->second;
    }

    if ( overlayWidget )
    {
        caf::TitledOverlayFrame* overlayItem = dataSet->legendConfig()->titledOverlayFrame();
        applyFontSizeToOverlayItem( overlayItem );
        resizeOverlayItemToFitPlot( overlayItem );
        overlayWidget->updateFromOverlayItem( overlayItem );
    }
    this->updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::removeDataSetLegend( RimGridCrossPlotDataSet* dataSetToShowLegendFor )
{
    auto it = m_legendWidgets.find( dataSetToShowLegendFor );
    if ( it != m_legendWidgets.end() )
    {
        if ( it->second != nullptr )
        {
            it->second->hide();
            it->second->deleteLater();
        }

        m_legendWidgets.erase( it );
    }

    this->updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::removeDanglingDataSetLegends()
{
    for ( auto it = m_legendWidgets.begin(); it != m_legendWidgets.end(); )
    {
        if ( it->first.isNull() )
        {
            if ( it->second != nullptr )
            {
                it->second->hide();
                it->second->deleteLater();
            }
            m_legendWidgets.erase( it++ );
        }
        else
        {
            ++it;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::updateLegendSizesToMatchPlot()
{
    RimGridCrossPlot* crossPlot = dynamic_cast<RimGridCrossPlot*>( plotDefinition() );
    if ( !crossPlot ) return;

    bool anyLegendResized = false;

    for ( RimGridCrossPlotDataSet* dataSet : crossPlot->dataSets() )
    {
        if ( !dataSet->isChecked() || !dataSet->legendConfig()->showLegend() ) continue;

        auto pairIt = m_legendWidgets.find( dataSet );
        if ( pairIt != m_legendWidgets.end() )
        {
            RiuCvfOverlayItemWidget* overlayWidget = pairIt->second;
            caf::TitledOverlayFrame* overlayItem   = dataSet->legendConfig()->titledOverlayFrame();
            applyFontSizeToOverlayItem( overlayItem );
            if ( resizeOverlayItemToFitPlot( overlayItem ) )
            {
                anyLegendResized = true;
                overlayWidget->updateFromOverlayItem( overlayItem );
            }
        }
    }
    if ( anyLegendResized )
    {
        updateLegendLayout();
    }
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
RimViewWindow* RiuGridCrossQwtPlot::ownerViewWindow() const
{
    return dynamic_cast<RimViewWindow*>( plotDefinition() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::setLegendFontSize( int fontSize )
{
    if ( legend() )
    {
        QFont font = legend()->font();
        font.setPointSize( fontSize );
        legend()->setFont( font );
        // Set font size for all existing labels
        QList<QwtLegendLabel*> labels = legend()->findChildren<QwtLegendLabel*>();
        for ( QwtLegendLabel* label : labels )
        {
            label->setFont( font );
        }
    }
    updateInfoBoxLayout();
}

//--------------------------------------------------------------------------------------------------
/// The internal qwt legend is not used in grid plot windows
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
void RiuGridCrossQwtPlot::updateLayout()
{
    QwtPlot::updateLayout();
    updateInfoBoxLayout();
    updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::updateInfoBoxLayout()
{
    RimGridCrossPlot* crossPlot = dynamic_cast<RimGridCrossPlot*>( plotDefinition() );
    if ( !crossPlot ) return;

    bool showInfo = false;
    if ( crossPlot->showInfoBox() )
    {
        QStringList curveInfoTexts;
        for ( auto dataSet : crossPlot->dataSets() )
        {
            QString curveInfoText = dataSet->infoText();
            if ( dataSet->isChecked() && !curveInfoText.isEmpty() )
            {
                curveInfoTexts += curveInfoText;
            }
        }
        QStringList infoText;
        infoText << QString( "<b>View ID:</b> %1<br/>" ).arg( crossPlot->id() );
        if ( curveInfoTexts.size() > 1 )
        {
            infoText += QString( "<ol style=\"margin-top: 0px; margin-left: 15px; -qt-list-indent:0;\">" );
            for ( QString curveInfoText : curveInfoTexts )
            {
                infoText += QString( "<li>%1</li>" ).arg( curveInfoText );
            }
            infoText += QString( "</ol>" );
        }
        else if ( curveInfoTexts.size() > 0 )
        {
            infoText += curveInfoTexts.front();
        }
        if ( !infoText.empty() )
        {
            m_infoBox->label()->setText( infoText.join( "\n" ) );
            QFont font = m_infoBox->label()->font();
            font.setPointSize( crossPlot->legendFontSize() );
            m_infoBox->label()->setFont( font );
            m_infoBox->adjustSize();
            QRect infoRect   = m_infoBox->frameGeometry();
            QRect canvasRect = canvas()->frameGeometry();
            infoRect.moveTop( canvasRect.top() + 4 );
            infoRect.moveRight( canvasRect.right() - 4 );
            m_infoBox->move( infoRect.topLeft() );
            showInfo = true;
        }
    }
    if ( showInfo )
    {
        m_infoBox->show();
    }
    else
    {
        m_infoBox->hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::updateLegendLayout()
{
    const int spacing      = 5;
    int       startMarginX = this->canvas()->pos().x() + spacing;
    int       startMarginY = this->canvas()->pos().y() + spacing;

    int xpos           = startMarginX;
    int ypos           = startMarginY;
    int maxColumnWidth = 0;

    removeDanglingDataSetLegends();

    RimGridCrossPlot* crossPlot = dynamic_cast<RimGridCrossPlot*>( plotDefinition() );

    if ( !crossPlot ) return;

    std::set<QString> legendTypes;

    for ( RimGridCrossPlotDataSet* dataSet : crossPlot->dataSets() )
    {
        if ( dataSet->isChecked() && dataSet->groupingEnabled() && dataSet->legendConfig()->showLegend() )
        {
            auto pairIt = m_legendWidgets.find( dataSet );
            if ( pairIt != m_legendWidgets.end() )
            {
                RiuCvfOverlayItemWidget* overlayWidget = pairIt->second;

                // Show only one copy of each legend type
                if ( !legendTypes.count( dataSet->groupParameter() ) )
                {
                    if ( ypos + overlayWidget->height() + spacing > this->canvas()->height() )
                    {
                        xpos += spacing + maxColumnWidth;
                        ypos           = startMarginY;
                        maxColumnWidth = 0;
                    }

                    overlayWidget->show();
                    overlayWidget->move( xpos, ypos );

                    ypos += pairIt->second->height() + spacing;
                    maxColumnWidth = std::max( maxColumnWidth, pairIt->second->width() );
                    legendTypes.insert( dataSet->groupParameter() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::resizeEvent( QResizeEvent* e )
{
    QwtPlot::resizeEvent( e );
    updateLegendSizesToMatchPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGridCrossQwtPlot::resizeOverlayItemToFitPlot( caf::TitledOverlayFrame* overlayItem )
{
    QSize       plotSize           = this->canvas()->contentsRect().size();
    cvf::Vec2ui existingRenderSize = overlayItem->renderSize();
    cvf::Vec2ui legendSize         = overlayItem->preferredSize();

    bool sizeAltered = false;

    if ( plotSize.width() > 0 && (double)legendSize.x() > 0.9 * plotSize.width() )
    {
        legendSize.x() = ( plotSize.width() * 9 ) / 10;
        sizeAltered    = true;
    }
    if ( plotSize.height() > 0 && (double)legendSize.y() > 0.9 * plotSize.height() )
    {
        legendSize.y() = ( plotSize.height() * 9 ) / 10;
        sizeAltered    = true;
    }
    overlayItem->setRenderSize( legendSize );

    if ( legendSize.x() != existingRenderSize.x() || legendSize.y() != existingRenderSize.y() )
    {
        sizeAltered = true;
    }

    return sizeAltered;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem( ownerViewWindow() );

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
void RiuGridCrossQwtPlot::selectPoint( QwtPlotCurve* curve, int pointNumber )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::clearPointSelection()
{
    m_selectedPointMarker->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGridCrossQwtPlot::curveText( const QwtPlotCurve* curve,
                                     QString*            curveTitle,
                                     QString*            xParamName,
                                     QString*            yParamName ) const
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
void RiuGridCrossQwtPlot::applyFontSizeToOverlayItem( caf::TitledOverlayFrame* overlayItem )
{
    RimGridCrossPlot*   crossPlot = static_cast<RimGridCrossPlot*>( ownerViewWindow() );
    int                 fontSize  = crossPlot->legendFontSize();
    cvf::ref<cvf::Font> cafFont   = RiaFontCache::getFont( RiaFontCache::fontSizeEnumFromPointSize( fontSize ) );
    overlayItem->setFont( cafFont.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridCrossQwtPlot::onZoomedSlot()
{
    plotDefinition()->updateZoomFromQwt();
}
