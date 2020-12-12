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
#include "RiaStatisticsTools.h"
#include "RiaSummaryCurveAnalyzer.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaTimeTTools.h"

#include "SummaryPlotCommands/RicSummaryPlotEditorUi.h"

#include "RifEnsembleStatisticsReader.h"
#include "RifReaderEclipseSummary.h"

#include "RigStatisticsMath.h"

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
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimTimeStepFilter.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuCvfOverlayItemWidget.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionDialog.h"
#include "RiuTextContentFrame.h"

#include "cafPdmObject.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafTitledOverlayFrame.h"

#include "cvfScalarMapper.h"

#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum statisticsCurveSymbolFromAddress( const RifEclipseSummaryAddress& address );
int                           statisticsCurveSymbolSize( RiuQwtSymbol::PointSymbolEnum symbol );

CAF_PDM_SOURCE_INIT( RimEnsembleCurveSet, "RimEnsembleCurveSet" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::RimEnsembleCurveSet()
{
    CAF_PDM_InitObject( "Ensemble Curve Set", ":/EnsembleCurveSet16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_curves, "EnsembleCurveSet", "Ensemble Curve Set", "", "", "" );
    m_curves.uiCapability()->setUiHidden( true );
    m_curves.uiCapability()->setUiTreeChildrenHidden( false );

    CAF_PDM_InitField( &m_showCurves, "IsActive", true, "Show Curves", "", "", "" );
    m_showCurves.uiCapability()->setUiHidden( true );

    // Y Values
    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryCaseCollection, "SummaryGroup", "Ensemble", "", "", "" );
    m_yValuesSummaryCaseCollection.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryCaseCollection.uiCapability()->setAutoAddingOptionFromValue( false );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddressUiField, "SelectedVariableDisplayVar", "Vector", "", "", "" );
    m_yValuesSummaryAddressUiField.xmlCapability()->disableIO();
    m_yValuesSummaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryAddress, "SummaryAddress", "Summary Address", "", "", "" );
    m_yValuesSummaryAddress.uiCapability()->setUiHidden( true );
    m_yValuesSummaryAddress.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_yPushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_yPushButtonSelectSummaryAddress );
    m_yPushButtonSelectSummaryAddress.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_yPushButtonSelectSummaryAddress = false;

    CAF_PDM_InitField( &m_colorMode, "ColorMode", caf::AppEnum<ColorMode>( ColorMode::SINGLE_COLOR ), "Coloring Mode", "", "", "" );

    CAF_PDM_InitField( &m_color, "Color", RiaColorTools::textColor3f(), "Color", "", "", "" );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Ensemble Parameter", "", "", "" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddressesUiField, "SelectedObjectiveSummaryVar", "Vector", "", "", "" );
    m_objectiveValuesSummaryAddressesUiField.xmlCapability()->disableIO();
    m_objectiveValuesSummaryAddressesUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSummaryAddresses, "ObjectiveSummaryAddress", "Summary Address", "", "", "" );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiHidden( true );
    m_objectiveValuesSummaryAddresses.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_objectiveValuesSelectSummaryAddressPushButton,
                                "SelectObjectiveSummaryAddress",
                                "",
                                "",
                                "",
                                "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_objectiveValuesSelectSummaryAddressPushButton );
    m_objectiveValuesSelectSummaryAddressPushButton.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_objectiveValuesSelectSummaryAddressPushButton = false;

    CAF_PDM_InitFieldNoDefault( &m_objectiveFunction, "ObjectiveFunction", "Objective Function", "", "", "" );
    m_objectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_customObjectiveFunction, "CustomObjectiveFunction", "Objective Function", "", "", "" );
    m_customObjectiveFunction.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_showObjectiveFunctionFormula, "ShowObjectiveFunctionFormula", true, "Show Formula in Plot", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_minDateRange, "MinDateRange", "From", "", "", "" );
    m_minDateRange.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_minTimeStep, "MinTimeStep", "", "", "", "" );
    m_minTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxDateRange, "MaxDateRange", "To", "", "", "" );
    m_maxDateRange.uiCapability()->setUiEditorTypeName( caf::PdmUiDateEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_maxTimeStep, "MaxTimeStep", "", "", "", "" );
    m_maxTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiSliderEditor::uiEditorTypeName() );

    // Time Step Selection
    CAF_PDM_InitFieldNoDefault( &m_timeStepFilter, "TimeStepFilter", "Available Time Steps", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Select Time Steps", "", "", "" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    CAF_PDM_InitFieldNoDefault( &m_plotAxis, "PlotAxis", "Axis", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setColorLegend(
        RimRegularLegendConfig::mapToColorLegend( RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE ) );

    CAF_PDM_InitFieldNoDefault( &m_curveFilters, "CurveFilters", "Curve Filters", "", "", "" );
    m_curveFilters = new RimEnsembleCurveFilterCollection();
    m_curveFilters->setUiTreeHidden( true );
    m_curveFilters->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_customObjectiveFunctions,
                                "CustomObjectiveFunctions",
                                "Custom Objective Functions",
                                "",
                                "",
                                "" );
    m_customObjectiveFunctions = new RimCustomObjectiveFunctionCollection();
    m_customObjectiveFunctions->objectiveFunctionChanged.connect( this, &RimEnsembleCurveSet::onObjectiveFunctionChanged );

    CAF_PDM_InitFieldNoDefault( &m_statistics, "Statistics", "Statistics", "", "", "" );
    m_statistics = new RimEnsembleStatistics();
    m_statistics.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitField( &m_userDefinedName, "UserDefinedName", QString( "Ensemble Curve Set" ), "Curve Set Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_autoGeneratedName, "AutoGeneratedName", "Curve Set Name", "", "", "" );
    m_autoGeneratedName.registerGetMethod( this, &RimEnsembleCurveSet::createAutoName );
    m_autoGeneratedName.uiCapability()->setUiReadOnly( true );
    m_autoGeneratedName.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_isUsingAutoName, "AutoName", true, "Auto Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_summaryAddressNameTools, "SummaryAddressNameTools", "SummaryAddressNameTools", "", "", "" );
    m_summaryAddressNameTools.uiCapability()->setUiHidden( true );
    m_summaryAddressNameTools.uiCapability()->setUiTreeChildrenHidden( true );

    m_summaryAddressNameTools = new RimSummaryCurveAutoName;

    m_qwtPlotCurveForLegendText = new QwtPlotCurve;
    m_qwtPlotCurveForLegendText->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );

    m_ensembleStatCase.reset( new RimEnsembleStatisticsCase( this ) );
    m_ensembleStatCase->createSummaryReaderInterface();
    m_ensembleStatCase->createRftReaderInterface();

    m_disableStatisticCurves = false;
    m_isCurveSetFiltered     = false;

    // Obsolete fields

    CAF_PDM_InitFieldNoDefault( &m_yValuesSummaryFilter_OBSOLETE, "VarListFilter", "Filter", "", "", "" );
    m_yValuesSummaryFilter_OBSOLETE.uiCapability()->setUiTreeChildrenHidden( true );
    m_yValuesSummaryFilter_OBSOLETE.uiCapability()->setUiHidden( true );
    m_yValuesSummaryFilter_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_yValuesSummaryFilter_OBSOLETE = new RimSummaryFilter_OBSOLETE;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::~RimEnsembleCurveSet()
{
    m_curves.deleteAllChildObjects();

    RimSummaryPlot* parentPlot;
    firstAncestorOrThisOfType( parentPlot );
    if ( parentPlot && parentPlot->viewer() )
    {
        m_qwtPlotCurveForLegendText->detach();
        if ( m_legendOverlayFrame )
        {
            parentPlot->viewer()->removeOverlayFrame( m_legendOverlayFrame );
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

    delete m_qwtPlotCurveForLegendText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::isCurvesVisible()
{
    RimEnsembleCurveSetCollection* coll = nullptr;
    firstAncestorOrThisOfType( coll );
    return m_showCurves() && ( coll ? coll->isCurveSetsVisible() : true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setColor( cvf::Color3f color )
{
    m_color = color;
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

    if ( updateParentPlot )
    {
        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted( parentPlot );
        parentPlot->updateAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setParentQwtPlotNoReplot( QwtPlot* plot )
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->setParentQwtPlotNoReplot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::detachQwtCurves()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->detachQwtCurve();
    }

    m_qwtPlotCurveForLegendText->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::reattachQwtCurves()
{
    for ( RimSummaryCurve* curve : m_curves )
    {
        curve->reattachQwtCurve();
    }

    m_qwtPlotCurveForLegendText->detach();

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    if ( plot )
    {
        m_qwtPlotCurveForLegendText->attach( plot->viewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::addCurve( RimSummaryCurve* curve )
{
    if ( curve )
    {
        RimSummaryPlot* plot;
        firstAncestorOrThisOfType( plot );
        if ( plot ) curve->setParentQwtPlotNoReplot( plot->viewer() );

        curve->setColor( m_color );
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
        m_curves.removeChildObject( curve );
        curve->markCachedDataForPurge();
        delete curve;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryAddress( RifEclipseSummaryAddress address )
{
    m_yValuesSummaryAddress->setAddress( address );
    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
    summaryAddress->setAddress( address );
    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimEnsembleCurveSet::summaryAddress() const
{
    return m_yValuesSummaryAddress->address();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimEnsembleCurveSet::curves() const
{
    return m_curves.childObjects();
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
        if ( curve->summaryAddressY().category() != RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
            curvesIndexesToDelete.push_back( c );
    }

    while ( curvesIndexesToDelete.size() > 0 )
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
        if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
            curvesIndexesToDelete.push_back( c );
    }

    while ( curvesIndexesToDelete.size() > 0 )
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
EnsembleParameter::Type RimEnsembleCurveSet::currentEnsembleParameterType() const
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
    return EnsembleParameter::TYPE_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAllCurves()
{
    RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();
    RimSummaryAddress*        addr  = m_yValuesSummaryAddress();

    if ( group && addr->address().category() != RifEclipseSummaryAddress::SUMMARY_INVALID )
    {
        std::vector<RimSummaryCase*> allCases      = group->allSummaryCases();
        std::vector<RimSummaryCase*> filteredCases = filterEnsembleCases( allCases );

        m_isCurveSetFiltered = filteredCases.size() < allCases.size();

        updateEnsembleCurves( filteredCases );
        updateStatisticsCurves( m_statistics->basedOnFilteredCases() ? filteredCases : allCases );
    }
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
std::vector<time_t> RimEnsembleCurveSet::selectedTimeSteps()
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
void RimEnsembleCurveSet::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    bool updateTextInPlot = false;

    if ( changedField == &m_showCurves )
    {
        loadDataAndUpdate( true );

        updateConnectedEditors();

        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted( summaryPlot );
        summaryPlot->updateConnectedEditors();

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
        // Empty address cache
        // m_allAddressesCache.clear();
        updateAllCurves();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_color )
    {
        updateCurveColors();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_ensembleParameter )
    {
        updateLegendMappingMode();
        updateCurveColors();
    }
    else if ( changedField == &m_objectiveFunction )
    {
        updateLegendMappingMode();
        updateCurveColors();
        updateTimeAnnotations();
        updateObjectiveFunctionLegend();
    }
    else if ( changedField == &m_objectiveValuesSummaryAddressesUiField )
    {
        updateAddressesUiField();

        std::vector<size_t> indices;
        indices.push_back( summaryCaseCollection()->objectiveFunction( m_objectiveFunction() )->range().first );
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
        m_objectiveFunction.uiCapability()->setUiHidden( m_colorMode() != ColorMode::BY_OBJECTIVE_FUNCTION );

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
            if ( m_objectiveValuesSummaryAddresses.size() == 0 )
            {
                RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                summaryAddress->setAddress( m_yValuesSummaryAddress->address() );
                m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                updateAddressesUiField();
                m_minTimeStep = *allAvailableTimeSteps().begin();
                m_maxTimeStep = *allAvailableTimeSteps().rbegin();
                updateMaxMinAndDefaultValues();
            }
        }

        updateCurveColors();
        updateTimeAnnotations();
        updateObjectiveFunctionLegend();

        updateTextInPlot = true;
    }
    else if ( changedField == &m_timeStepFilter )
    {
        m_selectedTimeSteps.v().clear();

        this->updateConnectedEditors();
    }
    else if ( changedField == &m_selectedTimeSteps )
    {
        summaryCaseCollection()->objectiveFunction( m_objectiveFunction() )->setTimeStepList( selectedTimeSteps() );
        updateCurveColors();
        updateTimeAnnotations();
        updateObjectiveFunctionLegend();
    }
    else if ( changedField == &m_minTimeStep || changedField == &m_maxTimeStep )
    {
        updateMaxMinAndDefaultValues();
        updateCurveColors();
        updateTimeAnnotations();
    }
    else if ( changedField == &m_minDateRange || changedField == &m_maxDateRange )
    {
        m_minTimeStep = RiaTimeTTools::fromQDateTime( QDateTime( m_minDateRange() ) );
        m_maxTimeStep = RiaTimeTTools::fromQDateTime( QDateTime( m_maxDateRange() ) );
        updateCurveColors();
        updateTimeAnnotations();
    }
    else if ( changedField == &m_plotAxis )
    {
        for ( RimSummaryCurve* curve : curves() )
        {
            curve->setLeftOrRightAxisY( m_plotAxis() );
        }

        updateQwtPlotAxis();
        plot->updateAxes();

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
                m_yValuesSummaryAddress->setAddress( curveSelection[0].summaryAddress() );

                this->loadDataAndUpdate( true );

                plot->updateAxes();
                plot->updatePlotTitle();
                plot->updateConnectedEditors();

                RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
                mainPlotWindow->updateSummaryPlotToolBar();
            }
        }

        m_yPushButtonSelectSummaryAddress = false;
    }
    else if ( changedField == &m_objectiveValuesSelectSummaryAddressPushButton )
    {
        RiuSummaryVectorSelectionDialog dlg( nullptr );
        dlg.enableMultiSelect( true );
        RimSummaryCaseCollection* candidateEnsemble = m_yValuesSummaryCaseCollection();

        std::vector<RifEclipseSummaryAddress> candidateAddresses;
        for ( auto address : m_objectiveValuesSummaryAddresses().childObjects() )
        {
            candidateAddresses.push_back( address->address() );
        }

        dlg.hideSummaryCases();
        dlg.setEnsembleAndAddresses( candidateEnsemble, candidateAddresses );

        if ( dlg.exec() == QDialog::Accepted )
        {
            auto curveSelection = dlg.curveSelection();
            if ( !curveSelection.empty() )
            {
                m_objectiveValuesSummaryAddresses.clear();
                for ( auto address : curveSelection )
                {
                    RimSummaryAddress* summaryAddress = new RimSummaryAddress();
                    summaryAddress->setAddress( address.summaryAddress() );
                    m_objectiveValuesSummaryAddresses.push_back( summaryAddress );
                }
                this->loadDataAndUpdate( true );
            }
        }

        m_objectiveValuesSelectSummaryAddressPushButton = false;
    }
    else if ( changedField == &m_customObjectiveFunction )
    {
        if ( m_customObjectiveFunction() )
        {
            if ( m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::M2 ) )
            {
                std::vector<size_t> indices;
                indices.push_back(
                    summaryCaseCollection()->objectiveFunction( RimObjectiveFunction::FunctionType::M2 )->range().first );
                setTimeSteps( indices );
            }

            m_minTimeStep = *allAvailableTimeSteps().begin();
            m_maxTimeStep = *allAvailableTimeSteps().rbegin();

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
void RimEnsembleCurveSet::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector Y" );
        curveDataGroup->add( &m_yValuesSummaryCaseCollection );
        curveDataGroup->add( &m_yValuesSummaryAddressUiField );
        curveDataGroup->add( &m_yPushButtonSelectSummaryAddress, { false, 1, 0 } );
        curveDataGroup->add( &m_plotAxis );
    }

    appendColorGroup( uiOrdering );

    {
        caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
        nameGroup->setCollapsedByDefault( true );
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

    caf::PdmUiGroup* statGroup = uiOrdering.addNewGroup( "Statistics" );
    m_statistics->defineUiOrdering( uiConfigName, *statGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateMaxMinAndDefaultValues()
{
    m_minDateRange = QDateTime::fromSecsSinceEpoch( m_minTimeStep ).date();
    m_maxDateRange = QDateTime::fromSecsSinceEpoch( m_maxTimeStep ).date();

    summaryCaseCollection()->objectiveFunction( m_objectiveFunction() )->setTimeStepRange( m_minTimeStep(), m_maxTimeStep() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::onObjectiveFunctionChanged( const caf::SignalEmitter* emitter )
{
    updateCurveColors();
    updateFilterLegend();
    updateObjectiveFunctionLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::appendColorGroup( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* colorsGroup = uiOrdering.addNewGroup( "Colors" );
    m_colorMode.uiCapability()->setUiReadOnly( !m_yValuesSummaryCaseCollection() );
    colorsGroup->add( &m_colorMode );

    if ( m_colorMode == ColorMode::SINGLE_COLOR )
    {
        colorsGroup->add( &m_color );
    }
    else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        m_ensembleParameter.uiCapability()->setUiReadOnly( !m_yValuesSummaryCaseCollection() );
        colorsGroup->add( &m_ensembleParameter );
    }
    else if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION || m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION )
        {
            m_objectiveFunction.uiCapability()->setUiReadOnly( !m_yValuesSummaryCaseCollection() );
            colorsGroup->add( &m_objectiveValuesSummaryAddressesUiField );
            colorsGroup->add( &m_objectiveValuesSelectSummaryAddressPushButton, { false, 1, 0 } );
            colorsGroup->add( &m_objectiveFunction );
        }
        else
        {
            colorsGroup->add( &m_customObjectiveFunction );
        }
        colorsGroup->add( &m_showObjectiveFunctionFormula );
        if ( ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION &&
               m_objectiveFunction() == RimObjectiveFunction::FunctionType::M1 ) ||
             ( m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() &&
               m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::M1 ) ) )
        {
            colorsGroup->add( &m_minDateRange );
            colorsGroup->add( &m_minTimeStep );
            colorsGroup->add( &m_maxDateRange );
            colorsGroup->add( &m_maxTimeStep );
        }
        if ( m_objectiveFunction() == RimObjectiveFunction::FunctionType::M2 ||
             ( m_customObjectiveFunction() &&
               m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::M2 ) ) )
        {
            colorsGroup->add( &m_timeStepFilter );
            colorsGroup->add( &m_selectedTimeSteps );
        }
    }
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

    caf::IconProvider iconProvider = this->uiIconProvider();
    if ( !iconProvider.valid() ) return;

    RimEnsembleCurveSetCollection* coll = nullptr;
    this->firstAncestorOrThisOfType( coll );
    if ( coll && coll->curveSetForSourceStepping() == this )
    {
        iconProvider.setOverlayResourceString( ":/StepUpDownCorner16x16.png" );
    }

    this->setUiIcon( iconProvider );
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
void RimEnsembleCurveSet::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
    if ( attrib )
    {
        attrib->m_buttonText = "...";
    }
    if ( field == &m_minTimeStep || field == &m_maxTimeStep )
    {
        caf::PdmUiSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>( attribute );
        if ( !myAttr )
        {
            return;
        }

        myAttr->m_minimum     = *allAvailableTimeSteps().begin();
        myAttr->m_maximum     = *allAvailableTimeSteps().rbegin();
        myAttr->m_showSpinBox = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveSet::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                          bool*                      useOptionsOnly )
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
        auto singleColorOption     = ColorModeEnum( ColorMode::SINGLE_COLOR );
        auto byEnsParamOption      = ColorModeEnum( ColorMode::BY_ENSEMBLE_PARAM );
        auto byObjFuncOption       = ColorModeEnum( ColorMode::BY_OBJECTIVE_FUNCTION );
        auto byCustomObjFuncOption = ColorModeEnum( ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION );

        options.push_back( caf::PdmOptionItemInfo( singleColorOption.uiText(), ColorMode::SINGLE_COLOR ) );

        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();
        if ( group && group->hasEnsembleParameters() )
        {
            options.push_back( caf::PdmOptionItemInfo( byEnsParamOption.uiText(), ColorMode::BY_ENSEMBLE_PARAM ) );
        }
        options.push_back( caf::PdmOptionItemInfo( byObjFuncOption.uiText(), ColorMode::BY_OBJECTIVE_FUNCTION ) );
        options.push_back(
            caf::PdmOptionItemInfo( byCustomObjFuncOption.uiText(), ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION ) );
    }
    else if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        auto params = correlationSortedEnsembleParameters();
        for ( const auto& paramCorrPair : params )
        {
            QString name = paramCorrPair.first.name;
            double  corr = paramCorrPair.second;
            options.push_back(
                caf::PdmOptionItemInfo( QString( "%1 (Avg. correlation: %2)" ).arg( name ).arg( corr, 5, 'f', 2 ), name ) );
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

        QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                        RiaQDateTimeTools::DATE_FORMAT_YEAR_MONTH_DAY );
        QString timeFormatString =
            RiaQDateTimeTools::timeFormatString( RiaPreferences::current()->timeFormat(),
                                                 RiaQDateTimeTools::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE );
        QString dateTimeFormatString = QString( "%1 %2" ).arg( dateFormatString ).arg( timeFormatString );

        bool showTime = m_timeStepFilter() == RimTimeStepFilter::TS_ALL ||
                        m_timeStepFilter() == RimTimeStepFilter::TS_INTERVAL_DAYS;

        for ( auto timeStepIndex : filteredTimeStepIndices )
        {
            QDateTime dateTime = allDateTimes[timeStepIndex];

            if ( showTime && dateTime.time() != QTime( 0, 0, 0 ) )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime,
                                                                                               dateTimeFormatString ),
                                            dateTime ) );
            }
            else
            {
                options.push_back(
                    caf::PdmOptionItemInfo( RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime, dateFormatString ),
                                            dateTime ) );
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

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimEnsembleCurveSet::allAvailableTimeSteps()
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
        m_analyserOfSelectedCurveDefs =
            std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>( new RiaSummaryCurveDefinitionAnalyser );
    }
    m_analyserOfSelectedCurveDefs->setCurveDefinitions( this->curveDefinitions() );
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
        curveDefs.push_back( dataEntry->curveDefinitionY() );
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
        const std::set<RifEclipseSummaryAddress>& addrs  = reader ? reader->allResultAddresses()
                                                                 : std::set<RifEclipseSummaryAddress>();

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

    options->push_front( caf::PdmOptionItemInfo( RiaDefines::undefinedResultName(),
                                                 QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateFilterLegend()
{
    RimSummaryPlot* plot;
    firstAncestorOrThisOfType( plot );
    if ( plot && plot->viewer() )
    {
        if ( m_curveFilters()->isActive() && m_curveFilters()->countActiveFilters() > 0 )
        {
            if ( !m_filterOverlayFrame )
            {
                m_filterOverlayFrame =
                    new RiuDraggableOverlayFrame( plot->viewer()->canvas(), plot->viewer()->overlayMargins() );
            }
            m_filterOverlayFrame->setContentFrame( m_curveFilters()->makeFilterDescriptionFrame() );
            plot->viewer()->addOverlayFrame( m_filterOverlayFrame );
        }
        else
        {
            if ( m_filterOverlayFrame )
            {
                plot->viewer()->removeOverlayFrame( m_filterOverlayFrame );
            }
        }
        plot->viewer()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateObjectiveFunctionLegend()
{
    RimSummaryPlot* plot;
    firstAncestorOrThisOfType( plot );
    if ( plot && plot->viewer() )
    {
        if ( ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION || m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION ) &&
             m_showObjectiveFunctionFormula() )
        {
            if ( !m_objectiveFunctionOverlayFrame )
            {
                m_objectiveFunctionOverlayFrame =
                    new RiuDraggableOverlayFrame( plot->viewer()->canvas(), plot->viewer()->overlayMargins() );
            }
            QString title;
            QString description;
            if ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION )
            {
                std::vector<RifEclipseSummaryAddress> addresses;
                for ( auto address : m_objectiveValuesSummaryAddresses().childObjects() )
                {
                    addresses.push_back( address->address() );
                }

                title = "Objective Function";
                description =
                    QString( "%0 = %1" )
                        .arg( m_yValuesSummaryCaseCollection()->objectiveFunction( m_objectiveFunction() )->uiName() )
                        .arg( m_yValuesSummaryCaseCollection()
                                  ->objectiveFunction( m_objectiveFunction() )
                                  ->formulaString( addresses ) );
            }
            else if ( m_colorMode() == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() )
            {
                std::vector<RifEclipseSummaryAddress> addresses;
                for ( auto address : m_objectiveValuesSummaryAddresses().childObjects() )
                {
                    addresses.push_back( address->address() );
                }

                title       = "Custom Objective Function";
                description = m_customObjectiveFunction()->formulaString( addresses );
            }
            if ( !title.isEmpty() && !description.isEmpty() )
            {
                m_objectiveFunctionOverlayFrame->setContentFrame( new RiuTextContentFrame( nullptr, title, description ) );
                m_objectiveFunctionOverlayFrame->setMaximumWidth( 10000 );
                plot->viewer()->addOverlayFrame( m_objectiveFunctionOverlayFrame );
            }
        }
        else
        {
            if ( m_objectiveFunctionOverlayFrame )
            {
                plot->viewer()->removeOverlayFrame( m_objectiveFunctionOverlayFrame );
            }
        }
    }
    plot->viewer()->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateCurveColors()
{
    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();

        QString parameterName = m_ensembleParameter();

        {
            QString legendTitle;
            if ( m_isUsingAutoName )
            {
                legendTitle = m_autoGeneratedName();
            }
            else
            {
                legendTitle += m_userDefinedName();
            }

            legendTitle += "\n";
            legendTitle += parameterName;

            m_legendConfig->setTitle( legendTitle );
        }

        if ( group && !parameterName.isEmpty() && !group->allSummaryCases().empty() )
        {
            auto ensembleParam = group->ensembleParameter( parameterName );
            if ( ensembleParam.isText() || ensembleParam.isNumeric() )
            {
                RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig, ensembleParam );
                for ( auto& curve : m_curves )
                {
                    if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
                        continue;
                    RimSummaryCase* rimCase = curve->summaryCaseY();
                    cvf::Color3f    curveColor =
                        RimEnsembleCurveSetColorManager::caseColor( m_legendConfig, rimCase, ensembleParam );
                    curve->setColor( curveColor );
                    curve->updateCurveAppearance();
                }
            }
        }
    }
    else if ( m_colorMode == ColorMode::SINGLE_COLOR )
    {
        for ( auto& curve : m_curves )
        {
            if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
                continue;

            curve->setColor( m_color );
            curve->updateCurveAppearance();
        }
    }
    else if ( m_colorMode == ColorMode::BY_OBJECTIVE_FUNCTION )
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();

        {
            QString legendTitle;
            if ( m_isUsingAutoName )
            {
                legendTitle = m_autoGeneratedName();
            }
            else
            {
                legendTitle += m_userDefinedName();
            }

            legendTitle += "\n";
            legendTitle += caf::AppEnum<RimObjectiveFunction::FunctionType>( m_objectiveFunction() ).uiText();

            m_legendConfig->setTitle( legendTitle );
        }

        if ( group && !group->allSummaryCases().empty() )
        {
            auto                                  objectiveFunction = group->objectiveFunction( m_objectiveFunction() );
            std::vector<RifEclipseSummaryAddress> summaryAddresses;
            for ( auto address : m_objectiveValuesSummaryAddresses() )
            {
                summaryAddresses.push_back( address->address() );
            }
            if ( objectiveFunction->isValid( summaryAddresses ) )
            {
                RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig, objectiveFunction, summaryAddresses );
                for ( auto& curve : m_curves )
                {
                    if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
                        continue;
                    RimSummaryCase* rimCase    = curve->summaryCaseY();
                    cvf::Color3f    curveColor = RimEnsembleCurveSetColorManager::caseColor( m_legendConfig,
                                                                                          rimCase,
                                                                                          objectiveFunction,
                                                                                          summaryAddresses );
                    curve->setColor( curveColor );
                    curve->updateCurveAppearance();
                }
            }
            else if ( m_legendOverlayFrame )
            {
                m_legendOverlayFrame->hide();
            }
        }
    }
    else if ( m_colorMode == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION )
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryCaseCollection();

        {
            QString legendTitle;
            if ( m_isUsingAutoName )
            {
                legendTitle = m_autoGeneratedName();
            }
            else
            {
                legendTitle += m_userDefinedName();
            }

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

        if ( group && !group->allSummaryCases().empty() && m_customObjectiveFunction() &&
             m_customObjectiveFunction->isValid() )
        {
            RimEnsembleCurveSetColorManager::initializeLegendConfig( m_legendConfig, m_customObjectiveFunction() );
            for ( auto& curve : m_curves )
            {
                if ( curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS )
                    continue;
                RimSummaryCase* rimCase = curve->summaryCaseY();
                cvf::Color3f    curveColor =
                    RimEnsembleCurveSetColorManager::caseColor( m_legendConfig, rimCase, m_customObjectiveFunction() );
                curve->setColor( curveColor );
                curve->updateCurveAppearance();
            }
        }
    }

    RimSummaryPlot* plot;
    firstAncestorOrThisOfType( plot );
    if ( plot && plot->viewer() )
    {
        if ( m_yValuesSummaryCaseCollection() && isCurvesVisible() && m_colorMode != ColorMode::SINGLE_COLOR &&
             m_legendConfig->showLegend() )
        {
            if ( !m_legendOverlayFrame )
            {
                m_legendOverlayFrame =
                    new RiuDraggableOverlayFrame( plot->viewer()->canvas(), plot->viewer()->overlayMargins() );
            }
            m_legendOverlayFrame->setContentFrame( m_legendConfig->makeLegendFrame() );
            plot->viewer()->addOverlayFrame( m_legendOverlayFrame );
        }
        else
        {
            if ( m_legendOverlayFrame )
            {
                plot->viewer()->removeOverlayFrame( m_legendOverlayFrame );
            }
        }
        plot->viewer()->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateTimeAnnotations()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    plot->removeAllTimeAnnotations();

    if ( ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION &&
           m_objectiveFunction() == RimObjectiveFunction::FunctionType::M1 ) ||
         ( m_colorMode() == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() &&
           m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::M1 ) ) )
    {
        plot->addTimeRangeAnnotation( m_minTimeStep, m_maxTimeStep );
    }

    if ( ( m_colorMode() == ColorMode::BY_OBJECTIVE_FUNCTION &&
           m_objectiveFunction() == RimObjectiveFunction::FunctionType::M2 ) ||
         ( m_colorMode() == ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION && m_customObjectiveFunction() &&
           m_customObjectiveFunction()->weightContainsFunctionType( RimObjectiveFunction::FunctionType::M2 ) ) )
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
    m_objectiveValuesSummaryAddressesUiField =
        QString::fromStdString( RifEclipseSummaryAddress::generateStringFromAddresses( addressVector ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateQwtPlotAxis()
{
    for ( RimSummaryCurve* curve : curves() )
    {
        curve->updateQwtPlotAxis();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEnsembleCurves( const std::vector<RimSummaryCase*>& sumCases )
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    deleteEnsembleCurves();
    m_qwtPlotCurveForLegendText->detach();
    deleteStatisticsCurves();

    if ( m_statistics->hideEnsembleCurves() ) return;

    RimSummaryAddress* addr = m_yValuesSummaryAddress();
    if ( plot && addr->address().category() != RifEclipseSummaryAddress::SUMMARY_INVALID )
    {
        if ( isCurvesVisible() )
        {
            std::vector<RimSummaryCurve*> newSummaryCurves;

            for ( auto& sumCase : sumCases )
            {
                RimSummaryCurve* curve = new RimSummaryCurve();
                curve->setSummaryCaseY( sumCase );
                curve->setSummaryAddressYAndApplyInterpolation( addr->address() );
                curve->setLeftOrRightAxisY( m_plotAxis() );

                addCurve( curve );

                curve->updateCurveVisibility();

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
                newSummaryCurves[i]->updateQwtPlotAxis();
                if ( newSummaryCurves[i]->qwtPlotCurve() )
                {
                    newSummaryCurves[i]->qwtPlotCurve()->setItemAttribute( QwtPlotItem::Legend, false );
                }
            }

            if ( plot->viewer() ) m_qwtPlotCurveForLegendText->attach( plot->viewer() );
        }

        if ( plot->viewer() )
        {
            if ( plot->legendsVisible() ) plot->viewer()->updateLegend();
            plot->viewer()->scheduleReplot();
            plot->updateAxes();
            plot->updatePlotInfoLabel();
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

    if ( !isCurvesVisible() || m_disableStatisticCurves || !group ||
         addr->address().category() == RifEclipseSummaryAddress::SUMMARY_INVALID )
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
        m_ensembleStatCase->calculate( statCases, m_statistics->includeIncompleteCurves() );
    }

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType( plot );
    CVF_ASSERT( plot );

    std::vector<RifEclipseSummaryAddress> addresses;
    if ( m_statistics->isActive() )
    {
        RifEclipseSummaryAddress dataAddress = m_yValuesSummaryAddress->address();

        if ( m_statistics->showP10Curve() && m_ensembleStatCase->hasP10Data() )
            addresses.push_back(
                SAddr::ensembleStatisticsAddress( ENSEMBLE_STAT_P10_QUANTITY_NAME, dataAddress.quantityName() ) );
        if ( m_statistics->showP50Curve() && m_ensembleStatCase->hasP50Data() )
            addresses.push_back(
                SAddr::ensembleStatisticsAddress( ENSEMBLE_STAT_P50_QUANTITY_NAME, dataAddress.quantityName() ) );
        if ( m_statistics->showP90Curve() && m_ensembleStatCase->hasP90Data() )
            addresses.push_back(
                SAddr::ensembleStatisticsAddress( ENSEMBLE_STAT_P90_QUANTITY_NAME, dataAddress.quantityName() ) );
        if ( m_statistics->showMeanCurve() && m_ensembleStatCase->hasMeanData() )
            addresses.push_back(
                SAddr::ensembleStatisticsAddress( ENSEMBLE_STAT_MEAN_QUANTITY_NAME, dataAddress.quantityName() ) );
    }

    deleteStatisticsCurves();
    for ( auto address : addresses )
    {
        auto curve = new RimSummaryCurve();
        curve->setParentQwtPlotNoReplot( plot->viewer() );
        m_curves.push_back( curve );
        curve->setColor( m_statistics->color() );

        auto symbol = statisticsCurveSymbolFromAddress( address );
        curve->setSymbol( symbol );
        curve->setSymbolSize( statisticsCurveSymbolSize( symbol ) );
        curve->setSymbolSkipDistance( 150 );
        if ( m_statistics->showCurveLabels() )
        {
            curve->setSymbolLabel( RiaStatisticsTools::replacePercentileByPValueText(
                QString::fromStdString( address.ensembleStatisticsQuantityName() ) ) );
        }
        curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
        curve->setSummaryCaseY( m_ensembleStatCase.get() );
        curve->setSummaryAddressYAndApplyInterpolation( address );
        curve->setLeftOrRightAxisY( m_plotAxis() );

        curve->updateCurveVisibility();
        curve->loadDataAndUpdate( false );
        curve->updateQwtPlotAxis();
    }

    if ( plot->viewer() )
    {
        plot->viewer()->updateLegend();
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
    RimEnsembleCurveSet* copy = dynamic_cast<RimEnsembleCurveSet*>(
        this->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    copy->m_yValuesSummaryCaseCollection = m_yValuesSummaryCaseCollection();

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
void RimEnsembleCurveSet::markCachedDataForPurge()
{
    for ( const auto& curve : m_curves )
    {
        curve->markCachedDataForPurge();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAllTextInPlot()
{
    updateEnsembleLegendItem();

    RimSummaryPlot* summaryPlot = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( summaryPlot );
    if ( summaryPlot->viewer() )
    {
        summaryPlot->updatePlotTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<EnsembleParameter> RimEnsembleCurveSet::variationSortedEnsembleParameters() const
{
    RimSummaryCaseCollection* ensemble = m_yValuesSummaryCaseCollection;
    if ( ensemble )
    {
        return ensemble->variationSortedEnsembleParameters();
    }
    else
    {
        return std::vector<EnsembleParameter>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<EnsembleParameter, double>> RimEnsembleCurveSet::correlationSortedEnsembleParameters() const
{
    RimSummaryCaseCollection* ensemble = m_yValuesSummaryCaseCollection;
    if ( ensemble )
    {
        return ensemble->correlationSortedEnsembleParameters( summaryAddress() );
    }
    else
    {
        return std::vector<std::pair<EnsembleParameter, double>>();
    }
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
    return m_ensembleStatCase->hasP10Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP50Data() const
{
    return m_ensembleStatCase->hasP50Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP90Data() const
{
    return m_ensembleStatCase->hasP90Data();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasMeanData() const
{
    return m_ensembleStatCase->hasMeanData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEnsembleLegendItem()
{
    m_qwtPlotCurveForLegendText->setTitle( name() );

    {
        QwtSymbol* symbol = nullptr;

        if ( m_colorMode == ColorMode::SINGLE_COLOR )
        {
            symbol = new QwtSymbol( QwtSymbol::HLine );

            QColor curveColor( m_color.value().rByte(), m_color.value().gByte(), m_color.value().bByte() );
            QPen   curvePen( curveColor );
            curvePen.setWidth( 2 );

            symbol->setPen( curvePen );
            symbol->setSize( 6, 6 );
        }
        else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
        {
            QPixmap p = QPixmap( ":/Legend.png" );

            symbol = new QwtSymbol;
            symbol->setPixmap( p );
            symbol->setSize( 8, 8 );
        }

        m_qwtPlotCurveForLegendText->setSymbol( symbol );
    }

    bool showLegendItem = isCurvesVisible();
    m_qwtPlotCurveForLegendText->setItemAttribute( QwtPlotItem::Legend, showLegendItem );
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
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted( plot );

    QString curveSetName = m_summaryAddressNameTools->curveNameY( m_yValuesSummaryAddress->address(),
                                                                  plot->activePlotTitleHelperAllCurves() );
    if ( curveSetName.isEmpty() )
    {
        curveSetName = m_summaryAddressNameTools->curveNameY( m_yValuesSummaryAddress->address(), nullptr );
    }

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
        case EnsembleParameter::TYPE_TEXT:
            if ( m_legendConfig->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
                m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER );
            break;

        case EnsembleParameter::TYPE_NUMERIC:
            if ( m_legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER )
                m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum statisticsCurveSymbolFromAddress( const RifEclipseSummaryAddress& address )
{
    auto qName = QString::fromStdString( address.quantityName() );

    if ( qName.contains( ENSEMBLE_STAT_P10_QUANTITY_NAME ) ) return RiuQwtSymbol::SYMBOL_TRIANGLE;
    if ( qName.contains( ENSEMBLE_STAT_P90_QUANTITY_NAME ) ) return RiuQwtSymbol::SYMBOL_DOWN_TRIANGLE;
    if ( qName.contains( ENSEMBLE_STAT_P50_QUANTITY_NAME ) ) return RiuQwtSymbol::SYMBOL_DIAMOND;
    return RiuQwtSymbol::SYMBOL_ELLIPSE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int statisticsCurveSymbolSize( RiuQwtSymbol::PointSymbolEnum symbol )
{
    if ( symbol == RiuQwtSymbol::SYMBOL_DIAMOND ) return 8;
    if ( symbol == RiuQwtSymbol::SYMBOL_TRIANGLE ) return 7;
    if ( symbol == RiuQwtSymbol::SYMBOL_DOWN_TRIANGLE ) return 7;
    return 6;
}
