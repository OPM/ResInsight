/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025   Equinor ASA
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

#include "RicExportSectorModelUi.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultTools.h"
#include "RigMainGrid.h"
#include "RigModelPaddingSettings.h"

#include "Jobs/RimKeywordBcprop.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimTools.h"
#include "Tools/RimEclipseViewTools.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiRadioButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"

#include <QFileInfo>

#include <utility>

namespace caf
{
template <>
void AppEnum<RicExportSectorModelUi::RefinementMode>::setUp()
{
    addItem( RicExportSectorModelUi::UNIFORM, "UNIFORM", "Uniform" );
    addItem( RicExportSectorModelUi::NON_UNIFORM, "NON_UNIFORM", "Non-Uniform" );
    setDefault( RicExportSectorModelUi::UNIFORM );
}
template <>
void AppEnum<RicExportSectorModelUi::NonUniformSubMode>::setUp()
{
    addItem( RicExportSectorModelUi::CUSTOM_WIDTHS, "CUSTOM_WIDTHS", "Custom Widths" );
    addItem( RicExportSectorModelUi::LINEAR_EQUAL_SPLIT, "LINEAR_EQUAL_SPLIT", "Linear (Equal Split)" );
    addItem( RicExportSectorModelUi::LOGARITHMIC_CENTER, "LOGARITHMIC_CENTER", "Logarithmic (Towards Center)" );
    setDefault( RicExportSectorModelUi::CUSTOM_WIDTHS );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RicExportSectorModelUi, "RicExportSectorModelUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportSectorModelUi::RicExportSectorModelUi()
    : m_visibleMin( caf::VecIjk0::ZERO )
    , m_visibleMax( caf::VecIjk0::ZERO )
    , m_totalCells( 0 )
{
    CAF_PDM_InitObject( "Export Sector Model for Simulation Input" );

    CAF_PDM_InitFieldNoDefault( &m_exportFolder, "ExportFolder", "Export Folder" );
    CAF_PDM_InitFieldNoDefault( &m_exportDeckName, "ExportDeckName", "Sector Model Name" );
    CAF_PDM_InitFieldNoDefault( &m_inputDeckName, "InputDeckName", "Source DATA File" );
    m_inputDeckName.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_porvMultiplier, "PorvMultiplier", 1.0e6, "PORV Multiplier" );
    CAF_PDM_InitFieldNoDefault( &m_boundaryCondition, "BoundaryCondition", "Boundary Condition Type:" );
    m_boundaryCondition.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_gridBoxSelection, "GridBoxSelection", "Cells to Export:" );
    m_gridBoxSelection.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_visibleWellsPadding, "VisibleWellsPadding", 2, "Well Padding", "", "Number of cells to add around visible wells", "" );
    m_visibleWellsPadding.setRange( 0, 100 );

    CAF_PDM_InitField( &m_minI, "MinI", 1, "Min I, J, K" );
    CAF_PDM_InitField( &m_minJ, "MinJ", 1, "" );
    m_minJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    CAF_PDM_InitField( &m_minK, "MinK", 1, "" );
    m_minK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    m_minI.setMinValue( 1 );
    m_minJ.setMinValue( 1 );
    m_minK.setMinValue( 1 );

    CAF_PDM_InitField( &m_maxI, "MaxI", 1, "Max I, J, K" );
    CAF_PDM_InitField( &m_maxJ, "MaxJ", 1, "" );
    m_maxJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );
    CAF_PDM_InitField( &m_maxK, "MaxK", 1, "" );
    m_maxK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );

    m_maxI.setMinValue( 1 );
    m_maxJ.setMinValue( 1 );
    m_maxK.setMinValue( 1 );

    CAF_PDM_InitField( &m_refineGrid, "RefineGrid", false, "Enable Grid Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_refineGrid );
    CAF_PDM_InitField( &m_refinementCountI, "RefinementCountI", 1, "Cell Count I, J, K" );
    CAF_PDM_InitField( &m_refinementCountJ, "RefinementCountJ", 1, "" );
    CAF_PDM_InitField( &m_refinementCountK, "RefinementCountK", 1, "" );

    m_refinementCountI.setRange( 1, 10 );
    m_refinementCountJ.setRange( 1, 10 );
    m_refinementCountK.setRange( 1, 10 );

    CAF_PDM_InitFieldNoDefault( &m_refinementMode, "RefinementMode", "Refinement Mode" );
    m_refinementMode.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_nonUniformEnableI, "NonUniformEnableI", false, "Enable I Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_nonUniformEnableI );
    CAF_PDM_InitField( &m_nonUniformRangeStartI, "NonUniformRangeStartI", 1, "Grid Cell Start" );
    m_nonUniformRangeStartI.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformRangeEndI, "NonUniformRangeEndI", 1, "Grid Cell End" );
    m_nonUniformRangeEndI.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformIntervalsI, "NonUniformIntervalsI", QString( "0.5, 0.5" ), "Fractional Widths" );

    CAF_PDM_InitField( &m_nonUniformEnableJ, "NonUniformEnableJ", false, "Enable J Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_nonUniformEnableJ );
    CAF_PDM_InitField( &m_nonUniformRangeStartJ, "NonUniformRangeStartJ", 1, "Grid Cell Start" );
    m_nonUniformRangeStartJ.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformRangeEndJ, "NonUniformRangeEndJ", 1, "Grid Cell End" );
    m_nonUniformRangeEndJ.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformIntervalsJ, "NonUniformIntervalsJ", QString( "0.5, 0.5" ), "Fractional Widths" );

    CAF_PDM_InitField( &m_nonUniformEnableK, "NonUniformEnableK", false, "Enable K Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_nonUniformEnableK );
    CAF_PDM_InitField( &m_nonUniformRangeStartK, "NonUniformRangeStartK", 1, "Grid Cell Start" );
    m_nonUniformRangeStartK.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformRangeEndK, "NonUniformRangeEndK", 1, "Grid Cell End" );
    m_nonUniformRangeEndK.setMinValue( 1 );
    CAF_PDM_InitField( &m_nonUniformIntervalsK, "NonUniformIntervalsK", QString( "0.5, 0.5" ), "Fractional Widths" );

    CAF_PDM_InitFieldNoDefault( &m_nonUniformSubMode, "NonUniformSubMode", "Non-Uniform Mode" );

    CAF_PDM_InitField( &m_nonUniformSubcellCountI, "NonUniformSubcellCountI", 2, "Subcells per Cell" );
    m_nonUniformSubcellCountI.setRange( 2, 100 );
    CAF_PDM_InitField( &m_nonUniformSubcellCountJ, "NonUniformSubcellCountJ", 2, "Subcells per Cell" );
    m_nonUniformSubcellCountJ.setRange( 2, 100 );
    CAF_PDM_InitField( &m_nonUniformSubcellCountK, "NonUniformSubcellCountK", 2, "Subcells per Cell" );
    m_nonUniformSubcellCountK.setRange( 2, 100 );

    CAF_PDM_InitField( &m_nonUniformTotalCellsI, "NonUniformTotalCellsI", 10, "Total Cells" );
    m_nonUniformTotalCellsI.setRange( 2, 1000 );
    CAF_PDM_InitField( &m_nonUniformTotalCellsJ, "NonUniformTotalCellsJ", 10, "Total Cells" );
    m_nonUniformTotalCellsJ.setRange( 2, 1000 );
    CAF_PDM_InitField( &m_nonUniformTotalCellsK, "NonUniformTotalCellsK", 10, "Total Cells" );
    m_nonUniformTotalCellsK.setRange( 2, 1000 );

    CAF_PDM_InitFieldNoDefault( &m_bcpropKeywords, "BcpropKeywords", "BCPROP Keywords" );
    m_bcpropKeywords.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_bcpropKeywords.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_keywordsToRemove, "KeywordsToRemove", "Keywords to Remove from Output" );
    m_keywordsToRemove.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    setDefaultKeywordsToRemove();

    // Model padding fields
    CAF_PDM_InitField( &m_enablePadding, "EnablePadding", false, "Enable Model Padding" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_enablePadding );

    CAF_PDM_InitField( &m_paddingNzUpper, "PaddingNzUpper", 0, "Number of Z Layers" );
    m_paddingNzUpper.setRange( 0, 100 );

    CAF_PDM_InitField( &m_paddingTopUpper, "PaddingTopUpper", 0.0, "Top Depth" );

    CAF_PDM_InitField( &m_paddingUpperPorosity, "PaddingUpperPorosity", 0.0, "Porosity" );
    m_paddingUpperPorosity.setRange( 0.0, 1.0 );

    CAF_PDM_InitField( &m_paddingUpperEquilnum, "PaddingUpperEquilnum", 1, "EQUILNUM" );
    m_paddingUpperEquilnum.setMinValue( 1 );

    CAF_PDM_InitField( &m_paddingNzLower, "PaddingNzLower", 0, "Number of Z Layers" );
    m_paddingNzLower.setRange( 0, 100 );

    CAF_PDM_InitField( &m_paddingBottomLower, "PaddingBottomLower", 0.0, "Bottom Depth" );

    CAF_PDM_InitField( &m_paddingMinThickness, "PaddingMinThickness", 0.1, "Minimum Layer Thickness" );
    m_paddingMinThickness.setMinValue( 0.001 );

    CAF_PDM_InitField( &m_paddingFillGaps, "PaddingFillGaps", false, "Fill Gaps in Z Direction" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_paddingFillGaps );

    CAF_PDM_InitField( &m_paddingMonotonicZcorn, "PaddingMonotonicZcorn", false, "Enforce Monotonic Z-Corners" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_paddingMonotonicZcorn );

    CAF_PDM_InitField( &m_paddingVerticalPillars, "PaddingVerticalPillars", false, "Make Pillars Vertical" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_paddingVerticalPillars );

    CAF_PDM_InitField( &m_createSimulationJob, "CreateSimulationJob", false, "Create New Simulation Job" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createSimulationJob );
    CAF_PDM_InitFieldNoDefault( &m_simulationJobFolder, "SimulationJobFolder", "Working Folder" );
    CAF_PDM_InitFieldNoDefault( &m_simulationJobName, "SimulationJobName", "Job Name" );
    CAF_PDM_InitField( &m_startSimulationJobAfterExport,
                       "StartSimulationJobAfterExport",
                       false,
                       "Start Simulation Job After Export",
                       "",
                       "Automatically start all jobs using the exported sector model file as input." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_startSimulationJobAfterExport );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Source Eclipse Case" );
    m_eclipseCase.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitFieldNoDefault( &m_eclipseView, "EclipseView", "Source View" );
    m_eclipseView.uiCapability()->setUiHidden( true );

    m_exportFolder = defaultFolder();

    m_pageNames << "General Settings";
    m_pageSubtitles << "Select the name (no extension) and the output folder of the new sector model.";

    m_pageNames << "Sector Model Definition";
    m_pageSubtitles << "Select grid box to export as a new sector model.";

    m_pageNames << "Refinement";
    m_pageSubtitles << "Set up optional sector model grid refinement.";

    m_pageNames << "Boundary Conditions";
    m_pageSubtitles << "Set up the boundary conditions of the new sector model.";

    m_pageNames << "Keyword Adjustments";
    m_pageSubtitles << "Set up special handling of certain keywords.";

    m_pageNames << "Model Padding";
    m_pageSubtitles << "Configure optional Z-direction padding for the sector model grid.";

    m_pageNames << "Simulation Job Settings";
    m_pageSubtitles << "Optionally create and/or run a simulation job using the exported sector model.";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportSectorModelUi::~RicExportSectorModelUi()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // must be kept in sync with the page names defined in the constructor
    CAF_ASSERT( m_pageNames.size() == (size_t)WizardPageEnum::TotalPages );

    if ( uiConfigName == m_pageNames[WizardPageEnum::ExportSettings] )
    {
        uiOrdering.add( &m_exportDeckName );
        uiOrdering.add( &m_exportFolder );

        auto infoGrp = uiOrdering.addNewGroup( "Source Information" );
        infoGrp->addNewLabel( QString( "Source Folder: " ) + m_eclipseCase->locationOnDisc() );
        infoGrp->addNewLabel( QString( "Source Case Name: " ) + m_eclipseCase->caseUserDescription() );
        infoGrp->add( &m_inputDeckName );
    }
    else if ( uiConfigName == m_pageNames[WizardPageEnum::GridBoxSelection] )
    {
        uiOrdering.add( &m_gridBoxSelection, { .newRow = true, .totalColumnSpan = 4, .leftLabelColumnSpan = 1 } );

        uiOrdering.addNewLabel( "" );

        uiOrdering.add( &m_minI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        uiOrdering.appendToRow( &m_minJ );
        uiOrdering.appendToRow( &m_minK );

        uiOrdering.add( &m_maxI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        uiOrdering.appendToRow( &m_maxJ );
        uiOrdering.appendToRow( &m_maxK );

        if ( m_gridBoxSelection() == RiaModelExportDefines::VISIBLE_WELLS_BOX )
        {
            uiOrdering.add( &m_visibleWellsPadding, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
            uiOrdering.addNewLabel( "(cells)", { .newRow = false } );
        }

        const bool boxReadOnly = ( m_gridBoxSelection() != RiaModelExportDefines::MANUAL_SELECTION );
        m_minI.uiCapability()->setUiReadOnly( boxReadOnly );
        m_minJ.uiCapability()->setUiReadOnly( boxReadOnly );
        m_minK.uiCapability()->setUiReadOnly( boxReadOnly );
        m_maxI.uiCapability()->setUiReadOnly( boxReadOnly );
        m_maxJ.uiCapability()->setUiReadOnly( boxReadOnly );
        m_maxK.uiCapability()->setUiReadOnly( boxReadOnly );

        uiOrdering.addNewLabel( "" );
        uiOrdering.addNewLabel( QString( "Total cells to export: %1" ).arg( m_totalCells ) );
    }
    else if ( uiConfigName == m_pageNames[WizardPageEnum::GridRefinement] )
    {
        uiOrdering.add( &m_refineGrid );
        uiOrdering.addNewLabel( "" );

        uiOrdering.add( &m_refinementMode );
        uiOrdering.addNewLabel( "" );

        bool isEnabled = m_refineGrid();

        if ( m_refinementMode() == UNIFORM )
        {
            uiOrdering.add( &m_refinementCountI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
            uiOrdering.appendToRow( &m_refinementCountJ );
            uiOrdering.appendToRow( &m_refinementCountK );

            m_refinementCountI.uiCapability()->setUiReadOnly( !isEnabled );
            m_refinementCountJ.uiCapability()->setUiReadOnly( !isEnabled );
            m_refinementCountK.uiCapability()->setUiReadOnly( !isEnabled );
        }
        else
        {
            uiOrdering.add( &m_nonUniformSubMode );
            m_nonUniformSubMode.uiCapability()->setUiReadOnly( !isEnabled );
            uiOrdering.addNewLabel( "" );

            auto subMode = m_nonUniformSubMode();

            auto addDimensionGroup = [&]( const QString&          label,
                                          caf::PdmField<bool>&    enableField,
                                          caf::PdmField<int>&     rangeStartField,
                                          caf::PdmField<int>&     rangeEndField,
                                          caf::PdmField<QString>& intervalsField,
                                          caf::PdmField<int>&     subcellCountField,
                                          caf::PdmField<int>&     totalCellsField )
            {
                auto* grp = uiOrdering.addNewGroup( label );
                grp->setCollapsedByDefault();
                grp->add( &enableField );
                grp->add( &rangeStartField );
                grp->add( &rangeEndField );

                if ( subMode == CUSTOM_WIDTHS )
                {
                    grp->add( &intervalsField );
                }
                else if ( subMode == LINEAR_EQUAL_SPLIT )
                {
                    grp->add( &subcellCountField );
                }
                else if ( subMode == LOGARITHMIC_CENTER )
                {
                    grp->add( &totalCellsField );
                }

                bool dimEnabled = isEnabled && enableField();
                enableField.uiCapability()->setUiReadOnly( !isEnabled );
                rangeStartField.uiCapability()->setUiReadOnly( !dimEnabled );
                rangeEndField.uiCapability()->setUiReadOnly( !dimEnabled );
                intervalsField.uiCapability()->setUiReadOnly( !dimEnabled );
                subcellCountField.uiCapability()->setUiReadOnly( !dimEnabled );
                totalCellsField.uiCapability()->setUiReadOnly( !dimEnabled );
            };

            addDimensionGroup( "I Direction",
                               m_nonUniformEnableI,
                               m_nonUniformRangeStartI,
                               m_nonUniformRangeEndI,
                               m_nonUniformIntervalsI,
                               m_nonUniformSubcellCountI,
                               m_nonUniformTotalCellsI );
            addDimensionGroup( "J Direction",
                               m_nonUniformEnableJ,
                               m_nonUniformRangeStartJ,
                               m_nonUniformRangeEndJ,
                               m_nonUniformIntervalsJ,
                               m_nonUniformSubcellCountJ,
                               m_nonUniformTotalCellsJ );
            addDimensionGroup( "K Direction",
                               m_nonUniformEnableK,
                               m_nonUniformRangeStartK,
                               m_nonUniformRangeEndK,
                               m_nonUniformIntervalsK,
                               m_nonUniformSubcellCountK,
                               m_nonUniformTotalCellsK );
        }

        m_refinementMode.uiCapability()->setUiReadOnly( !isEnabled );
    }
    else if ( uiConfigName == m_pageNames[WizardPageEnum::BoundaryConditions] )
    {
        uiOrdering.add( &m_boundaryCondition );
        uiOrdering.addNewLabel( "", { .newRow = false } ); // needed to get proper visual layout in BCCON/BCPROP case

        if ( m_boundaryCondition() == RiaModelExportDefines::OPERNUM_OPERATER )
        {
            uiOrdering.add( &m_porvMultiplier );
            uiOrdering.addNewLabel( "", { .newRow = false } ); // needed to get proper visual layout
        }
        else if ( m_boundaryCondition() == RiaModelExportDefines::BCCON_BCPROP )
        {
            uiOrdering.add( &m_bcpropKeywords );
        }
    }
    else if ( uiConfigName == m_pageNames[WizardPageEnum::KeywordAdjustments] )
    {
        uiOrdering.add( &m_keywordsToRemove );
        uiOrdering.addNewButton( "Reset to Default",
                                 [this]()
                                 {
                                     setDefaultKeywordsToRemove();
                                     updateConnectedEditors();
                                 } );
    }
    else if ( uiConfigName == m_pageNames[WizardPageEnum::ModelPadding] )
    {
        uiOrdering.add( &m_enablePadding );
        uiOrdering.addNewLabel( "" );

        if ( m_enablePadding() )
        {
            auto upperGrp = uiOrdering.addNewGroup( "Upper Padding" );
            upperGrp->add( &m_paddingNzUpper );
            upperGrp->add( &m_paddingTopUpper );
            upperGrp->add( &m_paddingUpperPorosity );
            upperGrp->add( &m_paddingUpperEquilnum );

            auto lowerGrp = uiOrdering.addNewGroup( "Lower Padding" );
            lowerGrp->add( &m_paddingNzLower );
            lowerGrp->add( &m_paddingBottomLower );

            auto optionsGrp = uiOrdering.addNewGroup( "Grid Options" );
            optionsGrp->add( &m_paddingMinThickness );
            optionsGrp->add( &m_paddingFillGaps );
            optionsGrp->add( &m_paddingMonotonicZcorn );
            optionsGrp->add( &m_paddingVerticalPillars );
        }
    }
    else if ( uiConfigName == m_pageNames[WizardPageEnum::SimulationJob] )
    {
        auto simGrp = uiOrdering.addNewGroup( "OPM Flow Simulation" );
        simGrp->add( &m_createSimulationJob );
        if ( m_createSimulationJob() )
        {
            if ( m_simulationJobName().isEmpty() ) m_simulationJobName = m_exportDeckName().trimmed();
            simGrp->add( &m_simulationJobName );
            simGrp->add( &m_simulationJobFolder );
        }
        uiOrdering.add( &m_startSimulationJobAfterExport );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RicExportSectorModelUi::pageNames() const
{
    return m_pageNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RicExportSectorModelUi::pageSubTitles() const
{
    return m_pageSubtitles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::setEclipseView( RimEclipseView* view )
{
    m_eclipseView = view;

    if ( view->eclipseCase() != m_eclipseCase() ) m_exportDeckName = "";

    m_eclipseCase = view->eclipseCase();

    cvf::UByteArray cellVisibility;
    view->calculateCurrentTotalCellVisibility( &cellVisibility, view->currentTimeStep() );

    const auto& [min, max] = RimEclipseViewTools::getVisibleCellRange( view, cellVisibility );
    m_visibleMax           = max;
    m_visibleMin           = min;

    applyBoundaryDefaults();

    // Initialize BCPROP keywords based on max BCCON value in the grid
    int maxBccon = RigEclipseResultTools::findMaxBcconValue( m_eclipseCase() );

    // Clear existing keywords
    m_bcpropKeywords.deleteChildren();

    // Add appropriate number of BCPROP keywords (max BCCON value or default to 6 for the 6 faces)
    int numKeywords = ( maxBccon > 0 ) ? maxBccon : 6;
    for ( int i = 0; i < numKeywords; ++i )
    {
        auto* keyword = new RimKeywordBcprop();
        keyword->setIndex( i + 1 ); // BCCON values start from 1, not 0
        m_bcpropKeywords.push_back( keyword );
    }

    if ( ( m_eclipseCase != nullptr ) && m_exportDeckName().isEmpty() )
        m_exportDeckName = m_eclipseCase->caseUserDescription().toUpper() + +"_SECTOR";

    m_createSimulationJob = false;

    if ( m_exportFolder().path().isEmpty() ) m_exportFolder = defaultFolder();

    // Get default input deck file name from eclipse case
    QFileInfo fi( view->eclipseCase()->gridFileName() );
    m_inputDeckName = fi.absolutePath() + "/" + fi.completeBaseName() + ".DATA";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_bcpropKeywords )
    {
        auto* tvAttr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttr )
        {
            tvAttr->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttr->alwaysEnforceResizePolicy = true;
        }
    }
    else if ( ( field == &m_exportFolder ) || ( field == &m_simulationJobFolder ) )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
    else if ( field == &m_keywordsToRemove )
    {
        auto attrib = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->heightHint = 200;
        }
    }
    else if ( field == &m_inputDeckName )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_fileSelectionFilter = "Eclipse DATA files (*.DATA);;All files (*.*)";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::setDefaultKeywordsToRemove()
{
    m_keywordsToRemove.setValue( { "ACTION", "ACTIONX", "ACTIONW", "BOX", "UDQ" } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RicExportSectorModelUi::min() const
{
    return caf::VecIjk0( m_minI() - 1, m_minJ() - 1, m_minK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RicExportSectorModelUi::max() const
{
    return caf::VecIjk0( m_maxI() - 1, m_maxJ() - 1, m_maxK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::setMin( const caf::VecIjk0& min )
{
    m_minI = static_cast<int>( min.x() ) + 1;
    m_minJ = static_cast<int>( min.y() ) + 1;
    m_minK = static_cast<int>( min.z() ) + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::setMax( const caf::VecIjk0& max )
{
    m_maxI = static_cast<int>( max.x() ) + 1;
    m_maxJ = static_cast<int>( max.y() ) + 1;
    m_maxK = static_cast<int>( max.z() ) + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicExportSectorModelUi::refinement() const
{
    if ( !m_refineGrid() || m_refinementMode() == NON_UNIFORM )
    {
        return cvf::Vec3st( 1, 1, 1 );
    }
    return cvf::Vec3st( m_refinementCountI(), m_refinementCountJ(), m_refinementCountK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportSectorModelUi::RefinementMode RicExportSectorModelUi::refinementMode() const
{
    if ( !m_refineGrid() ) return UNIFORM;
    return m_refinementMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNonUniformRefinement RicExportSectorModelUi::nonUniformRefinement() const
{
    cvf::Vec3st sectorSize( max().x() - min().x() + 1, max().y() - min().y() + 1, max().z() - min().z() + 1 );

    RigNonUniformRefinement result( sectorSize );

    if ( !m_refineGrid() || m_refinementMode() != NON_UNIFORM ) return result;

    struct DimensionConfig
    {
        bool                               enabled;
        int                                rangeStart;
        int                                rangeEnd;
        QString                            intervals;
        int                                subcellCount;
        int                                totalCells;
        RigNonUniformRefinement::Dimension dim;
    };

    std::vector<DimensionConfig> dims = {
        { m_nonUniformEnableI(),
          m_nonUniformRangeStartI(),
          m_nonUniformRangeEndI(),
          m_nonUniformIntervalsI(),
          m_nonUniformSubcellCountI(),
          m_nonUniformTotalCellsI(),
          RigNonUniformRefinement::DimI },
        { m_nonUniformEnableJ(),
          m_nonUniformRangeStartJ(),
          m_nonUniformRangeEndJ(),
          m_nonUniformIntervalsJ(),
          m_nonUniformSubcellCountJ(),
          m_nonUniformTotalCellsJ(),
          RigNonUniformRefinement::DimJ },
        { m_nonUniformEnableK(),
          m_nonUniformRangeStartK(),
          m_nonUniformRangeEndK(),
          m_nonUniformIntervalsK(),
          m_nonUniformSubcellCountK(),
          m_nonUniformTotalCellsK(),
          RigNonUniformRefinement::DimK },
    };

    // Sector min indices (1-based) for converting original grid coordinates to sector-relative
    int sectorMin[3] = { m_minI(), m_minJ(), m_minK() };

    const QString dimLabels[3] = { "I", "J", "K" };

    auto subMode = m_nonUniformSubMode();

    RiaLogging::info( QString( "Non-uniform refinement: sector size [%1, %2, %3], sector min [%4, %5, %6]" )
                          .arg( sectorSize.x() )
                          .arg( sectorSize.y() )
                          .arg( sectorSize.z() )
                          .arg( sectorMin[0] )
                          .arg( sectorMin[1] )
                          .arg( sectorMin[2] ) );

    for ( const auto& dc : dims )
    {
        if ( !dc.enabled ) continue;

        // Convert from original grid coordinates (1-based) to sector-relative (0-based)
        int sectorStart = dc.rangeStart - sectorMin[static_cast<size_t>( dc.dim )];
        int sectorEnd   = dc.rangeEnd - sectorMin[static_cast<size_t>( dc.dim )];

        // Clamp to sector bounds
        int sectorMaxIdx = static_cast<int>( result.sectorSize( dc.dim ) ) - 1;
        sectorStart      = std::max( 0, sectorStart );
        sectorEnd        = std::min( sectorMaxIdx, sectorEnd );
        if ( sectorStart > sectorEnd )
        {
            RiaLogging::warning( QString( "Non-uniform refinement %1: range [%2, %3] is outside sector after clamping, skipping" )
                                     .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                     .arg( sectorStart )
                                     .arg( sectorEnd ) );
            continue;
        }

        if ( subMode == CUSTOM_WIDTHS )
        {
            auto widths = parseWidths( dc.intervals );
            if ( widths.empty() )
            {
                RiaLogging::warning( QString( "Non-uniform refinement %1: no valid widths parsed from '%2'" )
                                         .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                         .arg( dc.intervals ) );
                continue;
            }

            RiaLogging::info(
                QString( "Non-uniform refinement %1: grid range [%2, %3] -> sector range [%4, %5] (sector size %6, %7 widths)" )
                    .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                    .arg( dc.rangeStart )
                    .arg( dc.rangeEnd )
                    .arg( sectorStart )
                    .arg( sectorEnd )
                    .arg( result.sectorSize( dc.dim ) )
                    .arg( widths.size() ) );

            result.distributeWidthsAcrossCells( dc.dim, sectorStart, sectorEnd, widths );
        }
        else if ( subMode == LINEAR_EQUAL_SPLIT )
        {
            // Each cell in the range gets exactly N equal subcells
            auto equalFractions = RigNonUniformRefinement::generateEqualFractions( static_cast<size_t>( dc.subcellCount ) );

            for ( int c = sectorStart; c <= sectorEnd; ++c )
            {
                result.setCumulativeFractions( dc.dim, static_cast<size_t>( c ), equalFractions );
            }

            RiaLogging::info( QString( "Non-uniform refinement %1 (linear): %2 subcells per cell in range [%3, %4]" )
                                  .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                  .arg( dc.subcellCount )
                                  .arg( sectorStart )
                                  .arg( sectorEnd ) );
        }
        else if ( subMode == LOGARITHMIC_CENTER )
        {
            auto widths = RigNonUniformRefinement::generateLogarithmicWidths( static_cast<size_t>( dc.totalCells ) );

            RiaLogging::info( QString( "Non-uniform refinement %1 (logarithmic): %2 total cells across range [%3, %4]" )
                                  .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                                  .arg( dc.totalCells )
                                  .arg( sectorStart )
                                  .arg( sectorEnd ) );

            result.distributeWidthsAcrossCells( dc.dim, sectorStart, sectorEnd, widths );
        }

        RiaLogging::info( QString( "Non-uniform refinement %1: total refined count = %2 (was %3)" )
                              .arg( dimLabels[static_cast<size_t>( dc.dim )] )
                              .arg( result.totalRefinedCount( dc.dim ) )
                              .arg( result.sectorSize( dc.dim ) ) );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSectorModelUi::hasNonUniformRefinement() const
{
    return m_refineGrid() && m_refinementMode() == NON_UNIFORM && ( m_nonUniformEnableI() || m_nonUniformEnableJ() || m_nonUniformEnableK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicExportSectorModelUi::keywordsToRemove() const
{
    return m_keywordsToRemove.value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimKeywordBcprop*> RicExportSectorModelUi::bcpropKeywords() const
{
    std::vector<RimKeywordBcprop*> keywords;
    for ( auto& kw : m_bcpropKeywords )
    {
        keywords.push_back( kw );
    }
    return keywords;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaModelExportDefines::BoundaryCondition RicExportSectorModelUi::boundaryCondition() const
{
    return m_boundaryCondition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicExportSectorModelUi::porvMultiplier() const
{
    return m_porvMultiplier();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSectorModelUi::exportDeckFilename() const
{
    QString fullpath = m_exportFolder().path() + "/" + m_exportDeckName + ".DATA";

    RiaApplication::instance()->setLastUsedDialogDirectory( "EXPORT_INPUT_GRID", fullpath );

    return fullpath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSectorModelUi::inputDeckFilename() const
{
    return m_inputDeckName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaModelExportDefines::GridBoxSelection RicExportSectorModelUi::gridBoxSelection() const
{
    return m_gridBoxSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicExportSectorModelUi::wellPadding() const
{
    return m_visibleWellsPadding();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_gridBoxSelection )
    {
        applyBoundaryDefaults();
        updateConnectedEditors();
    }
    if ( changedField == &m_visibleWellsPadding || changedField == &m_minI || changedField == &m_maxI || changedField == &m_minJ ||
         changedField == &m_maxJ || changedField == &m_minK || changedField == &m_maxK )
    {
        applyBoundaryDefaults();
    }
    else if ( ( changedField == &m_boundaryCondition ) || ( changedField == &m_refineGrid ) || ( changedField == &m_enablePadding ) ||
              ( changedField == &m_nonUniformEnableI ) || ( changedField == &m_nonUniformEnableJ ) ||
              ( changedField == &m_nonUniformEnableK ) || ( changedField == &m_nonUniformSubMode ) )
    {
        updateConnectedEditors();
    }
    else if ( changedField == &m_exportFolder )
    {
        if ( m_simulationJobFolder().path().isEmpty() )
        {
            m_simulationJobFolder = m_exportFolder().path() + "/job";
        }
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::applyBoundaryDefaults()
{
    auto caseData = m_eclipseCase->eclipseCaseData();
    if ( !caseData ) return;

    if ( m_gridBoxSelection == RiaModelExportDefines::ACTIVE_CELLS_BOX )
    {
        const auto& bbox = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->ijkBoundingBox();
        setMin( bbox.min() );
        setMax( bbox.max() );
    }
    else if ( m_gridBoxSelection == RiaModelExportDefines::VISIBLE_CELLS_BOX )
    {
        setMin( m_visibleMin );
        setMax( m_visibleMax );
    }
    else if ( m_gridBoxSelection == RiaModelExportDefines::VISIBLE_WELLS_BOX )
    {
        auto [minWellCells, maxWellCells] = RimEclipseViewTools::computeVisibleWellCells( m_eclipseView, caseData, m_visibleWellsPadding() );
        setMin( minWellCells );
        setMax( maxWellCells );
    }
    else if ( m_gridBoxSelection == RiaModelExportDefines::FULL_GRID_BOX )
    {
        const RigMainGrid* mainGrid = caseData->mainGrid();
        setMin( caf::VecIjk0::ZERO );
        cvf::Vec3st maxCounts = mainGrid->cellCounts() - cvf::Vec3st( 1, 1, 1 );
        setMax( caf::VecIjk0( maxCounts.x(), maxCounts.y(), maxCounts.z() ) );
    }
    else
    {
        const RigMainGrid* mainGrid = caseData->mainGrid();

        if ( m_maxI() > (int)mainGrid->cellCountI() )
        {
            m_maxI = (int)mainGrid->cellCountI();
        }
        if ( m_maxJ() > (int)mainGrid->cellCountJ() )
        {
            m_maxJ = (int)mainGrid->cellCountJ();
        }
        if ( m_maxK() > (int)mainGrid->cellCountK() )
        {
            m_maxK = (int)mainGrid->cellCountK();
        }
    }

    m_totalCells = std::max( 0, ( ( m_maxI() - m_minI() + 1 ) * ( m_maxJ() - m_minJ() + 1 ) * ( m_maxK() - m_minK() + 1 ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RicExportSectorModelUi::parseWidths( const QString& text )
{
    QStringList         parts = text.split( ",", Qt::SkipEmptyParts );
    std::vector<double> widths;
    for ( const auto& part : parts )
    {
        bool   ok    = false;
        double value = part.trimmed().toDouble( &ok );
        if ( ok && value > 0.0 ) widths.push_back( value );
    }
    return widths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSectorModelUi::defaultFolder()
{
    QString projectDirectory  = RiaApplication::instance()->currentProjectPath();
    QString fallbackDirectory = projectDirectory;
    if ( fallbackDirectory.isEmpty() )
    {
        QString generalFallback = RiaApplication::instance()->lastUsedDialogDirectory( "GENERAL_DATA" );
        fallbackDirectory       = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback( "BINARY_GRID", generalFallback );
    }
    return RiaApplication::instance()->lastUsedDialogDirectoryWithFallback( "EXPORT_INPUT_GRID", fallbackDirectory );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSectorModelUi::shouldCreateSimulationJob() const
{
    return m_createSimulationJob();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSectorModelUi::startSimulationJobAfterExport() const
{
    return m_startSimulationJobAfterExport();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSectorModelUi::newSimulationJobFolder() const
{
    return m_simulationJobFolder().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSectorModelUi::newSimulationJobName() const
{
    return m_simulationJobName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigModelPaddingSettings RicExportSectorModelUi::paddingSettings() const
{
    RigModelPaddingSettings settings;
    settings.setEnabled( m_enablePadding() );
    settings.setNzUpper( m_paddingNzUpper() );
    settings.setTopUpper( m_paddingTopUpper() );
    settings.setUpperPorosity( m_paddingUpperPorosity() );
    settings.setUpperEquilnum( m_paddingUpperEquilnum() );
    settings.setNzLower( m_paddingNzLower() );
    settings.setBottomLower( m_paddingBottomLower() );
    settings.setMinLayerThickness( m_paddingMinThickness() );
    settings.setFillGaps( m_paddingFillGaps() );
    settings.setMonotonicZcorn( m_paddingMonotonicZcorn() );
    settings.setVerticalPillars( m_paddingVerticalPillars() );
    return settings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RicExportSectorModelUi::validate( const QString& configName ) const
{
    std::map<QString, QString> fieldErrors;

    if ( configName == m_pageNames[WizardPageEnum::ExportSettings] )
    {
        if ( m_exportDeckName().trimmed().isEmpty() )
        {
            fieldErrors[m_exportDeckName.keyword()] = "Output sector model name cannot be empty.";
        }
        if ( m_exportFolder().path().trimmed().isEmpty() )
        {
            fieldErrors[m_exportFolder.keyword()] = "Export folder cannot be empty.";
        }
        if ( m_exportFolder().path() == m_eclipseCase->locationOnDisc() )
        {
            fieldErrors[m_exportFolder.keyword()] = "Export folder cannot be the same as the source case folder.";
        }
    }
    else if ( configName == m_pageNames[WizardPageEnum::GridBoxSelection] )
    {
        if ( ( m_eclipseCase == nullptr ) || ( m_eclipseCase->eclipseCaseData() == nullptr ) ||
             ( m_eclipseCase->eclipseCaseData()->mainGrid() == nullptr ) )
            return {};

        const RigMainGrid* mainGrid       = m_eclipseCase->eclipseCaseData()->mainGrid();
        const cvf::Vec3st  gridDimensions = mainGrid->cellCounts();

        for ( auto& field : { &m_minI, &m_minJ, &m_minK, &m_maxI, &m_maxJ, &m_maxK } )
        {
            auto errStr = field->validate();
            if ( !errStr.isEmpty() )
            {
                fieldErrors[field->keyword()] = errStr;
            }
        }

        if ( m_gridBoxSelection() == RiaModelExportDefines::MANUAL_SELECTION )
        {
            if ( m_minI() > m_maxI() )
            {
                fieldErrors[m_minI.keyword()] = "Min I cannot be larger than Max I.";
            }
            if ( m_minJ() > m_maxJ() )
            {
                fieldErrors[m_minJ.keyword()] = "Min J cannot be larger than Max J.";
            }
            if ( m_minK() > m_maxK() )
            {
                fieldErrors[m_minK.keyword()] = "Min K cannot be larger than Max K.";
            }
            if ( m_maxI() > (int)gridDimensions.x() )
            {
                fieldErrors[m_maxI.keyword()] = QString( "Max I cannot be larger than %1." ).arg( gridDimensions.x() );
            }
            if ( m_maxJ() > (int)gridDimensions.y() )
            {
                fieldErrors[m_maxJ.keyword()] = QString( "Max J cannot be larger than %1." ).arg( gridDimensions.y() );
            }
            if ( m_maxK() > (int)gridDimensions.z() )
            {
                fieldErrors[m_maxK.keyword()] = QString( "Max K cannot be larger than %1." ).arg( gridDimensions.z() );
            }
        }
        else if ( m_gridBoxSelection() == RiaModelExportDefines::VISIBLE_WELLS_BOX )
        {
            auto errStr = m_visibleWellsPadding.validate();
            if ( !errStr.isEmpty() )
            {
                fieldErrors[m_visibleWellsPadding.keyword()] = errStr;
            }
        }
    }
    else if ( configName == m_pageNames[WizardPageEnum::GridRefinement] )
    {
        if ( m_refineGrid() )
        {
            if ( m_refinementMode() == UNIFORM )
            {
                for ( auto& field : { &m_refinementCountI, &m_refinementCountJ, &m_refinementCountK } )
                {
                    auto errStr = field->validate();
                    if ( !errStr.isEmpty() )
                    {
                        fieldErrors[field->keyword()] = errStr;
                    }
                }
            }
            else
            {
                struct DimValidation
                {
                    const caf::PdmField<bool>&    enable;
                    const caf::PdmField<int>&     rangeStart;
                    const caf::PdmField<int>&     rangeEnd;
                    const caf::PdmField<QString>& intervals;
                    int                           sectorMin;
                    int                           sectorMax;
                    QString                       label;
                };

                std::vector<DimValidation> dims = {
                    { m_nonUniformEnableI, m_nonUniformRangeStartI, m_nonUniformRangeEndI, m_nonUniformIntervalsI, m_minI(), m_maxI(), "I" },
                    { m_nonUniformEnableJ, m_nonUniformRangeStartJ, m_nonUniformRangeEndJ, m_nonUniformIntervalsJ, m_minJ(), m_maxJ(), "J" },
                    { m_nonUniformEnableK, m_nonUniformRangeStartK, m_nonUniformRangeEndK, m_nonUniformIntervalsK, m_minK(), m_maxK(), "K" },
                };

                for ( const auto& dv : dims )
                {
                    if ( !dv.enable() ) continue;

                    if ( dv.rangeStart() > dv.rangeEnd() )
                    {
                        fieldErrors[dv.rangeStart.keyword()] =
                            QString( "%1 direction: Grid Cell Start cannot be larger than Grid Cell End." ).arg( dv.label );
                    }
                    if ( dv.rangeStart() < dv.sectorMin || dv.rangeEnd() > dv.sectorMax )
                    {
                        fieldErrors[dv.rangeEnd.keyword()] =
                            QString( "%1 direction: Refinement range [%2, %3] is outside the sector model range [%4, %5]." )
                                .arg( dv.label )
                                .arg( dv.rangeStart() )
                                .arg( dv.rangeEnd() )
                                .arg( dv.sectorMin )
                                .arg( dv.sectorMax );
                    }
                    if ( m_nonUniformSubMode() == CUSTOM_WIDTHS && parseWidths( dv.intervals() ).empty() )
                    {
                        fieldErrors[dv.intervals.keyword()] =
                            QString( "%1 direction: Fractional widths must contain at least one positive value." ).arg( dv.label );
                    }
                }
            }
        }
    }
    else if ( configName == m_pageNames[WizardPageEnum::BoundaryConditions] )
    {
        // no validation needed
    }
    else if ( configName == m_pageNames[WizardPageEnum::KeywordAdjustments] )
    {
        for ( auto& kw : m_keywordsToRemove() )
        {
            if ( kw.contains( ' ' ) )
            {
                fieldErrors[m_keywordsToRemove.keyword()] = "Keywords to remove cannot contain spaces: " + kw;
            }
        }
    }
    else if ( configName == m_pageNames[WizardPageEnum::ModelPadding] )
    {
        if ( m_enablePadding() )
        {
            auto settings   = paddingSettings();
            auto errMessage = settings.validate();
            if ( !errMessage.isEmpty() )
            {
                fieldErrors[m_enablePadding.keyword()] = errMessage;
            }
        }
    }
    else if ( configName == m_pageNames[WizardPageEnum::SimulationJob] )
    {
        if ( m_createSimulationJob() )
        {
            if ( m_simulationJobName().trimmed().isEmpty() )
            {
                fieldErrors[m_simulationJobName.keyword()] = "Simulation job name cannot be empty.";
            }
            if ( m_simulationJobFolder().path().trimmed().isEmpty() )
            {
                fieldErrors[m_simulationJobFolder.keyword()] = "Simulation job working folder cannot be empty.";
            }
            if ( m_simulationJobFolder().path() == m_exportFolder().path() )
            {
                fieldErrors[m_simulationJobFolder.keyword()] = "Simulation job working folder cannot be the same as the export folder.";
            }
            if ( m_simulationJobFolder().path() == m_eclipseCase->locationOnDisc() )
            {
                fieldErrors[m_simulationJobFolder.keyword()] =
                    "Simulation job working folder cannot be the same as the source case folder.";
            }
        }
    }

    return fieldErrors;
}
