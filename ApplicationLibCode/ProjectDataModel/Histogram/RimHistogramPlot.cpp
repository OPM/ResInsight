/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Statoil ASA
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

#include "RimHistogramPlot.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaCurveMerger.h"
#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"
// #include "RiaPreferencesHistogram.h"
// #include "RiaRegressionTestRunner.h"
#include "RiaStdStringTools.h"
// #include "Histogram/RiaHistogramCurveDefinition.h"
// #include "Histogram/RiaHistogramDefines.h"
// #include "Histogram/RiaHistogramPlotTools.h"
// #include "Histogram/RiaHistogramTools.h"

// #include "RifEclipseHistogramAddressDefines.h"

// #include "HistogramPlotCommands/RicHistogramPlotEditorUi.h"

// #include "Annotations/RimTimeAxisAnnotationUpdater.h"
// #include "RimAsciiDataCurve.h"
// #include "RimEnsembleCurveSet.h"
// #include "RimEnsembleCurveSetCollection.h"
// #include "RimGridTimeHistoryCurve.h"
#include "RimMultiPlot.h"
#include "RimPlotAxisLogRangeCalculator.h"
// #include "RimProject.h"
// #include "RimHistogramAddress.h"
// #include "RimHistogramAddressCollection.h"
// #include "RimHistogramCalculationCollection.h"
// #include "RimHistogramCase.h"
// #include "RimHistogramCurve.h"
// #include "RimHistogramCurveAppearanceCalculator.h"
#include "RimHistogramCurve.h"
#include "RimHistogramCurveCollection.h"
// #include "RimHistogramCurvesData.h"
// #include "RimHistogramEnsemble.h"
// #include "RimHistogramPlotAxisFormatter.h"
// #include "RimHistogramPlotControls.h"
// #include "RimHistogramPlotNameHelper.h"
// #include "RimHistogramTimeAxisProperties.h"
#include "Tools/RimPlotAxisTools.h"

// #include "RiuHistogramQwtPlot.h"
#include "RiuPlotAxis.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotWidget.h"

// #ifdef USE_QTCHARTS
// #include "RiuHistogramQtChartsPlot.h"
// #endif

#include "cvfColor3.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManager.h"

#include "qwt_date.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_textlabel.h"
#include "qwt_text.h"

#include <QDateTime>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QString>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

