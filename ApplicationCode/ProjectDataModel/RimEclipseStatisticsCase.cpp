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

#include "RicNewViewFeature.h"
#include "RicfCommandObject.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"

#include "RimCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseStatisticsCaseEvaluator.h"
#include "RimEclipseView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimIntersectionCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInViewCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmFieldIOScriptability.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafProgressInfo.h"

namespace caf
{
template <>
void caf::AppEnum<RimEclipseStatisticsCase::PercentileCalcType>::setUp()
{
    addItem( RimEclipseStatisticsCase::NEAREST_OBSERVATION, "NearestObservationPercentile", "Nearest Observation" );
    addItem( RimEclipseStatisticsCase::HISTOGRAM_ESTIMATED, "HistogramEstimatedPercentile", "Histogram based estimate" );
    addItem( RimEclipseStatisticsCase::INTERPOLATED_OBSERVATION,
             "InterpolatedObservationPercentile",
             "Interpolated Observation" );
    setDefault( RimEclipseStatisticsCase::INTERPOLATED_OBSERVATION );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimEclipseStatisticsCase, "RimStatisticalCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase::RimEclipseStatisticsCase()
    : RimEclipseCase()
{
    CAF_PDM_InitObject( "Case Group Statistics", ":/Histogram16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_calculateEditCommand, "m_editingAllowed", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_calculateEditCommand );
    m_calculateEditCommand = false;

    CAF_PDM_InitField( &m_selectionSummary, "SelectionSummary", QString( "" ), "Summary of Calculation Setup", "", "", "" );
    m_selectionSummary.xmlCapability()->disableIO();
    m_selectionSummary.uiCapability()->setUiReadOnly( true );
    m_selectionSummary.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_selectionSummary.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_resultType, "ResultType", "Result Type", "", "", "" );
    m_resultType.xmlCapability()->setIOWritable( false );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_porosityModel, "PorosityModel", "Porosity Model", "", "", "" );
    m_porosityModel.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedDynamicProperties,
                                                "DynamicPropertiesToCalculate",
                                                "Dyn Prop",
                                                "",
                                                "",
                                                "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedStaticProperties,
                                                "StaticPropertiesToCalculate",
                                                "Stat Prop",
                                                "",
                                                "",
                                                "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedGeneratedProperties, "GeneratedPropertiesToCalculate", "", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedInputProperties, "InputPropertiesToCalculate", "", "", "", "" );

    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedFractureDynamicProperties,
                                                "FractureDynamicPropertiesToCalculate",
                                                "",
                                                "",
                                                "",
                                                "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedFractureStaticProperties,
                                                "FractureStaticPropertiesToCalculate",
                                                "",
                                                "",
                                                "",
                                                "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedFractureGeneratedProperties,
                                                "FractureGeneratedPropertiesToCalculate",
                                                "",
                                                "",
                                                "",
                                                "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_selectedFractureInputProperties,
                                                "FractureInputPropertiesToCalculate",
                                                "",
                                                "",
                                                "",
                                                "" );

    m_selectedDynamicProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedStaticProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedGeneratedProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedInputProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    m_selectedFractureDynamicProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedFractureStaticProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedFractureGeneratedProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_selectedFractureInputProperties.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableFieldWithIO( &m_calculatePercentiles, "CalculatePercentiles", true, "Calculate Percentiles", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_percentileCalculationType, "PercentileCalculationType", "Method", "", "", "" );

