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

#include "RimEclipseResultDefinition.h"

#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RicfCommandObject.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigFormationNames.h"

#include "Rim3dView.h"
#include "Rim3dWellLogCurve.h"
#include "RimCellEdgeColors.h"
#include "RimContourMapProjection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinitionTools.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimFlowDiagnosticsTools.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotDataSet.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimIntersectionCollection.h"
#include "RimIntersectionResultDefinition.h"
#include "RimPlotCurve.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimViewLinker.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogTrack.h"

#ifdef USE_QTCHARTS
#include "RimGridStatisticsPlot.h"
#endif

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cafUtils.h"

#include <QRegularExpression>

namespace caf
{
template <>
void RimEclipseResultDefinition::FlowTracerSelectionEnum::setUp()
{
    addItem( RimEclipseResultDefinition::FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD, "FLOW_TR_INJ_AND_PROD", "All Injectors and Producers" );
    addItem( RimEclipseResultDefinition::FlowTracerSelectionType::FLOW_TR_PRODUCERS, "FLOW_TR_PRODUCERS", "All Producers" );
    addItem( RimEclipseResultDefinition::FlowTracerSelectionType::FLOW_TR_INJECTORS, "FLOW_TR_INJECTORS", "All Injectors" );
    addItem( RimEclipseResultDefinition::FlowTracerSelectionType::FLOW_TR_BY_SELECTION, "FLOW_TR_BY_SELECTION", "By Selection" );

    setDefault( RimEclipseResultDefinition::FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimEclipseResultDefinition, "ResultDefinition" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::RimEclipseResultDefinition( caf::PdmUiItemInfo::LabelPosType labelPosition )
    : m_isDeltaResultEnabled( false )
    , m_labelPosition( labelPosition )
    , m_ternaryEnabled( true )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Result Definition", "", "", "", "EclipseResult", "An eclipse result definition" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_resultType, "ResultType", "Type" );
    m_resultType.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_porosityModel, "PorosityModelType", "Porosity" );
    m_porosityModel.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableField( &m_resultVariable, "ResultVariable", RiaResultNames::undefinedResultName(), "Variable" );
    m_resultVariable.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_flowSolution, "FlowDiagSolution", "Solution" );
    m_flowSolution.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_timeLapseBaseTimestep, "TimeLapseBaseTimeStep", RigEclipseResultAddress::noTimeLapseValue(), "Base Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_differenceCase, "DifferenceCase", "Difference Case" );

