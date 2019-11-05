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
#include "RimPlotInterface.h"
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
RiuSummaryQwtPlot::RiuSummaryQwtPlot( RimPlotInterface* plotDefinition, QWidget* parent /*= nullptr*/ )
    : RiuQwtPlotWidget( plotDefinition, parent )
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

    setDefaults();
    new RiuQwtCurvePointTracker( this, true, &ensembleCurveInfoTextProvider );

    RiuQwtPlotTools::setCommonPlotBehaviour( this );
    RiuQwtPlotTools::setDefaultAxes( this );

    this->installEventFilter( this );
    this->canvas()->installEventFilter( this );

    setLegendVisible( true );
}

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
void RiuSummaryQwtPlot::addOrUpdateEnsembleCurveSetLegend( RimEnsembleCurveSet* curveSetToShowLegendFor )
{
    RiuCvfOverlayItemWidget* overlayWidget = nullptr;

    auto it = m_ensembleLegendWidgets.find( curveSetToShowLegendFor );
    if ( it == m_ensembleLegendWidgets.end() || it->second == nullptr )
    {
        overlayWidget                                    = new RiuCvfOverlayItemWidget( this, canvas() );
        m_ensembleLegendWidgets[curveSetToShowLegendFor] = overlayWidget;
    }
    else
    {
        overlayWidget = it->second;
    }

    if ( overlayWidget )
    {
        caf::TitledOverlayFrame* overlayItem = curveSetToShowLegendFor->legendConfig()->titledOverlayFrame();
        overlayItem->setRenderSize( overlayItem->preferredSize() );

        overlayWidget->updateFromOverlayItem( curveSetToShowLegendFor->legendConfig()->titledOverlayFrame() );
        overlayWidget->show();
    }

    this->updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::removeEnsembleCurveSetLegend( RimEnsembleCurveSet* curveSetToShowLegendFor )
{
    auto it = m_ensembleLegendWidgets.find( curveSetToShowLegendFor );
    if ( it != m_ensembleLegendWidgets.end() )
    {
        if ( it->second != nullptr ) it->second->deleteLater();

        m_ensembleLegendWidgets.erase( it );
    }

    this->updateLegendLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuSummaryQwtPlot::ownerViewWindow() const
{
    return dynamic_cast<RimViewWindow*>( plotDefinition() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::setLegendFontSize( int fontSize )
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
void RiuSummaryQwtPlot::keyPressEvent( QKeyEvent* keyEvent )
{
    RimSummaryPlot* summaryPlot = dynamic_cast<RimSummaryPlot*>( plotDefinition() );

    if ( summaryPlot )
    {
        summaryPlot->handleKeyPressEvent( keyEvent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem( plotOwner() );

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
void RiuSummaryQwtPlot::updateLayout()
{
    QwtPlot::updateLayout();
    updateLegendLayout();
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
    plotDefinition()->updateZoomFromQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQwtPlot::updateLegendLayout()
{
    const int spacing      = 5;
    int       startMarginX = this->canvas()->pos().x() + spacing;
    int       startMarginY = this->canvas()->pos().y() + spacing;

    int xpos           = startMarginX;
    int ypos           = startMarginY;
    int maxColumnWidth = 0;

    RimSummaryPlot* summaryPlot = dynamic_cast<RimSummaryPlot*>( plotDefinition() );

    if ( !summaryPlot || !summaryPlot->ensembleCurveSetCollection() ) return;

    for ( RimEnsembleCurveSet* curveSet : summaryPlot->ensembleCurveSetCollection()->curveSets() )
    {
        auto pairIt = m_ensembleLegendWidgets.find( curveSet );
        if ( pairIt != m_ensembleLegendWidgets.end() )
        {
            if ( ypos + pairIt->second->height() + spacing > this->canvas()->height() )
            {
                xpos += spacing + maxColumnWidth;
                ypos           = startMarginY;
                maxColumnWidth = 0;
            }

            RiuCvfOverlayItemWidget* overlayWidget = pairIt->second;
            overlayWidget->move( xpos, ypos );

            ypos += pairIt->second->height() + spacing;
            maxColumnWidth = std::max( maxColumnWidth, pairIt->second->width() );
        }
    }
}