    CAF_PDM_InitScriptableFieldWithIO( &m_lowPercentile, "LowPercentile", 10.0, "Low", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIO( &m_midPercentile, "MidPercentile", 50.0, "Mid", "", "", "" );
    CAF_PDM_InitScriptableFieldWithIO( &m_highPercentile, "HighPercentile", 90.0, "High", "", "", "" );

    CAF_PDM_InitScriptableFieldWithIO( &m_wellDataSourceCase,
                                       "WellDataSourceCase",
                                       RiaDefines::undefinedResultName(),
                                       "Well Data Source Case",
                                       "",
                                       "",
                                       "" );

    CAF_PDM_InitScriptableFieldWithIO( &m_useZeroAsInactiveCellValue,
                                       "UseZeroAsInactiveCellValue",
                                       false,
                                       "Use Zero as Inactive Cell Value",
                                       "",
                                       "",
                                       "" );

    m_populateSelectionAfterLoadingGrid = false;

    // These does not work properly for statistics case, so hide for now
    m_flipXAxis.uiCapability()->setUiHidden( true );
    m_flipYAxis.uiCapability()->setUiHidden( true );
    m_activeFormationNames.uiCapability()->setUiHidden( true );
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
    CVF_ASSERT( this->eclipseCaseData() );

    eclipseCaseData()->setMainGrid( mainGrid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseStatisticsCase::openEclipseGridFile()
{
    if ( this->eclipseCaseData() ) return true;

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( this );

    CVF_ASSERT( parentStatisticsCaseCollection() );

    RimIdenticalGridCaseGroup* gridCaseGroup = parentStatisticsCaseCollection()->parentCaseGroup();
    CVF_ASSERT( gridCaseGroup );

    RigMainGrid* mainGrid = gridCaseGroup->mainGrid();

    eclipseCase->setMainGrid( mainGrid );

    eclipseCase->setActiveCellInfo( RiaDefines::MATRIX_MODEL,
                                    gridCaseGroup->unionOfActiveCells( RiaDefines::MATRIX_MODEL ) );
    eclipseCase->setActiveCellInfo( RiaDefines::FRACTURE_MODEL,
                                    gridCaseGroup->unionOfActiveCells( RiaDefines::FRACTURE_MODEL ) );

    this->setReservoirData( eclipseCase.p() );

    loadSimulationWellDataFromSourceCase();

    if ( m_populateSelectionAfterLoadingGrid )
    {
        this->populateResultSelection();

        m_populateSelectionAfterLoadingGrid = false;
    }

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
    return dynamic_cast<RimCaseCollection*>( this->parentField()->ownerObject() );
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
void RimEclipseStatisticsCase::computeStatistics()
{
    if ( this->eclipseCaseData() == nullptr )
    {
        openEclipseGridFile();
    }

    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    CVF_ASSERT( gridCaseGroup );
    gridCaseGroup->computeUnionOfActiveCells();

    std::vector<RimEclipseCase*> sourceCases = getSourceCases();

    if ( sourceCases.size() == 0 || !sourceCases.at( 0 )->results( RiaDefines::MATRIX_MODEL ) )
    {
        return;
    }

    // The first source has been read completely from disk, and contains grid and meta data
    // Use this information for all cases in the case group
    size_t timeStepCount = sourceCases.at( 0 )->results( RiaDefines::MATRIX_MODEL )->maxTimeStepCount();

    RimStatisticsConfig statisticsConfig;

    statisticsConfig.m_calculatePercentiles = m_calculatePercentiles();
    statisticsConfig.m_pMaxPos              = m_highPercentile();
    statisticsConfig.m_pMidPos              = m_midPercentile();
    statisticsConfig.m_pMinPos              = m_lowPercentile();
    statisticsConfig.m_pValMethod           = m_percentileCalculationType();

    std::vector<size_t> timeStepIndices;
    for ( size_t i = 0; i < timeStepCount; i++ )
    {
        timeStepIndices.push_back( i );
    }

    // If no dynamic data is present, we might end up with no time steps. Make sure we have at least one.
    if ( timeStepIndices.empty() )
    {
        timeStepIndices.push_back( 0 );
    }

    RigEclipseCaseData* resultCase = eclipseCaseData();

    QList<RimEclipseStatisticsCaseEvaluator::ResSpec> resultSpecification;

    for ( size_t pIdx = 0; pIdx < m_selectedDynamicProperties().size(); ++pIdx )
    {
        resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::MATRIX_MODEL,
                                                                                RiaDefines::DYNAMIC_NATIVE,
                                                                                m_selectedDynamicProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedStaticProperties().size(); ++pIdx )
    {
        resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::MATRIX_MODEL,
                                                                                RiaDefines::STATIC_NATIVE,
                                                                                m_selectedStaticProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedGeneratedProperties().size(); ++pIdx )
    {
        resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::MATRIX_MODEL,
                                                                                RiaDefines::GENERATED,
                                                                                m_selectedGeneratedProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedInputProperties().size(); ++pIdx )
    {
        resultSpecification.append( RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::MATRIX_MODEL,
                                                                                RiaDefines::INPUT_PROPERTY,
                                                                                m_selectedInputProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedFractureDynamicProperties().size(); ++pIdx )
    {
        resultSpecification.append(
            RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::FRACTURE_MODEL,
                                                        RiaDefines::DYNAMIC_NATIVE,
                                                        m_selectedFractureDynamicProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedFractureStaticProperties().size(); ++pIdx )
    {
        resultSpecification.append(
            RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::FRACTURE_MODEL,
                                                        RiaDefines::STATIC_NATIVE,
                                                        m_selectedFractureStaticProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedFractureGeneratedProperties().size(); ++pIdx )
    {
        resultSpecification.append(
            RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::FRACTURE_MODEL,
                                                        RiaDefines::GENERATED,
                                                        m_selectedFractureGeneratedProperties()[pIdx] ) );
    }

    for ( size_t pIdx = 0; pIdx < m_selectedFractureInputProperties().size(); ++pIdx )
    {
        resultSpecification.append(
            RimEclipseStatisticsCaseEvaluator::ResSpec( RiaDefines::FRACTURE_MODEL,
                                                        RiaDefines::INPUT_PROPERTY,
                                                        m_selectedFractureInputProperties()[pIdx] ) );
    }

    RimEclipseStatisticsCaseEvaluator stat( sourceCases, timeStepIndices, statisticsConfig, resultCase, gridCaseGroup );

    if ( m_useZeroAsInactiveCellValue )
    {
        stat.useZeroAsValueForInActiveCellsBasedOnUnionOfActiveCells();
    }

    stat.evaluateForResults( resultSpecification );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::scheduleACTIVEGeometryRegenOnReservoirViews()
{
    for ( size_t i = 0; i < reservoirViews().size(); i++ )
    {
        RimEclipseView* reservoirView = reservoirViews()[i];
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

    uiOrdering.add( &caseUserDescription );
    uiOrdering.add( &caseId );

    uiOrdering.add( &m_calculateEditCommand );

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Summary of Calculation Setup" );
    group->add( &m_useZeroAsInactiveCellValue );
    m_useZeroAsInactiveCellValue.uiCapability()->setUiHidden( hasComputedStatistics() );
    group->add( &m_selectionSummary );

    group = uiOrdering.addNewGroup( "Properties to consider" );
    group->setUiHidden( hasComputedStatistics() );
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

    group = uiOrdering.addNewGroup( "Percentile setup" );
    group->setUiHidden( hasComputedStatistics() );
    group->add( &m_calculatePercentiles );
    group->add( &m_percentileCalculationType );
    group->add( &m_lowPercentile );
    group->add( &m_midPercentile );
    group->add( &m_highPercentile );

    group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_wellDataSourceCase );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
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
QList<caf::PdmOptionItemInfo>
    RimEclipseStatisticsCase::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( useOptionsOnly ) *useOptionsOnly = true;

    RimIdenticalGridCaseGroup* idgcg = caseGroup();
    if ( !( caseGroup() && caseGroup()->mainCase() && caseGroup()->mainCase()->eclipseCaseData() ) )
    {
        return options;
    }

    RigEclipseCaseData* caseData = idgcg->mainCase()->eclipseCaseData();

    if ( &m_selectedDynamicProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::MATRIX_MODEL )->resultNames( RiaDefines::DYNAMIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedStaticProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::MATRIX_MODEL )->resultNames( RiaDefines::STATIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedGeneratedProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::MATRIX_MODEL )->resultNames( RiaDefines::GENERATED );
        return toOptionList( varList );
    }
    else if ( &m_selectedInputProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::MATRIX_MODEL )->resultNames( RiaDefines::INPUT_PROPERTY );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureDynamicProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::FRACTURE_MODEL )->resultNames( RiaDefines::DYNAMIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureStaticProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::FRACTURE_MODEL )->resultNames( RiaDefines::STATIC_NATIVE );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureGeneratedProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::FRACTURE_MODEL )->resultNames( RiaDefines::GENERATED );
        return toOptionList( varList );
    }
    else if ( &m_selectedFractureInputProperties == fieldNeedingOptions )
    {
        QStringList varList = caseData->results( RiaDefines::FRACTURE_MODEL )->resultNames( RiaDefines::INPUT_PROPERTY );
        return toOptionList( varList );
    }

    else if ( &m_wellDataSourceCase == fieldNeedingOptions )
    {
        QStringList sourceCaseNames;
        sourceCaseNames += RiaDefines::undefinedResultName();

        for ( size_t i = 0; i < caseGroup()->caseCollection()->reservoirs().size(); i++ )
        {
            sourceCaseNames += caseGroup()->caseCollection()->reservoirs()[i]->caseUserDescription();
        }

        return toOptionList( sourceCaseNames );
    }

    if ( !options.size() ) options = RimEclipseCase::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
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

    if ( &m_wellDataSourceCase == changedField )
    {
        loadSimulationWellDataFromSourceCase();

        caf::ProgressInfo progInfo( reservoirViews().size() + 1, "Updating Well Data for Views" );

        // Update views
        for ( size_t i = 0; i < reservoirViews().size(); i++ )
        {
            RimEclipseView* reservoirView = reservoirViews()[i];
            CVF_ASSERT( reservoirView );

            reservoirView->wellCollection()->wells.deleteAllChildObjects();
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
            const cvf::Collection<RigSimWellData>& sourceCaseSimWellData =
                sourceResultCase->eclipseCaseData()->wellResults();

            this->eclipseCaseData()->setSimWellData( sourceCaseSimWellData );
        }
    }
    else
    {
        cvf::Collection<RigSimWellData> sourceCaseWellResults;
        this->eclipseCaseData()->setSimWellData( sourceCaseWellResults );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void addPropertySetToHtmlText( QString& html, const QString& heading, const std::vector<QString>& varNames )
{
    if ( varNames.size() )
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
        html += "Percentiles for : " + QString::number( m_lowPercentile() ) + ", " +
                QString::number( m_midPercentile() ) + ", " + QString::number( m_highPercentile() );
    }
    html += "</p>";

    addPropertySetToHtmlText( html, "Dynamic properties", m_selectedDynamicProperties() );
    addPropertySetToHtmlText( html, "Static properties", m_selectedStaticProperties() );
    addPropertySetToHtmlText( html, "Generated properties", m_selectedGeneratedProperties() );
    addPropertySetToHtmlText( html, "Input properties", m_selectedInputProperties() );

    addPropertySetToHtmlText( html, "Dynamic properties, fracture model", m_selectedFractureDynamicProperties() );
    addPropertySetToHtmlText( html, "Static properties, fracture model", m_selectedFractureStaticProperties() );
    addPropertySetToHtmlText( html, "Generated properties, fracture model", m_selectedFractureGeneratedProperties() );
    addPropertySetToHtmlText( html, "Input properties, fracture model", m_selectedFractureInputProperties() );

    m_selectionSummary = html;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                      QString                    uiConfigName,
                                                      caf::PdmUiEditorAttribute* attribute )
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
void RimEclipseStatisticsCase::updateSelectionListVisibilities()
{
    bool isLocked = hasComputedStatistics();
    m_resultType.uiCapability()->setUiHidden( isLocked );
    m_porosityModel.uiCapability()->setUiHidden(
        isLocked ); // ||
                    // !caseGroup()->mainCase()->reservoirData()->results(RiaDefines::FRACTURE_MODEL)->resultCount()

    m_selectedDynamicProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::MATRIX_MODEL && m_resultType() == RiaDefines::DYNAMIC_NATIVE ) );
    m_selectedStaticProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::MATRIX_MODEL && m_resultType() == RiaDefines::STATIC_NATIVE ) );
    m_selectedGeneratedProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::MATRIX_MODEL && m_resultType() == RiaDefines::GENERATED ) );
    m_selectedInputProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::MATRIX_MODEL && m_resultType() == RiaDefines::INPUT_PROPERTY ) );

    m_selectedFractureDynamicProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::FRACTURE_MODEL && m_resultType() == RiaDefines::DYNAMIC_NATIVE ) );
    m_selectedFractureStaticProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::FRACTURE_MODEL && m_resultType() == RiaDefines::STATIC_NATIVE ) );
    m_selectedFractureGeneratedProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::FRACTURE_MODEL && m_resultType() == RiaDefines::GENERATED ) );
    m_selectedFractureInputProperties.uiCapability()->setUiHidden(
        isLocked || !( m_porosityModel() == RiaDefines::FRACTURE_MODEL && m_resultType() == RiaDefines::INPUT_PROPERTY ) );
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
    if ( eclipseCaseData() && ( eclipseCaseData()->results( RiaDefines::MATRIX_MODEL )->existingResults().size() ||
                                eclipseCaseData()->results( RiaDefines::FRACTURE_MODEL )->existingResults().size() ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::updateConnectedEditorsAndReservoirViews()
{
    for ( size_t i = 0; i < reservoirViews.size(); ++i )
    {
        if ( reservoirViews[i] )
        {
            // As new result might have been introduced, update all editors connected
            reservoirViews[i]->cellResult()->updateConnectedEditors();

            // It is usually not needed to create new display model, but if any derived geometry based on generated data
            // (from Octave) a full display model rebuild is required
            reservoirViews[i]->hasUserRequestedAnimation = true;
            reservoirViews[i]->scheduleCreateDisplayModelAndRedraw();
            reservoirViews[i]->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
        }
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseStatisticsCase::clearComputedStatistics()
{
    if ( eclipseCaseData() && eclipseCaseData()->results( RiaDefines::MATRIX_MODEL ) )
    {
        eclipseCaseData()->results( RiaDefines::MATRIX_MODEL )->clearAllResults();
    }

    if ( eclipseCaseData() && eclipseCaseData()->results( RiaDefines::FRACTURE_MODEL ) )
    {
        eclipseCaseData()->results( RiaDefines::FRACTURE_MODEL )->clearAllResults();
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

    if ( reservoirViews.size() == 0 )
    {
        RicNewViewFeature::addReservoirView( this, nullptr );
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

    if ( m_selectedDynamicProperties().size() == 0 )
    {
        QStringList varList = caseData->results( RiaDefines::MATRIX_MODEL )->resultNames( RiaDefines::DYNAMIC_NATIVE );
        if ( varList.contains( "SOIL" ) ) m_selectedDynamicProperties.v().push_back( "SOIL" );
        if ( varList.contains( "PRESSURE" ) ) m_selectedDynamicProperties.v().push_back( "PRESSURE" );
    }

    if ( m_selectedStaticProperties().size() == 0 )
    {
        QStringList varList = caseData->results( RiaDefines::MATRIX_MODEL )->resultNames( RiaDefines::STATIC_NATIVE );
        if ( varList.contains( "PERMX" ) ) m_selectedStaticProperties.v().push_back( "PERMX" );
        if ( varList.contains( "PORO" ) ) m_selectedStaticProperties.v().push_back( "PORO" );
    }

    if ( m_selectedFractureDynamicProperties().size() == 0 )
    {
        QStringList varList = caseData->results( RiaDefines::FRACTURE_MODEL )->resultNames( RiaDefines::DYNAMIC_NATIVE );
        if ( varList.contains( "SOIL" ) ) m_selectedFractureDynamicProperties.v().push_back( "SOIL" );
        if ( varList.contains( "PRESSURE" ) ) m_selectedFractureDynamicProperties.v().push_back( "PRESSURE" );
    }

    if ( m_selectedFractureStaticProperties().size() == 0 )
    {
        QStringList varList = caseData->results( RiaDefines::FRACTURE_MODEL )->resultNames( RiaDefines::STATIC_NATIVE );
        if ( varList.contains( "PERMX" ) ) m_selectedFractureStaticProperties.v().push_back( "PERMX" );
        if ( varList.contains( "PORO" ) ) m_selectedFractureStaticProperties.v().push_back( "PORO" );
    }
}
