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

#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorSelectionDialog.h"

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

#include "qwt_column_symbol.h"
#include "qwt_legend.h"
#include "qwt_painter.h"
#include "qwt_plot_barchart.h"
#include "qwt_scale_draw.h"

#include "cafPdmUiActionPushButtonEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiListEditor.h"

#include <limits>
#include <map>

namespace caf
{
template <>
void caf::AppEnum<RimAnalysisPlot::SortGroupType>::setUp()
{
    addItem( RimAnalysisPlot::NONE, "NONE", "None" );
    addItem( RimAnalysisPlot::SUMMARY_ITEM, "SUMMARY_ITEM", "Summary Item" );
    addItem( RimAnalysisPlot::QUANTITY, "QUANTITY", "Quantity" );
    addItem( RimAnalysisPlot::CASE, "CASE", "Case" );
    addItem( RimAnalysisPlot::ENSEMBLE, "ENSEMBLE", "Ensemble" );
    addItem( RimAnalysisPlot::VALUE, "VALUE", "Value" );
    addItem( RimAnalysisPlot::ABS_VALUE, "ABS_VALUE", "abs(Value)" );
    addItem( RimAnalysisPlot::OTHER_VALUE, "OTHER_VALUE", "Other Value" );
    addItem( RimAnalysisPlot::ABS_OTHER_VALUE, "ABS_OTHER_VALUE", "abs(Other Value)" );
    addItem( RimAnalysisPlot::TIME_STEP, "TIME_STEP", "Time Step" );
    setDefault( RimAnalysisPlot::NONE );
}

template <>
void caf::AppEnum<RimAnalysisPlot::BarOrientation>::setUp()
{
    addItem( RimAnalysisPlot::BARS_HORIZONTAL, "BARS_HORIZONTAL", "Horizontal" );
    addItem( RimAnalysisPlot::BARS_VERTICAL, "BARS_VERTICAL", "Vertical" );
    setDefault( RimAnalysisPlot::BARS_VERTICAL );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimAnalysisPlot, "AnalysisPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnalysisPlot::RimAnalysisPlot()
    : RimPlot()
{
    CAF_PDM_InitObject( "Analysis Plot", ":/AnalysisPlot16x16.png", "", "" );

    // Variable selection

    CAF_PDM_InitFieldNoDefault( &m_selectedVarsUiField, "selectedVarsUiField", "Selected Variables", "", "", "" );
    m_selectedVarsUiField.xmlCapability()->disableIO();
    m_selectedVarsUiField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedVarsUiField.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_selectVariablesButtonField, "BrowseButton", false, "...", "", "", "" );
    caf::PdmUiActionPushButtonEditor::configureEditorForField( &m_selectVariablesButtonField );

    CAF_PDM_InitFieldNoDefault( &m_analysisPlotDataSelection, "AnalysisPlotData", "", "", "", "" );
    m_analysisPlotDataSelection.uiCapability()->setUiTreeChildrenHidden( true );
    m_analysisPlotDataSelection.uiCapability()->setUiTreeHidden( true );

    // Time Step Selection

    CAF_PDM_InitFieldNoDefault( &m_addTimestepUiField, "AddTimeStepsUiField", "Add Timestep:", "", "", "" );
    m_addTimestepUiField.xmlCapability()->disableIO();
    m_addTimestepUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "TimeSteps", "", "", "", "" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_selectedTimeSteps.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    // Options

    CAF_PDM_InitFieldNoDefault( &m_referenceCase, "ReferenceCase", "Reference Case", "", "", "" );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Title", "", "", "" );
    m_showPlotTitle.xmlCapability()->setIOWritable( false );
    m_showPlotTitle.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "IsUsingAutoName", true, "Auto", "", "", "" );
    m_useAutoPlotTitle.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "Analysis Plot" ), "Title", "", "", "" );
    m_description.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_barOrientation, "BarOrientation", "Bar Orientation", "", "", "" );

    // Grouping

    CAF_PDM_InitFieldNoDefault( &m_majorGroupType, "MajorGroupType", "Major Grouping", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_mediumGroupType, "MediumGroupType", "Medium Grouping", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_minorGroupType, "MinorGroupType", "Minor Grouping", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_valueSortOperation, "ValueSortOperation", "Sort by Value", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_sortGroupForLegend, "groupForLegend", "Legend Using", "", "", "" );

    CAF_PDM_InitField( &m_useTopBarsFilter, "UseTopBarsFilter", false, "Show Only Top", "", "", "" );
    m_useTopBarsFilter.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_maxBarCount, "MaxBarCount", 20, "Bar Count", "", "", "" );
    m_maxBarCount.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    // Bar text

    CAF_PDM_InitField( &m_useBarText, "UseBarText", true, "Activate Bar Labels", "", "", "" );
    m_useBarText.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_useCaseInBarText, "UseCaseInBarText", false, "Case Name", "", "", "" );
    CAF_PDM_InitField( &m_useEnsembleInBarText, "UseEnsembleInBarText", false, "Ensemble", "", "", "" );
    CAF_PDM_InitField( &m_useSummaryItemInBarText, "UseSummaryItemInBarText", false, "Summary Item", "", "", "" );
    CAF_PDM_InitField( &m_useTimeStepInBarText, "UseTimeStepInBarText", false, "Time Step", "", "", "" );
    CAF_PDM_InitField( &m_useQuantityInBarText, "UseQuantityInBarText", false, "Quantity", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_valueAxisProperties, "ValueAxisProperties", "ValueAxisProperties", "", "", "" );
    m_valueAxisProperties.uiCapability()->setUiTreeHidden( true );
    m_valueAxisProperties = new RimPlotAxisProperties;
    m_valueAxisProperties->setNameAndAxis( "Value-Axis", QwtPlot::yLeft );
    m_valueAxisProperties->enableRangeSettings( false );

    CAF_PDM_InitFieldNoDefault( &m_plotDataFilterCollection, "PlotDataFilterCollection", "PlotDataFilterCollection", "", "", "" );
    m_plotDataFilterCollection.uiCapability()->setUiTreeHidden( true );
    m_plotDataFilterCollection = new RimPlotDataFilterCollection;

    setDeletable( true );
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
    this->onLoadDataAndUpdate();
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
std::set<RifEclipseSummaryAddress> RimAnalysisPlot::unfilteredAddresses()
{
    std::set<RifEclipseSummaryAddress> addresses;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

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
std::set<EnsembleParameter> RimAnalysisPlot::ensembleParameters()
{
    std::set<EnsembleParameter> ensembleParms;

    RiaSummaryCurveDefinitionAnalyser* analyserOfSelectedCurveDefs = getOrCreateSelectedCurveDefAnalyser();

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
        std::vector<EnsembleParameter> parameters = ensemble->variationSortedEnsembleParameters();
        ensembleParms.insert( parameters.begin(), parameters.end() );
    }

    return ensembleParms;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimAnalysisPlot::ensembleParameter( const QString& ensembleParameterName )
{
    std::set<EnsembleParameter> ensembleParms = ensembleParameters();
    for ( const EnsembleParameter& eParam : ensembleParms )
    {
        if ( eParam.name == ensembleParameterName ) return eParam;
    }

    return EnsembleParameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::maxMinValueFromAddress( const RifEclipseSummaryAddress&           address,
                                              RimPlotDataFilterItem::TimeStepSourceType timeStepSourceType,
                                              const std::vector<QDateTime>&             timeRangeOrSelection,
                                              bool                                      useAbsValue,
                                              double*                                   minVal,
                                              double*                                   maxVal )
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
            selectedTimesteps.push_back( dateTime.toTime_t() );
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
            std::vector<double> values;
            reader->values( address, &values );

            const std::vector<time_t>& timesteps = reader->timeSteps( address );

            if ( timesteps.size() && values.size() )
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
                        time_t minTime = timeRangeOrSelection.front().toTime_t();
                        time_t maxTime = timeRangeOrSelection.back().toTime_t();

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

                    if ( !historyAddr.isHistoryQuantity() ) historyAddr.setQuantityName( address.quantityName() + "H" );

                    const std::vector<time_t>& historyTimesteps = reader->timeSteps( historyAddr );
                    if ( historyTimesteps.size() )
                    {
                        min = minOrAbsMin( min, values[historyTimesteps.size() - 1] );
                        max = maxOrAbsMax( max, values[historyTimesteps.size() - 1] );
                    }
                }
                else if ( selectedTimesteps.size() )
                {
                    std::vector<size_t> selectedTimestepIndices =
                        RimAnalysisPlot::findTimestepIndices( selectedTimesteps, timesteps );

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
void RimAnalysisPlot::onFiltersChanged()
{
    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimAnalysisPlot::selectedTimeSteps()
{
    std::vector<time_t> selectedTimeTTimeSteps;
    for ( const QDateTime& dateTime : m_selectedTimeSteps.v() )
    {
        selectedTimeTTimeSteps.push_back( dateTime.toTime_t() );
    }

    return selectedTimeTTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_selectVariablesButtonField )
    {
        // Do select variables
        RiuSummaryVectorSelectionDialog dlg( nullptr );

        dlg.enableMultiSelect( true );
        dlg.enableIndividualEnsembleCaseSelection( true );

        dlg.setCurveSelection( this->curveDefinitionsWithoutEnsembleReference() );
        dlg.setCaseAndAddress( nullptr, RifEclipseSummaryAddress() );

        if ( dlg.exec() == QDialog::Accepted )
        {
            std::vector<RiaSummaryCurveDefinition> summaryVectorDefinitions = dlg.curveSelection();
            m_analysisPlotDataSelection.deleteAllChildObjects();
            for ( const RiaSummaryCurveDefinition& vectorDef : summaryVectorDefinitions )
            {
                auto plotEntry = new RimAnalysisPlotDataEntry();
                plotEntry->setFromCurveDefinition( vectorDef );
                m_analysisPlotDataSelection.push_back( plotEntry );
            }
        }

        m_selectVariablesButtonField = false;
    }
    else if ( changedField == &m_addTimestepUiField )
    {
        m_selectedTimeSteps.v().push_back( m_addTimestepUiField() );
        m_addTimestepUiField = QDateTime();
    }

    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* selVectorsGrp = uiOrdering.addNewGroup( "Selected Vectors" );
    selVectorsGrp->add( &m_selectedVarsUiField );
    selVectorsGrp->add( &m_selectVariablesButtonField, {false} );

    QString vectorNames;
    if ( m_analyserOfSelectedCurveDefs )
    {
        for ( const std::string& quantityName : m_analyserOfSelectedCurveDefs->m_quantityNames )
        {
            vectorNames += QString::fromStdString( quantityName ) + ", ";
        }

        if ( !vectorNames.isEmpty() )
        {
            vectorNames.chop( 2 );
        }
    }

    m_selectedVarsUiField = vectorNames;

    caf::PdmUiGroup* timeStepGrp = uiOrdering.addNewGroup( "Time Steps" );
    timeStepGrp->add( &m_addTimestepUiField );
    timeStepGrp->add( &m_selectedTimeSteps );

    uiOrdering.add( &m_referenceCase, {true, 3, 2} );

    uiOrdering.add( &m_showPlotTitle );
    uiOrdering.add( &m_useAutoPlotTitle, {false} );
    uiOrdering.add( &m_description, {false} );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );

    uiOrdering.add( &m_barOrientation, {true, 3, 2} );

    caf::PdmUiGroup* sortGrp = uiOrdering.addNewGroup( "Sorting and Grouping" );
    sortGrp->add( &m_majorGroupType );
    sortGrp->add( &m_mediumGroupType );
    sortGrp->add( &m_minorGroupType );
    sortGrp->add( &m_valueSortOperation );
    sortGrp->add( &m_useTopBarsFilter );
    sortGrp->add( &m_maxBarCount, {false} );
    m_maxBarCount.uiCapability()->setUiReadOnly( !m_useTopBarsFilter() );

    caf::PdmUiGroup* legendGrp = uiOrdering.addNewGroup( "Legend" );
    legendGrp->add( &m_showPlotLegends );
    legendGrp->add( &m_sortGroupForLegend );
    legendGrp->add( &m_legendFontSize );
    m_sortGroupForLegend.uiCapability()->setUiReadOnly( !m_showPlotLegends() );
    m_legendFontSize.uiCapability()->setUiReadOnly( !m_showPlotLegends() );

    caf::PdmUiGroup* barLabelGrp = uiOrdering.addNewGroup( "Bar Labels" );
    barLabelGrp->add( &m_useBarText );
    barLabelGrp->add( &m_useQuantityInBarText );
    barLabelGrp->add( &m_useSummaryItemInBarText, {false} );
    barLabelGrp->add( &m_useCaseInBarText );
    barLabelGrp->add( &m_useEnsembleInBarText, {false} );
    barLabelGrp->add( &m_useTimeStepInBarText, {true, 4, 1} );

    m_useQuantityInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useSummaryItemInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useCaseInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useEnsembleInBarText.uiCapability()->setUiReadOnly( !m_useBarText );
    m_useTimeStepInBarText.uiCapability()->setUiReadOnly( !m_useBarText );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                             QString                    uiConfigName,
                                             caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_useTopBarsFilter || field == &m_useBarText || field == &m_showPlotTitle || field == &m_useAutoPlotTitle )
    {
        auto attrib = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_useNativeCheckBoxLabel = true;
        }
    }
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
QList<caf::PdmOptionItemInfo> RimAnalysisPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                      bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( !options.isEmpty() ) return options;

    if ( !m_analyserOfSelectedCurveDefs )
    {
        m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>(
            new RiaSummaryCurveDefinitionAnalyser( this->curveDefinitionsWithoutEnsembleReference() ) );
    }

    if ( fieldNeedingOptions == &m_addTimestepUiField )
    {
        options.push_back( {"None", QDateTime()} );

        std::set<time_t> timeStepUnion = allAvailableTimeSteps();

        for ( time_t timeT : timeStepUnion )
        {
            QDateTime dateTime     = RiaQDateTimeTools::fromTime_t( timeT );
            QString   formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( {dateTime} );

            options.push_back( {dateTime.toString( formatString ), dateTime} );
        }
    }
    else if ( fieldNeedingOptions == &m_valueSortOperation )
    {
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( NONE ), NONE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( VALUE ), VALUE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( ABS_VALUE ), ABS_VALUE ) );
    }
    else if ( fieldNeedingOptions == &m_majorGroupType || fieldNeedingOptions == &m_mediumGroupType ||
              fieldNeedingOptions == &m_minorGroupType || fieldNeedingOptions == &m_sortGroupForLegend )
    {
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( NONE ), NONE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( SUMMARY_ITEM ), SUMMARY_ITEM ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( QUANTITY ), QUANTITY ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( CASE ), CASE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( ENSEMBLE ), ENSEMBLE ) );
        options.push_back( caf::PdmOptionItemInfo( SortGroupAppEnum::uiText( TIME_STEP ), TIME_STEP ) );
    }
    else if ( fieldNeedingOptions == &m_referenceCase )
    {
        std::vector<RimSummaryCase*> allSummaryCases = RimProject::current()->allSummaryCases();

        options.push_back( {"None", nullptr} );

        for ( auto sumCase : allSummaryCases )
        {
            QString displayName = sumCase->displayCaseName();
            auto    caseColl    = dynamic_cast<RimSummaryCaseCollection*>( sumCase->parentField()->ownerObject() );
            if ( caseColl )
            {
                displayName = caseColl->name() + "/" + displayName;
            }

            options.push_back( {displayName, sumCase} );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimAnalysisPlot::allAvailableTimeSteps()
{
    std::set<time_t> timeStepUnion;

    for ( RimSummaryCase* sumCase : timestepDefiningSourceCases() )
    {
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
std::set<RimSummaryCase*> RimAnalysisPlot::timestepDefiningSourceCases()
{
    std::set<RimSummaryCase*> timeStepDefiningSumCases = m_analyserOfSelectedCurveDefs->m_singleSummaryCases;

    return timeStepDefiningSumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryCase*> RimAnalysisPlot::allSourceCases()
{
    std::set<RimSummaryCase*> allSumCases = m_analyserOfSelectedCurveDefs->m_singleSummaryCases;

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

    m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>(
        new RiaSummaryCurveDefinitionAnalyser( this->curveDefinitionsWithoutEnsembleReference() ) );

    if ( m_plotWidget )
    {
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotScale );

        RiuGroupedBarChartBuilder chartBuilder;

        // buildTestPlot( chartBuilder );
        addDataToChartBuilder( chartBuilder );

        chartBuilder.addBarChartToPlot( m_plotWidget,
                                        m_barOrientation == BARS_HORIZONTAL ? Qt::Horizontal : Qt::Vertical,
                                        m_useTopBarsFilter() ? m_maxBarCount : -1 );

        if ( m_showPlotLegends && m_plotWidget->legend() == nullptr )
        {
            QwtLegend* legend = new QwtLegend( m_plotWidget );
            m_plotWidget->insertLegend( legend, QwtPlot::RightLegend );
        }
        else if ( !m_showPlotLegends )
        {
            m_plotWidget->insertLegend( nullptr );
        }

        m_plotWidget->setLegendFontSize( m_legendFontSize() );
        m_plotWidget->updateLegend();
    }

    this->updateAxes();
    this->updatePlotTitle();
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
bool RimAnalysisPlot::applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                     int                         oldFontSize,
                                     int                         fontSize,
                                     bool                        forceChange /*= false */ )
{
    bool anyChange = false;

    if ( fontSettingType == RiaDefines::FontSettingType::PLOT_FONT && m_plotWidget )
    {
        for ( auto plotAxis : allPlotAxes() )
        {
            if ( forceChange || plotAxis->titleFontSize() == oldFontSize )
            {
                plotAxis->setTitleFontSize( fontSize );
                anyChange = true;
            }
            if ( forceChange || plotAxis->valuesFontSize() == oldFontSize )
            {
                plotAxis->setValuesFontSize( fontSize );
                anyChange = true;
            }
        }

        if ( forceChange || m_legendFontSize() == oldFontSize )
        {
            m_legendFontSize = fontSize;
            anyChange        = true;
        }

        if ( anyChange ) loadDataAndUpdate();
    }
    return anyChange;
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
RiuQwtPlotWidget* RimAnalysisPlot::doCreatePlotViewWidget( QWidget* mainWindowParent /*= nullptr */ )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

        this->connect( m_plotWidget, SIGNAL( plotZoomed() ), SLOT( onPlotZoomed() ) );

        // updatePlotTitle();
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
void RimAnalysisPlot::detachAllCurves()
{
    if ( m_plotWidget ) m_plotWidget->detachItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    QwtPlot::Axis qwtAxis = QwtPlot::yLeft;
    if ( m_barOrientation == BARS_HORIZONTAL )
    {
        qwtAxis = QwtPlot::xBottom;
        m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, false );
    }
    else
    {
        m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, false );
    }

    RimPlotAxisProperties* valAxisProperties = m_valueAxisProperties();
    if ( valAxisProperties->isActive() )
    {
        m_plotWidget->enableAxis( qwtAxis, true );
        m_valueAxisProperties->setNameAndAxis( "Value-Axis", qwtAxis );

        std::set<QString> timeHistoryQuantities;

        RimSummaryPlotAxisFormatter calc( valAxisProperties, {}, curveDefinitionsWithoutEnsembleReference(), {}, {} );
        calc.applyAxisPropertiesToPlot( m_plotWidget );
    }
    else
    {
        m_plotWidget->enableAxis( qwtAxis, false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::onAxisSelected( int axis, bool toggle )
{
    RiuPlotMainWindowTools::showPlotMainWindow();

    caf::PdmObject* itemToSelect = nullptr;
    if ( axis == QwtPlot::yLeft )
    {
        if ( m_barOrientation == BARS_VERTICAL )
        {
            itemToSelect = m_valueAxisProperties;
        }
        else
        {
            itemToSelect = this;
        }
    }
    else if ( axis == QwtPlot::xBottom )
    {
        if ( m_barOrientation == BARS_HORIZONTAL )
        {
            itemToSelect = m_valueAxisProperties;
        }
        else
        {
            itemToSelect = this;
        }
    }

    if ( toggle )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( itemToSelect );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( itemToSelect );
    }
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
QString assignGroupingText( RimAnalysisPlot::SortGroupType  sortGroup,
                            const RiaSummaryCurveDefinition dataEntry,
                            const QString&                  timestepString )
{
    QString groupingText;

    switch ( sortGroup )
    {
        case RimAnalysisPlot::SUMMARY_ITEM:
        {
            RifEclipseSummaryAddress addr = dataEntry.summaryAddress();
            groupingText                  = QString::fromStdString( addr.itemUiText() );
        }
        break;
        case RimAnalysisPlot::CASE:
        {
            if ( dataEntry.summaryCase() )
            {
                groupingText = dataEntry.summaryCase()->displayCaseName();
            }
        }
        break;
        case RimAnalysisPlot::ENSEMBLE:
        {
            if ( dataEntry.ensemble() )
            {
                groupingText = dataEntry.ensemble()->name();
            }
        }
        break;
        case RimAnalysisPlot::QUANTITY:
        {
            RifEclipseSummaryAddress addr = dataEntry.summaryAddress();

            groupingText = QString::fromStdString( addr.quantityName() );
        }
        break;
        case RimAnalysisPlot::TIME_STEP:
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
std::vector<size_t> RimAnalysisPlot::findTimestepIndices( std::vector<time_t>        selectedTimesteps,
                                                          const std::vector<time_t>& timesteps )
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
std::vector<RiaSummaryCurveDefinition> RimAnalysisPlot::filteredCurveDefs()
{
    std::vector<RiaSummaryCurveDefinition> dataDefinitions = curveDefinitionsWithEmbeddedEnsembleReference();

    // Split out the filter targets

    std::set<RimSummaryCase*>          filteredSumCases;
    std::set<RifEclipseSummaryAddress> filteredSummaryItems; // Stores only the unique summary items

    for ( const auto& curveDef : dataDefinitions )
    {
        filteredSumCases.insert( curveDef.summaryCase() );

        RifEclipseSummaryAddress address = curveDef.summaryAddress();

        address.setQuantityName( "" ); // Quantity name set to "" in order to store only unique summary items
        filteredSummaryItems.insert( address );
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
        RimSummaryCase*          sumCase = curveDefCandidate.summaryCase();
        RifEclipseSummaryAddress addr    = curveDefCandidate.summaryAddress();
        addr.setQuantityName( "" );

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
                                   std::set<RifEclipseSummaryAddress>* filteredSummaryItems )
{
    const std::vector<RiaSummaryCurveDefinition> curveDefsToFilter;

    if ( !filter->isActive() ) return;

    std::vector<RiaSummaryCurveDefinition> filteredCurveDefs;

    std::set<RimSummaryCase*>          casesToKeep;
    std::set<RifEclipseSummaryAddress> sumItemsToKeep;

    std::map<RimSummaryCase*, double>          casesToKeepWithValue;
    std::map<RifEclipseSummaryAddress, double> sumItemsToKeepWithValue;

    if ( filter->filterTarget() == RimPlotDataFilterItem::ENSEMBLE_CASE )
    {
        sumItemsToKeep = ( *filteredSummaryItems ); // Not filtering items

        EnsembleParameter eParam = this->ensembleParameter( filter->ensembleParameterName() );

        std::set<RimSummaryCase*> casesToRemove;
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
                    std::pair<double, double> minMax = filter->filterRangeMinMax();

                    if ( filter->useAbsoluteValues() ) value = fabs( value );

                    if ( minMax.first < value && value < minMax.second )
                    {
                        casesToKeep.insert( sumCase );
                    }
                }
                else if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N ||
                          filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
                {
                    if ( filter->useAbsoluteValues() ) value = fabs( value );
                    bool useLargest = filter->filterOperation() == RimPlotDataFilterItem::TOP_N;

                    auto itIsInsertedPair = casesToKeepWithValue.insert( {sumCase, value} );
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

                if ( crpValue.isText() &&
                     std::count( filterCategories.begin(), filterCategories.end(), crpValue.textValue() ) == 0 )
                {
                    casesToKeep.insert( sumCase );
                }
            }
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
            selectedTimesteps = this->selectedTimeSteps();
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
                    std::vector<double> values;
                    reader->values( addrToFilterValue, &values );
                    const std::vector<time_t>& timesteps = reader->timeSteps( addrToFilterValue );

                    if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::ALL_TIMESTEPS )
                    {
                        for ( size_t tIdx = 0; tIdx < timesteps.size(); ++tIdx )
                        {
                            double value = values[tIdx];

                            storeResultCoreLambda( value );
                        }
                    }
                    else if ( timesteps.size() )
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
                        else if ( selectedTimesteps.size() )
                        {
                            selectedTimestepIndices = RimAnalysisPlot::findTimestepIndices( selectedTimesteps, timesteps );
                        }
                        else if ( filter->consideredTimeStepsType() == RimPlotDataFilterItem::LAST_TIMESTEP_WITH_HISTORY )
                        {
                            RifEclipseSummaryAddress historyAddr = addrToFilterValue;

                            if ( !historyAddr.isHistoryQuantity() )
                                historyAddr.setQuantityName( addrToFilterValue.quantityName() + "H" );

                            const std::vector<time_t>& historyTimesteps = reader->timeSteps( historyAddr );
                            if ( historyTimesteps.size() )
                            {
                                selectedTimestepIndices =
                                    RimAnalysisPlot::findTimestepIndices( {historyTimesteps.back()}, timesteps );
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
                    if ( filter->useAbsoluteValues() ) value = fabs( value );

                    if ( minMax.first <= value && value <= minMax.second )
                    {
                        casesToKeep.insert( sumCaseInEvaluation );
                    }
                };
            }
            else if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N ||
                      filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
            {
                // clang-format off
                storeResultCoreLambda = [&]( double value ) // clang-format on
                {
                    if ( filter->useAbsoluteValues() ) value = fabs( value );
                    bool useLargest = filter->filterOperation() == RimPlotDataFilterItem::TOP_N;

                    auto itIsInsertedPair = casesToKeepWithValue.insert( {sumCaseInEvaluation, value} );
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

                quantityName = addrToFilterValue.quantityName();
            }

            for ( auto sumItem : *filteredSummaryItems )
            {
                RifEclipseSummaryAddress addrToFilterValue = sumItem;
                addrToFilterValue.setQuantityName( quantityName );

                if ( filter->filterOperation() == RimPlotDataFilterItem::RANGE )
                {
                    std::pair<double, double> minMax = filter->filterRangeMinMax();

                    // clang-format off
                    storeResultCoreLambda = [&]( double value ) // clang-format on
                    {
                        if ( filter->useAbsoluteValues() ) value = fabs( value );

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
                        if ( filter->useAbsoluteValues() ) value = fabs( value );
                        bool useLargest = filter->filterOperation() == RimPlotDataFilterItem::TOP_N;

                        auto itIsInsertedPair = sumItemsToKeepWithValue.insert( {sumItem, value} );
                        if ( !itIsInsertedPair.second ) // Already exists in map
                        {
                            double& insertedValue = itIsInsertedPair.first->second;
                            if ( ( useLargest && ( insertedValue < value ) ) ||
                                 ( !useLargest && ( value < insertedValue ) ) )
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

    if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N ||
         filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
    {
        if ( filter->filterTarget() == RimPlotDataFilterItem::SUMMARY_ITEM )
        {
            std::multimap<double, RifEclipseSummaryAddress> valueSortedSumItems;
            for ( const auto& itemValPair : sumItemsToKeepWithValue )
            {
                valueSortedSumItems.insert( {itemValPair.second, itemValPair.first} );
            }

            if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumItems.rbegin();
                      count < filter->topBottomN() && it != valueSortedSumItems.rend();
                      ++it )
                {
                    sumItemsToKeep.insert( it->second );
                    ++count;
                }
            }
            else if ( filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumItems.begin();
                      count < filter->topBottomN() && it != valueSortedSumItems.end();
                      ++it )
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
                valueSortedSumCases.insert( {caseValPair.second, caseValPair.first} );
            }

            if ( filter->filterOperation() == RimPlotDataFilterItem::TOP_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumCases.rbegin();
                      count < filter->topBottomN() && it != valueSortedSumCases.rend();
                      ++it )
                {
                    casesToKeep.insert( it->second );
                    ++count;
                }
            }
            else if ( filter->filterOperation() == RimPlotDataFilterItem::BOTTOM_N )
            {
                int count = 0;
                for ( auto it = valueSortedSumCases.begin();
                      count < filter->topBottomN() && it != valueSortedSumCases.end();
                      ++it )
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
void RimAnalysisPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder )
{
    std::vector<time_t> selectedTimesteps;
    for ( const QDateTime& dateTime : m_selectedTimeSteps.v() )
    {
        selectedTimesteps.push_back( dateTime.toTime_t() );
    }

    RifSummaryReaderInterface* referenceCaseReader = nullptr;

    if ( m_referenceCase ) referenceCaseReader = m_referenceCase->summaryReader();

    // Unpack ensemble curves and make one curve definition for each individual curve.
    // Store both ensemble and summary case in the definition

    std::vector<RiaSummaryCurveDefinition> barDataDefinitions = filteredCurveDefs();

    for ( const RiaSummaryCurveDefinition& curveDef : barDataDefinitions )
    {
        if ( !curveDef.summaryCase() ) continue;
        RifSummaryReaderInterface* reader = curveDef.summaryCase()->summaryReader();

        if ( !reader ) continue;

        // Todo:
        // If curveDef.summaryCase() is a RimGridSummaryCase and we are using summary item as legend and the summary
        // items are wells, then:
        /// use color from eclCase->defaultWellColor( wellName );

        std::vector<time_t>        timeStepStorage;
        const std::vector<time_t>* timeStepsPtr = &timeStepStorage;
        std::vector<double>        values;

        if ( referenceCaseReader )
        {
            std::pair<std::vector<time_t>, std::vector<double>> timeAndValues =
                RimDerivedSummaryCase::calculateDerivedValues( reader,
                                                               -1,
                                                               referenceCaseReader,
                                                               -1,
                                                               DerivedSummaryOperator::DERIVED_OPERATOR_SUB,
                                                               curveDef.summaryAddress() );
            timeStepStorage.swap( timeAndValues.first );
            values.swap( timeAndValues.second );
        }
        else
        {
            timeStepsPtr = &( reader->timeSteps( curveDef.summaryAddress() ) );

            reader->values( curveDef.summaryAddress(), &values );
        }

        const std::vector<time_t>& timesteps = *timeStepsPtr;

        if ( !( timesteps.size() && values.size() ) ) continue;

        // Find selected timestep indices

        std::vector<int> selectedTimestepIndices;

        for ( time_t tt : selectedTimesteps )
        {
            for ( int timestepIdx = 0; static_cast<unsigned>( timestepIdx ) < timesteps.size(); ++timestepIdx )
            {
                if ( timesteps[timestepIdx] == tt )
                {
                    selectedTimestepIndices.push_back( timestepIdx );
                    break;
                }
            }
        }

        for ( int timestepIdx : selectedTimestepIndices )
        {
            double sortValue = std::numeric_limits<double>::infinity();

            QDateTime dateTime     = RiaQDateTimeTools::fromTime_t( timesteps[timestepIdx] );
            QString   formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( {dateTime} );

            QString timestepString = dateTime.toString( formatString );

            QString majorText  = assignGroupingText( m_majorGroupType(), curveDef, timestepString );
            QString medText    = assignGroupingText( m_mediumGroupType(), curveDef, timestepString );
            QString minText    = assignGroupingText( m_minorGroupType(), curveDef, timestepString );
            QString legendText = assignGroupingText( m_sortGroupForLegend(), curveDef, timestepString );

            double value = values[timestepIdx];

            switch ( m_valueSortOperation() )
            {
                case VALUE:
                    sortValue = value;
                    break;
                case ABS_VALUE:
                    sortValue = fabs( value );
                    break;
            }

            QString barText;
            QString separator = " ";

            if ( m_useBarText() )
            {
                if ( m_useQuantityInBarText )
                {
                    barText += QString::fromStdString( curveDef.summaryAddress().quantityName() ) + separator;
                }

                if ( m_useSummaryItemInBarText )
                {
                    barText += QString::fromStdString( curveDef.summaryAddress().itemUiText() ) + separator;
                }

                if ( m_useCaseInBarText && curveDef.summaryCase() )
                {
                    barText += curveDef.summaryCase()->displayCaseName() + separator;
                }

                if ( m_useEnsembleInBarText && curveDef.ensemble() )
                {
                    barText += curveDef.ensemble()->name() + separator;
                }

                if ( m_useTimeStepInBarText )
                {
                    barText += timestepString + separator;
                }
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
    if ( m_useAutoPlotTitle && m_analyserOfSelectedCurveDefs )
    {
        QString autoTitle;
        QString separator = ", ";

        if ( m_analyserOfSelectedCurveDefs->m_ensembles.size() == 1 )
        {
            autoTitle += ( *m_analyserOfSelectedCurveDefs->m_ensembles.begin() )->name();
        }

        if ( m_analyserOfSelectedCurveDefs->m_singleSummaryCases.size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += ( *m_analyserOfSelectedCurveDefs->m_singleSummaryCases.begin() )->displayCaseName();
        }

        if ( m_analyserOfSelectedCurveDefs->m_summaryItems.size() == 1 )
        {
            if ( !autoTitle.isEmpty() ) autoTitle += separator;
            autoTitle += QString::fromStdString( m_analyserOfSelectedCurveDefs->m_summaryItems.begin()->itemUiText() );
        }

        for ( std::string quantName : m_analyserOfSelectedCurveDefs->m_quantityNames )
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

            QString formatString = RiaQDateTimeTools::createTimeFormatStringFromDates( {m_selectedTimeSteps()[0]} );
            autoTitle += m_selectedTimeSteps()[0].toString( formatString );
        }

        m_description = autoTitle;
    }

    if ( m_plotWidget )
    {
        QString plotTitle = description();
        m_plotWidget->setPlotTitle( plotTitle );
        m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && isMdiWindow() );
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryCurveDefinitionAnalyser* RimAnalysisPlot::getOrCreateSelectedCurveDefAnalyser()
{
    if ( !m_analyserOfSelectedCurveDefs )
    {
        m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>(
            new RiaSummaryCurveDefinitionAnalyser( this->curveDefinitionsWithoutEnsembleReference() ) );
    }

    return m_analyserOfSelectedCurveDefs.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimAnalysisPlot::curveDefinitionsWithoutEnsembleReference() const
{
    std::vector<RiaSummaryCurveDefinition> curveDefs;
    for ( auto dataEntry : m_analysisPlotDataSelection )
    {
        curveDefs.push_back( dataEntry->curveDefinition() );
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
/// The curve definitions returned contain both the case AND the ensemble from which it has been spawned
//--------------------------------------------------------------------------------------------------
std::vector<RiaSummaryCurveDefinition> RimAnalysisPlot::curveDefinitionsWithEmbeddedEnsembleReference()
{
    std::vector<RiaSummaryCurveDefinition> barDataDefinitions;

    for ( const RimAnalysisPlotDataEntry* dataEntry : m_analysisPlotDataSelection )
    {
        RiaSummaryCurveDefinition orgBarDataEntry = dataEntry->curveDefinition();

        if ( orgBarDataEntry.summaryCase() && orgBarDataEntry.summaryCase()->ensemble() )
        {
            barDataDefinitions.push_back( RiaSummaryCurveDefinition( orgBarDataEntry.summaryCase(),
                                                                     orgBarDataEntry.summaryAddress(),
                                                                     orgBarDataEntry.summaryCase()->ensemble() ) );
        }
        else
        {
            barDataDefinitions.push_back( orgBarDataEntry );
        }
    }

    return barDataDefinitions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimPlotAxisPropertiesInterface*> RimAnalysisPlot::allPlotAxes() const
{
    return {m_valueAxisProperties};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnalysisPlot::buildTestPlot( RiuGroupedBarChartBuilder& chartBuilder )
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
