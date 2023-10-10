/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimAnalysisPlot.h"

#include "RiaDefines.h"
#include "RiaPlotDefines.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaTextStringTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimAnalysisPlotDataEntry.h"
#include "RimDerivedSummaryCase.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "RiuContextMenuLauncher.h"
#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionDialog.h"

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot_barchart.h"
#include "qwt_scale_draw.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiActionPushButtonEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <cmath>
#include <limits>
#include <map>

namespace caf
{
template <>
void caf::AppEnum<RimAnalysisPlot::SortGroupType>::setUp()
{
    addItem( RimAnalysisPlot::SortGroupType::NONE, "NONE", "None" );
    addItem( RimAnalysisPlot::SortGroupType::SUMMARY_ITEM, "SUMMARY_ITEM", "Summary Item" );
    addItem( RimAnalysisPlot::SortGroupType::VECTOR, "VECTOR", "Vector" );
    addItem( RimAnalysisPlot::SortGroupType::CASE, "CASE", "Case" );
    addItem( RimAnalysisPlot::SortGroupType::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimAnalysisPlot::SortGroupType::VALUE, "VALUE", "Value" );
    addItem( RimAnalysisPlot::SortGroupType::ABS_VALUE, "ABS_VALUE", "abs(Value)" );
    addItem( RimAnalysisPlot::SortGroupType::OTHER_VALUE, "OTHER_VALUE", "Other Value" );
    addItem( RimAnalysisPlot::SortGroupType::ABS_OTHER_VALUE, "ABS_OTHER_VALUE", "abs(Other Value)" );
    addItem( RimAnalysisPlot::SortGroupType::TIME_STEP, "TIME_STEP", "Time Step" );
    setDefault( RimAnalysisPlot::SortGroupType::NONE );
}

template <>
void caf::AppEnum<RimAnalysisPlot::BarOrientation>::setUp()
{
    addItem( RimAnalysisPlot::BarOrientation::BARS_HORIZONTAL, "BARS_HORIZONTAL", "Horizontal" );
    addItem( RimAnalysisPlot::BarOrientation::BARS_VERTICAL, "BARS_VERTICAL", "Vertical" );
    setDefault( RimAnalysisPlot::BarOrientation::BARS_VERTICAL );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimAnalysisPlot, "AnalysisPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::RimAnalysisPlot()
{
    CAF_PDM_InitObject( "Analysis Plot", ":/AnalysisPlot16x16.png" );

    // Variable selection

    CAF_PDM_InitFieldNoDefault( &m_selectedVarsUiField, "selectedVarsUiField", "Selected Vectors" );
    m_selectedVarsUiField.xmlCapability()->disableIO();
    m_selectedVarsUiField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedVarsUiField.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_selectVariablesButtonField, "BrowseButton", false, "..." );
    caf::PdmUiActionPushButtonEditor::configureEditorForField( &m_selectVariablesButtonField );

    CAF_PDM_InitFieldNoDefault( &m_analysisPlotDataSelection, "AnalysisPlotData", "" );
    m_analysisPlotDataSelection.uiCapability()->setUiTreeChildrenHidden( true );
    m_analysisPlotDataSelection.uiCapability()->setUiTreeHidden( true );

    // Time Step Selection
    CAF_PDM_InitFieldNoDefault( &m_timeStepFilter, "TimeStepFilter", "Available Time Steps" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "Select Time Steps" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );

    // Options

    CAF_PDM_InitFieldNoDefault( &m_referenceCase, "ReferenceCase", "Reference Case" );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useAutoPlotTitle );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "Analysis Plot" ), "Title" );
    m_description.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_barOrientation, "BarOrientation", "Bar Orientation" );

    // Grouping

    CAF_PDM_InitFieldNoDefault( &m_majorGroupType, "MajorGroupType", "Major Grouping" );
    CAF_PDM_InitFieldNoDefault( &m_mediumGroupType, "MediumGroupType", "Medium Grouping" );
    CAF_PDM_InitFieldNoDefault( &m_minorGroupType, "MinorGroupType", "Minor Grouping" );

    CAF_PDM_InitFieldNoDefault( &m_valueSortOperation, "ValueSortOperation", "Sort by Value" );

    CAF_PDM_InitFieldNoDefault( &m_sortGroupForColors, "groupForColors", "Coloring Using" );
    m_sortGroupForColors = RimAnalysisPlot::SortGroupType::CASE;
    m_showPlotLegends    = false;

    CAF_PDM_InitField( &m_useTopBarsFilter, "UseTopBarsFilter", false, "Show Only Top" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useTopBarsFilter );

    CAF_PDM_InitField( &m_maxBarCount, "MaxBarCount", 20, "Bar Count" );
    m_maxBarCount.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    // Bar text

    CAF_PDM_InitField( &m_useBarText, "UseBarText", true, "Activate Bar Labels" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useBarText );

    CAF_PDM_InitField( &m_useCaseInBarText, "UseCaseInBarText", true, "Case Name" );
    CAF_PDM_InitField( &m_useEnsembleInBarText, "UseEnsembleInBarText", false, "Ensemble" );
    CAF_PDM_InitField( &m_useSummaryItemInBarText, "UseSummaryItemInBarText", false, "Summary Item" );
    CAF_PDM_InitField( &m_useTimeStepInBarText, "UseTimeStepInBarText", false, "Time Step" );
    CAF_PDM_InitField( &m_useVectorNameInBarText, "UseQuantityInBarText", false, "Vector" );

    CAF_PDM_InitFieldNoDefault( &m_barTextFontSize, "BarTextFontSize", "Font Size" );

    CAF_PDM_InitFieldNoDefault( &m_valueAxisProperties, "ValueAxisProperties", "ValueAxisProperties" );
    m_valueAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_valueAxisProperties = new RimPlotAxisProperties;
    m_valueAxisProperties->setNameAndAxis( "Value-Axis", "Value-Axis", RiuQwtPlotTools::fromQwtPlotAxis( QwtAxis::YLeft ) );
    m_valueAxisProperties->enableRangeSettings( false );

    CAF_PDM_InitFieldNoDefault( &m_plotDataFilterCollection, "PlotDataFilterCollection", "PlotDataFilterCollection" );
    m_plotDataFilterCollection.uiCapability()->setUiTreeHidden( true );
    m_plotDataFilterCollection = new RimPlotDataFilterCollection;

    connectAxisSignals( m_valueAxisProperties() );
    m_plotDataFilterCollection->filtersChanged.connect( this, &RimAnalysisPlot::onFiltersChanged );
    setDeletable( true );

    m_analyserOfSelectedCurveDefs = std::make_unique<RiaSummaryCurveDefinitionAnalyser>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::~RimAnalysisPlot()
{
    removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateCaseNameHasChanged()
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotDataFilterCollection* RimAnalysisPlot::plotDataFilterCollection() const
{
    return m_plotDataFilterCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions )
{
    m_analysisPlotDataSelection.deleteChildren();
    for ( const auto& curveDef : curveDefinitions )
    {
        auto dataEntry = new RimAnalysisPlotDataEntry();
        dataEntry->setFromCurveDefinition( curveDef );
        m_analysisPlotDataSelection.push_back( dataEntry );
    }
    connectAllCaseSignals();

    auto timeSteps = allAvailableTimeSteps();
    if ( m_selectedTimeSteps().empty() && !timeSteps.empty() )
    {
        m_selectedTimeSteps.v().push_back( RiaQDateTimeTools::fromTime_t( *timeSteps.rbegin() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::setTimeSteps( const std::vector<time_t>& timeSteps )
{
    m_selectedTimeSteps.v().clear();
    for ( auto time : timeSteps )
    {
        m_selectedTimeSteps.v().push_back( RiaQDateTimeTools::fromTime_t( time ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimAnalysisPlot::unfilteredAddresses() const
{
    std::set<RifEclipseSummaryAddress> addresses;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = updateAndGetCurveAnalyzer();

    for ( RimSummaryCase* sumCase : analyserOfSelectedCurveDefs->m_singleSummaryCases )
    {
        const std::set<RifEclipseSummaryAddress>& caseAddrs = sumCase->summaryReader()->allResultAddresses();
        addresses.insert( caseAddrs.begin(), caseAddrs.end() );
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigEnsembleParameter> RimAnalysisPlot::ensembleParameters() const
{
    std::set<RigEnsembleParameter> ensembleParms;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = updateAndGetCurveAnalyzer();

    std::set<RimSummaryCaseCollection*> ensembles;

    for ( RimSummaryCase* sumCase : analyserOfSelectedCurveDefs->m_singleSummaryCases )
    {
        if ( sumCase->ensemble() )
        {
            ensembles.insert( sumCase->ensemble() );
        }
    }

    for ( RimSummaryCaseCollection* ensemble : ensembles )
    {
        std::vector<RigEnsembleParameter> parameters = ensemble->variationSortedEnsembleParameters();
        ensembleParms.insert( parameters.begin(), parameters.end() );
    }

    return ensembleParms;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter RimAnalysisPlot::ensembleParameter( const QString& ensembleParameterName ) const
{
    std::set<RigEnsembleParameter> ensembleParms = ensembleParameters();
    for ( const RigEnsembleParameter& eParam : ensembleParms )
    {
        if ( eParam.name == ensembleParameterName ) return eParam;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::maxMinValueFromAddress( const RifEclipseSummaryAddress&           address,
                                              RimPlotDataFilterItem::TimeStepSourceType timeStepSourceType,
                                              const std::vector<QDateTime>&             timeRangeOrSelection,
                                              bool                                      useAbsValue,
                                              double*                                   minVal,
                                              double*                                   maxVal ) const
{
    double min = std::numeric_limits<double>::infinity();
    double max = useAbsValue ? 0.0 : -std::numeric_limits<double>::infinity();

    std::function<double( double, double )> minOrAbsMin;
    std::function<double( double, double )> maxOrAbsMax;

    if ( useAbsValue )
    {
        minOrAbsMin = []( double v1, double v2 ) { return std::min( fabs( v1 ), fabs( v2 ) ); };
        maxOrAbsMax = []( double v1, double v2 ) { return std::max( fabs( v1 ), fabs( v2 ) ); };
    }
    else
    {
        minOrAbsMin = []( double v1, double v2 ) { return std::min( v1, v2 ); };
        maxOrAbsMax = []( double v1, double v2 ) { return std::max( v1, v2 ); };
    }

    std::vector<time_t> selectedTimesteps;
    if ( timeStepSourceType == RimPlotDataFilterItem::SELECT_TIMESTEPS )
    {
        for ( const QDateTime& dateTime : timeRangeOrSelection )
        {
            selectedTimesteps.push_back( dateTime.toSecsSinceEpoch() );
        }
    }
    else if ( timeStepSourceType == RimPlotDataFilterItem::PLOT_SOURCE_TIMESTEPS )
    {
        selectedTimesteps = selectedTimeSteps();
    }

    std::set<RimSummaryCase*> allSumCases = allSourceCases();

    for ( RimSummaryCase* sumCase : allSumCases )
    {
        RifSummaryReaderInterface* reader = sumCase->summaryReader();
        if ( !reader ) continue;

        if ( reader->hasAddress( address ) )
        {
            auto [isOk, values]                  = reader->values( address );
            const std::vector<time_t>& timesteps = reader->timeSteps( address );

            if ( !timesteps.empty() && !values.empty() )
            {
                if ( timeStepSourceType == RimPlotDataFilterItem::LAST_TIMESTEP )
                {
                    min = minOrAbsMin( min, values[timesteps.size() - 1] );
                    max = maxOrAbsMax( max, values[timesteps.size() - 1] );
                }
                else if ( timeStepSourceType == RimPlotDataFilterItem::FIRST_TIMESTEP )
                {
                    min = minOrAbsMin( min, values[0] );
                    max = maxOrAbsMax( max, values[0] );
                }
                else if ( timeStepSourceType == RimPlotDataFilterItem::ALL_TIMESTEPS )
                {
                    for ( size_t tIdx = 0; tIdx < timesteps.size(); ++tIdx )
                    {
                        min = minOrAbsMin( min, values[tIdx] );
                        max = maxOrAbsMax( max, values[tIdx] );
                    }
                }
                else if ( timeStepSourceType == RimPlotDataFilterItem::SELECT_TIMESTEP_RANGE )
                {
                    if ( timeRangeOrSelection.size() >= 2 )
                    {
                        time_t minTime = timeRangeOrSelection.front().toSecsSinceEpoch();
                        time_t maxTime = timeRangeOrSelection.back().toSecsSinceEpoch();

                        for ( size_t tIdx = 0; tIdx < timesteps.size(); ++tIdx )
                        {
                            time_t dateTime = timesteps[tIdx];

                            if ( minTime <= dateTime && dateTime <= maxTime )
                            {
                                min = minOrAbsMin( min, values[tIdx] );
                                max = maxOrAbsMax( max, values[tIdx] );
                            }
                        }
                    }
                }
                else if ( timeStepSourceType == RimPlotDataFilterItem::LAST_TIMESTEP_WITH_HISTORY )
                {
                    RifEclipseSummaryAddress historyAddr = address;

                    if ( !historyAddr.isHistoryVector() ) historyAddr.setVectorName( address.vectorName() + "H" );

                    const std::vector<time_t>& historyTimesteps = reader->timeSteps( historyAddr );
                    if ( !historyTimesteps.empty() )
                    {
                        min = minOrAbsMin( min, values[historyTimesteps.size() - 1] );
                        max = maxOrAbsMax( max, values[historyTimesteps.size() - 1] );
                    }
                }
                else if ( !selectedTimesteps.empty() )
                {
                    std::vector<size_t> selectedTimestepIndices = RimAnalysisPlot::findTimestepIndices( selectedTimesteps, timesteps );

                    for ( size_t tsIdx : selectedTimestepIndices )
                    {
                        min = minOrAbsMin( min, values[tsIdx] );
                        max = maxOrAbsMax( max, values[tsIdx] );
                    }
                }
            }
        }
    }

    ( *minVal ) = min;
    ( *maxVal ) = max;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onFiltersChanged( const caf::SignalEmitter* emitter )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimAnalysisPlot::selectedTimeSteps() const
{
    std::vector<time_t> selectedTimeTTimeSteps;
    for ( const QDateTime& dateTime : m_selectedTimeSteps.v() )
    {
        selectedTimeTTimeSteps.push_back( dateTime.toSecsSinceEpoch() );
    }

    return selectedTimeTTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_selectVariablesButtonField )
    {
        // Do select variables
        RiuSummaryVectorSelectionDialog dlg( nullptr );

        dlg.enableMultiSelect( true );
        dlg.enableIndividualEnsembleCaseSelection( true );
        dlg.hideEnsembles();
        dlg.setCurveSelection( curveDefinitions() );

        if ( dlg.exec() == QDialog::Accepted )
        {
            std::vector<RiaSummaryCurveDefinition> summaryVectorDefinitions = dlg.curveSelection();

            m_analysisPlotDataSelection.deleteChildren();
            for ( const RiaSummaryCurveDefinition& vectorDef : summaryVectorDefinitions )
            {
                auto dataEntry = new RimAnalysisPlotDataEntry();
                dataEntry->setFromCurveDefinition( vectorDef );
                m_analysisPlotDataSelection.push_back( dataEntry );
            }
            connectAllCaseSignals();
        }

        m_selectVariablesButtonField = false;
    }
    else if ( changedField == &m_timeStepFilter )
    {
        m_selectedTimeSteps.v().clear();

        updateConnectedEditors();
    }

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* selVectorsGrp = uiOrdering.addNewGroup( "Selected Vectors" );
    selVectorsGrp->add( &m_selectedVarsUiField );
    selVectorsGrp->add( &m_selectVariablesButtonField, { false } );
    selVectorsGrp->add( &m_referenceCase, { true, 3, 2 } );

    QString vectorNames;
    if ( updateAndGetCurveAnalyzer() )
    {
        for ( const std::string& vectorName : updateAndGetCurveAnalyzer()->m_vectorNames )
        {
            vectorNames += QString::fromStdString( vectorName ) + ", ";
        }

        if ( !vectorNames.isEmpty() )
        {
            vectorNames.chop( 2 );
        }
    }

    if ( !vectorNames.isEmpty() )
    {
        m_selectedVarsUiField = vectorNames;
    }
    else
    {
        m_selectedVarsUiField = "Select Data Sources -->";
    }

    caf::PdmUiGroup* timeStepGrp = uiOrdering.addNewGroup( "Time Steps" );
    timeStepGrp->add( &m_timeStepFilter );
    timeStepGrp->add( &m_selectedTimeSteps );

    caf::PdmUiGroup* titleGrp = uiOrdering.addNewGroup( "Title and Legend" );
    titleGrp->add( &m_showPlotTitle );
    titleGrp->add( &m_useAutoPlotTitle, { false } );
    titleGrp->add( &m_description, { false } );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
    titleGrp->add( &m_showPlotLegends );
    titleGrp->add( &m_legendFontSize );
    m_legendFontSize.uiCapability()->setUiReadOnly( !m_showPlotLegends() );

    caf::PdmUiGroup* chartSettings = uiOrdering.addNewGroup( "Bar Settings" );
    chartSettings->add( &m_barOrientation, { true, 3, 2 } );

    chartSettings->add( &m_majorGroupType );
    chartSettings->add( &m_mediumGroupType );
    chartSettings->add( &m_minorGroupType );
    chartSettings->add( &m_valueSortOperation );
    chartSettings->add( &m_useTopBarsFilter );
    chartSettings->add( &m_maxBarCount, { false } );
    m_maxBarCount.uiCapability()->setUiReadOnly( !m_useTopBarsFilter() );
    chartSettings->add( &m_sortGroupForColors );

    caf::PdmUiGroup* barLabelGrp = uiOrdering.addNewGroup( "Bar Labels" );
    barLabelGrp->add( &m_useBarText );
    barLabelGrp->add( &m_barTextFontSize );
    barLabelGrp->add( &m_useVectorNameInBarText );
    barLabelGrp->add( &m_useSummaryItemInBarText );
    barLabelGrp->add( &m_useCaseInBarText );
    barLabelGrp->add( &m_useEnsembleInBarText );
    barLabelGrp->add( &m_useTimeStepInBarText );

    m_barTextFontSize.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useVectorNameInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useSummaryItemInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useCaseInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useEnsembleInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useTimeStepInBarText.uiCapability()->setUiReadOnly( !m_useBarText );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnalysisPlot::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimAnalysisPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );

    if ( !options.isEmpty() ) return options;

    if ( fieldNeedingOptions == &m_selectedTimeSteps )
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
    else if ( fieldNeedingOptions == &m_valueSortOperation )
    {
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::NONE ), SortGroupType::NONE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::VALUE ), SortGroupType::VALUE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::ABS_VALUE ), SortGroupType::ABS_VALUE ) );
    }
    else if ( fieldNeedingOptions == &m_majorGroupType || fieldNeedingOptions == &m_mediumGroupType ||
              fieldNeedingOptions == &m_minorGroupType || fieldNeedingOptions == &m_sortGroupForColors )
    {
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::NONE ), SortGroupType::NONE ) );
        QStringList currentSummaryItems;
        for ( const auto& summaryAddr : updateAndGetCurveAnalyzer()->m_summaryAdresses )
        {
            currentSummaryItems.push_back( QString::fromStdString( summaryAddr.itemUiText() ) );
        }
        currentSummaryItems.removeDuplicates();
        if ( !currentSummaryItems.isEmpty() )
        {
            QString exampleString = currentSummaryItems.join( ", " );
            if ( exampleString.length() > 16 )
            {
                exampleString = exampleString.left( 13 ) + "...";
            }

            QString summaryItemText = QString( "%1 (%2)" ).arg( SortGroupAppEnum::uiText( SortGroupType::SUMMARY_ITEM ) ).arg( exampleString );
            options.push_back( caf::PdmOptionItemInfo( summaryItemText, SortGroupType::SUMMARY_ITEM ) );
        }
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::VECTOR ), SortGroupType::VECTOR ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::CASE ), SortGroupType::CASE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::ENSEMBLE ), SortGroupType::ENSEMBLE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SortGroupType::TIME_STEP ), SortGroupType::TIME_STEP ) );
    }
    else if ( fieldNeedingOptions == &m_referenceCase )
    {
        std::vector<RimSummaryCase*> allSummaryCases = RimProject::current()->allSummaryCases();

        options.push_back( { "None", nullptr } );

        for ( auto sumCase : allSummaryCases )
        {
            QString displayName = sumCase->displayCaseName();
            auto    caseColl    = dynamic_cast<RimSummaryCaseCollection*>( sumCase->parentField()->ownerObject() );
            if ( caseColl )
            {
                displayName = caseColl->name() + "/" + displayName;
            }

            options.push_back( { displayName, sumCase } );
        }
    }
    else if ( fieldNeedingOptions == &m_barTextFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimAnalysisPlot::allAvailableTimeSteps() const
{
    std::set<time_t> timeStepUnion;

    for ( RimSummaryCase* sumCase : timestepDefiningSourceCases() )
    {
        if ( !sumCase || !sumCase->summaryReader() ) continue;

        const std::vector<time_t>& timeSteps = sumCase->summaryReader()->timeSteps( RifEclipseSummaryAddress() );

        for ( time_t t : timeSteps )
        {
            timeStepUnion.insert( t );
        }
    }

    return timeStepUnion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimAnalysisPlot::timestepDefiningSourceCases() const
{
    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = updateAndGetCurveAnalyzer();
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
std::set<RimSummaryCase*> RimAnalysisPlot::allSourceCases() const
{
    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = updateAndGetCurveAnalyzer();
    std::set<RimSummaryCase*>          allSumCases                 = analyserOfSelectedCurveDefs->m_singleSummaryCases;

    return allSumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimAnalysisPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    updateAndGetCurveAnalyzer();

    if ( m_plotWidget )
    {
        m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotScale );

        RiuGroupedBarChartBuilder chartBuilder;
        chartBuilder.setLabelFontSize( barTextFontSize() );
        // buildTestPlot( chartBuilder );
        addDataToChartBuilder( chartBuilder );

        chartBuilder.addBarChartToPlot( m_plotWidget->qwtPlot(),
                                        m_barOrientation == BarOrientation::BARS_HORIZONTAL ? Qt::Horizontal : Qt::Vertical,
                                        m_useTopBarsFilter() ? m_maxBarCount : -1 );

        if ( m_showPlotLegends && m_plotWidget->qwtPlot()->legend() == nullptr )
        {
            auto* legend = new QwtLegend( m_plotWidget );
            m_plotWidget->qwtPlot()->insertLegend( legend, QwtPlot::RightLegend );
        }
        else if ( !m_showPlotLegends )
        {
            m_plotWidget->qwtPlot()->insertLegend( nullptr );
        }

        m_plotWidget->setLegendFontSize( legendFontSize() );
        m_plotWidget->updateLegend();
    }

    updateAxes();
    updatePlotTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimAnalysisPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimAnalysisPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

        new RiuContextMenuLauncher( m_plotWidget, { "RicShowPlotDataFeature" } );
    }

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimAnalysisPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimAnalysisPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->qwtPlot()->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    RiuPlotAxis axis = RiuPlotAxis::defaultLeft();
    if ( m_barOrientation == BarOrientation::BARS_HORIZONTAL )
    {
        axis = RiuPlotAxis::defaultBottom();
        m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), false );
    }
    else
    {
        m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), false );
    }

    RimPlotAxisProperties* valAxisProperties = m_valueAxisProperties();
    if ( valAxisProperties->isActive() )
    {
        m_plotWidget->enableAxis( axis, true );
        m_valueAxisProperties->setNameAndAxis( "Value-Axis", "Value-Axis", axis.axis() );

        RimSummaryPlotAxisFormatter calc( valAxisProperties, {}, curveDefinitions(), {}, {} );
        calc.applyAxisPropertiesToPlot( m_plotWidget );
    }
    else
    {
        m_plotWidget->enableAxis( axis, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onAxisSelected( RiuPlotAxis axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();

    caf::PdmObject* itemToSelect = nullptr;
    if ( axis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
    {
        if ( m_barOrientation == BarOrientation::BARS_VERTICAL )
        {
            itemToSelect = m_valueAxisProperties;
        }
        else
        {
            itemToSelect = this;
        }
    }
    else if ( axis.axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
    {
        if ( m_barOrientation == BarOrientation::BARS_HORIZONTAL )
        {
            itemToSelect = m_valueAxisProperties;
        }
        else
        {
            itemToSelect = this;
        }
    }

    RiuPlotMainWindowTools::selectOrToggleObject( itemToSelect, toggle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::asciiDataForPlotExport() const
{
    RiuGroupedBarChartBuilder chartBuilder;
    addDataToChartBuilder( chartBuilder );

    return chartBuilder.plotContentAsText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::cleanupBeforeClose()
{
    detachAllCurves();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAnalysisPlot::assignGroupingText( RimAnalysisPlot::SortGroupType  sortGroup,
                                             const RiaSummaryCurveDefinition dataEntry,
                                             const QString&                  timestepString ) const
{
    QString groupingText = "";

    switch ( sortGroup )
    {
        case RimAnalysisPlot::SortGroupType::SUMMARY_ITEM:
        {
            RifEclipseSummaryAddress addr = dataEntry.summaryAddressY();
            groupingText                  = QString::fromStdString( addr.itemUiText() );
        }
        break;
        case RimAnalysisPlot::SortGroupType::CASE:
        {
            if ( dataEntry.summaryCaseY() )
            {
                groupingText = dataEntry.summaryCaseY()->displayCaseName();
            }
        }
        break;
        case RimAnalysisPlot::SortGroupType::ENSEMBLE:
        {
            if ( dataEntry.ensemble() )
            {
                groupingText = dataEntry.ensemble()->name();
            }
        }
        break;
        case RimAnalysisPlot::SortGroupType::VECTOR:
        {
            RifEclipseSummaryAddress addr = dataEntry.summaryAddressY();

            groupingText = QString::fromStdString( addr.vectorName() );
        }
        break;
        case RimAnalysisPlot::SortGroupType::TIME_STEP:
        {
            groupingText = timestepString;
        }
        break;
        default:
        {
            // Return empty string
        }
        break;
    }

    return groupingText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimAnalysisPlot::findTimestepIndices( std::vector<time_t> selectedTimesteps, const std::vector<time_t>& timesteps )
{
    std::vector<size_t> selectedTimestepIndices;

    for ( time_t tt : selectedTimesteps )
    {
        for ( size_t timestepIdx = 0; timestepIdx < timesteps.size(); ++timestepIdx )
        {
            if ( timesteps[timestepIdx] == tt )
            {
                selectedTimestepIndices.push_back( timestepIdx );
                break;
            }
        }
    }

    return selectedTimestepIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimAnalysisPlot::filteredCurveDefs() const
{
    std::vector<RiaSummaryCurveDefinition> dataDefinitions = curveDefinitions();

    // Split out the filter targets

    std::set<RimSummaryCase*>          filteredSumCases;
    std::set<RifEclipseSummaryAddress> filteredSummaryItems; // Stores only the unique summary items

    for ( const auto& curveDef : dataDefinitions )
    {
        if ( curveDef.summaryCaseY() )
        {
            filteredSumCases.insert( curveDef.summaryCaseY() );

            RifEclipseSummaryAddress address = curveDef.summaryAddressY();

            address.setVectorName( "" ); // Vector name set to "" in order to store only unique summary items
            filteredSummaryItems.insert( address );
        }
    }

    std::vector<RimPlotDataFilterItem*> filters = m_plotDataFilterCollection->filters();

    for ( RimPlotDataFilterItem* filter : filters )
    {
        applyFilter( filter, &filteredSumCases, &filteredSummaryItems );
    }

    // Remove all

    std::vector<RiaSummaryCurveDefinition> filteredDataDefinitions;

    for ( const RiaSummaryCurveDefinition& curveDefCandidate : dataDefinitions )
    {
        RimSummaryCase*          sumCase = curveDefCandidate.summaryCaseY();
        RifEclipseSummaryAddress addr    = curveDefCandidate.summaryAddressY();
        addr.setVectorName( "" );

        if ( filteredSumCases.count( sumCase ) && filteredSummaryItems.count( addr ) )
        {
            filteredDataDefinitions.push_back( curveDefCandidate );
        }
    }

    return filteredDataDefinitions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

void RimAnalysisPlot::applyFilter( const RimPlotDataFilterItem*        filter,
                                   std::set<RimSummaryCase*>*          filteredSumCases,
                                   std::set<RifEclipseSummaryAddress>* filteredSummaryItems ) const
{
    if ( !filter->isActive() || !filter->isValid() ) return;

    std::set<RimSummaryCase*>          casesToKeep;
    std::set<RifEclipseSummaryAddress> sumItemsToKeep;

    std::map<RimSummaryCase*, double>          casesToKeepWithValue;
    std::map<RifEclipseSummaryAddress, double> sumItemsToKeepWithValue;

    if ( filter->filterTarget() == RimPlotDataFilterItem::ENSEMBLE_CASE )
    {
        if ( !filter->ensembleParameterName().isEmpty() )
        {
            sumItemsToKeep = ( *filteredSummaryItems ); // Not filtering items

            RigEnsembleParameter eParam = ensembleParameter( filter->ensembleParameterName() );

            for ( auto sumCase : ( *filteredSumCases ) )
            {
                if ( !eParam.isValid() ) continue;
                if ( !sumCase->caseRealizationParameters() ) continue;

                RigCaseRealizationParameters::Value crpValue =
                    sumCase->caseRealizationParameters()->parameterValue( filter->ensembleParameterName() );

                if ( eParam.isNumeric() && crpValue.isNumeric() )
                {
                    double value = crpValue.numericValue();

                    if ( filter->filterOperation() == RimPlotDataFilterItem::RANGE )
                    {
                        auto [min, max] = filter->filterRangeMinMax();

                        if ( min <= value && value <= max )
                        {
                            casesToKeep.insert( sumCase );
                        }
                    }
                    else if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N ||
                              filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
                    {
                        bool useLargest = filter->filterOperation() == RimPlotDataFilterItem::TOP_N;

                        auto itIsInsertedPair = casesToKeepWithValue.insert( { sumCase, value } );
                        if ( !itIsInsertedPair.second ) // Already exists in map
                        {
                            double& insertedValue = itIsInsertedPair.first->second;
                            if ( ( useLargest && ( insertedValue < value ) ) || ( !useLargest && ( value < insertedValue ) ) )
                            {
                                insertedValue = value;
                            }
                        }
                    }
                }
                else if ( eParam.isText() && crpValue.isText() )
                {
                    const auto& filterCategories = filter->selectedEnsembleParameterCategories();

                    if ( crpValue.isText() && std::count( filterCategories.begin(), filterCategories.end(), crpValue.textValue() ) == 0 )
                    {
                        casesToKeep.insert( sumCase );
                    }
                }
            }
        }
        else
        {
            casesToKeep = ( *filteredSumCases );
        }
    }
    else
    {
        std::vector<time_t> selectedTimesteps;

        if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::SELECT_TIMESTEPS )
        {
            selectedTimesteps = filter->explicitlySelectedTimeSteps();
        }
        else if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::PLOT_SOURCE_TIMESTEPS )
        {
            selectedTimesteps = selectedTimeSteps();
        }

        std::function<void( double )> storeResultCoreLambda;

        RimSummaryCase* sumCaseInEvaluation = nullptr;

        // clang-format off
        std::function<void( RifEclipseSummaryAddress )> evaluateFilterForAllCases = 
        [&]( RifEclipseSummaryAddress addrToFilterValue ) // clang-format on
        {
            for ( auto sumCase : *filteredSumCases )
            {
                sumCaseInEvaluation = sumCase;

                RifSummaryReaderInterface* reader = sumCase->summaryReader();
                if ( !reader ) continue;

                if ( reader->hasAddress( addrToFilterValue ) )
                {
                    auto [isOk, values]                  = reader->values( addrToFilterValue );
                    const std::vector<time_t>& timesteps = reader->timeSteps( addrToFilterValue );

                    if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::ALL_TIMESTEPS )
                    {
                        for ( size_t tIdx = 0; tIdx < timesteps.size(); ++tIdx )
                        {
                            double value = values[tIdx];

                            storeResultCoreLambda( value );
                        }
                    }
                    else if ( !timesteps.empty() )
                    {
                        std::vector<size_t> selectedTimestepIndices;

                        if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::FIRST_TIMESTEP )
                        {
                            selectedTimestepIndices.push_back( 0 );
                        }
                        else if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::LAST_TIMESTEP )
                        {
                            size_t timeStepIdx = timesteps.size() - 1;
                            selectedTimestepIndices.push_back( timeStepIdx );
                        }
                        else if ( !selectedTimesteps.empty() )
                        {
                            selectedTimestepIndices = RimAnalysisPlot::findTimestepIndices( selectedTimesteps, timesteps );
                        }
                        else if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::LAST_TIMESTEP_WITH_HISTORY )
                        {
                            RifEclipseSummaryAddress historyAddr = addrToFilterValue;

                            if ( !historyAddr.isHistoryVector() ) historyAddr.setVectorName( addrToFilterValue.vectorName() + "H" );

                            std::vector<time_t> historyTimesteps = reader->timeSteps( historyAddr );
                            if ( !historyTimesteps.empty() )
                            {
                                selectedTimestepIndices = RimAnalysisPlot::findTimestepIndices( { historyTimesteps.back() }, timesteps );
                            }
                        }
                        else if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::SELECT_TIMESTEP_RANGE )
                        {
                            std::pair<time_t, time_t> timeMinMax = filter->timeRangeMinMax();

                            for ( size_t tIdx = 0; tIdx < timesteps.size(); ++tIdx )
                            {
                                time_t dateTime = timesteps[tIdx];

                                if ( timeMinMax.first <= dateTime && dateTime <= timeMinMax.second )
                                {
                                    selectedTimestepIndices.push_back( tIdx );
                                }
                            }
                        }

                        for ( size_t timeStepIdx : selectedTimestepIndices )
                        {
                            double value = values[timeStepIdx];
                            storeResultCoreLambda( value );
                        }
                    }
                }
            }
        };

        if ( filter->filterTarget() == RimPlotDataFilterItem::SUMMARY_CASE )
        {
            sumItemsToKeep = ( *filteredSummaryItems ); // Not filtering items

            RifEclipseSummaryAddress addrToFilterValue = filter->summaryAddress();

            if ( filter->filterOperation() == RimPlotDataFilterItem::RANGE )
            {
                std::pair<double, double> minMax = filter->filterRangeMinMax();

                // clang-format off
                storeResultCoreLambda = [&]( double value ) // clang-format on
                {
                    if ( minMax.first <= value && value <= minMax.second )
                    {
                        casesToKeep.insert( sumCaseInEvaluation );
                    }
                };
            }
            else if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N || filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
            {
                // clang-format off
                storeResultCoreLambda = [&]( double value ) // clang-format on
                {
                    bool useLargest = filter->filterOperation() == RimPlotDataFilterItem::TOP_N;

                    auto itIsInsertedPair = casesToKeepWithValue.insert( { sumCaseInEvaluation, value } );
                    if ( !itIsInsertedPair.second ) // Already exists in map
                    {
                        double& insertedValue = itIsInsertedPair.first->second;
                        if ( ( useLargest && ( insertedValue < value ) ) || ( !useLargest && ( value < insertedValue ) ) )
                        {
                            insertedValue = value;
                        }
                    }
                };
            }

            evaluateFilterForAllCases( addrToFilterValue );
        }
        else if ( filter->filterTarget() == RimPlotDataFilterItem::SUMMARY_ITEM )
        {
            casesToKeep = ( *filteredSumCases ); // Not filtering cases

            std::string quantityName;
            {
                RifEclipseSummaryAddress addrToFilterValue = filter->summaryAddress();

                quantityName = addrToFilterValue.vectorName();
            }

            for ( auto sumItem : *filteredSummaryItems )
            {
                RifEclipseSummaryAddress addrToFilterValue = sumItem;
                addrToFilterValue.setVectorName( quantityName );

                if ( filter->filterOperation() == RimPlotDataFilterItem::RANGE )
                {
                    std::pair<double, double> minMax = filter->filterRangeMinMax();

                    // clang-format off
                    storeResultCoreLambda = [&]( double value ) // clang-format on
                    {
                        if ( minMax.first <= value && value <= minMax.second )
                        {
                            sumItemsToKeep.insert( sumItem );
                        }
                    };
                }
                else if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N ||
                          filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
                {
                    // clang-format off
                    storeResultCoreLambda = [&]( double value ) // clang-format on
                    {
                        bool useLargest = filter->filterOperation() == RimPlotDataFilterItem::TOP_N;

                        auto itIsInsertedPair = sumItemsToKeepWithValue.insert( { sumItem, value } );
                        if ( !itIsInsertedPair.second ) // Already exists in map
                        {
                            double& insertedValue = itIsInsertedPair.first->second;
                            if ( ( useLargest && ( insertedValue < value ) ) || ( !useLargest && ( value < insertedValue ) ) )
                            {
                                insertedValue = value;
                            }
                        }
                    };
                }

                evaluateFilterForAllCases( addrToFilterValue );
            }
        }
    }

    // Handle top/bottom n filter

    if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N || filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
    {
        if ( filter->filterTarget() == RimPlotDataFilterItem::SUMMARY_ITEM )
        {
            std::multimap<double, RifEclipseSummaryAddress> valueSortedSumItems;
            for ( const auto& itemValPair : sumItemsToKeepWithValue )
            {
                valueSortedSumItems.insert( { itemValPair.second, itemValPair.first } );
            }

            if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumItems.rbegin(); count < filter->topBottomN() && it != valueSortedSumItems.rend(); ++it )
                {
                    sumItemsToKeep.insert( it->second );
                    ++count;
                }
            }
            else if ( filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumItems.begin(); count < filter->topBottomN() && it != valueSortedSumItems.end(); ++it )
                {
                    sumItemsToKeep.insert( it->second );
                    ++count;
                }
            }
        }
        else
        {
            std::multimap<double, RimSummaryCase*> valueSortedSumCases;
            for ( const auto& caseValPair : casesToKeepWithValue )
            {
                valueSortedSumCases.insert( { caseValPair.second, caseValPair.first } );
            }

            if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumCases.rbegin(); count < filter->topBottomN() && it != valueSortedSumCases.rend(); ++it )
                {
                    casesToKeep.insert( it->second );
                    ++count;
                }
            }
            else if ( filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumCases.begin(); count < filter->topBottomN() && it != valueSortedSumCases.end(); ++it )
                {
                    casesToKeep.insert( it->second );
                    ++count;
                }
            }
        }
    }

    ( *filteredSumCases )     = casesToKeep;
    ( *filteredSummaryItems ) = sumItemsToKeep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder ) const
{
    std::vector<time_t> selectedTimesteps;
    for ( const QDateTime& dateTime : m_selectedTimeSteps.v() )
    {
        selectedTimesteps.push_back( dateTime.toSecsSinceEpoch() );
    }

    RifSummaryReaderInterface* referenceCaseReader = nullptr;

    if ( m_referenceCase ) referenceCaseReader = m_referenceCase->summaryReader();

    // Unpack ensemble curves and make one curve definition for each individual curve.
    // Store both ensemble and summary case in the definition

    std::vector<RiaSummaryCurveDefinition> barDataDefinitions = filteredCurveDefs();

    for ( const RiaSummaryCurveDefinition& curveDef : barDataDefinitions )
    {
        if ( !curveDef.summaryCaseY() ) continue;
        RifSummaryReaderInterface* reader = curveDef.summaryCaseY()->summaryReader();

        if ( !reader ) continue;

        std::vector<time_t> timeSteps;
        std::vector<double> values;

        if ( referenceCaseReader )
        {
            std::pair<std::vector<time_t>, std::vector<double>> timeAndValues =
                RimDerivedSummaryCase::calculateDerivedValues( reader,
                                                               -1,
                                                               referenceCaseReader,
                                                               -1,
                                                               DerivedSummaryOperator::DERIVED_OPERATOR_SUB,
                                                               curveDef.summaryAddressY() );
            timeSteps.swap( timeAndValues.first );
            values.swap( timeAndValues.second );
        }
        else
        {
            timeSteps = reader->timeSteps( curveDef.summaryAddressY() );

            auto [isOk, readValues] = reader->values( curveDef.summaryAddressY() );
            values.swap( readValues );
        }

        if ( timeSteps.empty() || values.empty() ) continue;

        // Find selected timestep indices

        std::vector<int> selectedTimestepIndices;

        for ( time_t tt : selectedTimesteps )
        {
            for ( int timestepIdx = 0; static_cast<unsigned>( timestepIdx ) < timeSteps.size(); ++timestepIdx )
            {
                if ( timeSteps[timestepIdx] == tt )
                {
                    selectedTimestepIndices.push_back( timestepIdx );
                    break;
                }
            }
        }

        for ( int timestepIdx : selectedTimestepIndices )
        {
            double sortValue = std::numeric_limits<double>::infinity();

            QDateTime dateTime     = RiaQDateTimeTools::fromTime_t( timeSteps[timestepIdx] );
            QString   formatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat() );

            QString timestepString = dateTime.toString( formatString );

            QString majorText  = assignGroupingText( m_majorGroupType(), curveDef, timestepString );
            QString medText    = assignGroupingText( m_mediumGroupType(), curveDef, timestepString );
            QString minText    = assignGroupingText( m_minorGroupType(), curveDef, timestepString );
            QString legendText = assignGroupingText( m_sortGroupForColors(), curveDef, timestepString );

            double value = values[timestepIdx];

            switch ( m_valueSortOperation() )
            {
                case SortGroupType::VALUE:
                    sortValue = value;
                    break;
                case SortGroupType::ABS_VALUE:
                    sortValue = fabs( value );
                    break;
            }

            QString barText;
            QString separator = ", ";

            if ( m_useBarText() )
            {
                QStringList barTextComponents;
                if ( m_useVectorNameInBarText )
                {
                    barTextComponents += QString::fromStdString( curveDef.summaryAddressY().vectorName() );
                }

                if ( m_useSummaryItemInBarText )
                {
                    barTextComponents += QString::fromStdString( curveDef.summaryAddressY().itemUiText() );
                }

                if ( m_useCaseInBarText && curveDef.summaryCaseY() )
                {
                    barTextComponents += curveDef.summaryCaseY()->displayCaseName();
                }

                if ( m_useEnsembleInBarText && curveDef.ensemble() )
                {
                    barTextComponents += curveDef.ensemble()->name();
                }

                if ( m_useTimeStepInBarText )
                {
                    barTextComponents += timestepString;
                }
                barText = barTextComponents.join( separator );
            }

            chartBuilder.addBarEntry( majorText, medText, minText, sortValue, legendText, barText, value );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle )
    {
        QString autoTitle;
        QString separator = ", ";

        if ( updateAndGetCurveAnalyzer()->m_ensembles.size() == 1 )
        {
            autoTitle += ( *updateAndGetCurveAnalyzer()->m_ensembles.begin() )->name();
        }

        if ( updateAndGetCurveAnalyzer()->m_singleSummaryCases.size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += ( *updateAndGetCurveAnalyzer()->m_singleSummaryCases.begin() )->displayCaseName();
        }
        else if ( updateAndGetCurveAnalyzer()->m_singleSummaryCases.size() > 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;

            QStringList caseNameList;
            for ( auto summaryCase : updateAndGetCurveAnalyzer()->m_singleSummaryCases )
            {
                caseNameList.push_back( summaryCase->displayCaseName() );
            }

            QString root        = RiaTextStringTools::commonRoot( caseNameList );
            QString trimmedRoot = RiaTextStringTools::trimNonAlphaNumericCharacters( root );
            if ( !trimmedRoot.isEmpty() )
            {
                autoTitle += trimmedRoot;
            }
            else
            {
                autoTitle += "Several Summary Cases";
            }
        }

        if ( updateAndGetCurveAnalyzer()->m_summaryAdresses.size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += QString::fromStdString( updateAndGetCurveAnalyzer()->m_summaryAdresses.begin()->itemUiText() );
        }

        for ( const std::string& quantName : updateAndGetCurveAnalyzer()->m_vectorNames )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += QString::fromStdString( quantName );
        }

        if ( m_referenceCase() )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += "Compared to " + m_referenceCase->displayCaseName();
        }

        if ( m_useTopBarsFilter )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += " - ";
            autoTitle += "Top " + QString::number( m_maxBarCount() );
        }

        if ( m_selectedTimeSteps().size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += " @ ";

            QString formatString = RiaPreferences::current()->dateTimeFormat( RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY,
                                                                              RiaDefines::TimeFormatComponents::TIME_FORMAT_NONE );
            autoTitle += m_selectedTimeSteps()[0].toString( formatString );
        }

        m_description = autoTitle;
    }

    if ( m_plotWidget )
    {
        QString plotTitle = description();
        m_plotWidget->setPlotTitle( plotTitle );
        m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinitionAnalyser* RimAnalysisPlot::updateAndGetCurveAnalyzer() const
{
    m_analyserOfSelectedCurveDefs->setCurveDefinitions( curveDefinitions() );

    return m_analyserOfSelectedCurveDefs.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimAnalysisPlot::curveDefinitions() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;
    for ( const auto& dataEntry : m_analysisPlotDataSelection )
    {
        curveDefs.push_back( dataEntry->curveDefinition() );
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimPlotAxisPropertiesInterface*> RimAnalysisPlot::allPlotAxes() const
{
    return { m_valueAxisProperties };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimAnalysisPlot::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimAnalysisPlot::axisLogarithmicChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::buildTestPlot( RiuGroupedBarChartBuilder& chartBuilder ) const
{
    chartBuilder.addBarEntry( "T1_The_red_Fox", "", "", std::numeric_limits<double>::infinity(), "R1", "", 0.4 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "", "", std::numeric_limits<double>::infinity(), "R2", "", 0.45 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W1", "", std::numeric_limits<double>::infinity(), "R1", "", 0.5 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W1", "", std::numeric_limits<double>::infinity(), "R2", "", 0.55 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W3", "", std::numeric_limits<double>::infinity(), "R1", "", 0.7 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W3", "", std::numeric_limits<double>::infinity(), "R2", "", 0.75 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W2", "", std::numeric_limits<double>::infinity(), "R1", "", 1.05 );
    chartBuilder.addBarEntry( "T1_The_red_Fox", "W2", "", std::numeric_limits<double>::infinity(), "R2", "", 1.0 );

    chartBuilder.addBarEntry( "T2", "W1", "", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
    chartBuilder.addBarEntry( "T2", "W1", "", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
    chartBuilder.addBarEntry( "T2", "W2", "", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T2", "W2", "", std::numeric_limits<double>::infinity(), "R2", "", 2.0 );

    chartBuilder.addBarEntry( "T3", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
    chartBuilder.addBarEntry( "T3", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
    chartBuilder.addBarEntry( "T3", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T3", "W2", "4", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T3", "W2", "5", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );

    chartBuilder.addBarEntry( "T4", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.5 );
    chartBuilder.addBarEntry( "T4", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.5 );
    chartBuilder.addBarEntry( "T4", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.0 );
    chartBuilder.addBarEntry( "T4", "W2", "4", std::numeric_limits<double>::infinity(), "R2", "", 2.0 );
    chartBuilder.addBarEntry( "T4", "W1", "1", std::numeric_limits<double>::infinity(), "R1", "", 1.6 );
    chartBuilder.addBarEntry( "T4", "W1", "2", std::numeric_limits<double>::infinity(), "R2", "", 1.6 );
    chartBuilder.addBarEntry( "T4", "W2", "3", std::numeric_limits<double>::infinity(), "R1", "", 2.6 );
    chartBuilder.addBarEntry( "T4", "W2", "4", std::numeric_limits<double>::infinity(), "R2", "", -0.3 );

    chartBuilder.addBarEntry( "T5", "", "", 1.5, "R3", "G1", 1.5 );
    chartBuilder.addBarEntry( "T5", "", "", 1.5, "R3", "G2", 1.5 );
    chartBuilder.addBarEntry( "T5", "", "", 2.0, "R3", "G3", 2.0 );
    chartBuilder.addBarEntry( "T5", "", "", 2.0, "R3", "G4", 2.0 );
    chartBuilder.addBarEntry( "T5", "", "", 1.6, "R3", "G5", 1.6 );
    chartBuilder.addBarEntry( "T5", "", "", 1.6, "R3", "G6", 1.6 );
    chartBuilder.addBarEntry( "T5", "", "", 2.6, "R3", "G7", 2.6 );
    chartBuilder.addBarEntry( "T5", "", "", -0.1, "R3", "G8", -0.1 );

    chartBuilder.addBarEntry( "", "", "", 1.2, "", "A", 1.2 );
    chartBuilder.addBarEntry( "", "", "", 1.5, "", "B", 1.5 );
    chartBuilder.addBarEntry( "", "", "", 2.3, "", "C", 2.3 );
    chartBuilder.addBarEntry( "", "", "", 2.0, "", "D", 2.0 );
    chartBuilder.addBarEntry( "", "", "", 1.6, "", "E", 1.6 );
    chartBuilder.addBarEntry( "", "", "", 2.4, "", "F", -2.4 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAnalysisPlot::barTextFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_barTextFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::initAfterRead()
{
    connectAllCaseSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onCaseRemoved( const SignalEmitter* emitter, RimSummaryCase* summaryCase )
{
    for ( const auto& existingEntry : m_analysisPlotDataSelection )
    {
        if ( existingEntry->summaryCase() == summaryCase )
        {
            m_analysisPlotDataSelection.removeChild( existingEntry );
            delete existingEntry;
            break;
        }
    }
    loadDataAndUpdate();
    if ( m_plotWidget ) m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::connectAllCaseSignals()
{
    for ( const auto& dataEntry : m_analysisPlotDataSelection )
    {
        if ( dataEntry->ensemble() )
        {
            dataEntry->ensemble()->caseRemoved.connect( this, &RimAnalysisPlot::onCaseRemoved );
        }
    }
}
