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

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimMainPlotCollection.h"
#include "RimPlot.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
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
        RiuRimQwtPlotCurve* riuCurve = dynamic_cast<RiuRimQwtPlotCurve*>( curve );
        RimSummaryCurve*    sumCurve = nullptr;
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
RiuSummaryQwtPlot::~RiuSummaryQwtPlot()
{
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

    QwtPlotItem* closestItem       = nullptr;
    double       distanceFromClick = std::numeric_limits<double>::infinity();
    int          closestCurvePoint = -1;
    QPoint       globalPos         = event->globalPos();
    QPoint       localPos          = this->canvas()->mapFromGlobal( globalPos );

    findClosestPlotItem( localPos, &closestItem, &closestCurvePoint, &distanceFromClick );
    if ( closestItem && closestCurvePoint >= 0 )
    {
        RiuRimQwtPlotCurve* plotCurve = dynamic_cast<RiuRimQwtPlotCurve*>( closestItem );
        if ( plotCurve )
        {
            RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>( plotCurve->ownerRimCurve() );
            if ( summaryCurve && closestCurvePoint < (int)summaryCurve->timeStepsY().size() )
            {
                std::time_t timeStep = summaryCurve->timeStepsY()[closestCurvePoint];

                RimEnsembleCurveSet* ensembleCurveSet = nullptr;
                summaryCurve->firstAncestorOrThisOfType( ensembleCurveSet );

                if ( ensembleCurveSet )
                {
                    RimSummaryCaseCollection* ensemble = ensembleCurveSet->summaryCaseCollection();
                    if ( ensemble && ensemble->isEnsemble() )
                    {
                        EnsemblePlotParams params( ensemble,
                                                   QString::fromStdString(
                                                       ensembleCurveSet->summaryAddress().quantityName() ),
                                                   timeStep );
                        QVariant           variant = QVariant::fromValue( params );

                        menuBuilder.addCmdFeatureWithUserData( "RicNewAnalysisPlotFeature", "New Analysis Plot", variant );

                        menuBuilder.subMenuStart( "Create Correlation Plot From Curve Point",
                                                  *caf::IconProvider( ":/CorrelationPlots16x16.png" ).icon() );
                        {
                            menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationPlotFeature",
                                                                   "New Tornado Plot",
                                                                   variant );
                            menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationMatrixPlotFeature",
                                                                   "New Matrix Plot",
                                                                   variant );
                            menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationReportPlotFeature",
                                                                   "New Report Plot",
                                                                   variant );
                            menuBuilder.subMenuStart( "Cross Plots",
                                                      *caf::IconProvider( ":/CorrelationCrossPlot16x16.png" ).icon() );
                            std::vector<EnsembleParameter> ensembleParameters =
                                ensemble->variationSortedEnsembleParameters();
                            for ( const EnsembleParameter& param : ensembleParameters )
                            {
                                if ( param.variationBin >= (int)EnsembleParameter::LOW_VARIATION )
                                {
                                    params.ensembleParameter = param.name;
                                    variant                  = QVariant::fromValue( params );
                                    menuBuilder.addCmdFeatureWithUserData( "RicNewParameterResultCrossPlotFeature",
                                                                           QString( "New Cross Plot Against %1" )
                                                                               .arg( param.uiName() ),
                                                                           variant );
                                }
                            }
                            menuBuilder.subMenuEnd();
                        }
                        menuBuilder.subMenuEnd();
                    }
                }
            }
        }
    }

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
