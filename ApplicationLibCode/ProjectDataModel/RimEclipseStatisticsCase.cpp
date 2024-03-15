/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseStatisticsCase.h"

#include "RiaOptionItemFactory.h"
#include "RiaResultNames.h"

#include "RicNewViewFeature.h"
#include "RicfCommandObject.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigSimWellData.h"

#include "RimCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseStatisticsCaseEvaluator.h"
#include "RimEclipseView.h"
#include "RimGridCalculationCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersectionCollection.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInViewCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafProgressInfo.h"

namespace caf
{
template <>
void caf::AppEnum<RimEclipseStatisticsCase::PercentileCalcType>::setUp()
{
    addItem( RimEclipseStatisticsCase::PercentileCalcType::NEAREST_OBSERVATION, "NearestObservationPercentile", "Nearest Observation" );
    addItem( RimEclipseStatisticsCase::PercentileCalcType::HISTOGRAM_ESTIMATED, "HistogramEstimatedPercentile", "Histogram based estimate" );
    addItem( RimEclipseStatisticsCase::PercentileCalcType::INTERPOLATED_OBSERVATION,
             "InterpolatedObservationPercentile",
             "Interpolated Observation" );
    setDefault( RimEclipseStatisticsCase::PercentileCalcType::INTERPOLATED_OBSERVATION );
}
template <>
void caf::AppEnum<RimEclipseStatisticsCase::DataSourceType>::setUp()
{
    addItem( RimEclipseStatisticsCase::DataSourceType::GRID_CALCULATION, "GridCalculation", "Grid Calculation" );
    addItem( RimEclipseStatisticsCase::DataSourceType::CASE_PROPERTY, "CaseProperty", "Case Property" );
    setDefault( RimEclipseStatisticsCase::DataSourceType::CASE_PROPERTY );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimEclipseStatisticsCase, "RimStatisticalCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase::RimEclipseStatisticsCase()
    : RimEclipseCase()
{
    CAF_PDM_InitScriptableObject( "Case Group Statistics", ":/Histogram16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_calculateEditCommand, "m_editingAllowed", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_calculateEditCommand );
    m_calculateEditCommand = false;

    CAF_PDM_InitField( &m_selectionSummary, "SelectionSummary", QString( "" ), "Summary of Calculation Setup" );
    m_selectionSummary.xmlCapability()->disableIO();
    m_selectionSummary.uiCapability()->setUiReadOnly( true );
    m_selectionSummary.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_selectionSummary.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_dataSourceForStatistics, "DataSourceForStatistics", "Data Source" );

    CAF_PDM_InitFieldNoDefault( &m_gridCalculation, "GridCalculation", "Grid Calculation" );
    CAF_PDM_InitFieldNoDefault( &m_gridCalculationFilterView, "GridCalculationFilterView", "Filter By View" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedTimeSteps, "SelectedTimeSteps", "Time Step Selection" );
    m_selectedTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultType, "ResultType", "Result Type" );
    m_resultType.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitScriptableFieldNoDefault( &m_porosityModel, "PorosityModel", "Porosity Model" );
    m_porosityModel.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedDynamicProperties, "DynamicPropertiesToCalculate", "Dyn Prop" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedStaticProperties, "StaticPropertiesToCalculate", "Stat Prop" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedGeneratedProperties, "GeneratedPropertiesToCalculate", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedInputProperties, "InputPropertiesToCalculate", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedFractureDynamicProperties, "FractureDynamicPropertiesToCalculate", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedFractureStaticProperties, "FractureStaticPropertiesToCalculate", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedFractureGeneratedProperties, "FractureGeneratedPropertiesToCalculate", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedFractureInputProperties, "FractureInputPropertiesToCalculate", "" );

    m_selectedDynamicProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedStaticProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedGeneratedProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedInputProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_selectedFractureDynamicProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedFractureStaticProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedFractureGeneratedProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedFractureInputProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableField( &m_calculatePercentiles, "CalculatePercentiles", true, "Calculate Percentiles" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_percentileCalculationType, "PercentileCalculationType", "Method" );