    CAF_PDM_InitField( &m_divideByCellFaceArea, "DivideByCellFaceArea", false, "Divide By Area" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedInjectorTracers, "SelectedInjectorTracers", "Injector Tracers" );
    m_selectedInjectorTracers.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedProducerTracers, "SelectedProducerTracers", "Producer Tracers" );
    m_selectedProducerTracers.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_selectedSouringTracers, "SelectedSouringTracers", "Tracers" );
    m_selectedSouringTracers.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_flowTracerSelectionMode, "FlowTracerSelectionMode", "Tracers" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_phaseSelection, "PhaseSelection", "Phases" );
    m_phaseSelection.uiCapability()->setUiLabelPosition( m_labelPosition );

    CAF_PDM_InitScriptableField( &m_showOnlyVisibleCategoriesInLegend,
                                 "ShowOnlyVisibleCategoriesInLegend",
                                 true,
                                 "Show Only Visible Categories In Legend" );

    // Ui only fields

    CAF_PDM_InitFieldNoDefault( &m_resultTypeUiField, "MResultType", "Type" );
    m_resultTypeUiField.xmlCapability()->disableIO();
    m_resultTypeUiField.uiCapability()->setUiLabelPosition( m_labelPosition );

    CAF_PDM_InitFieldNoDefault( &m_porosityModelUiField, "MPorosityModelType", "Porosity" );
    m_porosityModelUiField.xmlCapability()->disableIO();
    m_porosityModelUiField.uiCapability()->setUiLabelPosition( m_labelPosition );

    CAF_PDM_InitField( &m_resultVariableUiField, "MResultVariable", RiaResultNames::undefinedResultName(), "Result Property" );
    m_resultVariableUiField.xmlCapability()->disableIO();
    m_resultVariableUiField.uiCapability()->setUiLabelPosition( m_labelPosition );
    m_resultVariableUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_inputPropertyFileName, "InputPropertyFileName", "File Name" );
    m_inputPropertyFileName.xmlCapability()->disableIO();
    m_inputPropertyFileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowSolutionUiField, "MFlowDiagSolution", "Solution" );
    m_flowSolutionUiField.xmlCapability()->disableIO();
    m_flowSolutionUiField.uiCapability()->setUiHidden( true ); // For now since there are only one to choose from

    CAF_PDM_InitField( &m_syncInjectorToProducerSelection, "MSyncSelectedInjProd", false, "Add Communicators ->" );
    m_syncInjectorToProducerSelection.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_syncProducerToInjectorSelection, "MSyncSelectedProdInj", false, "<- Add Communicators" );
    m_syncProducerToInjectorSelection.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_selectedInjectorTracersUiField, "MSelectedInjectorTracers", "Injector Tracers" );
    m_selectedInjectorTracersUiField.xmlCapability()->disableIO();
    m_selectedInjectorTracersUiField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_selectedProducerTracersUiField, "MSelectedProducerTracers", "Producer Tracers" );
    m_selectedProducerTracersUiField.xmlCapability()->disableIO();
    m_selectedProducerTracersUiField.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_selectedSouringTracersUiField, "MSelectedSouringTracers", "Tracers" );
    m_selectedSouringTracersUiField.xmlCapability()->disableIO();
    m_selectedSouringTracersUiField.uiCapability()->setUiLabelPosition( m_labelPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::~RimEclipseResultDefinition()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::simpleCopy( const RimEclipseResultDefinition* other )
{
    setResultVariable( other->resultVariable() );
    setPorosityModel( other->porosityModel() );
    setResultType( other->resultType() );
    setFlowSolution( other->m_flowSolution() );
    setSelectedInjectorTracers( other->m_selectedInjectorTracers() );
    setSelectedProducerTracers( other->m_selectedProducerTracers() );
    setSelectedSouringTracers( other->m_selectedSouringTracers() );
    m_flowTracerSelectionMode = other->m_flowTracerSelectionMode();
    m_phaseSelection          = other->m_phaseSelection;

    m_differenceCase        = other->m_differenceCase();
    m_timeLapseBaseTimestep = other->m_timeLapseBaseTimestep();
    m_divideByCellFaceArea  = other->m_divideByCellFaceArea();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;

    assignFlowSolutionFromCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimEclipseResultDefinition::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ResultCatType RimEclipseResultDefinition::resultType() const
{
    return m_resultType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RimEclipseResultDefinition::currentGridCellResults() const
{
    if ( !m_eclipseCase ) return nullptr;

    return m_eclipseCase->results( m_porosityModel() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( &m_flowSolutionUiField == changedField || &m_resultTypeUiField == changedField || &m_porosityModelUiField == changedField )
    {
        // If the user are seeing the list with the actually selected result,
        // select that result in the list. Otherwise select nothing.

        QStringList varList = RimEclipseResultDefinitionTools::getResultNamesForResultType( m_resultTypeUiField(), currentGridCellResults() );

        bool isFlowDiagFieldsRelevant = ( m_resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS );

        if ( ( m_flowSolutionUiField() == m_flowSolution() || !isFlowDiagFieldsRelevant ) && m_resultTypeUiField() == m_resultType() &&
             m_porosityModelUiField() == m_porosityModel() )
        {
            if ( varList.contains( resultVariable() ) )
            {
                m_resultVariableUiField = resultVariable();
            }

            if ( isFlowDiagFieldsRelevant )
            {
                m_selectedInjectorTracersUiField = m_selectedInjectorTracers();
                m_selectedProducerTracersUiField = m_selectedProducerTracers();
            }
            else
            {
                m_selectedInjectorTracersUiField = std::vector<QString>();
                m_selectedProducerTracersUiField = std::vector<QString>();
            }
        }
        else
        {
            m_resultVariableUiField          = "";
            m_selectedInjectorTracersUiField = std::vector<QString>();
            m_selectedProducerTracersUiField = std::vector<QString>();
        }
    }

    if ( &m_resultVariableUiField == changedField )
    {
        m_porosityModel  = m_porosityModelUiField;
        m_resultType     = m_resultTypeUiField;
        m_resultVariable = m_resultVariableUiField;

        if ( m_resultTypeUiField() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
        {
            m_flowSolution            = m_flowSolutionUiField();
            m_selectedInjectorTracers = m_selectedInjectorTracersUiField();
            m_selectedProducerTracers = m_selectedProducerTracersUiField();
        }
        else if ( m_resultTypeUiField() == RiaDefines::ResultCatType::INJECTION_FLOODING )
        {
            m_selectedSouringTracers = m_selectedSouringTracersUiField();
        }
        else if ( m_resultTypeUiField() == RiaDefines::ResultCatType::INPUT_PROPERTY )
        {
            m_inputPropertyFileName = RimEclipseResultDefinitionTools::getInputPropertyFileName( eclipseCase(), newValue.toString() );
        }
        loadDataAndUpdate();
    }

    if ( &m_porosityModelUiField == changedField )
    {
        m_porosityModel         = m_porosityModelUiField;
        m_resultVariableUiField = resultVariable();

        auto eclipseView = firstAncestorOrThisOfType<RimEclipseView>();
        if ( eclipseView )
        {
            // Active cells can be different between matrix and fracture, make sure all geometry is recreated
            eclipseView->scheduleReservoirGridGeometryRegen();
        }

        loadDataAndUpdate();
    }

    auto contourMapView = firstAncestorOrThisOfType<RimEclipseContourMapView>();

    if ( &m_differenceCase == changedField )
    {
        m_timeLapseBaseTimestep = RigEclipseResultAddress::noTimeLapseValue();

        if ( contourMapView )
        {
            contourMapView->contourMapProjection()->clearGridMappingAndRedraw();
        }

        loadDataAndUpdate();
    }

    if ( &m_timeLapseBaseTimestep == changedField )
    {
        if ( contourMapView )
        {
            contourMapView->contourMapProjection()->clearGridMappingAndRedraw();
        }

        loadDataAndUpdate();
    }

    if ( &m_divideByCellFaceArea == changedField )
    {
        loadDataAndUpdate();
    }

    if ( &m_flowTracerSelectionMode == changedField )
    {
        loadDataAndUpdate();
    }

    if ( &m_selectedInjectorTracersUiField == changedField )
    {
        changedTracerSelectionField( true );
    }

    if ( &m_selectedProducerTracersUiField == changedField )
    {
        changedTracerSelectionField( false );
    }

    if ( &m_syncInjectorToProducerSelection == changedField )
    {
        syncInjectorToProducerSelection();
        m_syncInjectorToProducerSelection = false;
    }

    if ( &m_syncProducerToInjectorSelection == changedField )
    {
        syncProducerToInjectorSelection();
        m_syncProducerToInjectorSelection = false;
    }

    if ( &m_selectedSouringTracersUiField == changedField )
    {
        if ( !m_resultVariable().isEmpty() )
        {
            m_selectedSouringTracers = m_selectedSouringTracersUiField();
            loadDataAndUpdate();
        }
    }

    if ( &m_phaseSelection == changedField )
    {
        if ( m_phaseSelection() != RigFlowDiagResultAddress::PHASE_ALL )
        {
            m_resultType            = m_resultTypeUiField;
            m_resultVariable        = RIG_FLD_TOF_RESNAME;
            m_resultVariableUiField = RIG_FLD_TOF_RESNAME;
        }
        loadDataAndUpdate();
    }

    if ( &m_showOnlyVisibleCategoriesInLegend == changedField )
    {
        loadDataAndUpdate();
    }

    updateAnyFieldHasChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::changedTracerSelectionField( bool injector )
{
    m_flowSolution = m_flowSolutionUiField();

    std::vector<QString>&       selectedTracers   = injector ? m_selectedInjectorTracers.v() : m_selectedProducerTracers.v();
    const std::vector<QString>& selectedTracersUi = injector ? m_selectedInjectorTracersUiField.v() : m_selectedProducerTracersUiField.v();

    selectedTracers = selectedTracersUi;

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::updateAnyFieldHasChanged()
{
    RimEclipsePropertyFilter* propFilter = firstAncestorOrThisOfType<RimEclipsePropertyFilter>();
    if ( propFilter )
    {
        propFilter->updateConnectedEditors();
    }

    RimEclipseFaultColors* faultColors = firstAncestorOrThisOfType<RimEclipseFaultColors>();
    if ( faultColors )
    {
        faultColors->updateConnectedEditors();
    }

    RimCellEdgeColors* cellEdgeColors = firstAncestorOrThisOfType<RimCellEdgeColors>();
    if ( cellEdgeColors )
    {
        cellEdgeColors->updateConnectedEditors();
    }

    RimEclipseCellColors* cellColors = firstAncestorOrThisOfType<RimEclipseCellColors>();
    if ( cellColors )
    {
        cellColors->updateConnectedEditors();
    }

    RimIntersectionResultDefinition* intersectResDef = firstAncestorOrThisOfType<RimIntersectionResultDefinition>();
    if ( intersectResDef )
    {
        intersectResDef->setDefaultEclipseLegendConfig();
        intersectResDef->updateConnectedEditors();
    }

    RimGridCrossPlotDataSet* crossPlotCurveSet = firstAncestorOrThisOfType<RimGridCrossPlotDataSet>();
    if ( crossPlotCurveSet )
    {
        crossPlotCurveSet->updateConnectedEditors();
    }

    RimPlotCurve* curve = firstAncestorOrThisOfType<RimPlotCurve>();
    if ( curve )
    {
        curve->updateConnectedEditors();
    }

    Rim3dWellLogCurve* rim3dWellLogCurve = firstAncestorOrThisOfType<Rim3dWellLogCurve>();
    if ( rim3dWellLogCurve )
    {
        rim3dWellLogCurve->resetMinMaxValues();
    }

    RimEclipseContourMapProjection* contourMap = firstAncestorOrThisOfType<RimEclipseContourMapProjection>();
    if ( contourMap )
    {
        contourMap->clearGridMappingAndRedraw();
    }

    RimWellLogTrack* wellLogTrack = firstAncestorOrThisOfType<RimWellLogTrack>();
    if ( wellLogTrack )
    {
        wellLogTrack->loadDataAndUpdate();
        wellLogTrack->updateEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setTofAndSelectTracer( const QString& tracerName )
{
    setResultType( RiaDefines::ResultCatType::FLOW_DIAGNOSTICS );
    setResultVariable( "TOF" );
    setFlowDiagTracerSelectionType( FlowTracerSelectionType::FLOW_TR_BY_SELECTION );

    if ( m_flowSolution() == nullptr )
    {
        assignFlowSolutionFromCase();
    }

    if ( m_flowSolution() )
    {
        RimFlowDiagSolution::TracerStatusType tracerStatus = m_flowSolution()->tracerStatusOverall( tracerName );

        std::vector<QString> tracers;
        tracers.push_back( tracerName );
        if ( ( tracerStatus == RimFlowDiagSolution::TracerStatusType::INJECTOR ) ||
             ( tracerStatus == RimFlowDiagSolution::TracerStatusType::VARYING ) )
        {
            setSelectedInjectorTracers( tracers );
        }

        if ( ( tracerStatus == RimFlowDiagSolution::TracerStatusType::PRODUCER ) ||
             ( tracerStatus == RimFlowDiagSolution::TracerStatusType::VARYING ) )
        {
            setSelectedProducerTracers( tracers );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::loadDataAndUpdate()
{
    auto view = firstAncestorOrThisOfType<Rim3dView>();

    loadResult();

    RimEclipsePropertyFilter* propFilter = firstAncestorOrThisOfType<RimEclipsePropertyFilter>();
    if ( propFilter )
    {
        propFilter->setToDefaultValues();
        propFilter->updateFilterName();

        if ( view )
        {
            view->scheduleGeometryRegen( PROPERTY_FILTERED );
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    RimEclipseCellColors* cellColors = firstAncestorOrThisOfType<RimEclipseCellColors>();
    if ( cellColors )
    {
        updateLegendCategorySettings();

        if ( view )
        {
            RimViewLinker* viewLinker = view->assosiatedViewLinker();
            if ( viewLinker )
            {
                viewLinker->updateCellResult();
            }
            RimGridView* eclView = dynamic_cast<RimGridView*>( view );
            if ( eclView ) eclView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
        }
    }

    RimIntersectionResultDefinition* sepIntersectionResDef = firstAncestorOrThisOfType<RimIntersectionResultDefinition>();
    if ( sepIntersectionResDef && sepIntersectionResDef->isInAction() )
    {
        if ( view ) view->scheduleCreateDisplayModelAndRedraw();
        RimGridView* gridView = dynamic_cast<RimGridView*>( view );
        if ( gridView ) gridView->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    }

    RimCellEdgeColors* cellEdgeColors = firstAncestorOrThisOfType<RimCellEdgeColors>();
    if ( cellEdgeColors )
    {
        cellEdgeColors->loadResult();

        if ( view )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }

    RimGridCrossPlotDataSet* crossPlotCurveSet = firstAncestorOrThisOfType<RimGridCrossPlotDataSet>();
    if ( crossPlotCurveSet )
    {
        crossPlotCurveSet->destroyCurves();
        crossPlotCurveSet->loadDataAndUpdate( true );
    }

    RimPlotCurve* curve = firstAncestorOrThisOfType<RimPlotCurve>();
    if ( curve )
    {
        curve->loadDataAndUpdate( true );
    }

    Rim3dWellLogCurve* rim3dWellLogCurve = firstAncestorOrThisOfType<Rim3dWellLogCurve>();
    if ( rim3dWellLogCurve )
    {
        rim3dWellLogCurve->updateCurveIn3dView();
    }

#ifdef USE_QTCHARTS
    RimGridStatisticsPlot* gridStatisticsPlot = firstAncestorOrThisOfType<RimGridStatisticsPlot>();
    if ( gridStatisticsPlot )
    {
        gridStatisticsPlot->loadDataAndUpdate();
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseResultDefinition::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_resultTypeUiField )
    {
        bool                  hasSourSimRLFile = false;
        RimEclipseResultCase* eclResCase       = dynamic_cast<RimEclipseResultCase*>( m_eclipseCase.p() );
        if ( eclResCase && eclResCase->eclipseCaseData() )
        {
            hasSourSimRLFile = eclResCase->hasSourSimFile();
        }

#ifndef USE_HDF5
        // If using ResInsight without HDF5 support, ignore SourSim files and
        // do not show it as a result category.
        hasSourSimRLFile = false;
#endif

        bool enableSouring = false;

#ifdef USE_HDF5
        if ( m_eclipseCase.notNull() )
        {
            RigCaseCellResultsData* cellResultsData = m_eclipseCase->results( porosityModel() );

            if ( cellResultsData && cellResultsData->hasFlowDiagUsableFluxes() )
            {
                enableSouring = true;
            }
        }
#endif /* USE_HDF5 */

        RimGridTimeHistoryCurve* timeHistoryCurve = firstAncestorOrThisOfType<RimGridTimeHistoryCurve>();

        bool isSeparateFaultResult = false;
        {
            RimEclipseFaultColors* sepFaultResult = firstAncestorOrThisOfType<RimEclipseFaultColors>();
            if ( sepFaultResult ) isSeparateFaultResult = true;
        }

        using ResCatEnum = caf::AppEnum<RiaDefines::ResultCatType>;
        for ( size_t i = 0; i < ResCatEnum::size(); ++i )
        {
            RiaDefines::ResultCatType resType = ResCatEnum::fromIndex( i );

            // Do not include flow diagnostics results if it is a time history curve

            if ( resType == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS && ( timeHistoryCurve ) )
            {
                continue;
            }

            if ( resType == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS && m_eclipseCase && m_eclipseCase->eclipseCaseData() &&
                 m_eclipseCase->eclipseCaseData()->hasFractureResults() )
            {
                // Flow diagnostics is not supported for dual porosity models
                continue;
            }

            // Do not include SourSimRL if no SourSim file is loaded

            if ( resType == RiaDefines::ResultCatType::SOURSIMRL && ( !hasSourSimRLFile ) )
            {
                continue;
            }

            if ( resType == RiaDefines::ResultCatType::INJECTION_FLOODING && !enableSouring )
            {
                continue;
            }

            if ( resType == RiaDefines::ResultCatType::ALLAN_DIAGRAMS && !isSeparateFaultResult )
            {
                continue;
            }

            QString uiString = ResCatEnum::uiTextFromIndex( i );
            options.push_back( caf::PdmOptionItemInfo( uiString, resType ) );
        }
    }

    if ( m_resultTypeUiField() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        if ( fieldNeedingOptions == &m_resultVariableUiField )
        {
            options.push_back( caf::PdmOptionItemInfo( RimEclipseResultDefinitionTools::timeOfFlightString( injectorSelectionState(),
                                                                                                            producerSelectionState(),
                                                                                                            false ),
                                                       RIG_FLD_TOF_RESNAME ) );
            if ( m_phaseSelection() == RigFlowDiagResultAddress::PHASE_ALL )
            {
                options.push_back( caf::PdmOptionItemInfo( "Tracer Cell Fraction (Sum)", RIG_FLD_CELL_FRACTION_RESNAME ) );
                options.push_back( caf::PdmOptionItemInfo( RimEclipseResultDefinitionTools::maxFractionTracerString( injectorSelectionState(),
                                                                                                                     producerSelectionState(),
                                                                                                                     false ),
                                                           RIG_FLD_MAX_FRACTION_TRACER_RESNAME ) );
                options.push_back( caf::PdmOptionItemInfo( "Injector Producer Communication", RIG_FLD_COMMUNICATION_RESNAME ) );
            }
        }
        else if ( fieldNeedingOptions == &m_flowSolutionUiField )
        {
            RimEclipseResultCase* eclCase = dynamic_cast<RimEclipseResultCase*>( m_eclipseCase.p() );
            if ( eclCase )
            {
                std::vector<RimFlowDiagSolution*> flowSols = eclCase->flowDiagSolutions();
                for ( RimFlowDiagSolution* flowSol : flowSols )
                {
                    options.push_back( caf::PdmOptionItemInfo( flowSol->userDescription(), flowSol ) );
                }
            }
        }
        else if ( fieldNeedingOptions == &m_selectedInjectorTracersUiField )
        {
            const bool isInjector = true;
            options               = RimFlowDiagnosticsTools::calcOptionsForSelectedTracerField( m_flowSolutionUiField(), isInjector );
        }
        else if ( fieldNeedingOptions == &m_selectedProducerTracersUiField )
        {
            const bool isInjector = false;
            options               = RimFlowDiagnosticsTools::calcOptionsForSelectedTracerField( m_flowSolutionUiField(), isInjector );
        }
    }
    else if ( m_resultTypeUiField() == RiaDefines::ResultCatType::INJECTION_FLOODING )
    {
        if ( fieldNeedingOptions == &m_selectedSouringTracersUiField )
        {
            RigCaseCellResultsData* cellResultsStorage = currentGridCellResults();
            if ( cellResultsStorage )
            {
                QStringList dynamicResultNames = cellResultsStorage->resultNames( RiaDefines::ResultCatType::DYNAMIC_NATIVE );

                for ( const QString& resultName : dynamicResultNames )
                {
                    if ( !resultName.endsWith( "F" ) || resultName == RiaResultNames::completionTypeResultName() )
                    {
                        continue;
                    }
                    options.push_back( caf::PdmOptionItemInfo( resultName, resultName ) );
                }
            }
        }
        else if ( fieldNeedingOptions == &m_resultVariableUiField )
        {
            options.push_back( caf::PdmOptionItemInfo( RIG_NUM_FLOODED_PV, RIG_NUM_FLOODED_PV ) );
        }
    }
    else
    {
        if ( fieldNeedingOptions == &m_resultVariableUiField )
        {
            options = calcOptionsForVariableUiFieldStandard( m_resultTypeUiField(),
                                                             currentGridCellResults(),
                                                             showDerivedResultsFirstInVariableUiField(),
                                                             addPerCellFaceOptionsForVariableUiField(),
                                                             m_ternaryEnabled );
        }
        else if ( fieldNeedingOptions == &m_differenceCase )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

            RimEclipseView* eclView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();

            auto eclipseCase = eclView->eclipseCase();
            if ( eclipseCase && eclipseCase->eclipseCaseData() && eclipseCase->eclipseCaseData()->mainGrid() )
            {
                RimProject* proj = RimProject::current();

                std::vector<RimEclipseCase*> allCases = proj->eclipseCases();
                for ( RimEclipseCase* otherCase : allCases )
                {
                    if ( otherCase == eclipseCase ) continue;

                    if ( otherCase->eclipseCaseData() && otherCase->eclipseCaseData()->mainGrid() )
                    {
                        options.push_back(
                            caf::PdmOptionItemInfo( QString( "%1 (#%2)" ).arg( otherCase->caseUserDescription() ).arg( otherCase->caseId() ),
                                                    otherCase,
                                                    false,
                                                    otherCase->uiIconProvider() ) );
                    }
                }
            }
        }
        else if ( fieldNeedingOptions == &m_timeLapseBaseTimestep )
        {
            RimEclipseView* eclView = firstAncestorOrThisOfTypeAsserted<RimEclipseView>();

            RimEclipseCase* currentCase = eclView->eclipseCase();

            RimEclipseCase* baseCase = currentCase;
            if ( m_differenceCase )
            {
                baseCase = m_differenceCase;
            }

            options.push_back( caf::PdmOptionItemInfo( "Disabled", RigEclipseResultAddress::noTimeLapseValue() ) );

            if ( baseCase )
            {
                std::vector<QDateTime> stepDates = baseCase->timeStepDates();
                for ( size_t stepIdx = 0; stepIdx < stepDates.size(); ++stepIdx )
                {
                    QString displayString = stepDates[stepIdx].toString( RiaQDateTimeTools::dateFormatString() );
                    displayString += QString( " (#%1)" ).arg( stepIdx );

                    options.push_back( caf::PdmOptionItemInfo( displayString, static_cast<int>( stepIdx ) ) );
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RimEclipseResultDefinition::eclipseResultAddress() const
{
    if ( !isChecked() ) return RigEclipseResultAddress();
    if ( isFlowDiagOrInjectionFlooding() ) return RigEclipseResultAddress();

    const RigCaseCellResultsData* gridCellResults = currentGridCellResults();
    if ( gridCellResults )
    {
        int timelapseTimeStep = RigEclipseResultAddress::noTimeLapseValue();
        int diffCaseId        = RigEclipseResultAddress::noCaseDiffValue();

        if ( isDeltaTimeStepActive() )
        {
            timelapseTimeStep = m_timeLapseBaseTimestep();
        }

        if ( isDeltaCaseActive() )
        {
            diffCaseId = m_differenceCase->caseId();
        }

        return RigEclipseResultAddress( m_resultType(), m_resultVariable(), timelapseTimeStep, diffCaseId, isDivideByCellFaceAreaActive() );
    }
    else
    {
        return RigEclipseResultAddress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setFromEclipseResultAddress( const RigEclipseResultAddress& address )
{
    RigEclipseResultAddress canonizedAddress = address;

    const RigCaseCellResultsData* gridCellResults = currentGridCellResults();
    if ( gridCellResults )
    {
        auto rinfo = gridCellResults->resultInfo( address );
        if ( rinfo ) canonizedAddress = rinfo->eclipseResultAddress();
    }

    m_resultType            = canonizedAddress.resultCatType();
    m_resultVariable        = canonizedAddress.resultName();
    m_timeLapseBaseTimestep = canonizedAddress.deltaTimeStepIndex();
    m_divideByCellFaceArea  = canonizedAddress.isDivideByCellFaceAreaActive();

    if ( canonizedAddress.isDeltaCaseActive() )
    {
        auto eclipseCases = RimProject::current()->eclipseCases();
        for ( RimEclipseCase* c : eclipseCases )
        {
            if ( c && c->caseId() == canonizedAddress.deltaCaseId() )
            {
                m_differenceCase = c;
            }
        }
    }

    updateUiFieldsFromActiveResult();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagResultAddress RimEclipseResultDefinition::flowDiagResAddress() const
{
    CVF_ASSERT( isFlowDiagOrInjectionFlooding() );

    if ( m_resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        size_t timeStep = 0;

        auto rimView = firstAncestorOrThisOfType<Rim3dView>();
        if ( rimView )
        {
            timeStep = rimView->currentTimeStep();
        }
        RimWellLogExtractionCurve* wellLogExtractionCurve = firstAncestorOrThisOfType<RimWellLogExtractionCurve>();
        if ( wellLogExtractionCurve )
        {
            timeStep = static_cast<size_t>( wellLogExtractionCurve->currentTimeStep() );
        }

        std::set<std::string> selTracerNames;
        if ( m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_BY_SELECTION )
        {
            for ( const QString& tName : m_selectedInjectorTracers() )
            {
                selTracerNames.insert( tName.toStdString() );
            }
            for ( const QString& tName : m_selectedProducerTracers() )
            {
                selTracerNames.insert( tName.toStdString() );
            }
        }
        else
        {
            RimFlowDiagSolution* flowSol = m_flowSolution();
            if ( flowSol )
            {
                std::vector<QString> tracerNames = flowSol->tracerNames();

                if ( m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_INJECTORS ||
                     m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD )
                {
                    for ( const QString& tracerName : tracerNames )
                    {
                        RimFlowDiagSolution::TracerStatusType status = flowSol->tracerStatusInTimeStep( tracerName, timeStep );
                        if ( status == RimFlowDiagSolution::TracerStatusType::INJECTOR )
                        {
                            selTracerNames.insert( tracerName.toStdString() );
                        }
                    }
                }

                if ( m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_PRODUCERS ||
                     m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_INJ_AND_PROD )
                {
                    for ( const QString& tracerName : tracerNames )
                    {
                        RimFlowDiagSolution::TracerStatusType status = flowSol->tracerStatusInTimeStep( tracerName, timeStep );
                        if ( status == RimFlowDiagSolution::TracerStatusType::PRODUCER )
                        {
                            selTracerNames.insert( tracerName.toStdString() );
                        }
                    }
                }
            }
        }

        return RigFlowDiagResultAddress( m_resultVariable().toStdString(), m_phaseSelection(), selTracerNames );
    }

    std::set<std::string> selTracerNames;
    for ( const QString& selectedTracerName : m_selectedSouringTracers() )
    {
        selTracerNames.insert( selectedTracerName.toUtf8().constData() );
    }
    return RigFlowDiagResultAddress( m_resultVariable().toStdString(), RigFlowDiagResultAddress::PHASE_ALL, selTracerNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setFlowDiagTracerSelectionType( FlowTracerSelectionType selectionType )
{
    m_flowTracerSelectionMode = selectionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::resultVariableUiName() const
{
    if ( resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        return flowDiagResUiText( false, 32 );
    }

    if ( isDivideByCellFaceAreaActive() )
    {
        return m_resultVariable() + " /A";
    }

    return m_resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::resultVariableUiShortName() const
{
    if ( resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        return flowDiagResUiText( true, 24 );
    }

    if ( isDivideByCellFaceAreaActive() )
    {
        return m_resultVariable() + " /A";
    }

    return m_resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::additionalResultText() const
{
    QStringList resultText;

    if ( isDeltaTimeStepActive() )
    {
        std::vector<QDateTime>        stepDates;
        const RigCaseCellResultsData* gridCellResults = currentGridCellResults();
        if ( gridCellResults )
        {
            stepDates = gridCellResults->timeStepDates();
            resultText += QString( "<b>Base Time Step</b>: %1" )
                              .arg( stepDates[m_timeLapseBaseTimestep()].toString( RiaQDateTimeTools::dateFormatString() ) );
        }
    }
    if ( isDeltaCaseActive() )
    {
        resultText += QString( "<b>Base Case</b>: %1" ).arg( m_differenceCase()->caseUserDescription() );
    }
    return resultText.join( "<br>" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::additionalResultTextShort() const
{
    QString resultTextShort;
    if ( isDeltaTimeStepActive() || isDeltaCaseActive() )
    {
        QStringList resultTextLines;
        resultTextLines += QString( "\nDiff. Options:" );
        if ( isDeltaCaseActive() )
        {
            resultTextLines += QString( "Base Case: #%1" ).arg( m_differenceCase()->caseId() );
        }
        if ( isDeltaTimeStepActive() )
        {
            resultTextLines += QString( "Base Time: #%1" ).arg( m_timeLapseBaseTimestep() );
        }
        resultTextShort = resultTextLines.join( "\n" );
    }

    return resultTextShort;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEclipseResultDefinition::timeLapseBaseTimeStep() const
{
    return m_timeLapseBaseTimestep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimEclipseResultDefinition::caseDiffIndex() const
{
    if ( m_differenceCase )
    {
        return m_differenceCase->caseId();
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::loadResult()
{
    if ( isFlowDiagOrInjectionFlooding() ) return; // Will load automatically on access

    if ( m_eclipseCase )
    {
        if ( !m_eclipseCase->ensureReservoirCaseIsOpen() )
        {
            RiaLogging::error( "Could not open the Eclipse Grid file: " + m_eclipseCase->gridFileName() );
            return;
        }
    }

    if ( m_differenceCase )
    {
        if ( !m_differenceCase->ensureReservoirCaseIsOpen() )
        {
            RiaLogging::error( "Could not open the Eclipse Grid file: " + m_eclipseCase->gridFileName() );
            return;
        }
    }

    RigCaseCellResultsData* gridCellResults = currentGridCellResults();
    if ( gridCellResults )
    {
        if ( isDeltaTimeStepActive() || isDeltaCaseActive() || isDivideByCellFaceAreaActive() )
        {
            gridCellResults->createResultEntry( eclipseResultAddress(), false );
        }

        QString           resultName                    = m_resultVariable();
        std::set<QString> eclipseResultNamesWithNncData = RiaResultNames::nncResultNames();
        if ( eclipseResultNamesWithNncData.find( resultName ) != eclipseResultNamesWithNncData.end() )
        {
            eclipseCase()->ensureFaultDataIsComputed();

            bool dataWasComputed = eclipseCase()->ensureNncDataIsComputed();
            if ( dataWasComputed )
            {
                eclipseCase()->createDisplayModelAndUpdateAllViews();
            }
        }

        gridCellResults->ensureKnownResultLoaded( eclipseResultAddress() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a single frame result
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasStaticResult() const
{
    if ( isFlowDiagOrInjectionFlooding() ) return false;

    const RigCaseCellResultsData* gridCellResults       = currentGridCellResults();
    RigEclipseResultAddress       gridScalarResultIndex = eclipseResultAddress();

    return hasResult() && gridCellResults->timeStepCount( gridScalarResultIndex ) == 1;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is loaded or possible to load from the result file
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasResult() const
{
    if ( isFlowDiagOrInjectionFlooding() )
    {
        if ( m_flowSolution() && !m_resultVariable().isEmpty() ) return true;
    }
    else if ( currentGridCellResults() )
    {
        const RigCaseCellResultsData* gridCellResults = currentGridCellResults();

        return gridCellResults->hasResultEntry( eclipseResultAddress() );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a multi frame result
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasDynamicResult() const
{
    if ( hasResult() )
    {
        if ( m_resultType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
        {
            return true;
        }
        else if ( m_resultType() == RiaDefines::ResultCatType::SOURSIMRL )
        {
            return true;
        }
        else if ( m_resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
        {
            return true;
        }
        else if ( m_resultType() == RiaDefines::ResultCatType::INJECTION_FLOODING )
        {
            return true;
        }

        if ( currentGridCellResults() )
        {
            const RigCaseCellResultsData* gridCellResults       = currentGridCellResults();
            RigEclipseResultAddress       gridScalarResultIndex = eclipseResultAddress();
            if ( gridCellResults->timeStepCount( gridScalarResultIndex ) > 1 )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::initAfterRead()
{
    if ( m_flowSolution() == nullptr )
    {
        assignFlowSolutionFromCase();
    }

    if ( m_resultVariable() == "Formation Allen" )
    {
        m_resultVariable = RiaResultNames::formationAllanResultName();
        m_resultType     = RiaDefines::ResultCatType::ALLAN_DIAGRAMS;
    }
    else if ( m_resultVariable() == "Binary Formation Allen" )
    {
        m_resultVariable = RiaResultNames::formationBinaryAllanResultName();
        m_resultType     = RiaDefines::ResultCatType::ALLAN_DIAGRAMS;
    }

    m_porosityModelUiField  = m_porosityModel;
    m_resultTypeUiField     = m_resultType;
    m_resultVariableUiField = m_resultVariable;

    m_flowSolutionUiField            = m_flowSolution();
    m_selectedInjectorTracersUiField = m_selectedInjectorTracers;

    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setResultType( RiaDefines::ResultCatType val )
{
    m_resultType        = val;
    m_resultTypeUiField = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PorosityModelType RimEclipseResultDefinition::porosityModel() const
{
    return m_porosityModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setPorosityModel( RiaDefines::PorosityModelType val )
{
    m_porosityModel        = val;
    m_porosityModelUiField = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::resultVariable() const
{
    if ( !isChecked() ) return RiaResultNames::undefinedResultName();

    return m_resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setResultVariable( const QString& val )
{
    m_resultVariable        = val;
    m_resultVariableUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// Return phase type if the current result is known to be of a particular
/// fluid phase type. Otherwise the method will return PHASE_NOT_APPLICABLE.
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimEclipseResultDefinition::resultPhaseType() const
{
    if ( QRegularExpression( "OIL" ).match( m_resultVariable() ).hasMatch() )
    {
        return RiaDefines::PhaseType::OIL_PHASE;
    }
    else if ( QRegularExpression( "GAS" ).match( m_resultVariable() ).hasMatch() )
    {
        return RiaDefines::PhaseType::GAS_PHASE;
    }
    else if ( QRegularExpression( "WAT" ).match( m_resultVariable() ).hasMatch() )
    {
        return RiaDefines::PhaseType::WATER_PHASE;
    }
    return RiaDefines::PhaseType::PHASE_NOT_APPLICABLE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution* RimEclipseResultDefinition::flowDiagSolution() const
{
    return m_flowSolution();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setFlowSolution( RimFlowDiagSolution* flowSol )
{
    m_flowSolution        = flowSol;
    m_flowSolutionUiField = flowSol;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setSelectedTracers( const std::vector<QString>& selectedTracers )
{
    if ( m_flowSolution() == nullptr )
    {
        assignFlowSolutionFromCase();
    }
    if ( m_flowSolution() )
    {
        std::vector<QString> injectorTracers;
        std::vector<QString> producerTracers;
        for ( const QString& tracerName : selectedTracers )
        {
            RimFlowDiagSolution::TracerStatusType tracerStatus = m_flowSolution()->tracerStatusOverall( tracerName );
            if ( tracerStatus == RimFlowDiagSolution::TracerStatusType::INJECTOR )
            {
                injectorTracers.push_back( tracerName );
            }
            else if ( tracerStatus == RimFlowDiagSolution::TracerStatusType::PRODUCER )
            {
                producerTracers.push_back( tracerName );
            }
            else if ( tracerStatus == RimFlowDiagSolution::TracerStatusType::VARYING ||
                      tracerStatus == RimFlowDiagSolution::TracerStatusType::UNDEFINED )
            {
                injectorTracers.push_back( tracerName );
                producerTracers.push_back( tracerName );
            }
        }
        setSelectedInjectorTracers( injectorTracers );
        setSelectedProducerTracers( producerTracers );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setSelectedInjectorTracers( const std::vector<QString>& selectedTracers )
{
    m_selectedInjectorTracers        = selectedTracers;
    m_selectedInjectorTracersUiField = selectedTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setSelectedProducerTracers( const std::vector<QString>& selectedTracers )
{
    m_selectedProducerTracers        = selectedTracers;
    m_selectedProducerTracersUiField = selectedTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setSelectedSouringTracers( const std::vector<QString>& selectedTracers )
{
    m_selectedSouringTracers        = selectedTracers;
    m_selectedSouringTracersUiField = selectedTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::updateUiFieldsFromActiveResult()
{
    m_resultTypeUiField              = m_resultType;
    m_resultVariableUiField          = resultVariable();
    m_selectedInjectorTracersUiField = m_selectedInjectorTracers;
    m_selectedProducerTracersUiField = m_selectedProducerTracers;
    m_selectedSouringTracersUiField  = m_selectedSouringTracers;
    m_porosityModelUiField           = m_porosityModel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::enableDeltaResults( bool enable )
{
    m_isDeltaResultEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isTernarySaturationSelected() const
{
    bool isTernary = ( m_resultType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE ) &&
                     ( m_resultVariable().compare( RiaResultNames::ternarySaturationResultName(), Qt::CaseInsensitive ) == 0 );

    return isTernary;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isCompletionTypeSelected() const
{
    return ( m_resultType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE && m_resultVariable() == RiaResultNames::completionTypeResultName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasCategoryResult() const
{
    if ( auto* gridCellResults = currentGridCellResults() )
    {
        const auto addresses = gridCellResults->existingResults();
        for ( const auto& address : addresses )
        {
            if ( address.resultCatType() == m_resultType() && address.resultName() == m_resultVariable() )
            {
                if ( address.dataType() == RiaDefines::ResultDataType::INTEGER ) return true;
                break;
            }
        }
    }

    if ( m_resultType() == RiaDefines::ResultCatType::FORMATION_NAMES && m_eclipseCase && m_eclipseCase->eclipseCaseData() &&
         !m_eclipseCase->eclipseCaseData()->formationNames().empty() )
        return true;

    if ( m_resultType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE && resultVariable() == RiaResultNames::completionTypeResultName() )
        return true;

    if ( m_resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS && m_resultVariable() == RIG_FLD_MAX_FRACTION_TRACER_RESNAME )
        return true;

    if ( resultVariable() == RiaResultNames::formationAllanResultName() || resultVariable() == RiaResultNames::formationBinaryAllanResultName() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isFlowDiagOrInjectionFlooding() const
{
    return m_resultType() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS || m_resultType() == RiaDefines::ResultCatType::INJECTION_FLOODING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_resultTypeUiField );

    if ( hasDualPorFractureResult() )
    {
        uiOrdering.add( &m_porosityModelUiField );
    }

    if ( m_resultTypeUiField() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        uiOrdering.add( &m_flowSolutionUiField );

        uiOrdering.add( &m_flowTracerSelectionMode );

        if ( m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_BY_SELECTION )
        {
            caf::PdmUiGroup* selectionGroup = uiOrdering.addNewGroup( "Tracer Selection" );
            selectionGroup->setEnableFrame( false );

            caf::PdmUiGroup* injectorGroup = selectionGroup->addNewGroup( "Injectors" );
            injectorGroup->add( &m_selectedInjectorTracersUiField );
            injectorGroup->add( &m_syncInjectorToProducerSelection );

            caf::PdmUiGroup* producerGroup = selectionGroup->addNewGroup( "Producers", { .newRow = false } );
            producerGroup->add( &m_selectedProducerTracersUiField );
            producerGroup->add( &m_syncProducerToInjectorSelection );
        }

        uiOrdering.add( &m_phaseSelection );

        if ( m_flowSolution() == nullptr )
        {
            assignFlowSolutionFromCase();
        }
    }

    if ( m_resultTypeUiField() == RiaDefines::ResultCatType::INJECTION_FLOODING )
    {
        uiOrdering.add( &m_selectedSouringTracersUiField );
    }

    uiOrdering.add( &m_resultVariableUiField );
    if ( m_resultTypeUiField() == RiaDefines::ResultCatType::INPUT_PROPERTY )
    {
        uiOrdering.add( &m_inputPropertyFileName );
    }

    if ( isDivideByCellFaceAreaPossible() )
    {
        uiOrdering.add( &m_divideByCellFaceArea );

        QString resultPropertyLabel = "Result Property";
        if ( isDivideByCellFaceAreaActive() )
        {
            resultPropertyLabel += QString( "\nDivided by Area" );
        }
        m_resultVariableUiField.uiCapability()->setUiName( resultPropertyLabel );
    }

    {
        caf::PdmUiGroup* legendGroup = uiOrdering.addNewGroup( "Legend" );
        legendGroup->add( &m_showOnlyVisibleCategoriesInLegend );

        bool showOnlyVisibleCategoriesOption = false;

        RimEclipseView* eclView = firstAncestorOrThisOfType<RimEclipseView>();

        if ( eclView )
        {
            if ( eclView->cellResult() == this && hasCategoryResult() ) showOnlyVisibleCategoriesOption = true;
        }

        if ( m_resultTypeUiField() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS &&
             m_resultVariableUiField() == RIG_FLD_MAX_FRACTION_TRACER_RESNAME )
        {
            showOnlyVisibleCategoriesOption = true;
        }

        legendGroup->setUiHidden( !showOnlyVisibleCategoriesOption );
    }

    if ( isDeltaCasePossible() || isDeltaTimeStepPossible() )
    {
        caf::PdmUiGroup* differenceGroup = uiOrdering.addNewGroup( "Difference Options" );
        differenceGroup->setUiReadOnly( !( isDeltaTimeStepPossible() || isDeltaCasePossible() ) );

        m_differenceCase.uiCapability()->setUiReadOnly( !isDeltaCasePossible() );
        m_timeLapseBaseTimestep.uiCapability()->setUiReadOnly( !isDeltaTimeStepPossible() );

        if ( isDeltaCasePossible() ) differenceGroup->add( &m_differenceCase );
        if ( isDeltaTimeStepPossible() ) differenceGroup->add( &m_timeLapseBaseTimestep );

        QString resultPropertyLabel = "Result Property";
        if ( isDeltaTimeStepActive() || isDeltaCaseActive() )
        {
            resultPropertyLabel += QString( "\n%1" ).arg( additionalResultTextShort() );
        }
        m_resultVariableUiField.uiCapability()->setUiName( resultPropertyLabel );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_resultTypeUiField() == RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        if ( field == &m_syncInjectorToProducerSelection || field == &m_syncProducerToInjectorSelection )
        {
            caf::PdmUiToolButtonEditorAttribute* toolButtonAttr = dynamic_cast<caf::PdmUiToolButtonEditorAttribute*>( attribute );
            if ( toolButtonAttr )
            {
                toolButtonAttr->m_sizePolicy.setHorizontalPolicy( QSizePolicy::MinimumExpanding );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::assignFlowSolutionFromCase()
{
    RimFlowDiagSolution* defaultFlowDiagSolution = nullptr;

    RimEclipseResultCase* eclCase = dynamic_cast<RimEclipseResultCase*>( m_eclipseCase.p() );

    if ( eclCase )
    {
        defaultFlowDiagSolution = eclCase->defaultFlowDiagSolution();
    }
    setFlowSolution( defaultFlowDiagSolution );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::hasDualPorFractureResult()
{
    if ( m_eclipseCase && m_eclipseCase->eclipseCaseData() )
    {
        return m_eclipseCase->eclipseCaseData()->hasFractureResults();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinition::flowDiagResUiText( bool shortLabel, int maxTracerStringLength ) const
{
    QString uiText = QString::fromStdString( flowDiagResAddress().variableName );
    if ( flowDiagResAddress().variableName == RIG_FLD_TOF_RESNAME )
    {
        uiText = RimEclipseResultDefinitionTools::timeOfFlightString( injectorSelectionState(), producerSelectionState(), shortLabel );
    }
    else if ( flowDiagResAddress().variableName == RIG_FLD_MAX_FRACTION_TRACER_RESNAME )
    {
        uiText = RimEclipseResultDefinitionTools::maxFractionTracerString( injectorSelectionState(), producerSelectionState(), shortLabel );
    }

    QString tracersString = RimEclipseResultDefinitionTools::selectedTracersString( injectorSelectionState(),
                                                                                    producerSelectionState(),
                                                                                    m_selectedInjectorTracers(),
                                                                                    m_selectedProducerTracers(),
                                                                                    maxTracerStringLength );

    if ( !tracersString.isEmpty() )
    {
        uiText += QString( "\n%1" ).arg( tracersString );
    }
    return uiText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard( RiaDefines::ResultCatType resultCatType,
                                                                                                 const RigCaseCellResultsData* results,
                                                                                                 bool showDerivedResultsFirst,
                                                                                                 bool addPerCellFaceOptionItems,
                                                                                                 bool ternaryEnabled )
{
    CVF_ASSERT( resultCatType != RiaDefines::ResultCatType::FLOW_DIAGNOSTICS && resultCatType != RiaDefines::ResultCatType::INJECTION_FLOODING );

    return RimEclipseResultDefinitionTools::calcOptionsForVariableUiFieldStandard( resultCatType,
                                                                                   results,
                                                                                   showDerivedResultsFirst,
                                                                                   addPerCellFaceOptionItems,
                                                                                   ternaryEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::setTernaryEnabled( bool enabled )
{
    m_ternaryEnabled = enabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::updateRangesForExplicitLegends( RimRegularLegendConfig* legendConfigToUpdate,
                                                                 RimTernaryLegendConfig* ternaryLegendConfigToUpdate,
                                                                 int                     currentTimeStep )

{
    if ( hasResult() )
    {
        if ( isFlowDiagOrInjectionFlooding() )
        {
            if ( currentTimeStep >= 0 )
            {
                RimEclipseResultDefinitionTools::updateLegendForFlowDiagnostics( this, legendConfigToUpdate, currentTimeStep );
            }
        }
        else
        {
            RimEclipseResultDefinitionTools::updateCellResultLegend( this, legendConfigToUpdate, currentTimeStep );
        }
    }

    if ( isTernarySaturationSelected() )
    {
        RimEclipseResultDefinitionTools::updateTernaryLegend( currentGridCellResults(), ternaryLegendConfigToUpdate, currentTimeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::updateLegendTitle( RimRegularLegendConfig* legendConfig, const QString& legendHeading )
{
    QString title = legendHeading + resultVariableUiName();
    if ( !additionalResultTextShort().isEmpty() )
    {
        title += additionalResultTextShort();
    }

    if ( hasDualPorFractureResult() )
    {
        QString porosityModelText = caf::AppEnum<RiaDefines::PorosityModelType>::uiText( porosityModel() );

        title += QString( "\nDual Por : %1" ).arg( porosityModelText );
    }

    legendConfig->setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::showOnlyVisibleCategoriesInLegend() const
{
    return m_showOnlyVisibleCategoriesInLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::FlowTracerSelectionState RimEclipseResultDefinition::injectorSelectionState() const
{
    const bool isInjector = true;
    return RimEclipseResultDefinitionTools::getFlowTracerSelectionState( isInjector,
                                                                         m_flowTracerSelectionMode(),
                                                                         m_flowSolutionUiField(),
                                                                         m_selectedInjectorTracers().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::FlowTracerSelectionState RimEclipseResultDefinition::producerSelectionState() const
{
    const bool isInjector = false;
    return RimEclipseResultDefinitionTools::getFlowTracerSelectionState( isInjector,
                                                                         m_flowTracerSelectionMode(),
                                                                         m_flowSolutionUiField(),
                                                                         m_selectedProducerTracers().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::syncInjectorToProducerSelection()
{
    int timeStep = 0;

    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        timeStep = rimView->currentTimeStep();
    }

    RimFlowDiagSolution* flowSol = m_flowSolution();
    if ( flowSol && m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_BY_SELECTION )
    {
        std::set<QString, RimFlowDiagnosticsTools::TracerComp> newProducerSelection =
            RimFlowDiagnosticsTools::setOfProducerTracersFromInjectors( m_flowSolutionUiField(), m_selectedInjectorTracers(), timeStep );
        // Add all currently selected producers to set
        for ( const QString& selectedProducer : m_selectedProducerTracers() )
        {
            newProducerSelection.insert( selectedProducer );
        }
        std::vector<QString> newProducerVector( newProducerSelection.begin(), newProducerSelection.end() );
        setSelectedProducerTracers( newProducerVector );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultDefinition::syncProducerToInjectorSelection()
{
    int timeStep = 0;

    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        timeStep = rimView->currentTimeStep();
    }

    RimFlowDiagSolution* flowSol = m_flowSolution();
    if ( flowSol && m_flowTracerSelectionMode == FlowTracerSelectionType::FLOW_TR_BY_SELECTION )
    {
        std::set<QString, RimFlowDiagnosticsTools::TracerComp> newInjectorSelection =
            RimFlowDiagnosticsTools::setOfInjectorTracersFromProducers( m_flowSolutionUiField(), m_selectedProducerTracers(), timeStep );

        // Add all currently selected injectors to set
        for ( const QString& selectedInjector : m_selectedInjectorTracers() )
        {
            newInjectorSelection.insert( selectedInjector );
        }
        std::vector<QString> newInjectorVector( newInjectorSelection.begin(), newInjectorSelection.end() );
        setSelectedInjectorTracers( newInjectorVector );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDeltaResultEnabled() const
{
    return m_isDeltaResultEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDeltaTimeStepPossible() const
{
    return isDeltaResultEnabled() && !isTernarySaturationSelected() &&
           ( m_resultTypeUiField() == RiaDefines::ResultCatType::DYNAMIC_NATIVE ||
             m_resultTypeUiField() == RiaDefines::ResultCatType::GENERATED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDeltaTimeStepActive() const
{
    return isDeltaTimeStepPossible() && m_timeLapseBaseTimestep() >= 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDeltaCasePossible() const
{
    return isDeltaResultEnabled() && !isTernarySaturationSelected() &&
           ( m_resultTypeUiField() == RiaDefines::ResultCatType::DYNAMIC_NATIVE ||
             m_resultTypeUiField() == RiaDefines::ResultCatType::STATIC_NATIVE ||
             m_resultTypeUiField() == RiaDefines::ResultCatType::GENERATED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDeltaCaseActive() const
{
    return isDeltaCasePossible() && m_differenceCase() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDivideByCellFaceAreaPossible() const
{
    return RimEclipseResultDefinitionTools::isDivideByCellFaceAreaPossible( m_resultVariable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::isDivideByCellFaceAreaActive() const
{
    return isDivideByCellFaceAreaPossible() && m_divideByCellFaceArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::showDerivedResultsFirstInVariableUiField() const
{
    // Cell Face result names
    bool                   showDerivedResultsFirstInList = false;
    RimEclipseFaultColors* rimEclipseFaultColors         = firstAncestorOrThisOfType<RimEclipseFaultColors>();

    if ( rimEclipseFaultColors ) showDerivedResultsFirstInList = true;

    return showDerivedResultsFirstInList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinition::addPerCellFaceOptionsForVariableUiField() const
{
    RimPlotCurve*             curve      = firstAncestorOrThisOfType<RimPlotCurve>();
    RimEclipsePropertyFilter* propFilter = firstAncestorOrThisOfType<RimEclipsePropertyFilter>();
    RimCellEdgeColors*        cellEdge   = firstAncestorOrThisOfType<RimCellEdgeColors>();

    return !( propFilter || curve || cellEdge );
}
