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

#include "RiaApplication.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"

#include "Commands/CorrelationPlotCommands/RicNewCorrelationPlotFeature.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimMainPlotCollection.h"
#include "RimPlot.h"
#include "RimPlotAxisAnnotation.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotAnnotationTool.h"
#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWidgetDragger.h"

#include "RiuPlotMainWindowTools.h"

#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafIconProvider.h"
#include "cafSelectionManager.h"
#include "cafTitledOverlayFrame.h"

#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QWheelEvent>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQtChartsPlot::RiuSummaryQtChartsPlot( RimSummaryPlot* plot, QWidget* parent /*= nullptr*/ )
    : RiuSummaryPlot( plot, parent )
{
    m_plotWidget = std::make_unique<RiuQtChartsPlotWidget>( plot );

    // // LeftButton for the zooming
    // m_zoomerLeft = new RiuQwtPlotZoomer( m_plotWidget->qwtPlot()->canvas() );
    // m_zoomerLeft->setTrackerMode( QwtPicker::AlwaysOff );
    // m_zoomerLeft->initMousePattern( 1 );

    // // Attach a zoomer for the right axis
    // m_zoomerRight = new RiuQwtPlotZoomer( m_plotWidget->qwtPlot()->canvas() );
    // m_zoomerRight->setAxis( QwtPlot::xTop, QwtPlot::yRight );
    // m_zoomerRight->setTrackerMode( QwtPicker::AlwaysOff );
    // m_zoomerRight->initMousePattern( 1 );

    // // MidButton for the panning
    // QwtPlotPanner* panner = new QwtPlotPanner( m_plotWidget->qwtPlot()->canvas() );
    // panner->setMouseButton( Qt::MidButton );

    // m_wheelZoomer = new RiuQwtPlotWheelZoomer( m_plotWidget->qwtPlot() );

    // connect( m_wheelZoomer, SIGNAL( zoomUpdated() ), SLOT( onZoomedSlot() ) );
    // connect( m_zoomerLeft, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    // connect( m_zoomerRight, SIGNAL( zoomed( const QRectF& ) ), SLOT( onZoomedSlot() ) );
    // connect( panner, SIGNAL( panned( int, int ) ), SLOT( onZoomedSlot() ) );

    // setDefaults();
    // new RiuQwtCurvePointTracker( m_plotWidget->qwtPlot(), true, &ensembleCurveInfoTextProvider );

    // RiuQwtPlotTools::setCommonPlotBehaviour( m_plotWidget->qwtPlot() );
    // RiuQwtPlotTools::setDefaultAxes( m_plotWidget->qwtPlot() );

    m_plotWidget->setInternalLegendVisible( true );

    // m_annotationTool = std::unique_ptr<RiuPlotAnnotationTool>( new RiuPlotAnnotationTool() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuSummaryQtChartsPlot::~RiuSummaryQtChartsPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::useDateBasedTimeAxis( const QString&                          dateFormat,
                                                   const QString&                          timeFormat,
                                                   RiaQDateTimeTools::DateFormatComponents dateComponents,
                                                   RiaQDateTimeTools::TimeFormatComponents timeComponents )
{
    m_plotWidget->setAxisScaleType( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, RiuPlotWidget::AxisScaleType::DATE );

    //     RiuQtChartsPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(),
    //                                                       dateFormat,
    //                                                       timeFormat,
    //                                                       dateComponents,
    //                                                       timeComponents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::useTimeBasedTimeAxis()
{
    m_plotWidget->setAxisScaleType( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, RiuPlotWidget::AxisScaleType::DATE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::setAxisIsLogarithmic( QwtPlot::Axis axis, bool logarithmic )
{
    //    if ( m_wheelZoomer ) m_wheelZoomer->setAxisIsLogarithmic( axis, logarithmic );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSummaryQtChartsPlot::updateAnnotationObjects( RimPlotAxisPropertiesInterface* axisProperties )
{
    // RiuPlotAnnotationTool::Orientation orientation = RiuPlotAnnotationTool::Orientation::HORIZONTAL;
    // if ( axisProperties->plotAxisType() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
    // {
    //     orientation = RiuPlotAnnotationTool::Orientation::VERTICAL;
    // }
    // m_annotationTool->detachAllAnnotations( orientation );

    // for ( auto annotation : axisProperties->annotations() )
    // {
    //     if ( annotation->annotationType() == RimPlotAxisAnnotation::AnnotationType::LINE )
    //     {
    //         m_annotationTool->attachAnnotationLine( m_plotWidget->qwtPlot(),
    //                                                 annotation->color(),
    //                                                 annotation->name(),
    //                                                 annotation->value(),
    //                                                 orientation );
    //     }
    //     else if ( annotation->annotationType() == RimPlotAxisAnnotation::AnnotationType::RANGE )
    //     {
    //         m_annotationTool->attachAnnotationRange( m_plotWidget->qwtPlot(),
    //                                                  annotation->color(),
    //                                                  annotation->name(),
    //                                                  annotation->rangeStart(),
    //                                                  annotation->rangeEnd(),
    //                                                  orientation );
    //     }
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RiuSummaryQtChartsPlot::contextMenuEvent( QContextMenuEvent* event )
// {
// QMenu                      menu;
// caf::CmdFeatureMenuBuilder menuBuilder;

// emit m_plotWidget->plotSelected( false );

// menuBuilder << "RicShowPlotDataFeature";
// menuBuilder << "RicSavePlotTemplateFeature";

// QwtPlotItem* closestItem       = nullptr;
// double       distanceFromClick = std::numeric_limits<double>::infinity();
// int          closestCurvePoint = -1;
// QPoint       globalPos         = event->globalPos();
// QPoint       localPos          = m_plotWidget->qwtPlot()->canvas()->mapFromGlobal( globalPos );

// m_plotWidget->findClosestPlotItem( localPos, &closestItem, &closestCurvePoint, &distanceFromClick );
// if ( closestItem && closestCurvePoint >= 0 )
// {
//     RiuPlotCurve* plotCurve = dynamic_cast<RiuPlotCurve*>( closestItem );
//     if ( plotCurve )
//     {
//         RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>( plotCurve->ownerRimCurve() );
//         if ( summaryCurve && closestCurvePoint < (int)summaryCurve->timeStepsY().size() )
//         {
//             std::time_t timeStep = summaryCurve->timeStepsY()[closestCurvePoint];

//             RimSummaryCaseCollection* ensemble = nullptr;
//             QString                   clickedQuantityName;
//             QStringList               allQuantityNamesInPlot;

//             RimEnsembleCurveSet* clickedEnsembleCurveSet = nullptr;
//             summaryCurve->firstAncestorOrThisOfType( clickedEnsembleCurveSet );

//             bool curveClicked = distanceFromClick < 50;

//             if ( clickedEnsembleCurveSet )
//             {
//                 ensemble = clickedEnsembleCurveSet->summaryCaseCollection();
//                 if ( ensemble && ensemble->isEnsemble() )
//                 {
//                     clickedQuantityName = QString::fromStdString(
//                     clickedEnsembleCurveSet->summaryAddress().uiText() );
//                 }
//             }

//             {
//                 auto summaryCase = summaryCurve->summaryCaseY();
//                 if ( summaryCase )
//                 {
//                     int      summaryCaseId = summaryCase->caseId();
//                     QVariant summaryCaseIdVariant( summaryCaseId );
//                     auto     modelName = summaryCase->nativeCaseName();

//                     menuBuilder.addCmdFeatureWithUserData( "RicImportGridModelFromSummaryCurveFeature",
//                                                            QString( "Open Grid Model '%1'" ).arg( modelName ),
//                                                            summaryCaseIdVariant );
//                 }
//             }

//             if ( !curveClicked )
//             {
//                 RimSummaryPlot* summaryPlot = static_cast<RimSummaryPlot*>( m_plotWidget->plotDefinition() );
//                 std::vector<RimEnsembleCurveSet*> allCurveSetsInPlot;
//                 summaryPlot->descendantsOfType( allCurveSetsInPlot );
//                 for ( auto curveSet : allCurveSetsInPlot )
//                 {
//                     allQuantityNamesInPlot.push_back( QString::fromStdString( curveSet->summaryAddress().uiText()
//                     ) );
//                 }
//             }
//             else
//             {
//                 allQuantityNamesInPlot.push_back( clickedQuantityName );
//             }

//             if ( !clickedQuantityName.isEmpty() || !allQuantityNamesInPlot.isEmpty() )
//             {
//                 if ( ensemble && ensemble->isEnsemble() )
//                 {
//                     EnsemblePlotParams params( ensemble, allQuantityNamesInPlot, clickedQuantityName, timeStep );
//                     QVariant           variant = QVariant::fromValue( params );

//                     menuBuilder.addCmdFeatureWithUserData( "RicNewAnalysisPlotFeature", "New Analysis Plot",
//                     variant );

//                     QString subMenuName = "Create Correlation Plot";
//                     if ( curveClicked )
//                     {
//                         subMenuName = "Create Correlation Plot From Curve Point";
//                     }
//                     menuBuilder.subMenuStart( subMenuName, *caf::IconProvider( ":/CorrelationPlots16x16.png"
//                     ).icon() );

//                     {
//                         if ( curveClicked )
//                         {
//                             menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationPlotFeature",
//                                                                    "New Tornado Plot",
//                                                                    variant );
//                         }
//                         menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationMatrixPlotFeature",
//                                                                "New Matrix Plot",
//                                                                variant );
//                         menuBuilder.addCmdFeatureWithUserData( "RicNewCorrelationReportPlotFeature",
//                                                                "New Report Plot",
//                                                                variant );
//                         if ( curveClicked )
//                         {
//                             menuBuilder.subMenuStart( "Cross Plots",
//                                                       *caf::IconProvider( ":/CorrelationCrossPlot16x16.png"
//                                                       ).icon() );
//                             std::vector<std::pair<RigEnsembleParameter, double>> ensembleParameters =
//                                 ensemble->parameterCorrelations( clickedEnsembleCurveSet->summaryAddress(),
//                                 timeStep );
//                             std::sort( ensembleParameters.begin(),
//                                        ensembleParameters.end(),
//                                        []( const std::pair<RigEnsembleParameter, double>& lhs,
//                                            const std::pair<RigEnsembleParameter, double>& rhs ) {
//                                            return std::fabs( lhs.second ) > std::fabs( rhs.second );
//                                        } );

//                             for ( const auto& param : ensembleParameters )
//                             {
//                                 if ( std::fabs( param.second ) >= 1.0e-6 )
//                                 {
//                                     params.ensembleParameter = param.first.name;
//                                     variant                  = QVariant::fromValue( params );
//                                     menuBuilder.addCmdFeatureWithUserData(
//                                     "RicNewParameterResultCrossPlotFeature",
//                                                                            QString( "New Cross Plot Against %1 "
//                                                                                     "(Correlation: %2)" )
//                                                                                .arg( param.first.name )
//                                                                                .arg( param.second, 5, 'f', 2 ),
//                                                                            variant );
//                                 }
//                             }
//                             menuBuilder.subMenuEnd();
//                         }
//                     }
//                     menuBuilder.subMenuEnd();
//                 }
//             }
//         }
//     }
// }

// menuBuilder.appendToMenu( &menu );

// if ( menu.actions().size() > 0 )
// {
//     menu.exec( event->globalPos() );

//     // Parts of progress dialog GUI can be present after menu has closed related to
//     // RicImportGridModelFromSummaryCurveFeature. Make sure the plot is updated, and call processEvents() to make
//     // sure all GUI events are processed
//     m_plotWidget->update();
//     QApplication::processEvents();
// }
//}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RiuSummaryQwtPlot::setDefaults()
// {
//     QString dateFormat = RiaPreferences::current()->dateFormat();
//     QString timeFormat = RiaPreferences::current()->timeFormat();

//     useDateBasedTimeAxis( dateFormat, timeFormat );
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// bool RiuSummaryQwtPlot::isZoomerActive() const
// {
//     return m_zoomerLeft->isActiveAndValid() || m_zoomerRight->isActiveAndValid();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RiuSummaryQwtPlot::endZoomOperations()
// {
//     m_zoomerLeft->endZoomOperation();
//     m_zoomerRight->endZoomOperation();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RiuSummaryQwtPlot::onZoomedSlot()
// {
//     emit m_plotWidget->plotZoomed();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RiuSummaryQtChartsPlot::plotWidget() const
{
    return m_plotWidget.get();
}
