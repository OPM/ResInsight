/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimHistogramMultiPlot.h"

#include "RiaApplication.h"
#include "RiaNumericalTools.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"

#include "RimAnnotationLineAppearance.h"
#include "RimEnsembleCurveSet.h"
#include "RimHistogramPlot.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimPlotAxisProperties.h"

// #include "RiuHistogramMultiPlotBook.h"
// #include "RiuHistogramVectorSelectionUi.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafSelectionManager.h"

#include "qwt_scale_engine.h"

#include <QKeyEvent>

#include <cmath>

namespace caf
{
template <>
void AppEnum<RimHistogramMultiPlot::AxisRangeAggregation>::setUp()
{
    addItem( RimHistogramMultiPlot::AxisRangeAggregation::NONE, "NONE", "Per Sub Plot" );
    addItem( RimHistogramMultiPlot::AxisRangeAggregation::SUB_PLOTS, "SUB_PLOTS", "All Sub Plots" );
    addItem( RimHistogramMultiPlot::AxisRangeAggregation::REALIZATIONS, "REALIZATIONS", "All Realizations" );
    setDefault( RimHistogramMultiPlot::AxisRangeAggregation::NONE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimHistogramMultiPlot, "MultiHistogramPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot::RimHistogramMultiPlot()
    : duplicatePlot( this )
{
    CAF_PDM_InitObject( "Multi Histogram Plot", ":/HistogramPlotLight16x16.png" );
    setDeletable( true );

    CAF_PDM_InitField( &m_autoPlotTitle, "AutoPlotTitle", true, "Auto Plot Title" );
    CAF_PDM_InitField( &m_autoSubPlotTitle, "AutoSubPlotTitle", true, "Auto Sub Plot Title" );

    CAF_PDM_InitField( &m_createPlotDuplicate, "DuplicatePlot", false, "", "", "Duplicate Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_createPlotDuplicate );
    m_createPlotDuplicate.uiCapability()->setUiIconFromResourceString( ":/Copy.svg" );

    CAF_PDM_InitField( &m_disableWheelZoom, "DisableWheelZoom", true, "", "", "Disable Mouse Wheel Zooming in Multi Histogram Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_disableWheelZoom );
    m_disableWheelZoom.uiCapability()->setUiIconFromResourceString( ":/DisableZoom.png" );

    CAF_PDM_InitField( &m_appendNextPlot, "AppendNextPlot", false, "", "", "Step Next and Add to New Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendNextPlot );
    m_appendNextPlot.uiCapability()->setUiIconFromResourceString( ":/AppendNext.png" );

    CAF_PDM_InitField( &m_appendPrevPlot, "AppendPrevPlot", false, "", "", "Step Previous and Add to New Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendPrevPlot );
    m_appendPrevPlot.uiCapability()->setUiIconFromResourceString( ":/AppendPrev.png" );

    CAF_PDM_InitField( &m_appendNextCurve, "AppendNextCurve", false, "", "", "Step Next and Add Curve to Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendNextCurve );
    m_appendNextCurve.uiCapability()->setUiIconFromResourceString( ":/AppendNextCurve.png" );

    CAF_PDM_InitField( &m_appendPrevCurve, "AppendPrevCurve", false, "", "", "Step Previous and Add Curve to Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_appendPrevCurve );
    m_appendPrevCurve.uiCapability()->setUiIconFromResourceString( ":/AppendPrevCurve.png" );

    CAF_PDM_InitField( &m_linkSubPlotAxes, "LinkSubPlotAxes", false, "Link Y Axes" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_linkSubPlotAxes );
    CAF_PDM_InitField( &m_autoAdjustAppearance, "AutoAdjustAppearance", true, "Auto Plot Settings" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_autoAdjustAppearance );
    CAF_PDM_InitField( &m_allow3DSelectionLink, "Allow3DSelectionLink", true, "Allow Well Selection from 3D View" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_allow3DSelectionLink );

    CAF_PDM_InitFieldNoDefault( &m_axisRangeAggregation, "AxisRangeAggregation", "Y Axis Range" );

    CAF_PDM_InitField( &m_hidePlotsWithValuesBelow, "HidePlotsWithValuesBelow", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_hidePlotsWithValuesBelow );

    CAF_PDM_InitField( &m_plotFilterYAxisThreshold, "PlotFilterYAxisThreshold", 0.0, "Y-Axis Filter Threshold" );

    setBottomMargin( 40 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot::~RimHistogramMultiPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::addPlot( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::addPlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::insertPlot( RimPlot* plot, size_t index )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        //        sumPlot->axisChanged.connect( this, &RimHistogramMultiPlot::onSubPlotAxisChanged );
        sumPlot->curvesChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        sumPlot->titleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        // sumPlot->axisChangedReloadRequired.connect( this, &RimHistogramMultiPlot::onSubPlotAxisReloadRequired );
        sumPlot->autoTitleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotAutoTitleChanged );

        // bool isMinMaxOverridden = m_axisRangeAggregation() != AxisRangeAggregation::NONE;
        // setAutoValueStatesForPlot( sumPlot, isMinMaxOverridden, m_autoAdjustAppearance() );

        auto plots = histogramPlots();
        // if ( !plots.empty() && m_linkTimeAxis )
        // {
        //     sumPlot->copyAxisPropertiesFromOther( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, *plots.front() );
        // }

        RimMultiPlot::insertPlot( plot, index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::removePlot( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::removePlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::removePlotNoUpdate( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::removePlotNoUpdate( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updateAfterPlotRemove()
{
    onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // auto axesGroup = uiOrdering.addNewGroup( "Axes" );
    // axesGroup->add( &m_axisRangeAggregation );
    // axesGroup->add( &m_linkSubPlotAxes );
    // axesGroup->add( &m_autoAdjustAppearance );

    // auto readOutGroup = uiOrdering.addNewGroup( "Mouse Cursor Readout" );
    // m_readOutSettings->uiOrdering( uiConfigName, *readOutGroup );

    // auto plotVisibilityFilterGroup = uiOrdering.addNewGroup( "Plot Visibility Filter" );
    // plotVisibilityFilterGroup->add( &m_plotFilterYAxisThreshold );
    // plotVisibilityFilterGroup->add( &m_hidePlotsWithValuesBelow );

    // auto dataSourceGroup = uiOrdering.addNewGroup( "Data Source" );
    // dataSourceGroup->setCollapsedByDefault();

    auto titlesGroup = uiOrdering.addNewGroup( "Main Plot Settings" );
    titlesGroup->setCollapsedByDefault();

    // If a checkbox is first in the group, it is not responding to mouse clicks. Set title as first element.
    // https://github.com/OPM/ResInsight/issues/10321
    titlesGroup->add( &m_plotWindowTitle );
    titlesGroup->add( &m_autoPlotTitle );
    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_titleFontSize );

    auto subPlotSettingsGroup = uiOrdering.addNewGroup( "Sub Plot Settings" );
    subPlotSettingsGroup->setCollapsedByDefault();
    subPlotSettingsGroup->add( &m_autoSubPlotTitle );
    subPlotSettingsGroup->add( &m_showIndividualPlotTitles );
    subPlotSettingsGroup->add( &m_subTitleFontSize );

    auto legendsGroup = uiOrdering.addNewGroup( "Legends" );
    legendsGroup->setCollapsedByDefault();
    legendsGroup->add( &m_showPlotLegends );
    legendsGroup->add( &m_plotLegendsHorizontal );
    legendsGroup->add( &m_legendPosition );
    legendsGroup->add( &m_legendFontSize );

    uiOrdering.skipRemainingFields( true );

    updateReadOnlyState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_autoPlotTitle || changedField == &m_autoSubPlotTitle )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
    else if ( changedField == &m_linkSubPlotAxes )
    {
        if ( m_linkSubPlotAxes() )
        {
            // Set the range aggregation to all sub plots when linking. This will ensure that the ZoomAll operation works on all sub plots.
            m_axisRangeAggregation = AxisRangeAggregation::SUB_PLOTS;
        }

        setAutoValueStates();
        syncAxisRanges();
        analyzePlotsAndAdjustAppearanceSettings();
        zoomAll();
    }
    else if ( changedField == &m_axisRangeAggregation )
    {
        setAutoValueStates();
        syncAxisRanges();
        analyzePlotsAndAdjustAppearanceSettings();
        zoomAll();
    }
    else if ( changedField == &m_hidePlotsWithValuesBelow )
    {
        m_hidePlotsWithValuesBelow = false;
        updatePlotVisibility();
    }
    else if ( changedField == &m_createPlotDuplicate )
    {
        m_createPlotDuplicate = false;
        duplicate();
    }
    // else if ( changedField == &m_appendNextPlot )
    // {
    //     m_appendNextPlot  = false;
    //     int stepDirection = 1;
    //     appendSubPlotByStepping( stepDirection );
    // }
    // else if ( changedField == &m_appendPrevPlot )
    // {
    //     m_appendPrevPlot  = false;
    //     int stepDirection = -1;
    //     appendSubPlotByStepping( stepDirection );
    // }
    // else if ( changedField == &m_appendNextCurve )
    // {
    //     m_appendNextCurve = false;
    //     int stepDirection = 1;
    //     appendCurveByStepping( stepDirection );
    // }
    // else if ( changedField == &m_appendPrevCurve )
    // {
    //     m_appendPrevCurve = false;
    //     int stepDirection = -1;
    //     appendCurveByStepping( stepDirection );
    // }
    else if ( changedField == &m_autoAdjustAppearance )
    {
        setAutoValueStates();
        analyzePlotsAndAdjustAppearanceSettings();
    }
    else if ( changedField == &m_plotWindowTitle )
    {
        // If the user has changed the plot title, disable the auto plot title
        // Workaround for https://github.com/OPM/ResInsight/issues/9681

        m_autoPlotTitle = false;
    }

    RimMultiPlot::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    // if ( changedChildField == &m_readOutSettings )
    // {
    //     updateReadOutSettings();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updatePlotTitles()
{
    if ( m_autoPlotTitle )
    {
        // populateNameHelper( m_nameHelper.get() );

        QString title;

        if ( title.isEmpty() )
        {
            auto collections = RimMainPlotCollection::current()->histogramMultiPlotCollection();

            size_t index = 0;
            for ( auto p : collections->histogramMultiPlots() )
            {
                index++;
                if ( p == this ) break;
            }

            title = QString( "Plot %1" ).arg( index );
        }

        setMultiPlotTitle( title );
    }

    // for ( auto plot : histogramPlots() )
    // {
    //     // if ( m_autoSubPlotTitle )
    //     // {
    //     //     auto subPlotNameHelper = plot->plotTitleHelper();

    //     //     // Disable auto plot title, as this is required to be able to include the information in the multi plot
    //     //     // title
    //     //     plot->enableAutoPlotTitle( false );

    //     //     auto plotName = subPlotNameHelper->aggregatedPlotTitle( *m_nameHelper );
    //     //     plot->setPlotTitleVisible( true );
    //     //     plot->setDescription( plotName );
    //     // }
    //     // plot->updatePlotTitle();
    // }

    if ( !m_viewer.isNull() ) m_viewer->scheduleTitleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setAutoPlotTitle( bool enable )
{
    m_autoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setAutoSubPlotTitle( bool enable )
{
    m_autoSubPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramPlot*> RimHistogramMultiPlot::histogramPlots() const
{
    std::vector<RimHistogramPlot*> typedPlots;

    for ( auto plot : plots() )
    {
        auto histogramPlot = dynamic_cast<RimHistogramPlot*>( plot );
        if ( histogramPlot ) typedPlots.push_back( histogramPlot );
    }

    return typedPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramPlot*> RimHistogramMultiPlot::visibleHistogramPlots() const
{
    std::vector<RimHistogramPlot*> visiblePlots;

    for ( auto plot : histogramPlots() )
    {
        if ( plot->showWindow() ) visiblePlots.push_back( plot );
    }

    return visiblePlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramMultiPlot::handleGlobalKeyEvent( QKeyEvent* keyEvent )
{
    if ( isMouseCursorInsidePlot() )
    {
        if ( keyEvent->key() == Qt::Key_PageUp )
        {
            m_viewer->goToPrevPage();
            return true;
        }

        if ( keyEvent->key() == Qt::Key_PageDown )
        {
            m_viewer->goToNextPage();
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramMultiPlot::handleGlobalWheelEvent( QWheelEvent* wheelEvent )
{
    if ( m_disableWheelZoom )
    {
        if ( isMouseCursorInsidePlot() )
        {
            if ( wheelEvent->angleDelta().y() > 0 )
            {
                m_viewer->goToPrevPage();
            }
            else if ( wheelEvent->angleDelta().y() < 0 )
            {
                m_viewer->goToNextPage();
            }

            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::initAfterRead()
{
    RimMultiPlot::initAfterRead();

    for ( auto plot : histogramPlots() )
    {
        // plot->axisChanged.connect( this, &RimHistogramMultiPlot::onSubPlotAxisChanged );
        plot->curvesChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        plot->titleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        // plot->axisChangedReloadRequired.connect( this, &RimHistogramMultiPlot::onSubPlotAxisReloadRequired );
        plot->autoTitleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotAutoTitleChanged );
    }
    // updateStepDimensionFromDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onLoadDataAndUpdate()
{
    RimMultiPlot::onLoadDataAndUpdate();
    updatePlotTitles();

    for ( auto p : histogramPlots() )
    {
        p->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::zoomAll()
{
    setAutoValueStates();

    // Reset zoom to make sure the complete range for min/max is available
    RimMultiPlot::zoomAll();

    syncAxisRanges();

    if ( m_linkSubPlotAxes() )
    {
        // Disable auto scaling for Y axis, use the axis values from the first plot
        setAutoScaleYEnabled( false );

        // if ( !histogramPlots().empty() )
        // {
        //     onSubPlotAxisChanged( nullptr, histogramPlots().front() );
        // }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramMultiPlot::updateTimeAxisRangesFromFirstTimePlot()
// {
//     if ( m_linkTimeAxis )
//     {
//         auto allPlots = histogramPlots();
//         for ( auto plot : allPlots )
//         {
//             auto curves = plot->histogramAndEnsembleCurves();
//             for ( auto curve : curves )
//             {
//                 if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::TIME )
//                 {
//                     setAutoScaleXEnabled( false );
//                     syncTimeAxisRanges( plot );

//                     return;
//                 }
//             }
//         }
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::syncAxisRanges()
{
    if ( m_axisRangeAggregation() == AxisRangeAggregation::NONE )
    {
        return;
    }

    // Reset zoom for axes with no custom range set to make sure the complete range for min/max is available

    for ( auto p : histogramPlots() )
    {
        for ( auto ax : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            ax->setAutoZoomIfNoCustomRangeIsSet();
        }
    }
    updateZoom();

    if ( m_axisRangeAggregation() == AxisRangeAggregation::SUB_PLOTS )
    {
        std::map<QString, std::pair<double, double>> axisRanges;

        // gather current min/max values for each category (axis label)
        for ( auto plot : histogramPlots() )
        {
            for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                double minVal = axis->visibleRangeMin();
                double maxVal = axis->visibleRangeMax();
                if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );

                auto key = axis->objectName();
                if ( axisRanges.count( key ) == 0 )
                {
                    axisRanges[key] = std::make_pair( minVal, maxVal );
                }
                else
                {
                    auto& [currentMin, currentMax] = axisRanges[key];
                    axisRanges[key]                = std::make_pair( std::min( currentMin, minVal ), std::max( currentMax, maxVal ) );
                }
            }
        }

        // set all plots to use the global min/max values for each category
        for ( auto plot : histogramPlots() )
        {
            for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                auto [minVal, maxVal] = axisRanges[axis->objectName()];
                if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );
                axis->setAutoZoom( false );

                axis->setAutoValueVisibleRangeMin( minVal );
                axis->setAutoValueVisibleRangeMax( maxVal );
            }

            plot->updateAxes();
        }
    }
    else
    {
        computeAggregatedAxisRange();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::computeAggregatedAxisRange()
{
    // auto readValues = []( RimHistogramCase* histogramCase, RifEclipseHistogramAddress addr )
    // {
    //     if ( histogramCase && histogramCase->histogramReader() )
    //     {
    //         RifHistogramReaderInterface* reader = histogramCase->histogramReader();
    //         auto [isOk, values]               = reader->values( addr );
    //         return values;
    //     }

    //     return std::vector<double>();
    // };

    // auto findMinMaxForHistogramCase = [readValues]( RimHistogramCase* histogramCase, RifEclipseHistogramAddress addr, bool
    // onlyPositiveValues )
    // {
    //     auto values = readValues( histogramCase, addr );
    //     if ( onlyPositiveValues )
    //     {
    //         std::vector<double> positiveValues;

    //         for ( const auto& v : values )
    //         {
    //             if ( v > 0.0 ) positiveValues.push_back( v );
    //         }

    //         values = positiveValues;
    //     }
    //     if ( values.empty() ) return std::make_pair( HUGE_VAL, -HUGE_VAL );

    //     auto   minMaxPair  = std::minmax_element( values.begin(), values.end() );
    //     double caseMinimum = *minMaxPair.first;
    //     double caseMaximum = *minMaxPair.second;

    //     return std::make_pair( caseMinimum, caseMaximum );
    // };

    // auto histogramCasesForCurve = []( RimHistogramCurve* curve, AxisRangeAggregation axisRangeAggregation )
    // {
    //     std::vector<RimHistogramCase*> histogramCases;

    //     if ( axisRangeAggregation == AxisRangeAggregation::REALIZATIONS )
    //     {
    //         if ( curve->histogramCaseY() )
    //         {
    //             auto ensemble = curve->histogramCaseY()->ensemble();
    //             if ( ensemble )
    //             {
    //                 histogramCases = ensemble->allHistogramCases();
    //             }
    //             else
    //             {
    //                 histogramCases.push_back( curve->histogramCaseY() );
    //             }
    //         }
    //     }
    //     else if ( axisRangeAggregation == AxisRangeAggregation::WELLS || axisRangeAggregation == AxisRangeAggregation::REGIONS )
    //     {
    //         // Use only the current histogram case when aggregation across wells/regions
    //         histogramCases.push_back( curve->histogramCaseY() );
    //     }

    //     return histogramCases;
    // };

    // auto addressesForCurve = []( RimHistogramCurve* curve, AxisRangeAggregation axisRangeAggregation )
    // {
    //     std::vector<RifEclipseHistogramAddress> addresses;

    //     auto addr = curve->histogramAddressY();
    //     if ( axisRangeAggregation == AxisRangeAggregation::REALIZATIONS )
    //     {
    //         addresses = { RifEclipseHistogramAddress::fieldAddress( addr.vectorName(), addr.id() ) };
    //     }
    //     else if ( axisRangeAggregation == AxisRangeAggregation::WELLS || axisRangeAggregation == AxisRangeAggregation::REGIONS )
    //     {
    //         RiaHistogramAddressAnalyzer  fallbackAnalyzer;
    //         RiaHistogramAddressAnalyzer* analyzer = nullptr;

    //         if ( curve->histogramCaseY() )
    //         {
    //             auto ensemble = curve->histogramCaseY()->ensemble();
    //             if ( ensemble )
    //             {
    //                 analyzer = ensemble->addressAnalyzer();
    //             }
    //             else
    //             {
    //                 fallbackAnalyzer.appendAddresses( curve->histogramCaseY()->histogramReader()->allResultAddresses() );
    //                 analyzer = &fallbackAnalyzer;
    //             }
    //         }

    //         if ( analyzer )
    //         {
    //             if ( axisRangeAggregation == AxisRangeAggregation::WELLS )
    //             {
    //                 for ( const auto& wellName : analyzer->wellNames() )
    //                 {
    //                     addresses.push_back( RifEclipseHistogramAddress::wellAddress( addr.vectorName(), wellName, addr.id() ) );
    //                 }
    //             }

    //             if ( axisRangeAggregation == AxisRangeAggregation::REGIONS )
    //             {
    //                 for ( auto regionNumber : analyzer->regionNumbers() )
    //                 {
    //                     addresses.push_back( RifEclipseHistogramAddress::regionAddress( addr.vectorName(), regionNumber, addr.id() ) );
    //                 }
    //             }
    //         }
    //     }

    //     return addresses;
    // };

    // auto findMinMaxForAddressesInHistogramCases = [findMinMaxForHistogramCase]( const std::vector<RifEclipseHistogramAddress>& addresses,
    //                                                                         const std::vector<RimHistogramCase*>& histogramCases, bool
    //                                                                         onlyPositiveValues )
    // {
    //     double minimum = HUGE_VAL;
    //     double maximum = -HUGE_VAL;
    //     for ( auto histogramCase : histogramCases )
    //     {
    //         for ( const auto& addr : addresses )
    //         {
    //             auto [caseMinimum, caseMaximum] = findMinMaxForHistogramCase( histogramCase, addr, onlyPositiveValues );
    //             minimum                         = std::min( minimum, caseMinimum );
    //             maximum                         = std::max( maximum, caseMaximum );
    //         }
    //     }

    //     return std::make_pair( minimum, maximum );
    // };

    // // gather current min/max values for each category (axis label)
    // for ( auto plot : histogramPlots() )
    // {
    //     std::map<RiuPlotAxis, std::pair<double, double>> axisRanges;

    //     for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
    //     {
    //         for ( auto curve : plot->histogramCurves() )
    //         {
    //             if ( curve->axisY() == axis->plotAxis() )
    //             {
    //                 std::vector<RimHistogramCase*>          histogramCases = histogramCasesForCurve( curve, m_axisRangeAggregation() );
    //                 std::vector<RifEclipseHistogramAddress> addresses    = addressesForCurve( curve, m_axisRangeAggregation() );

    //                 bool onlyPositiveValues = axis->isLogarithmicScaleEnabled();

    //                 auto [minimum, maximum] = findMinMaxForAddressesInHistogramCases( addresses, histogramCases, onlyPositiveValues );

    //                 if ( axisRanges.count( axis->plotAxis() ) == 0 )
    //                 {
    //                     axisRanges[axis->plotAxis()] = std::make_pair( minimum, maximum );
    //                 }
    //                 else
    //                 {
    //                     auto& [currentMin, currentMax] = axisRanges[axis->plotAxis()];
    //                     axisRanges[axis->plotAxis()]   = std::make_pair( std::min( currentMin, minimum ), std::max( currentMax, maximum )
    //                     );
    //                 }
    //             }
    //         }

    //         for ( auto curveSet : plot->curveSets() )
    //         {
    //             if ( !curveSet->histogramEnsemble() ) continue;

    //             if ( curveSet->axisY() == axis->plotAxis() )
    //             {
    //                 double minimum( std::numeric_limits<double>::infinity() );
    //                 double maximum( -std::numeric_limits<double>::infinity() );

    //                 auto curves = curveSet->curves();
    //                 if ( !curves.empty() )
    //                 {
    //                     // TODO: Use analyzer as input to addressesForCurve instead of curve

    //                     auto curve = curves.front();

    //                     std::vector<RifEclipseHistogramAddress> addresses = addressesForCurve( curve, m_axisRangeAggregation() );

    //                     for ( const auto& adr : addresses )
    //                     {
    //                         auto [min, max] = curveSet->histogramEnsemble()->minMax( adr );

    //                         minimum = std::min( min, minimum );
    //                         maximum = std::max( max, maximum );
    //                     }
    //                 }

    //                 if ( axisRanges.count( axis->plotAxis() ) == 0 )
    //                 {
    //                     axisRanges[axis->plotAxis()] = std::make_pair( minimum, maximum );
    //                 }
    //                 else
    //                 {
    //                     auto& [currentMin, currentMax] = axisRanges[axis->plotAxis()];
    //                     axisRanges[axis->plotAxis()]   = std::make_pair( std::min( currentMin, minimum ), std::max( currentMax, maximum )
    //                     );
    //                 }
    //             }
    //         }
    //     }

    //     // set all plots to use the global min/max values for each category
    //     for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
    //     {
    //         auto [minVal, maxVal] = axisRanges[axis->plotAxis()];
    //         if ( RiaDefines::isVertical( axis->plotAxis().axis() ) && !std::isinf( minVal ) && !std::isinf( maxVal ) )
    //         {
    //             axis->setAutoZoom( false );

    //             if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );

    //             auto [adjustedMinVal, adjustedMaxVal] = adjustedMinMax( axis, minVal, maxVal );

    //             axis->setAutoValueVisibleRangeMin( adjustedMinVal );
    //             axis->setAutoValueVisibleRangeMax( adjustedMaxVal );
    //         }
    //     }

    //     plot->updateAxes();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updatePlotVisibility()
{
    for ( auto plot : histogramPlots() )
    {
        plot->setShowWindow( true );
    }

    updateLayout();

    if ( !m_viewer.isNull() ) m_viewer->scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setAutoValueStates()
{
    // bool enableMinMaxAutoValue = m_axisRangeAggregation() != AxisRangeAggregation::NONE;
    // for ( auto p : histogramPlots() )
    // {
    //     setAutoValueStatesForPlot( p, enableMinMaxAutoValue, m_autoAdjustAppearance() );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::duplicate()
{
    duplicatePlot.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::analyzePlotsAndAdjustAppearanceSettings()
{
    if ( m_autoAdjustAppearance )
    {
        if ( !m_linkSubPlotAxes )
        {
            // Required to sync axis ranges before computing the auto scale
            syncAxisRanges();
        }

        // RiaHistogramAddressAnalyzer analyzer;

        // for ( auto p : histogramPlots() )
        // {
        //     auto addresses = RiaHistogramAddressModifier::allHistogramAddressesY( p );
        //     analyzer.appendAddresses( addresses );
        // }

        // bool canShowOneAxisTitlePerRow = analyzer.isSingleQuantityIgnoreHistory() && ( m_axisRangeAggregation() !=
        // AxisRangeAggregation::NONE );

        // const bool notifyFieldChanged = false;

        // for ( auto p : histogramPlots() )
        // {
        //     if ( auto timeAxisProp = p->timeAxisProperties() )
        //     {
        //         auto tickMarkCount = ( columnCount() < 3 ) ? RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT
        //                                                    : RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW;

        //         timeAxisProp->setAutoValueForMajorTickmarkCount( tickMarkCount, notifyFieldChanged );
        //     }

        //     for ( auto* axisProp : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        //     {
        //         auto tickMarkCount = ( rowsPerPage() == 1 ) ? RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT
        //                                                     : RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW;

        //         axisProp->setAutoValueForMajorTickmarkCount( tickMarkCount, notifyFieldChanged );

        //         axisProp->computeAndSetAutoValueForScaleFactor();

        //         if ( canShowOneAxisTitlePerRow )
        //         {
        //             auto [row, col] = gridLayoutInfoForSubPlot( p );

        //             bool isFirstColumn = ( col == 0 );
        //             axisProp->setShowUnitText( isFirstColumn );
        //             axisProp->setShowDescription( isFirstColumn );
        //         }
        //         else
        //         {
        //             axisProp->setShowUnitText( true );
        //             axisProp->setShowDescription( true );
        //         }
        //     }

        //     p->updateAxes();
        // }
    }
    else
    {
        for ( auto p : histogramPlots() )
        {
            for ( auto* axisProp : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                axisProp->computeAndSetAutoValueForScaleFactor();
                axisProp->setShowUnitText( true );
                axisProp->setShowDescription( true );
            }

            p->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::makeSureIsVisible( RimHistogramPlot* histogramPlot )
{
    if ( histogramPlot->plotWidget() && !m_viewer.isNull() ) m_viewer->scrollToPlot( histogramPlot->plotWidget() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setSubPlotAxesLinked( bool enable )
{
    m_linkSubPlotAxes = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramMultiPlot::isSubPlotAxesLinked() const
{
    return m_linkSubPlotAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimHistogramMultiPlot::gridLayoutInfoForSubPlot( RimHistogramPlot* histogramPlot ) const
{
    auto it = m_gridLayoutInfo.find( histogramPlot );
    if ( it != m_gridLayoutInfo.end() ) return it->second;

    return { -1, -1 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setLayoutInfo( RimHistogramPlot* histogramPlot, int row, int col )
{
    m_gridLayoutInfo[histogramPlot] = std::make_pair( row, col );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::clearLayoutInfo()
{
    m_gridLayoutInfo.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onSubPlotChanged( const caf::SignalEmitter* emitter )
{
    updatePlotTitles();
    applyPlotWindowTitleToWidgets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramMultiPlot::onSubPlotAxisChanged( const caf::SignalEmitter* emitter, RimHistogramPlot* histogramPlot )
// {
//     syncTimeAxisRanges( histogramPlot );

//     if ( m_linkSubPlotAxes() )
//     {
//         for ( auto plot : histogramPlots() )
//         {
//             if ( plot != histogramPlot )
//             {
//                 plot->copyMatchingAxisPropertiesFromOther( *histogramPlot );
//                 plot->updateAll();
//             }
//         }
//     }
//     else
//     {
//         syncAxisRanges();
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramMultiPlot::onSubPlotAxisReloadRequired( const caf::SignalEmitter* emitter, RimHistogramPlot* histogramPlot )
// {
//     if ( !histogramPlot ) return;

//     if ( m_linkTimeAxis() )
//     {
//         syncTimeAxisRanges( histogramPlot );

//         for ( auto plot : histogramPlots() )
//         {
//             plot->loadDataAndUpdate();
//         }
//     }
//     else
//     {
//         histogramPlot->loadDataAndUpdate();
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onSubPlotAutoTitleChanged( const caf::SignalEmitter* emitter, bool isEnabled )
{
    m_autoSubPlotTitle = isEnabled;

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updateReadOnlyState()
{
    m_axisRangeAggregation.uiCapability()->setUiReadOnly( m_linkSubPlotAxes() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updateReadOutSettings()
{
    // for ( auto plot : histogramPlots() )
    // {
    //     if ( !m_readOutSettings->enableVerticalLine() )
    //     {
    //         if ( auto axisProps = plot->timeAxisProperties() )
    //         {
    //             axisProps->removeAllAnnotations();
    //         }
    //     }

    //     if ( !m_readOutSettings->enableHorizontalLine() )
    //     {
    //         if ( auto axis = plot->axisPropertiesForPlotAxis( RiuPlotAxis::defaultLeft() ) )
    //         {
    //             axis->removeAllAnnotations();
    //         }
    //     }

    //     plot->enableCurvePointTracking( m_readOutSettings->enableCurvePointTracking() );

    //     plot->updateAndRedrawTimeAnnotations();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimHistogramMultiPlot::adjustedMinMax( const RimPlotAxisProperties* axis, double min, double max ) const
{
    if ( !axis->isLogarithmicScaleEnabled() )
    {
        int                  maxMajorTickIntervalCount = RimPlotAxisProperties::tickmarkCountFromEnum( axis->majorTickmarkCount() );
        double               stepSize                  = 0.0;
        QwtLinearScaleEngine scaleEngine;

        // Do not adjust minimum value, as we usually want to keep zero unchanged
        double adjustedMin = min;

        // Adjust the max value to get some space between the top of the plot and the top of the curve
        double adjustedMax = max * 1.05;

        scaleEngine.autoScale( maxMajorTickIntervalCount, adjustedMin, adjustedMax, stepSize );

        return { adjustedMin, adjustedMax };
    }

    auto adjustedMin = RiaNumericalTools::roundToClosestPowerOfTenFloor( min );
    auto adjustedMax = RiaNumericalTools::roundToClosestPowerOfTenCeil( max );

    return { adjustedMin, adjustedMax };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimHistogramMultiPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( m_viewer.isNull() )
    {
        m_viewer = new RiuMultiPlotBook( this, mainWindowParent );
    }
    recreatePlotWidgets();

    updateReadOutSettings();

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onPlotAdditionOrRemoval()
{
    m_disableWheelZoom = ( histogramPlots().size() > 1 );

    RimMultiPlot::onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::keepVisiblePageAfterUpdate( bool keepPage )
{
    if ( !m_viewer ) return;

    if ( keepPage ) m_viewer->keepCurrentPageAfterUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimHistogramMultiPlot::updateReadOutLines( double qwtTimeValue, double yValue )
// {
//     // if ( !m_readOutSettings->enableVerticalLine() && !m_readOutSettings->enableHorizontalLine() ) return;

//     // const auto    timeTValue       = RiaTimeTTools::fromDouble( qwtTimeValue );
//     // const QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
//     //                                                                       RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );

//     // for ( auto plot : histogramPlots() )
//     // {
//     //     if ( m_readOutSettings->enableVerticalLine() && plot->timeAxisProperties() )
//     //     {
//     //         plot->timeAxisProperties()->removeAllAnnotations();

//     //         auto lineAppearance = m_readOutSettings->lineAppearance();
//     //         auto anno           = RimTimeAxisAnnotation::createTimeAnnotation( timeTValue, lineAppearance->color() );

//     //         Qt::PenStyle style = lineAppearance->isDashed() ? Qt::PenStyle::DashLine : Qt::PenStyle::SolidLine;
//     //         anno->setPenStyle( style );
//     //         anno->setAlignment( m_readOutSettings->verticalLineLabelAlignment() );

//     //         plot->timeAxisProperties()->appendAnnotation( anno );
//     //     }

//     //     if ( m_readOutSettings->enableHorizontalLine() )
//     //     {
//     //         std::vector<RimHistogramCurve*> histogramCurves;

//     //         // 1. Check if any curves are highlighted
//     //         if ( auto plotWidget = dynamic_cast<RiuQwtPlotWidget*>( plot->plotWidget() ) )
//     //         {
//     //             for ( auto highlightCurve : plotWidget->highlightedCurves() )
//     //             {
//     //                 if ( auto histogramCurve = dynamic_cast<RimHistogramCurve*>( highlightCurve ) )
//     //                 {
//     //                     histogramCurves.push_back( histogramCurve );
//     //                 }
//     //             }
//     //         }

//     //         // 2. If no curves are highlighted, use histogram curves from single realizations
//     //         if ( histogramCurves.empty() )
//     //         {
//     //             histogramCurves = plot->histogramCurves();
//     //         }

//     //         auto annotationAxis = dynamic_cast<RimPlotAxisProperties*>( plot->axisPropertiesForPlotAxis( RiuPlotAxis::defaultLeft() )
//     );
//     //         if ( !histogramCurves.empty() )
//     //         {
//     //             auto firstCurve = histogramCurves.front();
//     //             yValue          = firstCurve->yValueAtTimeT( timeTValue );
//     //             annotationAxis  = dynamic_cast<RimPlotAxisProperties*>( plot->axisPropertiesForPlotAxis( firstCurve->axisY() ) );
//     //         }

//     //         if ( annotationAxis )
//     //         {
//     //             annotationAxis->removeAllAnnotations();

//     //             auto anno = new RimPlotAxisAnnotation();
//     //             anno->setAnnotationType( RimPlotAxisAnnotation::AnnotationType::LINE );

//     //             anno->setValue( yValue );

//     //             auto scaledValue = yValue / annotationAxis->scaleFactor();
//     //             auto valueText   = RiaNumberFormat::valueToText( scaledValue, RiaNumberFormat::NumberFormatType::FIXED, 2 );

//     //             anno->setName( valueText );

//     //             auto lineAppearance = m_readOutSettings->lineAppearance();

//     //             Qt::PenStyle style = lineAppearance->isDashed() ? Qt::PenStyle::DashLine : Qt::PenStyle::SolidLine;
//     //             anno->setPenStyle( style );
//     //             anno->setColor( lineAppearance->color() );
//     //             anno->setAlignment( m_readOutSettings->horizontalLineLabelAlignment() );

//     //             annotationAxis->appendAnnotation( anno );
//     //         }
//     //     }

//     //     // Calling plot->updateAxes() does not work for the first plot in a multiplot. Use fine grained update of annotation objects.
//     //     plot->updateAnnotationsInPlotWidget();
//     //     plot->updatePlotWidgetFromAxisRanges();
//     // }
// }