    CAF_PDM_InitScriptableField( &m_lowPercentile, "LowPercentile", 10.0, "Low" );
    CAF_PDM_InitScriptableField( &m_midPercentile, "MidPercentile", 50.0, "Mid" );
    CAF_PDM_InitScriptableField( &m_highPercentile, "HighPercentile", 90.0, "High" );

    CAF_PDM_InitScriptableField( &m_wellDataSourceCase, "WellDataSourceCase", RiaResultNames::undefinedResultName(), "Well Data Source Case" );

    CAF_PDM_InitScriptableField( &m_useZeroAsInactiveCellValue, "UseZeroAsInactiveCellValue", false, "Use Zero as Inactive Cell Value" );

    m_populateSelectionAfterLoadingGrid = false;

    // These does not work properly for statistics case, so hide for now
    m_flipXAxis.uiCapability()->setUiHidden( true );
    m_flipYAxis.uiCapability()->setUiHidden( true );
    m_activeFormationNames.uiCapability()->setUiHidden( true );

    m_displayNameOption = RimCaseDisplayNameTools::DisplayName::CUSTOM;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase::~RimEclipseStatisticsCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::setMainGrid( RigMainGrid* mainGrid )
{
    CVF_ASSERT( mainGrid );
    CVF_ASSERT( eclipseCaseData() );

    eclipseCaseData()->setMainGrid( mainGrid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseStatisticsCase::openEclipseGridFile()
{
    if ( eclipseCaseData() ) return true;

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( this );

    CVF_ASSERT( parentStatisticsCaseCollection() );

    RimIdenticalGridCaseGroup* gridCaseGroup = parentStatisticsCaseCollection()->parentCaseGroup();
    CVF_ASSERT( gridCaseGroup );

    RigMainGrid* mainGrid = gridCaseGroup->mainGrid();

    eclipseCase->setMainGrid( mainGrid );

    eclipseCase->setActiveCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                    gridCaseGroup->unionOfActiveCells( RiaDefines::PorosityModelType::MATRIX_MODEL ) );
    eclipseCase->setActiveCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                    gridCaseGroup->unionOfActiveCells( RiaDefines::PorosityModelType::FRACTURE_MODEL ) );

    setReservoirData( eclipseCase.p() );

    loadSimulationWellDataFromSourceCase();

    if ( m_populateSelectionAfterLoadingGrid )
    {
        populateResultSelection();

        m_populateSelectionAfterLoadingGrid = false;
    }

    initializeSelectedTimeSteps();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::reloadEclipseGridFile()
{
    setReservoirData( nullptr );
    openReserviorCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimEclipseStatisticsCase::parentStatisticsCaseCollection() const
{
    return dynamic_cast<RimCaseCollection*>( parentField()->ownerObject() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::populateResultSelectionAfterLoadingGrid()
{
    m_populateSelectionAfterLoadingGrid = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::setSourceProperties( RiaDefines::ResultCatType propertyType, const std::vector<QString>& propertyNames )
{
    switch ( propertyType )
    {
        case RiaDefines::ResultCatType::DYNAMIC_NATIVE:
            m_selectedDynamicProperties = propertyNames;
            break;
        case RiaDefines::ResultCatType::STATIC_NATIVE:
            m_selectedStaticProperties = propertyNames;
            break;
        case RiaDefines::ResultCatType::SOURSIMRL:
            break;
        case RiaDefines::ResultCatType::GENERATED:
            m_selectedGeneratedProperties = propertyNames;
            break;
        case RiaDefines::ResultCatType::INPUT_PROPERTY:
            m_selectedInputProperties = propertyNames;
            break;
        case RiaDefines::ResultCatType::FORMATION_NAMES:
            break;
        case RiaDefines::ResultCatType::ALLAN_DIAGRAMS:
            break;
        case RiaDefines::ResultCatType::FLOW_DIAGNOSTICS:
            break;
        case RiaDefines::ResultCatType::INJECTION_FLOODING:
            break;
        case RiaDefines::ResultCatType::REMOVED:
            break;
        case RiaDefines::ResultCatType::UNDEFINED:
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::selectAllTimeSteps()
{
    RimIdenticalGridCaseGroup* idgcg = caseGroup();
    if ( idgcg && idgcg->mainCase() )
    {
        int timeStepCount = idgcg->mainCase()->timeStepStrings().size();

        if ( timeStepCount > 0 )
        {
            std::vector<int> allTimeSteps;
            allTimeSteps.resize( timeStepCount );
            std::iota( allTimeSteps.begin(), allTimeSteps.end(), 0 );
            m_selectedTimeSteps = allTimeSteps;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::setWellDataSourceCase( const QString& reservoirDescription )
{
    m_wellDataSourceCase = reservoirDescription;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::computeStatistics()
{
    if ( eclipseCaseData() == nullptr )
    {
        openEclipseGridFile();
    }

    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    CVF_ASSERT( gridCaseGroup );
    gridCaseGroup->computeUnionOfActiveCells();

    std::vector<RimEclipseCase*> sourceCases = getSourceCases();

    if ( m_dataSourceForStatistics() == DataSourceType::GRID_CALCULATION && m_gridCalculation() )
    {
        auto proj     = RimProject::current();
        auto calcColl = proj->gridCalculationCollection();

        auto dependentCalculations = calcColl->dependentCalculations( m_gridCalculation() );
        if ( dependentCalculations.empty() )
        {
            // No calculations found, usually triggered by a circular dependency of calculations. Error message displayed by
            // dependentCalculations().
            return;
        }

        std::vector<size_t> timeStepIndices( m_selectedTimeSteps().begin(), m_selectedTimeSteps().end() );

        for ( auto calc : dependentCalculations )
        {
            cvf::UByteArray* inputValueVisibilityFilter = nullptr;
            if ( m_gridCalculationFilterView() )
            {
                inputValueVisibilityFilter = m_gridCalculationFilterView()->currentTotalCellVisibility().p();
            }

            bool evaluateDependentCalculations = false;
            calc->calculateForCases( sourceCases, inputValueVisibilityFilter, timeStepIndices, evaluateDependentCalculations );
        }
    }

    if ( sourceCases.empty() || !sourceCases.at( 0 )->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
    {
        return;
    }

    RimStatisticsConfig statisticsConfig;

    statisticsConfig.m_calculatePercentiles = m_calculatePercentiles();
    statisticsConfig.m_pMaxPos              = m_highPercentile();
    statisticsConfig.m_pMidPos              = m_midPercentile();
    statisticsConfig.m_pMinPos              = m_lowPercentile();
    statisticsConfig.m_pValMethod           = m_percentileCalculationType();

    auto timeStepIndices = m_selectedTimeSteps();

    // If no dynamic data is present, we might end up with no time steps. Make sure we have at least one.
    if ( timeStepIndices.empty() )
    {
        timeStepIndices.push_back( 0 );
    }

    RigEclipseCaseData* resultCase = eclipseCaseData();

    QList<RimEclipseStatisticsCaseEvaluator::ResSpec> resultSpecification;

    if ( m_dataSourceForStatistics() == DataSourceType::CASE_PROPERTY )
    {
        for ( size_t pIdx = 0; pIdx < m_selectedDynamicProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                    RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                    m_selectedDynamicProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedStaticProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                    RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    m_selectedStaticProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedGeneratedProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                    RiaDefines::ResultCatType::GENERATED,
                                                                                    m_selectedGeneratedProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedInputProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                    RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                                                    m_selectedInputProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedFractureDynamicProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                                    RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                    m_selectedFractureDynamicProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedFractureStaticProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                                    RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    m_selectedFractureStaticProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedFractureGeneratedProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                                    RiaDefines::ResultCatType::GENERATED,
                                                                                    m_selectedFractureGeneratedProperties()[pIdx] ) );
        }

        for ( size_t pIdx = 0; pIdx < m_selectedFractureInputProperties().size(); ++pIdx )
        {
            resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                                    RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                                                    m_selectedFractureInputProperties()[pIdx] ) );
        }
    }
    else if ( m_dataSourceForStatistics() == DataSourceType::GRID_CALCULATION && m_gridCalculation() )
    {
        auto calculationName = m_gridCalculation->shortName();

        resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                RiaDefines::ResultCatType::GENERATED,
                                                                                calculationName ) );
    }

    bool clearGridCalculationMemory = m_dataSourceForStatistics() == DataSourceType::GRID_CALCULATION;
    RimEclipseStatisticsCaseEvaluator stat( sourceCases, timeStepIndices, statisticsConfig, resultCase, gridCaseGroup, clearGridCalculationMemory );

    if ( m_useZeroAsInactiveCellValue )
    {
        stat.useZeroAsValueForInActiveCellsBasedOnUnionOfActiveCells();
    }

    stat.evaluateForResults( resultSpecification, m_gridCalculationFilterView() );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::scheduleACTIVEGeometryRegenOnReservoirViews()
{
    for ( RimEclipseView* reservoirView : reservoirViews() )
    {
        CVF_ASSERT( reservoirView );
        reservoirView->scheduleGeometryRegen( ACTIVE );
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseCase*> RimEclipseStatisticsCase::getSourceCases() const
{
    std::vector<RimEclipseCase*> sourceCases;

    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    if ( gridCaseGroup )
    {
        size_t caseCount = gridCaseGroup->caseCollection->reservoirs.size();
        for ( size_t i = 0; i < caseCount; i++ )
        {
            CVF_ASSERT( gridCaseGroup->caseCollection );
            CVF_ASSERT( gridCaseGroup->caseCollection->reservoirs[i] );

            RimEclipseCase* sourceCase = gridCaseGroup->caseCollection->reservoirs[i];
            sourceCases.push_back( sourceCase );
        }
    }

    return sourceCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimEclipseStatisticsCase::caseGroup() const
{
    RimCaseCollection* parentCollection = parentStatisticsCaseCollection();
    if ( parentCollection )
    {
        return parentCollection->parentCaseGroup();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    updateSelectionSummaryLabel();
    updateSelectionListVisibilities();
    updatePercentileUiVisibility();

    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_caseId );

    uiOrdering.add( &m_calculateEditCommand );

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Summary of Calculation Setup" );
        group->add( &m_useZeroAsInactiveCellValue );
        m_useZeroAsInactiveCellValue.uiCapability()->setUiHidden( hasComputedStatistics() );
        group->add( &m_selectionSummary );
    }

    {
        auto group = uiOrdering.addNewGroup( "Properties to consider" );
        group->setUiHidden( hasComputedStatistics() );
        group->add( &m_dataSourceForStatistics );

        if ( m_dataSourceForStatistics() == DataSourceType::GRID_CALCULATION )
        {
            group->add( &m_gridCalculation );
            group->add( &m_gridCalculationFilterView );
        }
        else
        {
            group->add( &m_resultType );
            group->add( &m_porosityModel );
            group->add( &m_selectedDynamicProperties );
            group->add( &m_selectedStaticProperties );
            group->add( &m_selectedGeneratedProperties );
            group->add( &m_selectedInputProperties );
            group->add( &m_selectedFractureDynamicProperties );
            group->add( &m_selectedFractureStaticProperties );
            group->add( &m_selectedFractureGeneratedProperties );
            group->add( &m_selectedFractureInputProperties );
        }
    }

    {
        auto group = uiOrdering.addNewGroup( "Time Step Selection" );
        group->setCollapsedByDefault();
        group->add( &m_selectedTimeSteps );
    }

    {
        auto group = uiOrdering.addNewGroup( "Percentile setup" );
        group->setUiHidden( hasComputedStatistics() );
        group->add( &m_calculatePercentiles );
        group->add( &m_percentileCalculationType );
        group->add( &m_lowPercentile );
        group->add( &m_midPercentile );
        group->add( &m_highPercentile );
    }

    {
        auto group = uiOrdering.addNewGroup( "Case Options" );
        group->add( &m_wellDataSourceCase );
        group->add( &m_activeFormationNames );
        group->add( &m_flipXAxis );
        group->add( &m_flipYAxis );
    }

    uiOrdering.skipRemainingFields();
}

QList<caf::PdmOptionItemInfo> toOptionList( const QStringList& varList )
{
    QList<caf::PdmOptionItemInfo> optionList;
    int                           i;
    for ( i = 0; i < varList.size(); ++i )
    {
        optionList.push_back( caf::PdmOptionItemInfo( varList[i], varList[i] ) );
    }
    return optionList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseStatisticsCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    RimIdenticalGridCaseGroup* idgcg = caseGroup();
    if ( !( caseGroup() && caseGroup()->mainCase() && caseGroup()->mainCase()->eclipseCaseData() ) )
    {
        return {};
    }

    if ( &m_dataSourceForStatistics == fieldNeedingOptions )
    {
        QList<caf::PdmOptionItemInfo> options;

        {
            caf::IconProvider iconProvider( ":/Case48x48.png" );
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<DataSourceType>::uiText( DataSourceType::CASE_PROPERTY ),
                                                       DataSourceType::CASE_PROPERTY,
                                                       false,
                                                       iconProvider ) );
        }
        {
            caf::IconProvider iconProvider( ":/Calculator.svg" );
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<DataSourceType>::uiText( DataSourceType::GRID_CALCULATION ),
                                                       DataSourceType::GRID_CALCULATION,
                                                       false,
                                                       iconProvider ) );
        }

        return options;
    }

    if ( &m_gridCalculation == fieldNeedingOptions )
    {
        QList<caf::PdmOptionItemInfo> options;

        for ( auto calc : RimProject::current()->gridCalculationCollection()->calculations() )
        {
            options.push_back( caf::PdmOptionItemInfo( calc->shortName(), calc ) );
        }

        return options;
    }

    RigEclipseCaseData* caseData = idgcg->mainCase()->eclipseCaseData();

    if ( &m_selectedTimeSteps == fieldNeedingOptions )
    {
        QList<caf::PdmOptionItemInfo> options;

        const auto timeStepStrings = idgcg->mainCase()->timeStepStrings();

        int index = 0;
        for ( const auto& text : timeStepStrings )
        {
            options.push_back( caf::PdmOptionItemInfo( text, index++ ) );
        }

        return options;
    }

    if ( &m_gridCalculationFilterView == fieldNeedingOptions )
    {
        QList<caf::PdmOptionItemInfo> options;

        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        for ( const auto& view : views() )
        {
            RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName( view, &options );
        }

        return options;
    }

    if ( &m_selectedDynamicProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->resultNames( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedStaticProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedGeneratedProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->resultNames( RiaDefines::ResultCatType::GENERATED );
        return toOptionList( varList );
    }
    else if ( &m_selectedInputProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->resultNames( RiaDefines::ResultCatType::INPUT_PROPERTY );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureDynamicProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->resultNames( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureStaticProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureGeneratedProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->resultNames( RiaDefines::ResultCatType::GENERATED );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureInputProperties == fieldNeedingOptions )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->resultNames( RiaDefines::ResultCatType::INPUT_PROPERTY );
        return toOptionList( varList );
    }

    else if ( &m_wellDataSourceCase == fieldNeedingOptions )
    {
        QStringList sourceCaseNames;
        sourceCaseNames += RiaResultNames::undefinedResultName();

        for ( size_t i = 0; i < caseGroup()->caseCollection()->reservoirs().size(); i++ )
        {
            sourceCaseNames += caseGroup()->caseCollection()->reservoirs()[i]->caseUserDescription();
        }

        return toOptionList( sourceCaseNames );
    }

    return RimEclipseCase::calculateValueOptions( fieldNeedingOptions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimEclipseCase::fieldChangedByUi( changedField, oldValue, newValue );

    if ( &m_resultType == changedField || &m_porosityModel == changedField )
    {
    }

    if ( &m_calculateEditCommand == changedField )
    {
        if ( hasComputedStatistics() )
        {
            clearComputedStatistics();
        }
        else
        {
            computeStatisticsAndUpdateViews();
        }
        m_calculateEditCommand = false;
    }

    if ( &m_dataSourceForStatistics == changedField && m_gridCalculation() == nullptr )
    {
        auto calculations = RimProject::current()->gridCalculationCollection()->calculations();
        if ( !calculations.empty() )
        {
            m_gridCalculation = dynamic_cast<RimGridCalculation*>( calculations.front() );
        }
    }

    if ( &m_wellDataSourceCase == changedField )
    {
        loadSimulationWellDataFromSourceCase();

        caf::ProgressInfo progInfo( reservoirViews().size() + 1, "Updating Well Data for Views" );

        // Update views
        for ( RimEclipseView* reservoirView : reservoirViews() )
        {
            CVF_ASSERT( reservoirView );

            reservoirView->wellCollection()->wells.deleteChildren();
            reservoirView->updateDisplayModelForWellResults();
            reservoirView->wellCollection()->updateConnectedEditors();

            progInfo.incrementProgress();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::loadSimulationWellDataFromSourceCase()
{
    // Find or load well data for given case
    RimEclipseCase* sourceResultCase = caseGroup()->caseCollection()->findByDescription( m_wellDataSourceCase );
    if ( sourceResultCase )
    {
        sourceResultCase->openEclipseGridFile();

        // Propagate well info to statistics case
        if ( sourceResultCase->eclipseCaseData() )
        {
            const cvf::Collection<RigSimWellData>& sourceCaseSimWellData = sourceResultCase->eclipseCaseData()->wellResults();

            eclipseCaseData()->setSimWellData( sourceCaseSimWellData );
        }
    }
    else
    {
        cvf::Collection<RigSimWellData> sourceCaseWellResults;
        eclipseCaseData()->setSimWellData( sourceCaseWellResults );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void addPropertySetToHtmlText( QString& html, const QString& heading, const std::vector<QString>& varNames )
{
    if ( !varNames.empty() )
    {
        html += "<p><b>" + heading + "</b></p>";
        html += "<p class=indent>";
        for ( size_t pIdx = 0; pIdx < varNames.size(); ++pIdx )
        {
            html += varNames[pIdx];
            if ( ( pIdx + 1 ) % 6 == 0 )
                html += "<br>";
            else if ( pIdx != varNames.size() - 1 )
                html += ", ";
        }
        html += "</p>";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::updateSelectionSummaryLabel()
{
    QString html;

    html += "<style> "
            "p{ margin-top:0px; margin-bottom:0px;} "
            "p.indent{margin-left:20px; margin-top:0px;} "
            "p.indent2{margin-left:40px; margin-top:0px;} "
            "</style>";

    html += "<p><b>Statistical variables to compute:</b></p>";
    html += "<p class=indent>";
    html += "Min, Max, Sum, Range, Mean, Std.dev";
    ;
    if ( m_calculatePercentiles() )
    {
        html += "<br>";
        html += "Percentiles for : " + QString::number( m_lowPercentile() ) + ", " + QString::number( m_midPercentile() ) + ", " +
                QString::number( m_highPercentile() );
    }
    html += "</p>";

    if ( m_dataSourceForStatistics() == DataSourceType::CASE_PROPERTY )
    {
        addPropertySetToHtmlText( html, "Dynamic properties", m_selectedDynamicProperties() );
        addPropertySetToHtmlText( html, "Static properties", m_selectedStaticProperties() );
        addPropertySetToHtmlText( html, "Generated properties", m_selectedGeneratedProperties() );
        addPropertySetToHtmlText( html, "Input properties", m_selectedInputProperties() );

        addPropertySetToHtmlText( html, "Dynamic properties, fracture model", m_selectedFractureDynamicProperties() );
        addPropertySetToHtmlText( html, "Static properties, fracture model", m_selectedFractureStaticProperties() );
        addPropertySetToHtmlText( html, "Generated properties, fracture model", m_selectedFractureGeneratedProperties() );
        addPropertySetToHtmlText( html, "Input properties, fracture model", m_selectedFractureInputProperties() );
    }

    if ( m_dataSourceForStatistics() == DataSourceType::GRID_CALCULATION && m_gridCalculation() )
    {
        html += "<p><b>Grid calculation:</b></p>";
        html += "<p class=indent>";
        html += m_gridCalculation()->shortName();
        html += "</p>";
    }

    m_selectionSummary = html;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( &m_selectionSummary == field )
    {
        caf::PdmUiTextEditorAttribute* textEditAttrib = dynamic_cast<caf::PdmUiTextEditorAttribute*>( attribute );
        if ( textEditAttrib )
        {
            textEditAttrib->textMode = caf::PdmUiTextEditorAttribute::HTML;
        }
    }

    if ( &m_calculateEditCommand == field )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = hasComputedStatistics() ? "Edit (Will DELETE current results)" : "Compute";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::initializeSelectedTimeSteps()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2023.10.0" ) )
    {
        selectAllTimeSteps();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::updateSelectionListVisibilities()
{
    bool isLocked = hasComputedStatistics();
    m_resultType.uiCapability()->setUiHidden( isLocked );
    m_porosityModel.uiCapability()->setUiHidden(
        isLocked ); // ||
                    // !caseGroup()->mainCase()->reservoirData()->results(RiaDefines::FRACTURE_MODEL)->resultCount()

    m_selectedDynamicProperties.uiCapability()->setUiHidden( isLocked || m_porosityModel() != RiaDefines::PorosityModelType::MATRIX_MODEL ||
                                                             m_resultType() != RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_selectedStaticProperties.uiCapability()->setUiHidden( isLocked || m_porosityModel() != RiaDefines::PorosityModelType::MATRIX_MODEL ||
                                                            m_resultType() != RiaDefines::ResultCatType::STATIC_NATIVE );
    m_selectedGeneratedProperties.uiCapability()->setUiHidden( isLocked || m_porosityModel() != RiaDefines::PorosityModelType::MATRIX_MODEL ||
                                                               m_resultType() != RiaDefines::ResultCatType::GENERATED );
    m_selectedInputProperties.uiCapability()->setUiHidden( isLocked || m_porosityModel() != RiaDefines::PorosityModelType::MATRIX_MODEL ||
                                                           m_resultType() != RiaDefines::ResultCatType::INPUT_PROPERTY );

    m_selectedFractureDynamicProperties.uiCapability()->setUiHidden( isLocked ||
                                                                     m_porosityModel() != RiaDefines::PorosityModelType::FRACTURE_MODEL ||
                                                                     m_resultType() != RiaDefines::ResultCatType::DYNAMIC_NATIVE );
    m_selectedFractureStaticProperties.uiCapability()->setUiHidden( isLocked ||
                                                                    m_porosityModel() != RiaDefines::PorosityModelType::FRACTURE_MODEL ||
                                                                    m_resultType() != RiaDefines::ResultCatType::STATIC_NATIVE );
    m_selectedFractureGeneratedProperties.uiCapability()->setUiHidden( isLocked ||
                                                                       m_porosityModel() != RiaDefines::PorosityModelType::FRACTURE_MODEL ||
                                                                       m_resultType() != RiaDefines::ResultCatType::GENERATED );
    m_selectedFractureInputProperties.uiCapability()->setUiHidden( isLocked ||
                                                                   m_porosityModel() != RiaDefines::PorosityModelType::FRACTURE_MODEL ||
                                                                   m_resultType() != RiaDefines::ResultCatType::INPUT_PROPERTY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::updatePercentileUiVisibility()
{
    bool isLocked = hasComputedStatistics();
    m_calculatePercentiles.uiCapability()->setUiHidden( isLocked );
    m_percentileCalculationType.uiCapability()->setUiHidden( isLocked || !m_calculatePercentiles() );
    m_lowPercentile.uiCapability()->setUiHidden( isLocked || !m_calculatePercentiles() );
    m_midPercentile.uiCapability()->setUiHidden( isLocked || !m_calculatePercentiles() );
    m_highPercentile.uiCapability()->setUiHidden( isLocked || !m_calculatePercentiles() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseStatisticsCase::hasComputedStatistics() const
{
    return eclipseCaseData() && ( !eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->existingResults().empty() ||
                                  !eclipseCaseData()->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->existingResults().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::updateConnectedEditorsAndReservoirViews()
{
    auto views = reservoirViews();
    for ( RimEclipseView* view : reservoirViews() )
    {
        // As new result might have been introduced, update all editors connected
        view->cellResult()->updateConnectedEditors();

        // It is usually not needed to create new display model, but if any derived geometry based on generated data
        // (from Octave) a full display model rebuild is required
        view->scheduleCreateDisplayModelAndRedraw();
        view->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::clearComputedStatistics()
{
    if ( eclipseCaseData() && eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
    {
        eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->clearAllResults();
    }

    if ( eclipseCaseData() && eclipseCaseData()->results( RiaDefines::PorosityModelType::FRACTURE_MODEL ) )
    {
        eclipseCaseData()->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->clearAllResults();
    }

    updateConnectedEditorsAndReservoirViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::computeStatisticsAndUpdateViews()
{
    if ( getSourceCases().empty() ) return;

    computeStatistics();
    scheduleACTIVEGeometryRegenOnReservoirViews();
    updateConnectedEditorsAndReservoirViews();

    if ( reservoirViews().empty() )
    {
        RicNewViewFeature::addReservoirView( this, nullptr );
    }

    if ( reservoirViews().size() == 1 )
    {
        // If only one view, set the first result as active

        if ( auto cellResultsData = results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
        {
            auto firstView = reservoirViews()[0];

            std::vector<RigEclipseResultAddress> resAddresses = cellResultsData->existingResults();
            if ( firstView && !resAddresses.empty() )
            {
                firstView->cellResult()->setFromEclipseResultAddress( resAddresses[0] );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::populateResultSelection()
{
    RimIdenticalGridCaseGroup* idgcg = caseGroup();
    if ( !( caseGroup() && caseGroup()->mainCase() && caseGroup()->mainCase()->eclipseCaseData() ) )
    {
        return;
    }

    RigEclipseCaseData* caseData = idgcg->mainCase()->eclipseCaseData();

    if ( m_selectedDynamicProperties().empty() )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->resultNames( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
        if ( varList.contains( RiaResultNames::soil() ) ) m_selectedDynamicProperties.v().push_back( RiaResultNames::soil() );
        if ( varList.contains( "PRESSURE" ) ) m_selectedDynamicProperties.v().push_back( "PRESSURE" );
    }

    if ( m_selectedStaticProperties().empty() )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );
        if ( varList.contains( "PERMX" ) ) m_selectedStaticProperties.v().push_back( "PERMX" );
        if ( varList.contains( "PORO" ) ) m_selectedStaticProperties.v().push_back( "PORO" );
    }

    if ( m_selectedFractureDynamicProperties().empty() )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->resultNames( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
        if ( varList.contains( RiaResultNames::soil() ) ) m_selectedFractureDynamicProperties.v().push_back( RiaResultNames::soil() );
        if ( varList.contains( "PRESSURE" ) ) m_selectedFractureDynamicProperties.v().push_back( "PRESSURE" );
    }

    if ( m_selectedFractureStaticProperties().empty() )
    {
        QStringList varList =
            caseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->resultNames( RiaDefines::ResultCatType::STATIC_NATIVE );
        if ( varList.contains( "PERMX" ) ) m_selectedFractureStaticProperties.v().push_back( "PERMX" );
        if ( varList.contains( "PORO" ) ) m_selectedFractureStaticProperties.v().push_back( "PORO" );
    }
}
