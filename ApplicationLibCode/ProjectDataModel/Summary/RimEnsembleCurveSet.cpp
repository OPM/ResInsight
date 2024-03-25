/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleCurveSet.h"

#include "RiaColorTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaTimeTTools.h"

#include "RimSummaryCalculationCollection.h"
#include "SummaryPlotCommands/RicSummaryPlotEditorUi.h"

#include "RimCustomObjectiveFunction.h"
#include "RimCustomObjectiveFunctionCollection.h"
#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimEnsembleStatistics.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimObjectiveFunction.h"
#include "RimObjectiveFunctionTools.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryAddress.h"
#include "RimSummaryAddressSelector.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"
#include "RimTimeStepFilter.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuGuiTheme.h"
#include "RiuPlotCurve.h"
#include "RiuPlotCurveSymbol.h"
#include "RiuPlotMainWindow.h"
#include "RiuSummaryVectorSelectionDialog.h"
#include "RiuTextContentFrame.h"

#include "cafPdmObject.h"
#include "cafPdmUiColorEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafTitledOverlayFrame.h"

#include <algorithm>
#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimEnsembleCurveSet::ParameterSorting>::setUp()
{
    addItem( RimEnsembleCurveSet::ParameterSorting::ABSOLUTE_VALUE, "ABSOLUTE_VALUE", "Absolute Correlation" );
    addItem( RimEnsembleCurveSet::ParameterSorting::ALPHABETICALLY, "ALPHABETICALLY", "Alphabetically" );
    setDefault( RimEnsembleCurveSet::ParameterSorting::ABSOLUTE_VALUE );
}
template <>
void AppEnum<RimEnsembleCurveSet::AppearanceMode>::setUp()
{
    addItem( RimEnsembleCurveSet::AppearanceMode::DEFAULT, "DEFAULT", "Default" );
    addItem( RimEnsembleCurveSet::AppearanceMode::CUSTOM, "CUSTOM", "Custom" );
    setDefault( RimEnsembleCurveSet::AppearanceMode::DEFAULT );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::PointSymbolEnum statisticsCurveSymbolFromAddress( const RifEclipseSummaryAddress& address );
int                                 statisticsCurveSymbolSize( RiuPlotCurveSymbol::PointSymbolEnum symbol );

CAF_PDM_SOURCE_INIT( RimEnsembleCurveSet, "RimEnsembleCurveSet" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::RimEnsembleCurveSet()
    : filterChanged( this )

{
    CAF_PDM_InitObject( "Ensemble Curve Set", ":/EnsembleCurveSet16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_curves, "EnsembleCurveSet", "Ensemble Curve Set" );
    m_curves.uiCapability()->setUiTreeChildrenHidden( false );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves" );
    m_showCurves.uiCapability()->setUiHidden( true );

    // Y Values
    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryCaseCollection, "SummaryGroup", "Ensemble" );
    m_yValuesSummaryCaseCollection.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryCaseCollection.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddressUiField, "SelectedVariableDisplayVar", "Vector" );
    m_yValuesSummaryAddressUiField.xmlCapability()->disableIO();
    m_yValuesSummaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddress, "SummaryAddress", "Summary Address" );
    m_yValuesSummaryAddress.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_yPushButtonSelectSummaryAddress, "SelectAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_yPushButtonSelectSummaryAddress );
    m_yPushButtonSelectSummaryAddress = false;

    CAF_PDM_InitFieldNoDefault( &m_resampling, "Resampling", "Resampling" );

    // X Axis
    CAF_PDM_InitField( &m_xAxisType,
                       "HorizontalAxisType",
                       caf::AppEnum<RiaDefines::HorizontalAxisType>( RiaDefines::HorizontalAxisType::TIME ),
                       "Axis Type" );

    CAF_PDM_InitFieldNoDefault( &m_xAddressSelector, "XAddressSelector", "" );
    m_xAddressSelector = new RimSummaryAddressSelector;
    m_xAddressSelector->setAxisOrientation( RimPlotAxisProperties::Orientation::HORIZONTAL );
    m_xAddressSelector->setShowResampling( false );
    m_xAddressSelector.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_colorMode, "ColorMode", caf::AppEnum<ColorMode>( ColorMode::SINGLE_COLOR_WITH_ALPHA ), "Coloring Mode" );

    CAF_PDM_InitField( &m_colorForRealizations, "Color", RiaColorTools::textColor3f(), "Color" );
    CAF_PDM_InitField( &m_mainEnsembleColor, "MainEnsembleColor", RiaColorTools::textColor3f(), "Color" );
    CAF_PDM_InitField( &m_colorTransparency, "ColorTransparency", 0.3, "Transparency" );
    m_colorTransparency.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Parameter" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameterSorting, "EnsembleParameterSorting", "Parameter Sorting" );

    auto defaultLineStyle   = LineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    auto defaultPointSymbol = PointSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_CROSS );

    CAF_PDM_InitFieldNoDefault( &m_useCustomAppearance, "UseCustomAppearance", "Appearance" );
    CAF_PDM_InitField( &m_lineStyle, "LineStyle", defaultLineStyle, "Line Style" );
    CAF_PDM_InitField( &m_pointSymbol, "PointSymbol", defaultPointSymbol, "Symbol" );
    CAF_PDM_InitField( &m_symbolSize, "SymbolSize", 6, "Symbol Size" );

    auto defaultStatisticsPointSymbol = PointSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_ELLIPSE );
    CAF_PDM_InitFieldNoDefault( &m_statisticsUseCustomAppearance, "StatisticsUseCustomAppearance", "Appearance" );
    CAF_PDM_InitField( &m_statisticsLineStyle, "StatisticsLineStyle", defaultLineStyle, "Line Style" );
    CAF_PDM_InitField( &m_statisticsPointSymbol, "StatisticsPointSymbol", defaultStatisticsPointSymbol, "Symbol" );
    CAF_PDM_InitField( &m_statisticsSymbolSize, "StatisticsSymbolSize", 6, "Symbol Size" );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddressesUiField, "SelectedObjectiveSummaryVar", "Vector" );
    m_objectiveValuesSummaryAddressesUiField.xmlCapability()->disableIO();
    m_objectiveValuesSummaryAddressesUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddresses, "ObjectiveSummaryAddress", "Summary Address" );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSelectSummaryAddressPushButton, "SelectObjectiveSummaryAddress", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_objectiveValuesSelectSummaryAddressPushButton );
    m_objectiveValuesSelectSummaryAddressPushButton = false;

    CAF_PDM_InitFieldNoDefault( &m_customObjectiveFunction, "CustomObjectiveFunction", "Objective Function" );
    m_customObjectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showObjectiveFunctionFormula, "ShowObjectiveFunctionFormula", true, "Show Text Box in Plot" );

    CAF_PDM_InitFieldNoDefault( &m_minDateRange, "MinDateRange", "From" );
    m_minDateRange.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_minTimeSliderPosition, "MinTimeSliderPosition", 0, "" );
    m_minTimeSliderPosition.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxDateRange, "MaxDateRange", "To" );
    m_maxDateRange.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_maxTimeSliderPosition, "MaxTimeSliderPosition", 100, "" );
    m_maxTimeSliderPosition.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    // Time Step Selection
    CAF_PDM_InitFieldNoDefault( &m_timeStepFilter, "TimeStepFilter", "Available Time Steps" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Select Time Steps" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_plotAxis_OBSOLETE, "PlotAxis", "Axis" );
    m_plotAxis_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_yPlotAxisProperties, "Axis", "Axis" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setColorLegend( RimRegularLegendConfig::mapToColorLegend( RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE ) );

    CAF_PDM_InitFieldNoDefault( &m_curveFilters, "CurveFilters", "Curve Filters" );
    m_curveFilters = new RimEnsembleCurveFilterCollection();
    m_curveFilters->setUiTreeHidden( true );
    m_curveFilters->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_customObjectiveFunctions, "CustomObjectiveFunctions", "Custom Objective Functions" );
    m_customObjectiveFunctions = new RimCustomObjectiveFunctionCollection();
    m_customObjectiveFunctions->objectiveFunctionChanged.connect( this, &RimEnsembleCurveSet::onCustomObjectiveFunctionChanged );

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunction, "ObjectiveFunction", "Objective Function" );
    m_objectiveFunction = new RimObjectiveFunction();
    m_objectiveFunction->changed.connect( this, &RimEnsembleCurveSet::onObjectiveFunctionChanged );

    CAF_PDM_InitFieldNoDefault( &m_statistics, "Statistics", "Statistics" );
    m_statistics = new RimEnsembleStatistics( this );

    CAF_PDM_InitField( &m_userDefinedName, "UserDefinedName", QString( "Ensemble Curve Set" ), "Curve Set Name" );

    CAF_PDM_InitFieldNoDefault( &m_autoGeneratedName, "AutoGeneratedName", "Curve Set Name" );
    m_autoGeneratedName.registerGetMethod( this, &RimEnsembleCurveSet::createAutoName );
    m_autoGeneratedName.uiCapability()->setUiReadOnly( true );
    m_autoGeneratedName.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_isUsingAutoName, "AutoName", true, "Auto Name" );
    CAF_PDM_InitFieldNoDefault( &m_summaryAddressNameTools, "SummaryAddressNameTools", "SummaryAddressNameTools" );
    m_summaryAddressNameTools.uiCapability()->setUiTreeChildrenHidden( true );

    m_summaryAddressNameTools = new RimSummaryCurveAutoName;

    m_ensembleStatCaseY.reset( new RimEnsembleStatisticsCase() );
    m_ensembleStatCaseXY.reset( new RimEnsembleCrossPlotStatisticsCase() );

    m_disableStatisticCurves = false;
    m_isCurveSetFiltered     = false;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::~RimEnsembleCurveSet()
{
    m_curves.deleteChildren();

    auto parentPlot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( parentPlot && parentPlot->plotWidget() )
    {
        if ( m_plotCurveForLegendText ) m_plotCurveForLegendText->detach();
        if ( m_legendOverlayFrame )
        {
            parentPlot->plotWidget()->removeOverlayFrame( m_legendOverlayFrame );
        }
    }
    if ( m_legendOverlayFrame )
    {
        m_legendOverlayFrame->setParent( nullptr );
        delete m_legendOverlayFrame;
    }
    if ( m_filterOverlayFrame )
    {
        m_filterOverlayFrame->setParent( nullptr );
        delete m_filterOverlayFrame;
    }
    if ( m_objectiveFunctionOverlayFrame )
    {
        m_objectiveFunctionOverlayFrame->setParent( nullptr );
        delete m_objectiveFunctionOverlayFrame;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::isCurvesVisible() const
{
    auto coll = firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();
    return m_showCurves() && ( coll ? coll->isCurveSetsVisible() : true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setColor( cvf::Color3f color )
{
    m_mainEnsembleColor = color;

    setTransparentCurveColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setStatisticsColor( const cvf::Color3f& color )
{
    m_statistics->setColor( color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::enableStatisticsLables( bool enable )
{
    m_statistics->enableCurveLabels( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::loadDataAndUpdate( bool updateParentPlot )
{
    m_yValuesSummaryAddressUiField = m_yValuesSummaryAddress->address();

    m_curveFilters->loadDataAndUpdate();

    updateAddressesUiField();

    updateAllCurves();
    updateFilterLegend();
    updateObjectiveFunctionLegend();
    updateTimeAnnotations();

    if ( updateParentPlot )
    {
        auto parentPlot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
        parentPlot->updateAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setParentPlotNoReplot( RiuPlotWidget* plot )
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->setParentPlotNoReplot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deletePlotCurves()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->deletePlotCurve();
    }

    if ( m_plotCurveForLegendText )
    {
        m_plotCurveForLegendText->detach();
        m_plotCurveForLegendText = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::reattachPlotCurves()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->reattach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::addCurve( RimSummaryCurve* curve )
{
    if ( curve )
    {
        auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
        if ( plot && plot->plotWidget() ) curve->setParentPlotNoReplot( plot->plotWidget() );

        curve->setColor( m_colorForRealizations );
        m_curves.push_back( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteCurve( RimSummaryCurve* curve )
{
    if ( curve )
    {
        m_curves.removeChild( curve );
        delete curve;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryAddressY( RifEclipseSummaryAddress address )
{
    m_yValuesSummaryAddress->setAddress( address );
    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
    summaryAddress->setAddress( address );
    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setCurveAddress( RiaSummaryCurveAddress address )
{
    setSummaryAddressY( address.summaryAddressY() );
    setSummaryAddressX( address.summaryAddressX() );

    if ( address.summaryAddressX().category() == SummaryCategory::SUMMARY_TIME )
    {
        m_xAxisType = RiaDefines::HorizontalAxisType::TIME;
    }
    else
    {
        m_xAxisType = RiaDefines::HorizontalAxisType::SUMMARY_VECTOR;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryAddressX( RifEclipseSummaryAddress address )
{
    m_xAddressSelector->setAddress( address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimEnsembleCurveSet::fullTimeStepRange() const
{
    if ( !allAvailableTimeSteps().empty() )
    {
        auto min = *allAvailableTimeSteps().begin();
        auto max = *allAvailableTimeSteps().rbegin();

        return { min, max };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimEnsembleCurveSet::selectedTimeStepRange() const
{
    // Scale the slider values to the full time step range

    auto [min, max]  = fullTimeStepRange();
    auto range       = max - min;
    auto selectedMin = min + static_cast<time_t>( range * ( m_minTimeSliderPosition / 100.0 ) );
    auto selectedMax = min + static_cast<time_t>( range * ( m_maxTimeSliderPosition / 100.0 ) );

    return { selectedMin, selectedMax };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::isXAxisSummaryVector() const
{
    return m_xAxisType() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::HorizontalAxisType RimEnsembleCurveSet::xAxisType() const
{
    return m_xAxisType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::findOrAssignBottomAxisX( RiuPlotAxis plotAxis )
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( !plot ) return;

    if ( auto axis = plot->axisPropertiesForPlotAxis( plotAxis ) )
    {
        m_xAddressSelector->setPlotAxisProperties( axis );
    }
    else
    {
        RimPlotAxisProperties* newPlotAxisProperties = plot->addNewAxisProperties( plotAxis, "Bottom Axis" );
        plot->updateConnectedEditors();

        m_xAddressSelector->setPlotAxisProperties( newPlotAxisProperties );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryAddressYAndStatisticsFlag( RifEclipseSummaryAddress address )
{
    setSummaryAddressY( address );
    m_statistics->setShowStatisticsCurves( !address.isHistoryVector() );
    m_statistics->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimEnsembleCurveSet::summaryAddressY() const
{
    return m_yValuesSummaryAddress->address();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveAddress RimEnsembleCurveSet::curveAddress() const
{
    if ( m_xAxisType() == RiaDefines::HorizontalAxisType::TIME )
    {
        return RiaSummaryCurveAddress( RifEclipseSummaryAddress::timeAddress(), summaryAddressY() );
    }

    return RiaSummaryCurveAddress( m_xAddressSelector->summaryAddress(), summaryAddressY() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimEnsembleCurveSet::curves() const
{
    return m_curves.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomObjectiveFunctionCollection* RimEnsembleCurveSet::customObjectiveFunctionCollection()
{
    return m_customObjectiveFunctions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteEnsembleCurves()
{
    std::vector<size_t> curvesIndexesToDelete;
    for ( size_t c = 0; c < m_curves.size(); c++ )
    {
        RimSummaryCurve* curve = m_curves[c];
        if ( curve->summaryAddressY().category() != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS )
            curvesIndexesToDelete.push_back( c );
    }

    while ( !curvesIndexesToDelete.empty() )
    {
        size_t currIndex = curvesIndexesToDelete.back();
        delete m_curves[currIndex];
        m_curves.erase( currIndex );
        curvesIndexesToDelete.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteStatisticsCurves()
{
    std::vector<size_t> curvesIndexesToDelete;
    for ( size_t c = 0; c < m_curves.size(); c++ )
    {
        RimSummaryCurve* curve = m_curves[c];
        if ( curve->summaryAddressY().category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS )
            curvesIndexesToDelete.push_back( c );
    }

    while ( !curvesIndexesToDelete.empty() )
    {
        size_t currIndex = curvesIndexesToDelete.back();
        delete m_curves[currIndex];
        m_curves.erase( currIndex );
        curvesIndexesToDelete.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEnsembleCurveSet::legendConfig()
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDraggableOverlayFrame* RimEnsembleCurveSet::legendFrame() const
{
    return m_legendOverlayFrame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::onLegendDefinitionChanged()
{
    updateCurveColors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryCaseCollection( RimSummaryCaseCollection* sumCaseCollection )
{
    m_yValuesSummaryCaseCollection = sumCaseCollection;
    m_xAddressSelector->setEnsemble( sumCaseCollection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimEnsembleCurveSet::summaryCaseCollection() const
{
    return m_yValuesSummaryCaseCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection* RimEnsembleCurveSet::filterCollection() const
{
    return m_curveFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::ColorMode RimEnsembleCurveSet::colorMode() const
{
    return m_colorMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setColorMode( ColorMode mode )
{
    m_colorMode = mode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setEnsembleParameter( const QString& parameterName )
{
    m_ensembleParameter = parameterName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter::Type RimEnsembleCurveSet::currentEnsembleParameterType() const
{
    if ( m_colorMode() == ColorMode::BY_ENSEMBLE_PARAM )
    {
        RimSummaryCaseCollection* group         = m_yValuesSummaryCaseCollection();
        QString                   parameterName = m_ensembleParameter();

        if ( group && !parameterName.isEmpty() )
        {
            auto eParam = group->ensembleParameter( parameterName );
            return eParam.type;
        }
    }
    return RigEnsembleParameter::TYPE_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAllCurves()
{
    RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();
    RimSummaryAddress*        addr  = m_yValuesSummaryAddress();

    if ( group && addr->address().category() != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID )
    {
        std::vector<RimSummaryCase*> allCases      = group->allSummaryCases();
        std::vector<RimSummaryCase*> filteredCases = filterEnsembleCases( allCases );

        m_isCurveSetFiltered = filteredCases.size() < allCases.size();

        updateEnsembleCurves( filteredCases );
        updateStatisticsCurves( m_statistics->basedOnFilteredCases() ? filteredCases : allCases );
    }

    filterChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEditors()
{
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setTimeSteps( const std::vector<size_t>& timeStepIndices )
{
    m_selectedTimeSteps.v().clear();
    std::set<time_t> timeSteps = allAvailableTimeSteps();
    size_t           index     = 0;
    for ( auto time : timeSteps )
    {
        if ( std::find( timeStepIndices.begin(), timeStepIndices.end(), index++ ) != timeStepIndices.end() )
        {
            m_selectedTimeSteps.v().push_back( RiaQDateTimeTools::fromTime_t( time ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimEnsembleCurveSet::selectedTimeSteps() const
{
    std::vector<time_t> selectedTimeTTimeSteps;
    for ( const QDateTime& dateTime : m_selectedTimeSteps.v() )
    {
        selectedTimeTTimeSteps.push_back( RiaTimeTTools::fromQDateTime( dateTime ) );
    }

    return selectedTimeTTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();

    bool updateTextInPlot = false;

    if ( changedField == &m_showCurves )
    {
        updateConnectedEditors();

        // When multiple ensemble curve sets are toggled on/off, it is required to do a full RimSummaryPlot::loadDataAndUpdate() that will
        // call RimEnsembleCurveSet::updateCurves() on all ensembles in the plot. This can be a heavy operation, but will happen only once.
        // https://github.com/OPM/ResInsight/issues/9956
        plot->loadDataAndUpdate();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_yValuesSummaryAddressUiField )
    {
        m_yValuesSummaryAddress->setAddress( m_yValuesSummaryAddressUiField() );

        updateAllCurves();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_yValuesSummaryCaseCollection )
    {
        updateAllCurves();

        updateTextInPlot = true;
    }
    else if ( &m_xAxisType == changedField )
    {
        if ( m_xAxisType() == RiaDefines::HorizontalAxisType::TIME )
        {
            // TODO: How to handle this?
            // m_xPlotAxisProperties = plot->timeAxisProperties();
        }
        else if ( isXAxisSummaryVector() )
        {
            if ( !m_xAddressSelector->ensemble() )
            {
                m_xAddressSelector->setEnsemble( summaryCaseCollection() );
            }

            if ( !m_xAddressSelector->summaryAddress().isValid() )
            {
                m_xAddressSelector->setAddress( summaryAddressY() );
            }

            findOrAssignBottomAxisX( RiuPlotAxis::defaultBottomForSummaryVectors() );
        }
        plot->updateAxes();
        plot->updatePlotTitle();
        updateAllCurves();
        plot->zoomAll();
    }
    else if ( changedField == &m_resampling || changedField == &m_useCustomAppearance || changedField == &m_lineStyle ||
              changedField == &m_pointSymbol || changedField == &m_symbolSize || changedField == &m_statisticsLineStyle ||
              changedField == &m_statisticsPointSymbol || changedField == &m_statisticsSymbolSize ||
              changedField == &m_statisticsUseCustomAppearance )
    {
        updateAllCurves();
    }
    else if ( changedField == &m_colorForRealizations || changedField == &m_mainEnsembleColor || changedField == &m_colorTransparency )
    {
        setTransparentCurveColor();

        updateCurveColors();

        updateConnectedEditors();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_ensembleParameter )
    {
        updateLegendMappingMode();
        updateCurveColors();
    }
    else if ( changedField == &m_objectiveValuesSummaryAddressesUiField )
    {
        updateAddressesUiField();

        std::vector<size_t> indices;
        indices.push_back( 0 );
        setTimeSteps( indices );

        updateMaxMinAndDefaultValues();
        updateCurveColors();
        updateObjectiveFunctionLegend();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_showObjectiveFunctionFormula )
    {
        updateObjectiveFunctionLegend();
    }
    else if ( changedField == &m_colorMode )
    {
        m_ensembleParameter.uiCapability()->setUiHidden( m_colorMode() != ColorMode::BY_ENSEMBLE_PARAM );

        if ( m_colorMode() == ColorMode::BY_ENSEMBLE_PARAM )
        {
            if ( m_ensembleParameter().isEmpty() )
            {
                auto params         = variationSortedEnsembleParameters();
                m_ensembleParameter = !params.empty() ? params.front().name : "";
            }
        }

        if ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION || m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
        {
            if ( m_objectiveValuesSummaryAddresses.empty() )
            {
                RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                summaryAddress->setAddress( m_yValuesSummaryAddress->address() );
                m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                updateAddressesUiField();
                m_minTimeSliderPosition = 0;
                m_maxTimeSliderPosition = 100;
                updateMaxMinAndDefaultValues();
            }
        }

        setTransparentCurveColor();
        updateCurveColors();
        updateTimeAnnotations();
        updateObjectiveFunctionLegend();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_timeStepFilter )
    {
        m_selectedTimeSteps.v().clear();

        updateConnectedEditors();
    }
    else if ( changedField == &m_selectedTimeSteps )
    {
        updateCurveColors();
        updateTimeAnnotations();
        updateObjectiveFunctionLegend();
        updateMaxMinAndDefaultValues();
    }
    else if ( changedField == &m_minTimeSliderPosition || changedField == &m_maxTimeSliderPosition )
    {
        updateMaxMinAndDefaultValues();
        updateCurveColors();
        updateTimeAnnotations();
    }
    else if ( changedField == &m_minDateRange || changedField == &m_maxDateRange )
    {
        auto [min, max] = fullTimeStepRange();
        auto range      = max - min;

        auto minTime = RiaTimeTTools::fromQDateTime( RiaQDateTimeTools::createDateTime( m_minDateRange() ) );
        minTime      = std::clamp( minTime, min, max );
        auto maxTime = RiaTimeTTools::fromQDateTime( RiaQDateTimeTools::createDateTime( m_maxDateRange() ) );
        maxTime      = std::clamp( maxTime, min, max );

        // Convert from date to normalized value between 0 and 100
        m_minTimeSliderPosition = static_cast<int>( ( double( minTime - min ) / double( range ) ) * 100 );
        m_maxTimeSliderPosition = static_cast<int>( ( double( maxTime - min ) / double( range ) ) * 100 );

        updateCurveColors();
        updateTimeAnnotations();
    }
    else if ( changedField == &m_yPlotAxisProperties )
    {
        for ( RimSummaryCurve* curve : curves() )
        {
            curve->setLeftOrRightAxisY( axisY() );
        }

        updatePlotAxis();
        plot->updateAxes();
        plot->updateAll();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_isUsingAutoName )
    {
        if ( !m_isUsingAutoName )
        {
            m_userDefinedName = createAutoName();
        }

        updateTextInPlot = true;
    }
    else if ( changedField == &m_userDefinedName )
    {
        updateTextInPlot = true;
    }
    else if ( changedField == &m_yPushButtonSelectSummaryAddress )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimSummaryCaseCollection*       candidateEnsemble = m_yValuesSummaryCaseCollection();
        RifEclipseSummaryAddress        candicateAddress  = m_yValuesSummaryAddress->address();

        dlg.hideSummaryCases();
        dlg.setEnsembleAndAddress( candidateEnsemble, candicateAddress );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_yValuesSummaryCaseCollection = curveSelection[0].ensemble();
                m_yValuesSummaryAddress->setAddress( curveSelection[0].summaryAddressY() );

                loadDataAndUpdate( true );

                plot->updateAxes();
                plot->updatePlotTitle();
                plot->updateConnectedEditors();

                RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
                mainPlotWindow->updateMultiPlotToolBar();
            }
        }

        m_yPushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        RimObjectiveFunctionTools::configureDialogForObjectiveFunctions( &dlg );
        RimSummaryCaseCollection* candidateEnsemble = m_yValuesSummaryCaseCollection();

        std::vector<RifEclipseSummaryAddress> candidateAddresses;
        for ( auto address : m_objectiveValuesSummaryAddresses().childrenByType() )
        {
            candidateAddresses.push_back( address->address() );
        }

        dlg.setEnsembleAndAddresses( candidateEnsemble, candidateAddresses );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_objectiveValuesSummaryAddresses.deleteChildren();
                for ( auto address : curveSelection )
                {
                    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                    summaryAddress->setAddress( address.summaryAddressY() );
                    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                }
                loadDataAndUpdate( true );
            }
        }

        m_objectiveValuesSelectSummaryAddressPushButton = false;
    }
    else if ( changedField == &m_customObjectiveFunction )
    {
        if ( m_customObjectiveFunction() )
        {
            if ( m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::F2 ) )
            {
                std::vector<size_t> indices;
                indices.push_back( 0 );
                setTimeSteps( indices );
            }

            m_minTimeSliderPosition = 0;
            m_maxTimeSliderPosition = 100;

            updateLegendMappingMode();
            updateCurveColors();
            updateTimeAnnotations();
            updateObjectiveFunctionLegend();
        }
    }

    if ( updateTextInPlot )
    {
        updateAllTextInPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        treeItemAttribute->tags.clear();
        auto tag     = caf::PdmUiTreeViewItemAttribute::createTag();
        tag->bgColor = RiaColorTools::toQColor( m_colorForRealizations );
        tag->fgColor = RiaColorTools::toQColor( m_statistics->color() );
        tag->text    = "---";

        tag->clicked.connect( this, &RimEnsembleCurveSet::onColorTagClicked );

        treeItemAttribute->tags.push_back( std::move( tag ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    if ( changedChildField == &m_xAddressSelector )
    {
        updateAllCurves();

        // The recommended way to trigger update in a parent object is by using caf::Signal. Here we need to update two parent classes, and
        // they are multiple levels away. To avoid a long signal path that is hard to debug, we use firstAncestorOrThisOfType()

        auto summaryPlot = firstAncestorOrThisOfType<RimSummaryPlot>();
        if ( summaryPlot )
        {
            summaryPlot->updateAll();
            summaryPlot->zoomAll();
        }

        auto multiPlot = firstAncestorOrThisOfType<RimSummaryMultiPlot>();
        if ( multiPlot )
        {
            multiPlot->updatePlotTitles();
        }

        // Trigger update, as the axis object name might have changed. Will update the axis object of the curve set.
        updateConnectedEditors();
    }
    if ( changedChildField == &m_statistics )
    {
        if ( auto summaryPlot = firstAncestorOrThisOfType<RimSummaryPlot>() )
        {
            summaryPlot->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_yValuesSummaryAddressUiField = m_yValuesSummaryAddress->address();

    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );
        curveDataGroup->add( &m_yValuesSummaryCaseCollection );
        curveDataGroup->add( &m_yValuesSummaryAddressUiField );
        curveDataGroup->add( &m_yPushButtonSelectSummaryAddress, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );

        if ( !isXAxisSummaryVector() )
        {
            // Resampling is automatic for cross plot curves
            curveDataGroup->add( &m_resampling );
        }

        curveDataGroup->add( &m_yPlotAxisProperties );
    }

    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector X Axis" );
        curveDataGroup->add( &m_xAxisType );

        if ( isXAxisSummaryVector() )
        {
            m_xAddressSelector->uiOrdering( uiConfigName, *curveDataGroup );
        }
    }

    appendColorGroup( uiOrdering );

    {
        caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
        nameGroup->setCollapsedByDefault();
        nameGroup->add( &m_isUsingAutoName );
        if ( m_isUsingAutoName )
        {
            nameGroup->add( &m_autoGeneratedName );
            m_summaryAddressNameTools->uiOrdering( uiConfigName, *nameGroup );
        }
        else
        {
            nameGroup->add( &m_userDefinedName );
        }
    }

    caf::PdmUiGroup* statGroup           = uiOrdering.addNewGroup( "Statistics" );
    bool             showStatisticsColor = m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR;
    m_statistics->showColorField( showStatisticsColor );

    m_statistics->defaultUiOrdering( isXAxisSummaryVector(), *statGroup );

    caf::PdmUiGroup* statAppearance = statGroup->addNewGroupWithKeyword( "Appearance", "StatisticsAppearance" );
    statAppearance->add( &m_statisticsUseCustomAppearance );
    if ( m_statisticsUseCustomAppearance() == AppearanceMode::CUSTOM )
    {
        statAppearance->add( &m_statisticsLineStyle );
        statAppearance->add( &m_statisticsPointSymbol );
        statAppearance->add( &m_statisticsSymbolSize );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateMaxMinAndDefaultValues()
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();

    m_minDateRange = QDateTime::fromSecsSinceEpoch( minTimeStep ).date();
    m_maxDateRange = QDateTime::fromSecsSinceEpoch( maxTimeStep ).date();

    for ( auto filter : m_curveFilters->filters() )
    {
        filter->updateMaxMinAndDefaultValuesFromParent();
    }

    for ( auto objFunc : m_customObjectiveFunctions->objectiveFunctions() )
    {
        objFunc->clearCache();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimEnsembleCurveFilterCollection* RimEnsembleCurveSet::curveFilters() const
{
    return m_curveFilters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::onCustomObjectiveFunctionChanged( const caf::SignalEmitter* emitter )
{
    updateCurveColors();
    updateFilterLegend();
    updateObjectiveFunctionLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setTransparentCurveColor()
{
    if ( m_colorMode() == RimEnsembleCurveSet::ColorMode::SINGLE_COLOR_WITH_ALPHA )
    {
        auto backgroundColor = RiuGuiTheme::getColorByVariableName( "backgroundColor1" );

        auto sourceColor      = RiaColorTools::toQColor( m_mainEnsembleColor );
        auto sourceWeight     = 100;
        int  backgroundWeight = std::max( 1, static_cast<int>( sourceWeight * 10 * m_colorTransparency ) );
        auto blendedColor     = RiaColorTools::blendQColors( backgroundColor, sourceColor, backgroundWeight, sourceWeight );

        m_colorForRealizations = RiaColorTools::fromQColorTo3f( blendedColor );
        setStatisticsColor( m_mainEnsembleColor );
        updateStatisticsCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RimEnsembleCurveSet::mainEnsembleColor() const
{
    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR_WITH_ALPHA )
    {
        return RiaColorTools::toQColor( m_mainEnsembleColor );
    }

    return RiaColorTools::toQColor( m_colorForRealizations );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::onColorTagClicked( const SignalEmitter* emitter, size_t index )
{
    caf::PdmField<cvf::Color3f>* colorToModify = nullptr;
    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR )
    {
        colorToModify = &m_colorForRealizations;
    }
    else
    {
        colorToModify = &m_mainEnsembleColor;
    }

    if ( colorToModify )
    {
        QColor sourceColor = RiaColorTools::toQColor( *colorToModify );
        QColor newColor    = caf::PdmUiColorEditor::getColor( sourceColor );

        if ( newColor.isValid() && newColor != sourceColor )
        {
            auto myColor = RiaColorTools::fromQColorTo3f( newColor );

            colorToModify->setValueWithFieldChanged( myColor );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::onObjectiveFunctionChanged( const caf::SignalEmitter* emitter )
{
    updateCurveColors();
    updateFilterLegend();
    updateObjectiveFunctionLegend();
    updateTimeAnnotations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::appendColorGroup( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* colorsGroup = uiOrdering.addNewGroup( "Appearance" );
    m_colorMode.uiCapability()->setUiReadOnly( !m_yValuesSummaryCaseCollection() );
    colorsGroup->add( &m_colorMode );

    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR )
    {
        colorsGroup->add( &m_colorForRealizations );
    }
    else if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR_WITH_ALPHA )
    {
        colorsGroup->add( &m_mainEnsembleColor );
        colorsGroup->add( &m_colorTransparency );
    }
    else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        m_ensembleParameter.uiCapability()->setUiReadOnly( !m_yValuesSummaryCaseCollection() );
        colorsGroup->add( &m_ensembleParameterSorting );
        colorsGroup->add( &m_ensembleParameter );
    }
    else if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION || m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION )
        {
            colorsGroup->add( &m_objectiveValuesSummaryAddressesUiField );
            colorsGroup->add( &m_objectiveValuesSelectSummaryAddressPushButton,
                              { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );

            {
                auto equationGroup = colorsGroup->addNewGroup( "Equation" );
                m_objectiveFunction->uiOrdering( "", *equationGroup );
                equationGroup->add( &m_showObjectiveFunctionFormula );
            }
        }
        else
        {
            colorsGroup->add( &m_customObjectiveFunction );
            colorsGroup->add( &m_showObjectiveFunctionFormula );
        }

        {
            auto timeSelectionGroup = colorsGroup->addNewGroup( "Time Selection" );

            if ( ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION &&
                   m_objectiveFunction()->functionType() == RimObjectiveFunction::FunctionType::F1 ) ||
                 ( m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() &&
                   m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::F1 ) ) )
            {
                timeSelectionGroup->add( &m_minDateRange );
                timeSelectionGroup->add( &m_minTimeSliderPosition );
                timeSelectionGroup->add( &m_maxDateRange );
                timeSelectionGroup->add( &m_maxTimeSliderPosition );
            }
            if ( m_objectiveFunction()->functionType() == RimObjectiveFunction::FunctionType::F2 ||
                 ( m_customObjectiveFunction() &&
                   m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::F2 ) ) )
            {
                timeSelectionGroup->add( &m_timeStepFilter );
                timeSelectionGroup->add( &m_selectedTimeSteps );
            }
        }
    }

    colorsGroup->add( &m_useCustomAppearance );
    if ( m_useCustomAppearance() == AppearanceMode::CUSTOM )
    {
        colorsGroup->add( &m_lineStyle );
        colorsGroup->add( &m_pointSymbol );
        colorsGroup->add( &m_symbolSize );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        uiTreeOrdering.add( m_legendConfig() );
    }

    if ( uiConfigName != RicSummaryPlotEditorUi::CONFIGURATION_NAME )
    {
        for ( auto filter : m_curveFilters->filters() )
        {
            uiTreeOrdering.add( filter );
        }

        uiTreeOrdering.add( m_customObjectiveFunctions() );
    }

    uiTreeOrdering.skipRemainingChildren( true );

    caf::IconProvider iconProvider = uiIconProvider();
    if ( !iconProvider.valid() ) return;

    auto coll = firstAncestorOrThisOfType<RimEnsembleCurveSetCollection>();
    if ( coll && coll->curveSetForSourceStepping() == this )
    {
        iconProvider.setOverlayResourceString( ":/StepUpDownCorner16x16.png" );
    }

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveSet::userDescriptionField()
{
    if ( m_isUsingAutoName )
    {
        return &m_autoGeneratedName;
    }
    else
    {
        return &m_userDefinedName;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveSet::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) )
    {
        attrib->m_buttonText = "...";
    }

    if ( field == &m_minTimeSliderPosition || field == &m_maxTimeSliderPosition )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum     = 0;
            myAttr->m_maximum     = 100;
            myAttr->m_showSpinBox = false;
        }
    }
    if ( field == &m_colorTransparency )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            myAttr->m_minimum  = 0.001;
            myAttr->m_maximum  = 1.0;
            myAttr->m_decimals = 2;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveSet::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_yValuesSummaryCaseCollection )
    {
        RimProject*                            proj   = RimProject::current();
        std::vector<RimSummaryCaseCollection*> groups = proj->summaryGroups();

        for ( RimSummaryCaseCollection* group : groups )
        {
            if ( group->isEnsemble() ) options.push_back( caf::PdmOptionItemInfo( group->name(), group ) );
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    else if ( fieldNeedingOptions == &m_colorMode )
    {
        auto singleColorOption          = ColorModeEnum( ColorMode::SINGLE_COLOR );
        auto singleColorWithAlphaOption = ColorModeEnum( ColorMode::SINGLE_COLOR_WITH_ALPHA );
        auto byEnsParamOption           = ColorModeEnum( ColorMode::BY_ENSEMBLE_PARAM );
        auto byObjFuncOption            = ColorModeEnum( ColorMode::BY_OBJECTIVE_FUNCTION );
        auto byCustomObjFuncOption      = ColorModeEnum( ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION );

        options.push_back( caf::PdmOptionItemInfo( singleColorOption.uiText(), ColorMode::SINGLE_COLOR ) );
        options.push_back( caf::PdmOptionItemInfo( singleColorWithAlphaOption.uiText(), ColorMode::SINGLE_COLOR_WITH_ALPHA ) );

        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();
        if ( group && group->hasEnsembleParameters() )
        {
            options.push_back( caf::PdmOptionItemInfo( byEnsParamOption.uiText(), ColorMode::BY_ENSEMBLE_PARAM ) );
        }
        options.push_back( caf::PdmOptionItemInfo( byObjFuncOption.uiText(), ColorMode::BY_OBJECTIVE_FUNCTION ) );
        options.push_back( caf::PdmOptionItemInfo( byCustomObjFuncOption.uiText(), ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION ) );
    }
    else if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        auto params = ensembleParameters( m_ensembleParameterSorting() );
        for ( const auto& paramCorrPair : params )
        {
            QString name = paramCorrPair.first.name;
            double  corr = paramCorrPair.second;
            options.push_back( caf::PdmOptionItemInfo( QString( "%1 (Avg. correlation: %2)" ).arg( name ).arg( corr, 5, 'f', 2 ), name ) );
        }
    }
    else if ( fieldNeedingOptions == &m_yValuesSummaryAddressUiField )
    {
        appendOptionItemsForSummaryAddresses( &options, m_yValuesSummaryCaseCollection() );
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        std::set<time_t>    allTimeSteps = allAvailableTimeSteps();
        std::set<QDateTime> currentlySelectedTimeSteps( m_selectedTimeSteps().begin(), m_selectedTimeSteps().end() );

        if ( allTimeSteps.empty() )
        {
            return options;
        }

        std::set<int>          currentlySelectedTimeStepIndices;
        std::vector<QDateTime> allDateTimes;
        for ( time_t timeStep : allTimeSteps )
        {
            QDateTime dateTime = RiaQDateTimeTools::fromTime_t( timeStep );
            if ( currentlySelectedTimeSteps.count( dateTime ) )
            {
                currentlySelectedTimeStepIndices.insert( (int)allDateTimes.size() );
            }
            allDateTimes.push_back( dateTime );
        }

        std::vector<int> filteredTimeStepIndices =
            RimTimeStepFilter::filteredTimeStepIndices( allDateTimes, 0, (int)allDateTimes.size() - 1, m_timeStepFilter(), 1 );

        // Add existing time steps to list of options to avoid removing them when changing filter.
        filteredTimeStepIndices.insert( filteredTimeStepIndices.end(),
                                        currentlySelectedTimeStepIndices.begin(),
                                        currentlySelectedTimeStepIndices.end() );
        std::sort( filteredTimeStepIndices.begin(), filteredTimeStepIndices.end() );
        filteredTimeStepIndices.erase( std::unique( filteredTimeStepIndices.begin(), filteredTimeStepIndices.end() ),
                                       filteredTimeStepIndices.end() );

        QString dateFormatString     = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                        RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
        QString timeFormatString     = RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                                                        RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );
        QString dateTimeFormatString = QString( "%1 %2" ).arg( dateFormatString ).arg( timeFormatString );

        bool showTime = m_timeStepFilter() == RimTimeStepFilter::TS_ALL || m_timeStepFilter() == RimTimeStepFilter::TS_INTERVAL_DAYS;

        for ( auto timeStepIndex : filteredTimeStepIndices )
        {
            QDateTime dateTime = allDateTimes[timeStepIndex];

            if ( showTime && dateTime.time() != QTime( 0, 0, 0 ) )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime, dateTimeFormatString ), dateTime ) );
            }
            else
            {
                options.push_back(
                    caf::PdmOptionItemInfo( RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime, dateFormatString ), dateTime ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_customObjectiveFunction )
    {
        auto functions = m_customObjectiveFunctions()->objectiveFunctions();
        for ( const auto& objFunc : functions )
        {
            QString name = objFunc->title();
            options.push_back( caf::PdmOptionItemInfo( name, objFunc ) );
        }
    }
    else if ( fieldNeedingOptions == &m_yPlotAxisProperties )
    {
        auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::VERTICAL ) )
        {
            options.push_back( caf::PdmOptionItemInfo( axis->objectName(), axis ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimEnsembleCurveSet::allAvailableTimeSteps() const
{
    std::set<time_t> timeStepUnion;

    for ( RimSummaryCase* sumCase : summaryCaseCollection()->allSummaryCases() )
    {
        if ( sumCase->summaryReader() )
        {
            std::vector<time_t> timeSteps;
            for ( auto address : m_objectiveValuesSummaryAddresses() )
            {
                for ( auto timeStep : sumCase->summaryReader()->timeSteps( address->address() ) )
                {
                    timeSteps.push_back( timeStep );
                }
            }

            for ( time_t t : timeSteps )
            {
                timeStepUnion.insert( t );
            }
        }
    }
    return timeStepUnion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimEnsembleCurveSet::timestepDefiningSourceCases()
{
    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();
    std::set<RimSummaryCase*>          timeStepDefiningSumCases    = analyserOfSelectedCurveDefs->m_singleSummaryCases;
    for ( auto ensemble : analyserOfSelectedCurveDefs->m_ensembles )
    {
        auto allSumCases = ensemble->allSummaryCases();
        timeStepDefiningSumCases.insert( allSumCases.begin(), allSumCases.end() );
    }

    return timeStepDefiningSumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinitionAnalyser* RimEnsembleCurveSet::getOrCreateSelectedCurveDefAnalyser()
{
    if ( !m_analyserOfSelectedCurveDefs )
    {
        m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>( new RiaSummaryCurveDefinitionAnalyser );
    }
    m_analyserOfSelectedCurveDefs->setCurveDefinitions( curveDefinitions() );
    return m_analyserOfSelectedCurveDefs.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimEnsembleCurveSet::curveDefinitions() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;
    for ( auto dataEntry : m_curves() )
    {
        curveDefs.push_back( dataEntry->curveDefinition() );
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// Optimization candidate
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options,
                                                                RimSummaryCaseCollection*      summaryCaseGroup )
{
    if ( !summaryCaseGroup ) return;

    std::set<RifEclipseSummaryAddress> addrSet;
    for ( RimSummaryCase* summaryCase : summaryCaseGroup->allSummaryCases() )
    {
        RifSummaryReaderInterface*                reader = summaryCase->summaryReader();
        const std::set<RifEclipseSummaryAddress>& addrs  = reader ? reader->allResultAddresses() : std::set<RifEclipseSummaryAddress>();

        for ( auto& addr : addrs )
        {
            addrSet.insert( addr );
        }
    }

    for ( auto& addr : addrSet )
    {
        std::string name = addr.uiText();
        QString     s    = QString::fromStdString( name );
        options->push_back( caf::PdmOptionItemInfo( s, QVariant::fromValue( addr ) ) );
    }

    options->push_front( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateFilterLegend()
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot && plot->plotWidget() )
    {
        if ( m_curveFilters()->isActive() && m_curveFilters()->countActiveFilters() > 0 )
        {
            if ( !m_filterOverlayFrame )
            {
                m_filterOverlayFrame = new RiuDraggableOverlayFrame( plot->plotWidget(), plot->plotWidget()->overlayMargins() );
            }
            m_filterOverlayFrame->setContentFrame( m_curveFilters()->makeFilterDescriptionFrame() );
            plot->plotWidget()->addOverlayFrame( m_filterOverlayFrame );
        }
        else
        {
            if ( m_filterOverlayFrame )
            {
                plot->plotWidget()->removeOverlayFrame( m_filterOverlayFrame );
            }
        }
        plot->scheduleReplotIfVisible();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateObjectiveFunctionLegend()
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot && plot->plotWidget() )
    {
        if ( ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION || m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION ) &&
             m_showObjectiveFunctionFormula() )
        {
            if ( !m_objectiveFunctionOverlayFrame )
            {
                m_objectiveFunctionOverlayFrame = new RiuDraggableOverlayFrame( plot->plotWidget(), plot->plotWidget()->overlayMargins() );
            }
            QString title;
            QString description;
            if ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION )
            {
                std::vector<RifEclipseSummaryAddress> addresses;
                for ( auto address : m_objectiveValuesSummaryAddresses().childrenByType() )
                {
                    addresses.push_back( address->address() );
                }

                title = "Objective Function";
                description =
                    QString( "%0::%1" ).arg( m_objectiveFunction()->shortName() ).arg( m_objectiveFunction()->formulaString( addresses ) );
            }
            else if ( m_colorMode() == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() )
            {
                std::vector<RifEclipseSummaryAddress> addresses;
                for ( auto address : m_objectiveValuesSummaryAddresses().childrenByType() )
                {
                    addresses.push_back( address->address() );
                }

                title       = "Custom Objective Function";
                description = m_customObjectiveFunction()->formulaString( addresses );
            }
            if ( !title.isEmpty() && !description.isEmpty() )
            {
                m_objectiveFunctionOverlayFrame->setContentFrame( new RiuTextContentFrame( nullptr, title, description, -1 ) );
                m_objectiveFunctionOverlayFrame->setMaximumWidth( 10000 );
                plot->plotWidget()->addOverlayFrame( m_objectiveFunctionOverlayFrame );
            }
        }
        else
        {
            if ( m_objectiveFunctionOverlayFrame )
            {
                plot->plotWidget()->removeOverlayFrame( m_objectiveFunctionOverlayFrame );
            }
        }
    }
    plot->scheduleReplotIfVisible();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectiveFunctionTimeConfig RimEnsembleCurveSet::objectiveFunctionTimeConfig() const
{
    auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();
    return { minTimeStep, maxTimeStep, selectedTimeSteps() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateLegendTitle()
{
    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        QString parameterName = m_ensembleParameter();

        QString legendTitle = "Ensemble Parameter";
        legendTitle += "\n";
        legendTitle += parameterName;

        m_legendConfig->setTitle( legendTitle );
    }
    else if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION )
    {
        QString legendTitle = "Objective Function";

        legendTitle += "\n";
        legendTitle += caf::AppEnum<RimObjectiveFunction::FunctionType>( m_objectiveFunction()->functionType() ).uiText();

        m_legendConfig->setTitle( legendTitle );
    }
    else if ( m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        QString legendTitle = "Custom\nObjective Function";
        legendTitle += "\n";
        if ( m_customObjectiveFunction() && m_customObjectiveFunction()->isValid() )
        {
            QString descriptions = m_customObjectiveFunction()->title();
            descriptions.truncate( 30 );
            legendTitle += descriptions;
            if ( m_customObjectiveFunction()->title().length() > descriptions.length() )
            {
                legendTitle += "...";
            }
        }
        else
        {
            legendTitle += "(Invalid Objective Function)";
        }

        m_legendConfig->setTitle( legendTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Color3f> RimEnsembleCurveSet::generateColorsForCases( const std::vector<RimSummaryCase*>& summaryCases ) const
{
    std::vector<cvf::Color3f> caseColors;

    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();

        QString parameterName = m_ensembleParameter();

        if ( group && !parameterName.isEmpty() && !group->allSummaryCases().empty() )
        {
            auto ensembleParam = group->ensembleParameter( parameterName );
            if ( ensembleParam.isText() || ensembleParam.isNumeric() )
            {
                RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig, ensembleParam );
                for ( auto& rimCase : summaryCases )
                {
                    caseColors.push_back( RimEnsembleCurveSetColorManager::caseColor( m_legendConfig, rimCase, ensembleParam ) );
                }
            }
        }
    }
    else if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( m_colorMode() ) )
    {
        caseColors.resize( summaryCases.size(), m_colorForRealizations );
    }
    else if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION )
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();

        if ( group && !group->allSummaryCases().empty() )
        {
            auto                                  objectiveFunction = m_objectiveFunction();
            std::vector<RifEclipseSummaryAddress> summaryAddresses;
            for ( auto address : m_objectiveValuesSummaryAddresses() )
            {
                summaryAddresses.push_back( address->address() );
            }

            RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig,
                                                                     objectiveFunction,
                                                                     group->allSummaryCases(),
                                                                     summaryAddresses,
                                                                     objectiveFunctionTimeConfig() );
            for ( auto& rimCase : summaryCases )
            {
                cvf::Color3f curveColor = RimEnsembleCurveSetColorManager::caseColor( m_legendConfig,
                                                                                      rimCase,
                                                                                      objectiveFunction,
                                                                                      summaryAddresses,
                                                                                      objectiveFunctionTimeConfig() );
                caseColors.push_back( curveColor );
            }
        }
    }
    else if ( m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();

        if ( group && !group->allSummaryCases().empty() && m_customObjectiveFunction() && m_customObjectiveFunction->isValid() )
        {
            RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig, m_customObjectiveFunction() );
            for ( auto& rimCase : summaryCases )
            {
                cvf::Color3f curveColor = RimEnsembleCurveSetColorManager::caseColor( m_legendConfig, rimCase, m_customObjectiveFunction() );
                caseColors.push_back( curveColor );
            }
        }
    }

    return caseColors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateCurveColors()
{
    updateLegendTitle();

    // Find the curves to color (skip the statistics)
    std::vector<RimSummaryCurve*> curvesToColor;
    std::vector<RimSummaryCase*>  summaryCases;
    for ( auto& curve : m_curves )
    {
        if ( curve->summaryAddressY().category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_ENSEMBLE_STATISTICS )
            continue;

        curvesToColor.push_back( curve );
        summaryCases.push_back( curve->summaryCaseY() );
    }

    // Get the colors
    std::vector<cvf::Color3f> caseColors = generateColorsForCases( summaryCases );

    // Apply the colors
    if ( caseColors.size() != curvesToColor.size() ) return;
    for ( size_t i = 0; i < curvesToColor.size(); i++ )
    {
        curvesToColor[i]->setColor( caseColors[i] );
        curvesToColor[i]->updateCurveAppearance();
    }

    if ( m_plotCurveForLegendText )
    {
        m_plotCurveForLegendText->setColor( mainEnsembleColor() );
    }

    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot && plot->plotWidget() )
    {
        if ( m_yValuesSummaryCaseCollection() && isCurvesVisible() &&
             !RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( m_colorMode() ) && m_legendConfig->showLegend() )
        {
            if ( !m_legendOverlayFrame )
            {
                m_legendOverlayFrame =
                    new RiuDraggableOverlayFrame( plot->plotWidget()->getParentForOverlay(), plot->plotWidget()->overlayMargins() );
            }
            m_legendOverlayFrame->setContentFrame( m_legendConfig->makeLegendFrame() );
            plot->plotWidget()->addOverlayFrame( m_legendOverlayFrame );
        }
        else
        {
            if ( m_legendOverlayFrame )
            {
                plot->plotWidget()->removeOverlayFrame( m_legendOverlayFrame );
            }
        }
        plot->scheduleReplotIfVisible();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateTimeAnnotations()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    plot->removeAllTimeAnnotations();

    if ( ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION &&
           m_objectiveFunction()->functionType() == RimObjectiveFunction::FunctionType::F1 ) ||
         ( m_colorMode() == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() &&
           m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::F1 ) ) )
    {
        auto [minTimeStep, maxTimeStep] = selectedTimeStepRange();
        plot->addTimeRangeAnnotation( minTimeStep, maxTimeStep );
    }

    if ( ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION &&
           m_objectiveFunction()->functionType() == RimObjectiveFunction::FunctionType::F2 ) ||
         ( m_colorMode() == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() &&
           m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::F2 ) ) )
    {
        for ( QDateTime timeStep : m_selectedTimeSteps() )
        {
            plot->addTimeAnnotation( RiaTimeTTools::fromQDateTime( timeStep ) );
        }
    }
    plot->updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAddressesUiField()
{
    std::vector<RifEclipseSummaryAddress> addressVector;
    for ( RimSummaryAddress* address : m_objectiveValuesSummaryAddresses )
    {
        addressVector.push_back( address->address() );
    }
    m_objectiveValuesSummaryAddressesUiField = QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updatePlotAxis()
{
    for ( RimSummaryCurve* curve : curves() )
    {
        curve->updatePlotAxis();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEnsembleCurves( const std::vector<RimSummaryCase*>& sumCases )
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();

    deleteEnsembleCurves();
    deleteStatisticsCurves();

    if ( m_statistics->hideEnsembleCurves() ) return;

    RimSummaryAddress* addr = m_yValuesSummaryAddress();
    if ( plot && addr->address().category() != RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID )
    {
        if ( isCurvesVisible() )
        {
            std::vector<RimSummaryCurve*> newSummaryCurves;

            for ( auto& sumCase : sumCases )
            {
                RimSummaryCurve* curve = new RimSummaryCurve();
                curve->setSummaryCaseY( sumCase );
                curve->setSummaryAddressYAndApplyInterpolation( addr->address() );
                curve->setResampling( m_resampling() );

                int lineThickness = 1;
                if ( addr->address().isHistoryVector() )
                {
                    lineThickness = 2;
                    curve->setCurveAppearanceFromCaseType();
                }
                curve->setLineThickness( lineThickness );

                if ( m_useCustomAppearance() == AppearanceMode::CUSTOM )
                {
                    curve->setLineStyle( m_lineStyle() );
                    curve->setSymbol( m_pointSymbol() );
                    curve->setSymbolSize( m_symbolSize() );
                }

                addCurve( curve );

                curve->setLeftOrRightAxisY( axisY() );
                if ( isXAxisSummaryVector() )
                {
                    curve->setAxisTypeX( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR );
                    curve->setSummaryCaseX( sumCase );
                    curve->setSummaryAddressX( m_xAddressSelector->summaryAddress() );
                    if ( m_xAddressSelector->plotAxisProperties() )
                        curve->setTopOrBottomAxisX( m_xAddressSelector->plotAxisProperties()->plotAxis() );
                }

                newSummaryCurves.push_back( curve );
            }

#pragma omp parallel for
            for ( int i = 0; i < (int)newSummaryCurves.size(); ++i )
            {
                newSummaryCurves[i]->valuesX();
            }

            for ( int i = 0; i < (int)newSummaryCurves.size(); ++i )
            {
                newSummaryCurves[i]->loadDataAndUpdate( false );
                newSummaryCurves[i]->updatePlotAxis();
                newSummaryCurves[i]->setShowInLegend( false );
            }
        }

        if ( plot->plotWidget() )
        {
            if ( plot->legendsVisible() ) plot->plotWidget()->updateLegend();
            plot->scheduleReplotIfVisible();
            plot->updateAxes();
            plot->updatePlotInfoLabel();

            if ( !m_plotCurveForLegendText )
            {
                m_plotCurveForLegendText.reset( plot->plotWidget()->createPlotCurve( nullptr, "" ) );

                int curveThickness = 3;
                m_plotCurveForLegendText->setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID,
                                                         RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT,
                                                         curveThickness,
                                                         RiaColorTools::toQColor( m_mainEnsembleColor() ) );
            }
            m_plotCurveForLegendText->attachToPlot( plot->plotWidget() );
            updateEnsembleLegendItem();
        }
    }
    updateCurveColors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateStatisticsCurves( const std::vector<RimSummaryCase*>& sumCases )
{
    using SAddr = RifEclipseSummaryAddress;

    RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();
    RimSummaryAddress*        addr  = m_yValuesSummaryAddress();

    if ( m_disableStatisticCurves || !group || addr->address().category() == RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_INVALID )
        return;

    // Calculate
    {
        std::vector<RimSummaryCase*> statCases = sumCases;
        if ( statCases.empty() )
        {
            if ( m_statistics->basedOnFilteredCases() )
                statCases = filterEnsembleCases( group->allSummaryCases() );
            else
                statCases = group->allSummaryCases();
        }

        if ( isXAxisSummaryVector() )
        {
            m_ensembleStatCaseXY->calculate( statCases,
                                             m_xAddressSelector->summaryAddress(),
                                             summaryAddressY(),
                                             m_statistics->includeIncompleteCurves(),
                                             m_statistics->crossPlotCurvesBinCount(),
                                             m_statistics->crossPlotRealizationCountThresholdPerBin() );
        }
        else
        {
            m_ensembleStatCaseY->calculate( statCases, summaryAddressY(), m_statistics->includeIncompleteCurves() );
        }
    }

    std::vector<RiaSummaryCurveAddress> addresses;
    if ( m_statistics->isActive() )
    {
        if ( isXAxisSummaryVector() )
        {
            RifEclipseSummaryAddress dataAddressY = m_yValuesSummaryAddress->address();
            RifEclipseSummaryAddress dataAddressX = m_xAddressSelector->summaryAddress();

            auto getStatisticsAddress = []( const std::string&              statisticsVectorName,
                                            const RifEclipseSummaryAddress& addrX,
                                            const RifEclipseSummaryAddress& addrY ) -> RiaSummaryCurveAddress
            {
                auto xStatAddress = SAddr::ensembleStatisticsAddress( statisticsVectorName, addrX.vectorName() );
                auto yStatAddress = SAddr::ensembleStatisticsAddress( statisticsVectorName, addrY.vectorName() );

                return RiaSummaryCurveAddress( xStatAddress, yStatAddress );
            };

            if ( m_statistics->showP10Curve() && m_ensembleStatCaseXY->hasP10Data() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameP10(), dataAddressX, dataAddressY ) );
            if ( m_statistics->showP50Curve() && m_ensembleStatCaseXY->hasP50Data() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameP50(), dataAddressX, dataAddressY ) );
            if ( m_statistics->showP90Curve() && m_ensembleStatCaseXY->hasP90Data() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameP90(), dataAddressX, dataAddressY ) );
            if ( m_statistics->showMeanCurve() && m_ensembleStatCaseXY->hasMeanData() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameMean(), dataAddressX, dataAddressY ) );
        }
        else
        {
            RifEclipseSummaryAddress dataAddressY = m_yValuesSummaryAddress->address();

            auto getStatisticsAddress = []( const std::string& statisticsVectorName, const RifEclipseSummaryAddress& addrY ) -> RiaSummaryCurveAddress
            {
                auto xStatAddress = RifEclipseSummaryAddress::timeAddress();
                auto yStatAddress = SAddr::ensembleStatisticsAddress( statisticsVectorName, addrY.vectorName() );

                return RiaSummaryCurveAddress( xStatAddress, yStatAddress );
            };

            if ( m_statistics->showP10Curve() && m_ensembleStatCaseY->hasP10Data() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameP10(), dataAddressY ) );
            if ( m_statistics->showP50Curve() && m_ensembleStatCaseY->hasP50Data() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameP50(), dataAddressY ) );
            if ( m_statistics->showP90Curve() && m_ensembleStatCaseY->hasP90Data() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameP90(), dataAddressY ) );
            if ( m_statistics->showMeanCurve() && m_ensembleStatCaseY->hasMeanData() )
                addresses.push_back( getStatisticsAddress( RifEclipseSummaryAddressDefines::statisticsNameMean(), dataAddressY ) );
        }
    }

    deleteStatisticsCurves();

    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
    if ( plot && plot->plotWidget() )
    {
        RimSummaryCase* summaryCase = nullptr;

        if ( isXAxisSummaryVector() )
            summaryCase = m_ensembleStatCaseXY.get();
        else
            summaryCase = m_ensembleStatCaseY.get();

        for ( const auto& address : addresses )
        {
            auto curve = new RimSummaryCurve();
            curve->setParentPlotNoReplot( plot->plotWidget() );
            m_curves.push_back( curve );
            curve->setColor( m_statistics->color() );
            curve->setResampling( m_resampling() );

            curve->setCheckState( isCurvesVisible() );

            if ( m_statisticsUseCustomAppearance() == AppearanceMode::DEFAULT )
            {
                auto symbol = statisticsCurveSymbolFromAddress( address.summaryAddressY() );
                curve->setSymbol( symbol );
                curve->setSymbolSize( statisticsCurveSymbolSize( symbol ) );
                curve->setSymbolSkipDistance( 150 );

                if ( m_statistics->showCurveLabels() )
                {
                    curve->setSymbolLabel( QString::fromStdString( address.summaryAddressY().ensembleStatisticsVectorName() ) );
                }
                curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
            }
            else
            {
                curve->setLineStyle( m_statisticsLineStyle() );
                curve->setSymbol( m_statisticsPointSymbol() );
                curve->setSymbolSize( m_statisticsSymbolSize() );
            }

            curve->setSummaryCaseY( summaryCase );
            curve->setSummaryAddressYAndApplyInterpolation( address.summaryAddressY() );
            curve->setLeftOrRightAxisY( axisY() );

            if ( isXAxisSummaryVector() )
            {
                curve->setAxisTypeX( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR );
                curve->setSummaryCaseX( summaryCase );
                curve->setSummaryAddressX( address.summaryAddressX() );
                if ( m_xAddressSelector->plotAxisProperties() )
                    curve->setTopOrBottomAxisX( m_xAddressSelector->plotAxisProperties()->plotAxis() );
            }

            curve->setShowInLegend( m_statistics->showStatisticsCurveLegends() );

            curve->updateCurveVisibility();
            curve->loadDataAndUpdate( false );
            curve->updatePlotAxis();
        }

        plot->plotWidget()->updateLegend();
        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateStatisticsCurves()
{
    updateStatisticsCurves( std::vector<RimSummaryCase*>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveSet::clone() const
{
    RimEnsembleCurveSet* copy =
        dynamic_cast<RimEnsembleCurveSet*>( xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    copy->setSummaryCaseCollection( m_yValuesSummaryCaseCollection() );

    // Update summary case references
    for ( size_t i = 0; i < m_curves.size(); i++ )
    {
        copy->m_curves[i]->setSummaryCaseY( m_curves[i]->summaryCaseY() );
    }
    return copy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::showCurves( bool show )
{
    m_showCurves = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAllTextInPlot()
{
    updateEnsembleLegendItem();

    auto summaryPlot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    if ( summaryPlot->plotWidget() )
    {
        summaryPlot->updatePlotTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEnsembleParameter> RimEnsembleCurveSet::variationSortedEnsembleParameters() const
{
    RimSummaryCaseCollection* ensemble = m_yValuesSummaryCaseCollection;
    if ( ensemble )
    {
        return ensemble->variationSortedEnsembleParameters();
    }
    else
    {
        return std::vector<RigEnsembleParameter>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigEnsembleParameter, double>> RimEnsembleCurveSet::ensembleParameters( ParameterSorting sortingMode ) const
{
    RimSummaryCaseCollection* ensemble = m_yValuesSummaryCaseCollection;
    if ( ensemble )
    {
        if ( sortingMode == ParameterSorting::ABSOLUTE_VALUE )
        {
            return ensemble->correlationSortedEnsembleParameters( summaryAddressY() );
        }

        if ( sortingMode == ParameterSorting::ALPHABETICALLY )
        {
            auto parameters = ensemble->parameterCorrelationsAllTimeSteps( summaryAddressY() );
            std::sort( parameters.begin(),
                       parameters.end(),
                       []( const auto& lhs, const auto& rhs ) { return lhs.first.name < rhs.first.name; } );

            return parameters;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleCurveSet::filterEnsembleCases( const std::vector<RimSummaryCase*>& sumCases )
{
    auto filteredCases = sumCases;

    for ( auto& filter : m_curveFilters->filters() )
    {
        filteredCases = filter->applyFilter( filteredCases );
    }
    return filteredCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::disableStatisticCurves()
{
    m_disableStatisticCurves = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::isFiltered() const
{
    return m_isCurveSetFiltered;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP10Data() const
{
    if ( isXAxisSummaryVector() ) return m_ensembleStatCaseXY->hasP10Data();

    return m_ensembleStatCaseY->hasP10Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP50Data() const
{
    if ( isXAxisSummaryVector() ) return m_ensembleStatCaseXY->hasP50Data();

    return m_ensembleStatCaseY->hasP50Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP90Data() const
{
    if ( isXAxisSummaryVector() ) return m_ensembleStatCaseXY->hasP90Data();

    return m_ensembleStatCaseY->hasP90Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasMeanData() const
{
    if ( isXAxisSummaryVector() ) return m_ensembleStatCaseXY->hasMeanData();

    return m_ensembleStatCaseY->hasMeanData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimEnsembleStatistics* RimEnsembleCurveSet::statisticsOptions() const
{
    return m_statistics();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEnsembleLegendItem()
{
    if ( !m_plotCurveForLegendText ) return;

    m_plotCurveForLegendText->setTitle( name() );

    RiuPlotCurveSymbol* symbol = m_plotCurveForLegendText->createSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );

    if ( RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( m_colorMode() ) )
    {
        QColor curveColor = mainEnsembleColor();
        QPen   curvePen( curveColor );
        curvePen.setWidth( 2 );

        symbol->setPen( curvePen );
        symbol->setSize( 6, 6 );
    }
    else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        QPixmap p = QPixmap( ":/Legend.png" );
        symbol->setPixmap( p );
        symbol->setSize( 8, 8 );
    }

    m_plotCurveForLegendText->setSymbol( symbol );

    bool showLegendItem = isCurvesVisible();
    m_plotCurveForLegendText->setVisibleInLegend( showLegendItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEnsembleCurveSet::ensembleId() const
{
    if ( m_yValuesSummaryCaseCollection() != nullptr ) return m_yValuesSummaryCaseCollection()->ensembleId();

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveSet::name() const
{
    QString curveSetName;
    if ( m_isUsingAutoName )
    {
        curveSetName = m_autoGeneratedName();
    }
    else
    {
        curveSetName += m_userDefinedName();
    }

    return curveSetName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveSet::createAutoName() const
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();

    QString curveSetName = m_summaryAddressNameTools->curveName( curveAddress(), plot->plotTitleHelper(), plot->plotTitleHelper() );

    if ( curveSetName.isEmpty() )
    {
        curveSetName = "Name Placeholder";
    }

    return curveSetName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateLegendMappingMode()
{
    switch ( currentEnsembleParameterType() )
    {
        case RigEnsembleParameter::TYPE_TEXT:
            if ( m_legendConfig->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
                m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER );
            break;

        case RigEnsembleParameter::TYPE_NUMERIC:
            if ( m_legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
                m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveSymbol::PointSymbolEnum statisticsCurveSymbolFromAddress( const RifEclipseSummaryAddress& address )
{
    auto qName = address.vectorName();

    if ( qName.find( RifEclipseSummaryAddressDefines::statisticsNameP10() ) != std::string::npos )
        return RiuPlotCurveSymbol::SYMBOL_DOWN_TRIANGLE;
    if ( qName.find( RifEclipseSummaryAddressDefines::statisticsNameP50() ) != std::string::npos )
        return RiuPlotCurveSymbol::SYMBOL_DIAMOND;
    if ( qName.find( RifEclipseSummaryAddressDefines::statisticsNameP90() ) != std::string::npos )
        return RiuPlotCurveSymbol::SYMBOL_TRIANGLE;

    return RiuPlotCurveSymbol::SYMBOL_ELLIPSE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int statisticsCurveSymbolSize( RiuPlotCurveSymbol::PointSymbolEnum symbol )
{
    if ( symbol == RiuPlotCurveSymbol::SYMBOL_DIAMOND ) return 8;
    if ( symbol == RiuPlotCurveSymbol::SYMBOL_TRIANGLE ) return 7;
    if ( symbol == RiuPlotCurveSymbol::SYMBOL_DOWN_TRIANGLE ) return 7;
    return 6;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimEnsembleCurveSet::axisY() const
{
    if ( m_yPlotAxisProperties )
        return m_yPlotAxisProperties->plotAxis();
    else
        return RiuPlotAxis::defaultLeft();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimEnsembleCurveSet::axisX() const
{
    if ( m_xAddressSelector->plotAxisProperties() )
        return m_xAddressSelector->plotAxisProperties()->plotAxis();
    else
        return RiuPlotAxis::defaultBottomForSummaryVectors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setLeftOrRightAxisY( RiuPlotAxis plotAxis )
{
    auto plot             = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    m_yPlotAxisProperties = plot->axisPropertiesForPlotAxis( plotAxis );

    for ( RimSummaryCurve* curve : curves() )
    {
        curve->setLeftOrRightAxisY( axisY() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setBottomOrTopAxisX( RiuPlotAxis plotAxis )
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();
    m_xAddressSelector->setPlotAxisProperties( plot->axisPropertiesForPlotAxis( plotAxis ) );

    for ( RimSummaryCurve* curve : curves() )
    {
        curve->setTopOrBottomAxisX( axisX() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::initAfterRead()
{
    if ( m_yPlotAxisProperties.value() == nullptr )
    {
        auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();
        if ( plot )
        {
            m_yPlotAxisProperties = plot->axisPropertiesForPlotAxis( RiuPlotAxis( m_plotAxis_OBSOLETE() ) );
        }
    }
}