CAF_PDM_SOURCE_INIT( RimHistogramPlot, "HistogramPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot::RimHistogramPlot()
    : curvesChanged( this )
    , axisChanged( this )
    , titleChanged( this )
    , m_isValid( true )
    , axisChangedReloadRequired( this )
    , autoTitleChanged( this )
    , m_legendPosition( RiuPlotWidget::Legend::BOTTOM )
{
    CAF_PDM_InitScriptableObject( "Histogram Plot", ":/HistogramPlotLight16x16.png", "", "A Histogram Plot" );

    CAF_PDM_InitScriptableField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto Title" );
    CAF_PDM_InitScriptableField( &m_description, "PlotDescription", QString( "Histogram Plot" ), "Name" );
    CAF_PDM_InitScriptableField( &m_normalizeCurveYValues, "normalizeCurveYValues", false, "Normalize all curves" );

    CAF_PDM_InitFieldNoDefault( &m_histogramCurveCollection, "HistogramCurveCollection", "" );
    m_histogramCurveCollection = new RimHistogramCurveCollection();

    CAF_PDM_InitFieldNoDefault( &m_axisPropertiesArray, "AxisProperties", "Axes", ":/Axes16x16.png" );
    m_axisPropertiesArray.uiCapability()->setUiTreeHidden( false );

    auto leftAxis = addNewAxisProperties( RiuPlotAxis::defaultLeft(), "Left" );
    leftAxis->setAlwaysRequired( true );

    auto rightAxis = addNewAxisProperties( RiuPlotAxis::defaultRight(), "Right" );
    rightAxis->setAlwaysRequired( true );

    CAF_PDM_InitFieldNoDefault( &m_fallbackPlotName, "AlternateName", "AlternateName" );
    m_fallbackPlotName.uiCapability()->setUiReadOnly( true );
    m_fallbackPlotName.uiCapability()->setUiHidden( true );
    m_fallbackPlotName.xmlCapability()->disableIO();

    // setPlotInfoLabel( "Filters Active" );

    // ensureRequiredAxisObjectsForCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot::~RimHistogramPlot()
{
    m_isValid = false;

    removeMdiWindowFromMdiArea();

    deletePlotCurvesAndPlotWidget();

    delete m_histogramCurveCollection;
    // delete m_ensembleCurveSetCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAxes()
{
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

    updateAnnotationsInPlotWidget();

    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    updateNumericalAxis( RiaDefines::PlotAxis::PLOT_AXIS_TOP );
    // updateTimeAxis( timeAxisProperties() );

    updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAnnotationsInPlotWidget()
{
    // if ( m_histogramPlot ) m_histogramPlot->clearAnnotationObjects();

    // if ( !plotWidget() ) return;

    // if ( timeAxisProperties() )
    // {
    //     m_histogramPlot->updateAnnotationObjects( timeAxisProperties() );
    // }

    // if ( auto leftYAxisProperties = axisPropertiesForPlotAxis( RiuPlotAxis::defaultLeft() ) )
    // {
    //     m_histogramPlot->updateAnnotationObjects( leftYAxisProperties );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isLogarithmicScaleEnabled( RiuPlotAxis plotAxis ) const
{
    auto axisProperties = axisPropertiesForPlotAxis( plotAxis );
    if ( !axisProperties ) return false;

    return axisProperties->isLogarithmicScaleEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isCurveHighlightSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimHistogramTimeAxisProperties* RimHistogramPlot::timeAxisProperties()
// {
//     // Find the first time axis (which is correct since there is only one).
//     // for ( const auto& ap : m_axisPropertiesArray )
//     // {
//     //     auto* timeAxis = dynamic_cast<RimHistogramTimeAxisProperties*>( ap.p() );
//     //     if ( timeAxis ) return timeAxis;
//     // }

//     return nullptr;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimHistogramPlot::viewWidget()
{
    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimHistogramPlot::plotWidget()
{
    if ( !m_histogramPlot ) return nullptr;

    return m_histogramPlot.get(); //->plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramPlot::asciiDataForPlotExport() const
{
    return asciiDataForHistogramPlotExport( RiaDefines::DateTimePeriod::YEAR, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramPlot::asciiDataForHistogramPlotExport( RiaDefines::DateTimePeriod resamplingPeriod, bool showTimeAsLongString ) const
{
    // std::vector<RimHistogramCurve*> allCurves = descendantsIncludingThisOfType<RimHistogramCurve>();

    // std::vector<RimHistogramCurve*> crossPlotCurves;
    // std::vector<RimHistogramCurve*> curves;
    // std::vector<RimHistogramCurve*> observedCurves;

    // for ( auto c : allCurves )
    // {
    //     if ( c->axisTypeX() == RiaDefines::HorizontalAxisType::HISTOGRAM_VECTOR )
    //     {
    //         crossPlotCurves.push_back( c );
    //     }
    //     else if ( c->histogramCaseY() && c->histogramCaseY()->isObservedData() && !c->isRegressionCurve() )
    //     {
    //         observedCurves.push_back( c );
    //     }
    //     else
    //     {
    //         curves.push_back( c );
    //     }
    // }

    // auto gridCurves  = m_gridTimeHistoryCurves.childrenByType();
    // auto asciiCurves = m_asciiDataCurves.childrenByType();

    QString text;
    // text += RimHistogramCurvesData::createTextForExport( curves, asciiCurves, gridCurves, resamplingPeriod, showTimeAsLongString );

    // if ( !observedCurves.empty() )
    // {
    //     text += "\n\n------------ Observed Curves --------------";
    //     text += RimHistogramCurvesData::createTextForExport( observedCurves, {}, {}, RiaDefines::DateTimePeriod::NONE,
    //     showTimeAsLongString );
    // }

    // text += RimHistogramCurvesData::createTextForCrossPlotCurves( crossPlotCurves );

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onAxisSelected( RiuPlotAxis axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();

    caf::PdmObject* itemToSelect = axisPropertiesForPlotAxis( axis );

    RiuPlotMainWindowTools::selectOrToggleObject( itemToSelect, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// std::vector<RimEnsembleCurveSet*> RimHistogramPlot::curveSets() const
// {
//     return ensembleCurveSetCollection()->curveSets();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// std::vector<RimHistogramCurve*> RimHistogramPlot::allCurves() const
// {
//     return histogramCurves();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// std::vector<RimHistogramCurve*> RimHistogramPlot::histogramAndEnsembleCurves() const
// {
//     std::vector<RimHistogramCurve*> curves = histogramCurves();

//     for ( const auto& curveSet : ensembleCurveSetCollection()->curveSets() )
//     {
//         for ( const auto& curve : curveSet->curves() )
//         {
//             curves.push_back( curve );
//         }
//     }
//     return curves;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// std::set<RiaHistogramCurveDefinition> RimHistogramPlot::histogramAndEnsembleCurveDefinitions() const
// {
//     std::set<RiaHistogramCurveDefinition> allCurveDefs;

//     for ( const auto& curve : histogramAndEnsembleCurves() )
//     {
//         allCurveDefs.insert( RiaHistogramCurveDefinition( curve->histogramCaseY(), curve->histogramAddressY(), curve->isEnsembleCurve() )
//         );
//     }
//     return allCurveDefs;
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// std::vector<RimHistogramCurve*> RimHistogramPlot::histogramCurves() const
// {
//     return m_histogramCurveCollection->curves();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::deleteAllHistogramCurves()
// {
//     m_histogramCurveCollection->deleteAllCurves();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimHistogramCurveCollection* RimHistogramPlot::histogramCurveCollection() const
// {
//     return m_histogramCurveCollection();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updatePlotTitle()
{
    // m_nameHelperAllCurves->clear();
    // updateNameHelperWithCurveData( m_nameHelperAllCurves.get() );
    // if ( m_useAutoPlotTitle )
    // {
    //     m_description = m_nameHelperAllCurves->plotTitle();
    // }

    if ( m_description().isEmpty() )
    {
        auto multiPlot = firstAncestorOrThisOfType<RimMultiPlot>();

        size_t index = 0;
        if ( multiPlot ) index = multiPlot->plotIndex( this );

        QString title      = QString( "Sub Plot %1" ).arg( index + 1 );
        m_fallbackPlotName = title;
    }

    updateCurveNames();
    updateMdiWindowTitle();

    if ( plotWidget() )
    {
        QString plotTitle = description();
        plotWidget()->setPlotTitle( plotTitle );
        plotWidget()->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
        scheduleReplotIfVisible();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// const RimHistogramNameHelper* RimHistogramPlot::activePlotTitleHelperAllCurves() const
// {
//     if ( m_useAutoPlotTitle() )
//     {
//         return m_nameHelperAllCurves.get();
//     }

//     return nullptr;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::copyAxisPropertiesFromOther( const RimHistogramPlot& sourceHistogramPlot )
// {
//     for ( auto ap : sourceHistogramPlot.allPlotAxes() )
//     {
//         QString data = ap->writeObjectToXmlString();

//         auto axisProperty = axisPropertiesForPlotAxis( ap->plotAxis() );
//         if ( axisProperty )
//         {
//             axisProperty->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
//         }
//     }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::copyAxisPropertiesFromOther( RiaDefines::PlotAxis plotAxisType, const RimHistogramPlot& sourceHistogramPlot )
// {
//     for ( auto ap : sourceHistogramPlot.allPlotAxes() )
//     {
//         if ( ap->plotAxis().axis() != plotAxisType ) continue;

//         QString data = ap->writeObjectToXmlString();

//         auto axisProperty = axisPropertiesForPlotAxis( ap->plotAxis() );
//         if ( axisProperty )
//         {
//             axisProperty->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
//         }
//     }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::copyMatchingAxisPropertiesFromOther( const RimHistogramPlot& histogramPlot )
// {
//     for ( auto apToCopy : histogramPlot.allPlotAxes() )
//     {
//         for ( auto ap : allPlotAxes() )
//         {
//             if ( ap->objectName().compare( apToCopy->objectName() ) == 0 )
//             {
//                 QString data = apToCopy->writeObjectToXmlString();
//                 ap->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );
//             }
//         }
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAll()
{
    if ( plotWidget() )
    {
        updatePlotTitle();
        plotWidget()->updateLegend();
        updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateLegend()
{
    if ( plotWidget() )
    {
        if ( m_showPlotLegends && !isSubPlot() )
        {
            plotWidget()->insertLegend( m_legendPosition );
        }
        else
        {
            plotWidget()->clearLegend();
        }

        // for ( auto c : histogramCurves() )
        // {
        //     c->updateLegendEntryVisibilityNoPlotUpdate();
        // }
    }

    reattachAllCurves();
    if ( plotWidget() )
    {
        plotWidget()->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setLegendPosition( RiuPlotWidget::Legend position )
{
    m_legendPosition = position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// size_t RimHistogramPlot::singleColorCurveCount() const
// {
//     auto allCurveSets = ensembleCurveSetCollection()->curveSets();

//     size_t colorIndex =
//         std::count_if( allCurveSets.begin(),
//                        allCurveSets.end(),
//                        []( RimEnsembleCurveSet* curveSet )
//                        { return RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ); } );

//     colorIndex += curveCount();

//     return colorIndex;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::applyDefaultCurveAppearances()
{
    // applyDefaultCurveAppearances( histogramCurves() );
    // applyDefaultCurveAppearances( ensembleCurveSetCollection()->curveSets() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::applyDefaultCurveAppearances( std::vector<RimHistogramCurve*> curvesToUpdate )
// {
//     // std::set<RiaHistogramCurveDefinition> allCurveDefs = histogramAndEnsembleCurveDefinitions();
//     // RimHistogramCurveAppearanceCalculator curveLookCalc( allCurveDefs );

//     // for ( auto& curve : curvesToUpdate )
//     // {
//     //     curve->resetAppearance();
//     //     curveLookCalc.setupCurveLook( curve );
//     // }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::applyDefaultCurveAppearances( std::vector<RimEnsembleCurveSet*> ensembleCurvesToUpdate )
// {
//     // std::vector<QColor> usedColors;
//     // for ( auto c : ensembleCurveSetCollection()->curveSets() )
//     // {
//     //     // ensembleCurvesToUpdate can be present in the ensembleCurveSetCollection()->curveSets() vector, exclude this from used
//     //     // colors
//     //     if ( std::find( ensembleCurvesToUpdate.begin(), ensembleCurvesToUpdate.end(), c ) == ensembleCurvesToUpdate.end() )
//     //     {
//     //         usedColors.push_back( c->mainEnsembleColor() );
//     //     }
//     // }

//     for ( auto curveSet : ensembleCurvesToUpdate )
//     {
//         cvf::Color3f curveColor = cvf::Color3f::ORANGE;

//         // const auto adr = curveSet->histogramAddressY();
//         // if ( adr.isHistoryVector() )
//         // {
//         //     curveColor = RiaPreferencesHistogram::current()->historyCurveContrastColor();
//         // }
//         // else
//         // {
//         //     if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ) )
//         //     {
//         //         std::vector<QColor> candidateColors;
//         //         if ( RiaPreferencesHistogram::current()->colorCurvesByPhase() )
//         //         {
//         //             // Put the the phase color as first candidate, will then be used if there is only one ensemble in the plot
//         //             candidateColors.push_back( RiaColorTools::toQColor( RimHistogramCurveAppearanceCalculator::assignColorByPhase( adr
//         )
//         //             ) );
//         //         }

//         //         auto histogramColors = RiaColorTables::histogramCurveDefaultPaletteColors();
//         //         for ( int i = 0; i < static_cast<int>( histogramColors.size() ); i++ )
//         //         {
//         //             candidateColors.push_back( histogramColors.cycledQColor( i ) );
//         //         }

//         //         for ( const auto& candidateCol : candidateColors )
//         //         {
//         //             if ( std::find( usedColors.begin(), usedColors.end(), candidateCol ) == usedColors.end() )
//         //             {
//         //                 curveColor = RiaColorTools::fromQColorTo3f( candidateCol );
//         //                 usedColors.push_back( candidateCol );
//         //                 break;
//         //             }
//         //         }
//         //     }
//         // }

//         curveSet->setColor( curveColor );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setNormalizationEnabled( bool enable )
{
    m_normalizeCurveYValues = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isNormalizationEnabled()
{
    return m_normalizeCurveYValues();
}

//--------------------------------------------------------------------------------------------------
///
/// Update regular axis with numerical axis values - i.e. not time axis.
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateNumericalAxis( RiaDefines::PlotAxis plotAxis )
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        RiuPlotAxis riuPlotAxis = axisProperties->plotAxis();
        if ( riuPlotAxis.axis() == plotAxis )
        {
            auto* axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
            if ( !axisProps ) continue;

            // if ( axisProperties->isActive() && hasVisibleCurvesForAxis( riuPlotAxis ) )
            // {
            //     plotWidget()->enableAxis( riuPlotAxis, true );
            // }
            // else
            // {
            //     plotWidget()->enableAxis( riuPlotAxis, false );
            // }

            // if ( !hasVisibleCurvesForAxis( riuPlotAxis ) )
            // {
            //     axisProps->setNameForUnusedAxis();
            // }
            // else
            // {
            //     // std::set<QString> timeHistoryQuantities;

            //     // RimHistogramPlotAxisFormatter calc( axisProps, {}, curveDefs, visibleAsciiDataCurvesForAxis( riuPlotAxis ),
            //     // timeHistoryQuantities );

            //     // calc.applyAxisPropertiesToPlot( plotWidget() );
            // }

            plotWidget()->enableAxisNumberLabels( riuPlotAxis, axisProps->showNumbers() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateZoomForAxis( RimPlotAxisPropertiesInterface* axisProperties )
{
    if ( auto axisProps = dynamic_cast<RimPlotAxisProperties*>( axisProperties ) )
    {
        updateZoomForNumericalAxis( axisProps );
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateZoomForNumericalAxis( RimPlotAxisProperties* axisProperties )
{
    // if ( !axisProperties ) return;

    // const auto plotAxis = axisProperties->plotAxis();
    // if ( axisProperties->isAutoZoom() )
    // {
    //     if ( axisProperties->isLogarithmicScaleEnabled() )
    //     {
    //         plotWidget()->setAxisScaleType( plotAxis, RiuQwtPlotWidget::AxisScaleType::LOGARITHMIC );
    //         std::vector<const RimPlotCurve*> plotCurves;

    //         for ( RimHistogramCurve* c : visibleHistogramCurvesForAxis( plotAxis ) )
    //         {
    //             plotCurves.push_back( c );
    //         }

    //         double                        min, max;
    //         RimPlotAxisLogRangeCalculator calc( plotAxis.axis(), plotCurves );
    //         calc.computeAxisRange( &min, &max );

    //         if ( axisProperties->isAxisInverted() )
    //         {
    //             std::swap( min, max );
    //         }

    //         plotWidget()->setAxisScale( axisProperties->plotAxis(), min, max );
    //     }
    //     else if ( ( plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || plotAxis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
    //     &&
    //               isOnlyWaterCutCurvesVisible( plotAxis ) )
    //     {
    //         plotWidget()->setAxisScale( axisProperties->plotAxis(), 0.0, 1.0 );
    //     }
    //     else
    //     {
    //         plotWidget()->setAxisAutoScale( axisProperties->plotAxis(), true );
    //     }
    // }
    // else
    // {
    //     double min = axisProperties->visibleRangeMin();
    //     double max = axisProperties->visibleRangeMax();
    //     if ( axisProperties->isAxisInverted() ) std::swap( min, max );
    //     plotWidget()->setAxisScale( axisProperties->plotAxis(), min, max );
    // }

    // plotWidget()->setAxisInverted( axisProperties->plotAxis(), axisProperties->isAxisInverted() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::enableCurvePointTracking( bool enable )
{
    // m_histogramPlot->enableCurvePointTracking( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RiuPlotAxis RimHistogramPlot::plotAxisForTime()
// {
//     return RiuPlotAxis::defaultBottom();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::findOrAssignPlotAxisX( RimHistogramCurve* curve )
// {
//     for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
//     {
//         if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
//         {
//             auto propertyAxis = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
//             if ( propertyAxis )
//             {
//                 curve->setTopOrBottomAxisX( propertyAxis->plotAxis() );

//                 return;
//             }
//         }
//     }

//     if ( curve->histogramCaseX() != nullptr )
//     {
//         if ( !plotWidget() )
//         {
//             // Assign a default bottom axis if no plot widget is present. This can happens during project load and transformation to new
//             // cross plot structure in RimMainPlotCollection::initAfterRead()

//             QString axisObjectName = "New Axis";
//             if ( !curve->histogramAddressX().uiText().empty() )
//                 axisObjectName = QString::fromStdString( curve->histogramAddressX().uiText() );

//             RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultBottomForHistogramVectors();
//             addNewAxisProperties( newPlotAxis, axisObjectName );

//             curve->setTopOrBottomAxisX( newPlotAxis );

//             return;
//         }

//         if ( plotWidget()->isMultiAxisSupported() )
//         {
//             QString axisObjectName = "New Axis";
//             if ( !curve->histogramAddressX().uiText().empty() )
//                 axisObjectName = QString::fromStdString( curve->histogramAddressX().uiText() );

//             RiuPlotAxis newPlotAxis = plotWidget()->createNextPlotAxis( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
//             addNewAxisProperties( newPlotAxis, axisObjectName );
//             if ( plotWidget() )
//             {
//                 plotWidget()->ensureAxisIsCreated( newPlotAxis );
//             }

//             updateAxes();
//             curve->setTopOrBottomAxisX( newPlotAxis );
//         }
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::scheduleReplotIfVisible()
{
    if ( showWindow() && plotWidget() ) plotWidget()->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// std::vector<RimHistogramCurve*> RimHistogramPlot::visibleHistogramCurvesForAxis( RiuPlotAxis plotAxis ) const
// {
//     std::vector<RimHistogramCurve*> curves;

//     if ( m_histogramCurveCollection && m_histogramCurveCollection->isCurvesVisible() )
//     {
//         for ( RimHistogramCurve* curve : m_histogramCurveCollection->curves() )
//         {
//             if ( curve->isChecked() && ( curve->axisY() == plotAxis || curve->axisX() == plotAxis ) )
//             {
//                 curves.push_back( curve );
//             }
//         }
//     }

//     // if ( m_ensembleCurveSetCollection && m_ensembleCurveSetCollection->isCurveSetsVisible() )
//     // {
//     //     for ( RimEnsembleCurveSet* curveSet : m_ensembleCurveSetCollection->curveSets() )
//     //     {
//     //         for ( RimHistogramCurve* curve : curveSet->curves() )
//     //         {
//     //             if ( curve->isChecked() && ( curve->axisY() == plotAxis || curve->axisX() == plotAxis ) )
//     //             {
//     //                 curves.push_back( curve );
//     //             }
//     //         }
//     //     }
//     // }

//     return curves;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// bool RimHistogramPlot::hasVisibleCurvesForAxis( RiuPlotAxis plotAxis ) const
// {
//     if ( !visibleHistogramCurvesForAxis( plotAxis ).empty() )
//     {
//         return true;
//     }

//     if ( !visibleTimeHistoryCurvesForAxis( plotAxis ).empty() )
//     {
//         return true;
//     }

//     if ( !visibleAsciiDataCurvesForAxis( plotAxis ).empty() )
//     {
//         return true;
//     }

//     return false;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisPropertiesInterface* RimHistogramPlot::axisPropertiesForPlotAxis( RiuPlotAxis plotAxis ) const
{
    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( axisProperties->plotAxis() == plotAxis ) return axisProperties;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateCaseNameHasChanged()
{
    // if ( m_histogramCurveCollection )
    // {
    //     m_histogramCurveCollection->updateCaseNameHasChanged();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );
    updatePlotWidgetFromAxisRanges();

    axisChanged.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::addCurveAndUpdate( RimHistogramCurve* curve, bool autoAssignPlotAxis )
// {
//     if ( curve )
//     {
//         m_histogramCurveCollection->addCurve( curve );
//         connectCurveToPlot( curve, true, autoAssignPlotAxis );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::addCurveNoUpdate( RimHistogramCurve* curve, bool autoAssignPlotAxis )
{
    if ( curve )
    {
        m_histogramCurveCollection->addCurve( curve );
        connectCurveToPlot( curve, false, autoAssignPlotAxis );
    }
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::insertCurve( RimHistogramCurve* curve, size_t insertAtPosition )
// {
//     if ( curve )
//     {
//         m_histogramCurveCollection->insertCurve( curve, insertAtPosition );
//         connectCurveToPlot( curve, false, true );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::connectCurveToPlot( RimHistogramCurve* curve, bool update, bool autoAssignPlotAxis )
{
    // if ( autoAssignPlotAxis ) assignPlotAxis( curve );

    connectCurveSignals( curve );
    if ( plotWidget() )
    {
        plotWidget()->ensureAxisIsCreated( curve->axisY() );

        if ( update )
        {
            curve->setParentPlotAndReplot( plotWidget() );
            updateAxes();
        }
        else
        {
            curve->setParentPlotNoReplot( plotWidget() );
        }
    }
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::deleteCurves()
// {
//     // for ( const auto curve : curves )
//     // {
//     //     if ( m_histogramCurveCollection )
//     //     {
//     //         for ( auto& c : m_histogramCurveCollection->curves() )
//     //         {
//     //             if ( c == curve )
//     //             {
//     //                 disconnectCurveSignals( curve );
//     //                 m_histogramCurveCollection->deleteCurve( curve );
//     //                 continue;
//     //             }
//     //         }
//     //     }
//     //     if ( m_ensembleCurveSetCollection )
//     //     {
//     //         for ( auto& curveSet : m_ensembleCurveSetCollection->curveSets() )
//     //         {
//     //             for ( auto& c : curveSet->curves() )
//     //             {
//     //                 if ( c == curve )
//     //                 {
//     //                     curveSet->deleteCurve( curve );
//     //                     if ( curveSet->curves().empty() )
//     //                     {
//     //                         if ( curveSet->colorMode() == RimEnsembleCurveSet::ColorMode::BY_ENSEMBLE_PARAM && plotWidget() &&
//     //                              curveSet->legendFrame() )
//     //                         {
//     //                             plotWidget()->removeOverlayFrame( curveSet->legendFrame() );
//     //                         }
//     //                         m_ensembleCurveSetCollection->deleteCurveSet( curveSet );
//     //                     }
//     //                     continue;
//     //                 }
//     //             }
//     //         }
//     //     }
//     // }

//     RiuPlotMainWindowTools::refreshToolbars();

//     updateCaseNameHasChanged();

//     curvesChanged.send();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimHistogramPlot::userDescriptionField()
{
    if ( m_description().isEmpty() )
    {
        return &m_fallbackPlotName;
    }
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::zoomAllForMultiPlot()
{
    if ( auto multiPlot = firstAncestorOrThisOfType<RimMultiPlot>() )
    {
        multiPlot->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_description )
    {
        m_useAutoPlotTitle = false;
    }

    if ( changedField == &m_useAutoPlotTitle || changedField == &m_description )
    {
        autoTitleChanged.send( m_useAutoPlotTitle() );
    }

    if ( changedField == &m_showPlotTitle || changedField == &m_description || changedField == &m_useAutoPlotTitle )
    {
        updatePlotTitle();
        updateConnectedEditors();

        if ( !m_useAutoPlotTitle )
        {
            // When auto name of plot is turned off, update the auto name for all curves

            // for ( auto c : histogramCurves() )
            // {
            //     c->updateCurveNameNoLegendUpdate();
            // }
        }

        titleChanged.send();
    }

    if ( changedField == &m_showPlotLegends ) updateLegend();

    if ( changedField == &m_normalizeCurveYValues )
    {
        loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateStackedCurveData()
{
    auto anyStackedCurvesPresent = true; // updateStackedCurveDataForRelevantAxes();

    if ( plotWidget() && anyStackedCurvesPresent )
    {
        reattachAllCurves();
        scheduleReplotIfVisible();
    }
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// bool RimHistogramPlot::updateStackedCurveDataForRelevantAxes()
// {
//     bool anyStackedCurvesPresent = false;
//     for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
//     {
//         if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
//              axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
//         {
//             anyStackedCurvesPresent |= updateStackedCurveDataForAxis( axisProperties->plotAxis() );
//         }
//     }

//     return anyStackedCurvesPresent;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimHistogramPlot::snapshotWindowContent()
{
    QImage image;

    if ( plotWidget() )
    {
        QPixmap pix = plotWidget()->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( !m_isValid ) return;

    // bool isPlotEditor = ( uiConfigName == RicHistogramPlotEditorUi::CONFIGURATION_NAME );

    // if ( !isPlotEditor ) uiTreeOrdering.add( &m_axisPropertiesArray );

    for ( auto& curve : m_histogramCurveCollection->curves() )
    {
        uiTreeOrdering.add( curve );
    }

    // for ( auto& curveSet : m_ensembleCurveSetCollection->curveSets() )
    // {
    //     uiTreeOrdering.add( curveSet );
    // }

    // if ( !isPlotEditor )
    // {
    //     uiTreeOrdering.add( &m_gridTimeHistoryCurves );
    //     uiTreeOrdering.add( &m_asciiDataCurves );
    // }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onLoadDataAndUpdate()
{
    updatePlotTitle();

    auto plotWindow = firstAncestorOrThisOfType<RimMultiPlot>();
    if ( plotWindow == nullptr ) updateMdiWindowVisibility();

    if ( m_histogramCurveCollection )
    {
        m_histogramCurveCollection->loadDataAndUpdate( false );
    }

    // m_ensembleCurveSetCollection->loadDataAndUpdate( false );

    // for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    // {
    //     curve->loadDataAndUpdate( false );
    // }

    // for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    // {
    //     curve->loadDataAndUpdate( false );
    // }

    // // Load data for regression curves, as they depend on data loaded by curves updated previously in this function
    // if ( m_histogramCurveCollection )
    // {
    //     auto curves = m_histogramCurveCollection->curves();
    //     for ( auto c : curves )
    //     {
    //         if ( c->isRegressionCurve() )
    //         {
    //             c->loadDataAndUpdate( false );
    //         }
    //     }
    // }

    if ( plotWidget() )
    {
        if ( m_showPlotLegends && !isSubPlot() )
        {
            plotWidget()->insertLegend( m_legendPosition );
        }
        else
        {
            plotWidget()->clearLegend();
        }
        plotWidget()->setLegendFontSize( legendFontSize() );
        plotWidget()->updateLegend();
    }
    updateAxes();

    updateStackedCurveData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updatePlotWidgetFromAxisRanges()
{
    if ( !plotWidget() ) return;

    for ( const auto& axisProperty : m_axisPropertiesArray )
    {
        updateZoomForAxis( axisProperty );
    }

    plotWidget()->updateAxes();
    updateAxisRangesFromPlotWidget();
    plotWidget()->updateZoomDependentCurveProperties();

    // // Must create and set new custom tickmarks for time axis after zoom update
    // auto* timeAxisProps = timeAxisProperties();
    // if ( timeAxisProps && timeAxisProps->tickmarkType() == RimHistogramTimeAxisProperties::TickmarkType::TICKMARK_CUSTOM )
    // {
    //     // Protection against too many custom tickmarks
    //     const bool showErrorMessageBox = false;
    //     overrideTimeAxisSettingsIfTooManyCustomTickmarks( timeAxisProps, showErrorMessageBox );

    //     // Create and set tickmarks based on settings
    //     createAndSetCustomTimeAxisTickmarks( timeAxisProps );
    // }

    scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateAxisRangesFromPlotWidget()
{
    if ( !plotWidget() ) return;

    for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
    {
        if ( !axisProperties ) continue;

        auto [axisMin, axisMax] = plotWidget()->axisRange( axisProperties->plotAxis() );
        if ( axisProperties->isAxisInverted() ) std::swap( axisMin, axisMax );

        if ( auto propertyAxis = dynamic_cast<RimPlotAxisProperties*>( axisProperties ) )
        {
            propertyAxis->setAutoValueVisibleRangeMax( axisMax );
            propertyAxis->setAutoValueVisibleRangeMin( axisMin );
        }

        axisProperties->setVisibleRangeMax( axisMax );
        axisProperties->setVisibleRangeMin( axisMin );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::deletePlotCurvesAndPlotWidget()
{
    if ( isDeletable() )
    {
        detachAllPlotItems();

        if ( plotWidget() )
        {
            plotWidget()->setParent( nullptr );
        }

        deleteAllPlotCurves();

        if ( m_histogramPlot )
        {
            m_histogramPlot.reset();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::connectCurveSignals( RimStackablePlotCurve* curve )
{
    curve->dataChanged.connect( this, &RimHistogramPlot::curveDataChanged );
    curve->visibilityChanged.connect( this, &RimHistogramPlot::curveVisibilityChanged );
    curve->appearanceChanged.connect( this, &RimHistogramPlot::curveAppearanceChanged );
    curve->stackingChanged.connect( this, &RimHistogramPlot::curveStackingChanged );
    curve->stackingColorsChanged.connect( this, &RimHistogramPlot::curveStackingColorsChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::disconnectCurveSignals( RimStackablePlotCurve* curve )
{
    curve->dataChanged.disconnect( this );
    curve->visibilityChanged.disconnect( this );
    curve->appearanceChanged.disconnect( this );
    curve->stackingChanged.disconnect( this );
    curve->stackingColorsChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveDataChanged( const caf::SignalEmitter* emitter )
{
    loadDataAndUpdate();

    curvesChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveVisibilityChanged( const caf::SignalEmitter* emitter, bool visible )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveAppearanceChanged( const caf::SignalEmitter* emitter )
{
    scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveStackingChanged( const caf::SignalEmitter* emitter, bool stacked )
{
    loadDataAndUpdate();

    // Change of stacking can result in very large y-axis changes, so zoom all
    zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::curveStackingColorsChanged( const caf::SignalEmitter* emitter, bool stackWithPhaseColors )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimHistogramPlot::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimHistogramPlot::axisLogarithmicChanged );
    axis->axisPositionChanged.connect( this, &RimHistogramPlot::axisPositionChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    axisChanged.send( this );
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    axisChanged.send( this );
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimHistogramPlot::addNewAxisProperties( RiaDefines::PlotAxis plotAxis, const QString& name )
{
    RiuPlotAxis newPlotAxis = plotWidget()->createNextPlotAxis( plotAxis );
    return addNewAxisProperties( newPlotAxis, name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotAxisProperties* RimHistogramPlot::addNewAxisProperties( RiuPlotAxis plotAxis, const QString& name )
{
    auto* axisProperties = new RimPlotAxisProperties;
    axisProperties->enableAutoValueForAllFields( true );
    axisProperties->setNameAndAxis( name, name, plotAxis.axis(), plotAxis.index() );
    m_axisPropertiesArray.push_back( axisProperties );
    connectAxisSignals( axisProperties );

    return axisProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RimHistogramPlot::visibleCurvesForLegend()
{
    std::vector<RimPlotCurve*> curves;

    // for ( auto c : histogramCurves() )
    // {
    //     if ( !c->isChecked() ) continue;
    //     if ( !c->showInLegend() ) continue;
    //     curves.push_back( c );
    // }

    // for ( auto curveSet : curveSets() )
    // {
    //     if ( !curveSet->isCurvesVisible() ) continue;
    //     if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( curveSet->colorMode() ) )
    //     {
    //         auto curveSetCurves = curveSet->curves();

    //         if ( !curveSetCurves.empty() ) curves.push_back( curveSetCurves.front() );
    //     }
    // }

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::axisPositionChanged( const caf::SignalEmitter* emitter,
                                            RimPlotAxisProperties*    axisProperties,
                                            RiuPlotAxis               oldPlotAxis,
                                            RiuPlotAxis               newPlotAxis )
{
    if ( !axisProperties ) return;

    if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
    {
        // Make sure the new axis on the correct side exists.
        RiuPlotAxis fixedUpPlotAxis = plotWidget()->createNextPlotAxis( newPlotAxis.axis() );
        // The index can change so need to update.
        axisProperties->setNameAndAxis( axisProperties->objectName(),
                                        axisProperties->axisTitleText(),
                                        fixedUpPlotAxis.axis(),
                                        fixedUpPlotAxis.index() );

        // // Move all attached curves
        // for ( auto curve : histogramCurves() )
        // {
        //     if ( curve->axisY() == oldPlotAxis ) curve->setLeftOrRightAxisY( fixedUpPlotAxis );
        // }

        // for ( auto curveSet : ensembleCurveSetCollection()->curveSets() )
        // {
        //     if ( curveSet->axisY() == oldPlotAxis ) curveSet->setLeftOrRightAxisY( fixedUpPlotAxis );
        // }

        // Remove the now unused axis (but keep the default axis)
        if ( oldPlotAxis != RiuPlotAxis::defaultLeft() && oldPlotAxis != RiuPlotAxis::defaultRight() )
        {
            auto oldAxisProperties = axisPropertiesForPlotAxis( oldPlotAxis );
            if ( oldAxisProperties ) m_axisPropertiesArray.removeChild( oldAxisProperties );
            plotWidget()->moveAxis( oldPlotAxis, newPlotAxis );
        }

        updateAxes();
    }

    // This is probably to much, but difficult to find the required updates
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::enableAutoPlotTitle( bool enable )
{
    m_useAutoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::autoPlotTitle() const
{
    return m_useAutoPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimHistogramPlot::CurveInfo RimHistogramPlot::handleHistogramCaseDrop( RimHistogramCase* histogramCase )
// {
//     std::map<std::pair<RifEclipseHistogramAddress, RifEclipseHistogramAddress>, std::set<RimHistogramCase*>> dataVectorMap;

//     for ( auto& curve : histogramCurves() )
//     {
//         const auto addr  = curve->histogramAddressY();
//         const auto addrX = curve->histogramAddressX();

//         // NB! This concept is used to make it possible to avoid adding curves for a case that is already present
//         // To be complete, the histogramCaseX() should also be checked, but this is not done for now
//         dataVectorMap[std::make_pair( addr, addrX )].insert( curve->histogramCaseY() );
//     }

//     std::vector<RimHistogramCurve*> curves;

//     for ( const auto& [addressPair, cases] : dataVectorMap )
//     {
//         if ( cases.count( histogramCase ) > 0 ) continue;

//         const auto& [addrY, addrX] = addressPair;

//         curves.push_back( addNewCurve( addrY, histogramCase, addrX, histogramCase ) );
//     }

//     return { .curveCount = static_cast<int>( curves.size() ), .curves = curves, .curveSets = {} };
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RimHistogramPlot::CurveInfo RimHistogramPlot::handleEnsembleDrop( RimHistogramEnsemble* ensemble )
// {
//     int                               newCurves = 0;
//     std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

//     std::map<RiaHistogramCurveAddress, std::set<RimHistogramEnsemble*>> dataVectorMap;

//     for ( auto& curve : curveSets() )
//     {
//         const auto addr = curve->curveAddress();
//         dataVectorMap[addr].insert( curve->histogramEnsemble() );
//     }

//     for ( const auto& [addr, ensembles] : dataVectorMap )
//     {
//         if ( ensembles.count( ensemble ) > 0 ) continue;

//         auto curveSet = RiaHistogramPlotTools::addNewEnsembleCurve( this, addr, ensemble );
//         curveSetsToUpdate.push_back( curveSet );
//         newCurves++;
//     }

//     return { newCurves, {}, curveSetsToUpdate };
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RimHistogramPlot::CurveInfo RimHistogramPlot::handleAddressCollectionDrop( RimHistogramAddressCollection* addressCollection )
// {
//     int                               newCurves = 0;
//     std::vector<RimHistogramCurve*>   curves;
//     std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

//     auto droppedName = addressCollection->name().toStdString();

//     auto histogramCase = RiaHistogramTools::histogramCaseById( addressCollection->caseId() );
//     auto ensembleCase  = RiaHistogramTools::ensembleById( addressCollection->ensembleId() );

//     std::vector<RiaHistogramCurveDefinition>                     sourceCurveDefs;
//     std::map<RiaHistogramCurveDefinition, std::set<std::string>> newCurveDefsWithObjectNames;

//     if ( histogramCase && !ensembleCase )
//     {
//         for ( auto& curve : histogramCurves() )
//         {
//             sourceCurveDefs.push_back( curve->curveDefinition() );
//         }
//     }

//     if ( ensembleCase )
//     {
//         auto curveSets = m_ensembleCurveSetCollection->curveSets();
//         for ( auto curveSet : curveSets )
//         {
//             sourceCurveDefs.emplace_back( ensembleCase, curveSet->curveAddress() );
//         }
//     }

//     for ( auto& curveDef : sourceCurveDefs )
//     {
//         auto       newCurveDef = curveDef;
//         const auto curveAdr    = newCurveDef.histogramAddressY();

//         std::string objectIdentifierString;
//         if ( ( curveAdr.category() == RifEclipseHistogramAddressDefines::HistogramCategory::HISTOGRAM_WELL ) &&
//              ( addressCollection->contentType() == RimHistogramAddressCollection::CollectionContentType::WELL ) )
//         {
//             objectIdentifierString = curveAdr.wellName();
//         }
//         else if ( ( curveAdr.category() == RifEclipseHistogramAddressDefines::HistogramCategory::HISTOGRAM_GROUP ) &&
//                   ( addressCollection->contentType() == RimHistogramAddressCollection::CollectionContentType::GROUP ) )
//         {
//             objectIdentifierString = curveAdr.groupName();
//         }
//         else if ( ( curveAdr.category() == RifEclipseHistogramAddressDefines::HistogramCategory::HISTOGRAM_NETWORK ) &&
//                   ( addressCollection->contentType() == RimHistogramAddressCollection::CollectionContentType::NETWORK ) )
//         {
//             objectIdentifierString = curveAdr.networkName();
//         }
//         else if ( ( curveAdr.category() == RifEclipseHistogramAddressDefines::HistogramCategory::HISTOGRAM_REGION ) &&
//                   ( addressCollection->contentType() == RimHistogramAddressCollection::CollectionContentType::REGION ) )
//         {
//             objectIdentifierString = std::to_string( curveAdr.regionNumber() );
//         }
//         else if ( ( curveAdr.category() == RifEclipseHistogramAddressDefines::HistogramCategory::HISTOGRAM_WELL_SEGMENT ) &&
//                   ( addressCollection->contentType() == RimHistogramAddressCollection::CollectionContentType::WELL_SEGMENT ) )
//         {
//             objectIdentifierString = std::to_string( curveAdr.wellSegmentNumber() );
//         }

//         if ( !objectIdentifierString.empty() )
//         {
//             newCurveDef.setIdentifierText( curveAdr.category(), droppedName );

//             newCurveDefsWithObjectNames[newCurveDef].insert( objectIdentifierString );
//             const auto& addr = curveDef.histogramAddressY();
//             if ( !addr.isHistoryVector() && RiaPreferencesHistogram::current()->appendHistoryVectors() )
//             {
//                 auto historyAddr = addr;
//                 historyAddr.setVectorName( addr.vectorName() + RifEclipseHistogramAddressDefines::historyIdentifier() );

//                 auto historyCurveDef = newCurveDef;
//                 historyCurveDef.setHistogramAddressY( historyAddr );
//                 newCurveDefsWithObjectNames[historyCurveDef].insert( objectIdentifierString );
//             }
//         }
//     }

//     for ( auto& [curveDef, objectNames] : newCurveDefsWithObjectNames )
//     {
//         // Skip adding new curves if the object name is already present for the curve definition
//         if ( objectNames.count( droppedName ) > 0 ) continue;

//         if ( curveDef.ensemble() )
//         {
//             auto addresses = curveDef.ensemble()->ensembleHistogramAddresses();
//             if ( addresses.find( curveDef.histogramAddressY() ) != addresses.end() )
//             {
//                 auto curveSet = RiaHistogramPlotTools::addNewEnsembleCurve( this, curveDef.histogramCurveAddress(), curveDef.ensemble()
//                 ); curveSetsToUpdate.push_back( curveSet ); newCurves++;
//             }
//         }
//         else if ( curveDef.histogramCaseY() )
//         {
//             if ( curveDef.histogramCaseY()->histogramReader() &&
//                  curveDef.histogramCaseY()->histogramReader()->hasAddress( curveDef.histogramAddressY() ) )
//             {
//                 auto curve = addNewCurve( curveDef.histogramAddressY(),
//                                           curveDef.histogramCaseY(),
//                                           curveDef.histogramAddressX(),
//                                           curveDef.histogramCaseX() );
//                 curves.push_back( curve );
//                 if ( curveDef.histogramCaseX() )
//                 {
//                     curve->setAxisTypeX( RiaDefines::HorizontalAxisType::HISTOGRAM_VECTOR );
//                     curve->setHistogramCaseX( curveDef.histogramCaseX() );
//                     curve->setHistogramAddressX( curveDef.histogramAddressX() );
//                     findOrAssignPlotAxisX( curve );
//                 }
//                 newCurves++;
//             }
//         }
//     }

//     return { newCurves, curves, curveSetsToUpdate };
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimHistogramPlot::CurveInfo RimHistogramPlot::handleHistogramAddressDrop( RimHistogramAddress* histogramAddr )
// {
//     int                               newCurves = 0;
//     std::vector<RimHistogramCurve*>   curves;
//     std::vector<RimEnsembleCurveSet*> curveSetsToUpdate;

//     std::vector<RifEclipseHistogramAddress> newCurveAddresses;
//     newCurveAddresses.push_back( histogramAddr->address() );
//     if ( !histogramAddr->address().isHistoryVector() && RiaPreferencesHistogram::current()->appendHistoryVectors() )
//     {
//         auto historyAddr = histogramAddr->address();
//         historyAddr.setVectorName( histogramAddr->address().vectorName() + RifEclipseHistogramAddressDefines::historyIdentifier() );
//         newCurveAddresses.push_back( historyAddr );
//     }

//     if ( histogramAddr->isEnsemble() )
//     {
//         std::map<RifEclipseHistogramAddress, std::set<RimHistogramEnsemble*>> dataVectorMap;

//         for ( auto& curve : curveSets() )
//         {
//             const auto addr = curve->histogramAddressY();
//             dataVectorMap[addr].insert( curve->histogramEnsemble() );
//         }

//         auto ensemble = RiaHistogramTools::ensembleById( histogramAddr->ensembleId() );
//         if ( ensemble )
//         {
//             for ( const auto& droppedAddress : newCurveAddresses )
//             {
//                 auto addresses = ensemble->ensembleHistogramAddresses();
//                 if ( addresses.find( droppedAddress ) == addresses.end() ) continue;

//                 bool skipAddress = false;
//                 if ( dataVectorMap.count( droppedAddress ) > 0 )
//                 {
//                     skipAddress = ( dataVectorMap[droppedAddress].count( ensemble ) > 0 );
//                 }

//                 if ( !skipAddress )
//                 {
//                     auto curveSet =
//                         RiaHistogramPlotTools::addNewEnsembleCurve( this,
//                                                                     RiaHistogramCurveAddress( RifEclipseHistogramAddress::timeAddress(),
//                                                                                               droppedAddress ),
//                                                                     ensemble );

//                     curveSetsToUpdate.push_back( curveSet );
//                     newCurves++;
//                 }
//             }
//         }
//     }
//     else
//     {
//         std::map<RifEclipseHistogramAddress, std::set<RimHistogramCase*>> dataVectorMap;

//         for ( auto& curve : histogramCurves() )
//         {
//             const auto addr = curve->histogramAddressY();
//             dataVectorMap[addr].insert( curve->histogramCaseY() );
//         }

//         auto histogramCase = RiaHistogramTools::histogramCaseById( histogramAddr->caseId() );
//         if ( histogramCase )
//         {
//             for ( const auto& droppedAddress : newCurveAddresses )
//             {
//                 if ( !histogramCase->histogramReader() || !histogramCase->histogramReader()->hasAddress( droppedAddress ) ) continue;

//                 bool skipAddress = false;

//                 if ( dataVectorMap.count( droppedAddress ) > 0 )
//                 {
//                     skipAddress = ( dataVectorMap[droppedAddress].count( histogramCase ) > 0 );
//                 }

//                 if ( !skipAddress )
//                 {
//                     curves.push_back( addNewCurve( droppedAddress, histogramCase, RifEclipseHistogramAddress::timeAddress(), nullptr ) );
//                     newCurves++;
//                 }
//             }
//         }
//     }
//     return { newCurves, curves, curveSetsToUpdate };
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::handleDroppedObjects( const std::vector<caf::PdmObjectHandle*>& objects )
// {
//     CurveInfo curveInfo;
//     for ( auto obj : objects )
//     {
//         if ( auto histogramCase = dynamic_cast<RimHistogramCase*>( obj ) )
//         {
//             curveInfo.appendCurveInfo( handleHistogramCaseDrop( histogramCase ) );
//         }
//         else if ( auto ensemble = dynamic_cast<RimHistogramEnsemble*>( obj ) )
//         {
//             curveInfo.appendCurveInfo( handleEnsembleDrop( ensemble ) );
//         }
//         else if ( auto histogramAddr = dynamic_cast<RimHistogramAddress*>( obj ) )
//         {
//             curveInfo.appendCurveInfo( handleHistogramAddressDrop( histogramAddr ) );
//         }

//         else if ( auto addressCollection = dynamic_cast<RimHistogramAddressCollection*>( obj ) )
//         {
//             if ( addressCollection->isFolder() )
//             {
//                 for ( auto coll : addressCollection->subFolders() )
//                 {
//                     auto localInfo = handleAddressCollectionDrop( coll );
//                     curveInfo.appendCurveInfo( localInfo );
//                 }
//             }
//             else
//             {
//                 curveInfo.appendCurveInfo( handleAddressCollectionDrop( addressCollection ) );
//             }
//         }
//     }

//     if ( curveInfo.curveCount > 0 )
//     {
//         applyDefaultCurveAppearances( curveInfo.curves );
//         applyDefaultCurveAppearances( curveInfo.curveSets );

//         loadDataAndUpdate();
//         zoomAll();

//         curvesChanged.send();
//     }

//     updateConnectedEditors();
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// RimHistogramCurve* RimHistogramPlot::addNewCurve( const RifEclipseHistogramAddress& address,
//                                                   RimHistogramCase*                 histogramCase,
//                                                   const RifEclipseHistogramAddress& addressX,
//                                                   RimHistogramCase*                 histogramCaseX )
// {
//     auto newCurve = RiaHistogramPlotTools::createCurve( histogramCase, address );

//     // This address is RifEclipseHistogramAddress::time() if the curve is a time plot. Otherwise it is the address of the histogram vector
//     // used for the x-axis
//     if ( addressX.category() != RifEclipseHistogramAddressDefines::HistogramCategory::HISTOGRAM_TIME )
//     {
//         newCurve->setAxisTypeX( RiaDefines::HorizontalAxisType::HISTOGRAM_VECTOR );
//         newCurve->setHistogramAddressX( addressX );
//         newCurve->setHistogramCaseX( histogramCaseX );
//     }

//     addCurveNoUpdate( newCurve );

//     return newCurve;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onPlotZoomed()
{
    // Disable auto scale in plot engine
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );

    // Disable auto value for min/max fields
    for ( auto p : plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
    {
        p->enableAutoValueMinMax( false );
    }

    updateAxisRangesFromPlotWidget();

    axisChanged.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // {
    //     auto group = uiOrdering.addNewGroup( "Data Source" );
    //     m_sourceStepping()->uiOrdering( uiConfigName, *group );
    // }

    caf::PdmUiGroup* mainOptions = uiOrdering.addNewGroup( "General Plot Options" );

    if ( isMdiWindow() )
    {
        mainOptions->add( &m_showPlotTitle );
        if ( m_showPlotTitle )
        {
            mainOptions->add( &m_useAutoPlotTitle );
            mainOptions->add( &m_description );
        }
    }
    else
    {
        mainOptions->add( &m_useAutoPlotTitle );
        mainOptions->add( &m_description );
        mainOptions->add( &m_colSpan );
    }

    mainOptions->add( &m_normalizeCurveYValues );

    if ( isMdiWindow() )
    {
        RimPlotWindow::uiOrderingForLegendsAndFonts( uiConfigName, uiOrdering );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimHistogramPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    if ( !plotWidget() )
    {
        m_histogramPlot = std::make_unique<RiuQwtPlotWidget>( this, mainWindowParent );

        QObject::connect( plotWidget(), SIGNAL( curveOrderNeedsUpdate() ), this, SLOT( onUpdateCurveOrder() ) );

        for ( const auto& axisProperties : m_axisPropertiesArray )
        {
            plotWidget()->ensureAxisIsCreated( axisProperties->plotAxis() );
        }

        // for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
        // {
        //     curve->setParentPlotNoReplot( plotWidget() );
        // }

        // for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
        // {
        //     curve->setParentPlotNoReplot( plotWidget() );
        // }

        if ( m_histogramCurveCollection )
        {
            m_histogramCurveCollection->setParentPlotNoReplot( plotWidget() );
        }

        // if ( m_ensembleCurveSetCollection )
        // {
        //     m_ensembleCurveSetCollection->setParentPlotNoReplot( plotWidget() );
        // }

        connect( plotWidget(), SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        updatePlotTitle();
    }

    plotWidget()->setParent( mainWindowParent );

    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::deleteViewWidget()
{
    deletePlotCurvesAndPlotWidget();
}

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::initAfterRead()
// {
//     RimViewWindow::initAfterRead();

//     if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2021.10.2" ) )
//     {
//         auto copyAxis = [this]( RiuPlotAxis axis, auto sourceObject )
//         {
//             auto axisProperties = axisPropertiesForPlotAxis( axis );
//             if ( axisProperties )
//             {
//                 QString data = sourceObject->writeObjectToXmlString();

//                 // This operation will overwrite the plot axis side, default is left
//                 axisProperties->readObjectFromXmlString( data, caf::PdmDefaultObjectFactory::instance() );

//                 auto plotAxisProperties = dynamic_cast<RimPlotAxisProperties*>( axisProperties );
//                 if ( plotAxisProperties )
//                 {
//                     // Reset the plot axis for the axis property
//                     plotAxisProperties->setNameAndAxis( axisProperties->objectName(), axisProperties->axisTitleText(), axis.axis(), 0 );
//                 }
//             }
//         };

//     }

//     for ( const auto& axisProperties : m_axisPropertiesArray )
//     {
//         auto plotAxisProperties = dynamic_cast<RimPlotAxisProperties*>( axisProperties.p() );
//         if ( plotAxisProperties )
//         {
//             connectAxisSignals( plotAxisProperties );
//         }
//         auto* timeAxis = dynamic_cast<RimHistogramTimeAxisProperties*>( axisProperties.p() );
//         if ( timeAxis )
//         {
//             timeAxis->settingsChanged.connect( this, &RimHistogramPlot::timeAxisSettingsChanged );
//             timeAxis->requestLoadDataAndUpdate.connect( this, &RimHistogramPlot::timeAxisSettingsChangedReloadRequired );
//         }
//     }

//     for ( auto curve : histogramCurves() )
//     {
//         connectCurveSignals( curve );
//     }

//     for ( auto curve : gridTimeHistoryCurves() )
//     {
//         connectCurveSignals( curve );
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::updateNameHelperWithCurveData( RimHistogramPlotNameHelper* nameHelper ) const
// {
//     if ( !nameHelper ) return;

//     nameHelper->clear();
//     std::vector<RiaHistogramCurveAddress> addresses;
//     std::vector<RimHistogramCase*>        sumCases;
//     std::vector<RimHistogramEnsemble*>    ensembleCases;

//     if ( m_histogramCurveCollection && m_histogramCurveCollection->isCurvesVisible() )
//     {
//         for ( RimHistogramCurve* curve : m_histogramCurveCollection->curves() )
//         {
//             addresses.push_back( curve->curveAddress() );
//             sumCases.push_back( curve->histogramCaseY() );

//             if ( curve->histogramCaseX() )
//             {
//                 sumCases.push_back( curve->histogramCaseX() );
//             }
//         }
//     }

//     for ( auto curveSet : m_ensembleCurveSetCollection->curveSets() )
//     {
//         addresses.push_back( curveSet->curveAddress() );
//         ensembleCases.push_back( curveSet->histogramEnsemble() );
//     }

//     nameHelper->clear();
//     nameHelper->appendAddresses( addresses );
//     nameHelper->setHistogramCases( sumCases );
//     nameHelper->setEnsembleCases( ensembleCases );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::doUpdateLayout()
{
    updateFonts();

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::detachAllPlotItems()
{
    // if ( m_histogramCurveCollection )
    // {
    //     m_histogramCurveCollection->detachPlotCurves();
    // }

    // if ( m_ensembleCurveSetCollection )
    // {
    //     m_ensembleCurveSetCollection->detachPlotCurves();
    // }

    // for ( RimGridTimeHistoryCurve* curve : m_gridTimeHistoryCurves )
    // {
    //     curve->detach();
    // }

    // for ( RimAsciiDataCurve* curve : m_asciiDataCurves )
    // {
    //     curve->detach();
    // }

    // m_plotInfoLabel->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::deleteAllPlotCurves()
{
    // for ( auto* c : histogramCurves() )
    // {
    //     c->deletePlotCurve();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::updateCurveNames()
{
    // if ( m_histogramCurveCollection->isCurvesVisible() )
    // {
    //     for ( auto c : histogramCurves() )
    //     {
    //         if ( c->isChecked() )
    //         {
    //             c->updateCurveNameNoLegendUpdate();
    //         }
    //     }
    // }

    // for ( auto curveSet : m_ensembleCurveSetCollection->curveSets() )
    // {
    //     curveSet->updateEnsembleLegendItem();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::detachAllCurves()
{
    detachAllPlotItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::reattachAllCurves()
{
    if ( m_histogramCurveCollection )
    {
        m_histogramCurveCollection->reattachPlotCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onCurveCollectionChanged( const SignalEmitter* emitter )
{
    curvesChanged.send();

    updateStackedCurveData();
    scheduleReplotIfVisible();

    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex )
{
    // auto wrapper = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
    // if ( !wrapper ) return;

    // auto qwtPlotItem = wrapper->qwtPlotItem();
    // if ( !qwtPlotItem ) return;

    // auto riuPlotCurve = dynamic_cast<RiuQwtPlotCurve*>( qwtPlotItem );
    // if ( !riuPlotCurve ) return;

    // auto rimPlotCurve = riuPlotCurve->ownerRimCurve();

    // RiuPlotMainWindowTools::selectOrToggleObject( rimPlotCurve, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setAutoScaleXEnabled( bool enabled )
{
    for ( const auto& ap : m_axisPropertiesArray )
    {
        if ( ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_TOP || ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
        {
            ap->setAutoZoom( enabled );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::setAutoScaleYEnabled( bool enabled )
{
    for ( const auto& ap : m_axisPropertiesArray )
    {
        if ( ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT || ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
        {
            ap->setAutoZoom( enabled );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimHistogramPlot::curveCount() const
{
    return m_histogramCurveCollection->curves().size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramPlot::isDeletable() const
{
    auto plotWindow = firstAncestorOrThisOfType<RimMultiPlot>();
    return plotWindow == nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisPropertiesInterface*> RimHistogramPlot::allPlotAxes() const
{
    return m_axisPropertiesArray.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotAxisProperties*> RimHistogramPlot::plotAxes( RimPlotAxisProperties::Orientation orientation ) const
{
    std::vector<RimPlotAxisProperties*> axisProps;
    for ( const auto& ap : m_axisPropertiesArray )
    {
        auto plotAxisProp = dynamic_cast<RimPlotAxisProperties*>( ap.p() );
        if ( !plotAxisProp ) continue;

        if ( ( orientation == RimPlotAxisProperties::Orientation::ANY ) ||
             ( orientation == RimPlotAxisProperties::Orientation::VERTICAL && plotAxisProp->plotAxis().isVertical() ) )
        {
            axisProps.push_back( plotAxisProp );
        }
        else if ( ( orientation == RimPlotAxisProperties::Orientation::ANY ) ||
                  ( orientation == RimPlotAxisProperties::Orientation::HORIZONTAL && plotAxisProp->plotAxis().isHorizontal() ) )
        {
            axisProps.push_back( plotAxisProp );
        }
    }

    return axisProps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::assignPlotAxis( RimHistogramCurve* destinationCurve )
// {
//     assignXPlotAxis( destinationCurve );
//     assignYPlotAxis( destinationCurve );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// auto countAxes = []( const std::vector<RimPlotAxisPropertiesInterface*>& axes, RiaDefines::PlotAxis axis )
// { return std::count_if( axes.begin(), axes.end(), [axis]( const auto& ap ) { return ap->plotAxis().axis() == axis; } ); };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::assignYPlotAxis( RimHistogramCurve* curve )
// {
// enum class AxisAssignmentStrategy
// {
//     ALTERNATING,
//     USE_MATCHING_UNIT,
//     USE_MATCHING_VECTOR
// };

// auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

// auto destinationUnit = RiaStdStringTools::toUpper( curve->unitNameY() );
// if ( destinationUnit.empty() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

// auto anyCurveWithUnitText = [this, curve]
// {
//     for ( auto c : histogramCurves() )
//     {
//         if ( c == curve ) continue;

//         if ( !c->unitNameY().empty() ) return true;
//     }

//     return false;
// };

// if ( !anyCurveWithUnitText() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

// if ( strategy == AxisAssignmentStrategy::USE_MATCHING_VECTOR )
// {
//     // Special handling if curve unit is matching. Try to match on histogram vector name to avoid creation of new axis

//     for ( auto c : histogramCurves() )
//     {
//         if ( c == curve ) continue;

//         auto incomingAxisText = RimPlotAxisTools::axisTextForAddress( curve->histogramAddressY() );
//         auto currentAxisText  = RimPlotAxisTools::axisTextForAddress( c->histogramAddressY() );
//         if ( incomingAxisText == currentAxisText )
//         {
//             curve->setLeftOrRightAxisY( c->axisY() );
//             return;
//         }
//     }
// }
// else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
// {
//     for ( auto c : histogramCurves() )
//     {
//         if ( c == curve ) continue;

//         auto currentUnit = RiaStdStringTools::toUpper( c->unitNameY() );
//         if ( currentUnit == destinationUnit )
//         {
//             for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
//             {
//                 if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
//                      axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT )
//                 {
//                     curve->setLeftOrRightAxisY( c->axisY() );

//                     return;
//                 }
//             }
//         }
//     }

//     strategy = AxisAssignmentStrategy::ALTERNATING;
// }

// auto isDefaultLeftAndRightUsed = [this]( RimHistogramCurve* currentCurve ) -> std::pair<bool, bool>
// {
//     bool defaultLeftUsed  = false;
//     bool defaultRightUsed = false;

//     for ( auto c : histogramCurves() )
//     {
//         if ( c == currentCurve ) continue;

//         if ( c->axisY() == RiuPlotAxis::defaultLeft() ) defaultLeftUsed = true;
//         if ( c->axisY() == RiuPlotAxis::defaultRight() ) defaultRightUsed = true;
//     }

//     return std::make_pair( defaultLeftUsed, defaultRightUsed );
// };

// auto [defaultLeftUsed, defaultRightUsed] = isDefaultLeftAndRightUsed( curve );
// if ( !defaultLeftUsed )
// {
//     curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
//     return;
// }

// if ( !defaultRightUsed )
// {
//     curve->setLeftOrRightAxisY( RiuPlotAxis::defaultRight() );
//     return;
// }

// RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
// if ( strategy == AxisAssignmentStrategy::ALTERNATING )
// {
//     size_t axisCountLeft  = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
//     size_t axisCountRight = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

//     if ( axisCountLeft > axisCountRight ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
// }

// if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
// {
//     auto newPlotAxis = plotWidget()->createNextPlotAxis( plotAxisType );
//     addNewAxisProperties( newPlotAxis, "New Axis" );

//     curve->setLeftOrRightAxisY( newPlotAxis );
//     return;
// }

// // If we get here, we have no more axes to assign to, use left axis as fallback
// curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
//}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramPlot::assignXPlotAxis( RimHistogramCurve* curve )
// {
// RiuPlotAxis newPlotAxis = RimHistogramPlot::plotAxisForTime();

// if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::HISTOGRAM_VECTOR )
// {
//     enum class AxisAssignmentStrategy
//     {
//         ALL_TOP,
//         ALL_BOTTOM,
//         ALTERNATING,
//         USE_MATCHING_UNIT,
//         USE_MATCHING_VECTOR
//     };

//     auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

//     auto destinationUnit = RiaStdStringTools::toUpper( curve->unitNameX() );
//     if ( destinationUnit.empty() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

//     auto anyCurveWithUnitText = [this, curve]
//     {
//         for ( auto c : histogramCurves() )
//         {
//             if ( c == curve ) continue;

//             if ( !c->unitNameX().empty() ) return true;
//         }

//         return false;
//     };

//     if ( !anyCurveWithUnitText() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

//     if ( strategy == AxisAssignmentStrategy::USE_MATCHING_VECTOR )
//     {
//         // Special handling if curve unit is matching. Try to match on histogram vector name to avoid creation of new axis

//         for ( auto c : histogramCurves() )
//         {
//             if ( c == curve ) continue;

//             auto incomingAxisText = RimPlotAxisTools::axisTextForAddress( curve->histogramAddressY() );
//             auto currentAxisText  = RimPlotAxisTools::axisTextForAddress( c->histogramAddressY() );
//             if ( incomingAxisText == currentAxisText )
//             {
//                 curve->setTopOrBottomAxisX( c->axisX() );
//                 return;
//             }
//         }
//     }
//     else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
//     {
//         bool isTopUsed    = false;
//         bool isBottomUsed = false;

//         for ( auto c : histogramCurves() )
//         {
//             if ( c == curve ) continue;

//             if ( c->axisX() == RiuPlotAxis::defaultTop() ) isTopUsed = true;
//             if ( c->axisX() == RiuPlotAxis::defaultBottomForHistogramVectors() ) isBottomUsed = true;

//             auto currentUnit = RiaStdStringTools::toUpper( c->unitNameX() );

//             if ( currentUnit == destinationUnit )
//             {
//                 for ( RimPlotAxisPropertiesInterface* axisProperties : m_axisPropertiesArray )
//                 {
//                     if ( axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_TOP ||
//                          axisProperties->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
//                     {
//                         curve->setTopOrBottomAxisX( c->axisX() );

//                         return;
//                     }
//                 }
//             }
//         }

//         if ( !isTopUsed )
//         {
//             curve->setTopOrBottomAxisX( RiuPlotAxis::defaultTop() );
//             return;
//         }

//         if ( !isBottomUsed )
//         {
//             curve->setTopOrBottomAxisX( RiuPlotAxis::defaultBottomForHistogramVectors() );
//             return;
//         }

//         strategy = AxisAssignmentStrategy::ALTERNATING;
//     }

//     RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_TOP;

//     if ( strategy == AxisAssignmentStrategy::ALTERNATING )
//     {
//         size_t axisCountTop = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_TOP );
//         size_t axisCountBot = countAxes( m_axisPropertiesArray.childrenByType(), RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );

//         if ( axisCountTop > axisCountBot ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
//     }
//     else if ( strategy == AxisAssignmentStrategy::ALL_TOP )
//     {
//         plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_TOP;
//     }
//     else if ( strategy == AxisAssignmentStrategy::ALL_BOTTOM )
//     {
//         plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
//     }

//     RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultBottomForHistogramVectors();
//     if ( plotWidget() && plotWidget()->isMultiAxisSupported() )
//     {
//         newPlotAxis = plotWidget()->createNextPlotAxis( plotAxisType );
//         addNewAxisProperties( newPlotAxis, "New Axis" );
//     }
// }

// curve->setTopOrBottomAxisX( newPlotAxis );
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    if ( childArray == &m_axisPropertiesArray )
    {
        // for ( caf::PdmObjectHandle* reffingObj : referringObjects )
        // {
        //     // auto* curve    = dynamic_cast<RimHistogramCurve*>( reffingObj );
        //     // auto* curveSet = dynamic_cast<RimEnsembleCurveSet*>( reffingObj );
        //     // if ( curve )
        //     // {
        //     //     curve->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
        //     // }
        //     // else if ( curveSet )
        //     // {
        //     //     curveSet->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
        //     // }
        // }

        if ( plotWidget() )
        {
            std::set<RiuPlotAxis> usedPlotAxis;
            for ( const auto& axisProperties : m_axisPropertiesArray )
            {
                usedPlotAxis.insert( axisProperties->plotAxis() );
            }

            plotWidget()->pruneAxes( usedPlotAxis );
            updateAxes();
            scheduleReplotIfVisible();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// RimHistogramPlotSourceStepping* RimHistogramPlot::sourceStepper()
// {
//     return m_sourceStepping();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramPlot::onUpdateCurveOrder()
{
    //    m_histogramCurveCollection->updateCurveOrder();
}
