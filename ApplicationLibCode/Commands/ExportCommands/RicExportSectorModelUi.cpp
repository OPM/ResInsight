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

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultTools.h"
#include "RigMainGrid.h"

#include "Jobs/RimKeywordBcprop.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimTools.h"
#include "Tools/RimEclipseViewTools.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiRadioButtonEditor.h"
#include "cafPdmUiTableViewEditor.h"

#include <utility>

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
    CAF_PDM_InitField( &m_porvMultiplier, "PorvMultiplier", 1.0e6, "PORV Multiplier" );
    CAF_PDM_InitFieldNoDefault( &m_boundaryCondition, "BoundaryCondition", "Boundary Condition Type:" );
    m_boundaryCondition.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_gridBoxSelection, "GridBoxSelection", "Cells to Export:" );
    m_gridBoxSelection.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_visibleWellsPadding, "VisibleWellsPadding", 2, "Well Padding", "", "Number of cells to add around visible wells", "" );

    CAF_PDM_InitField( &m_minI, "MinI", std::numeric_limits<int>::max(), "Min I, J, K" );
    CAF_PDM_InitField( &m_minJ, "MinJ", std::numeric_limits<int>::max(), "" );
    m_minJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_minK, "MinK", std::numeric_limits<int>::max(), "" );
    m_minK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_maxI, "MaxI", -std::numeric_limits<int>::max(), "Max I, J, K" );
    CAF_PDM_InitField( &m_maxJ, "MaxJ", -std::numeric_limits<int>::max(), "" );
    m_maxJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &m_maxK, "MaxK", -std::numeric_limits<int>::max(), "" );
    m_maxK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitField( &m_refineGrid, "RefineGrid", false, "Enable Grid Refinement" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_refineGrid );
    CAF_PDM_InitField( &m_refinementCountI, "RefinementCountI", 1, "Cell Count I, J, K" );
    CAF_PDM_InitField( &m_refinementCountJ, "RefinementCountJ", 1, "" );
    CAF_PDM_InitField( &m_refinementCountK, "RefinementCountK", 1, "" );

    CAF_PDM_InitFieldNoDefault( &m_bcpropKeywords, "BcpropKeywords", "BCPROP Keywords" );
    m_bcpropKeywords.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_bcpropKeywords.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

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
    CAF_ASSERT( m_pageNames.size() == 5 );

    if ( uiConfigName == m_pageNames[0] )
    {
        uiOrdering.add( &m_exportDeckName );
        uiOrdering.add( &m_exportFolder );

        auto infoGrp = uiOrdering.addNewGroup( "Source Information" );
        infoGrp->addNewLabel( QString( "Source Folder: " ) + m_eclipseCase->locationOnDisc() );
        infoGrp->addNewLabel( QString( "Source Case Name: " ) + m_eclipseCase->caseUserDescription() );
    }
    else if ( uiConfigName == m_pageNames[1] )
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

        if ( m_totalCells > 0 )
        {
            uiOrdering.addNewLabel( "" );
            uiOrdering.addNewLabel( QString( "Total cells to export: %1" ).arg( m_totalCells ) );
        }
    }
    else if ( uiConfigName == m_pageNames[2] )
    {
        uiOrdering.add( &m_refineGrid );
        uiOrdering.addNewLabel( "" );

        uiOrdering.add( &m_refinementCountI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        uiOrdering.appendToRow( &m_refinementCountJ );
        uiOrdering.appendToRow( &m_refinementCountK );

        m_refinementCountI.uiCapability()->setUiReadOnly( !m_refineGrid() );
        m_refinementCountJ.uiCapability()->setUiReadOnly( !m_refineGrid() );
        m_refinementCountK.uiCapability()->setUiReadOnly( !m_refineGrid() );
    }
    else if ( uiConfigName == m_pageNames[3] )
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
    else if ( uiConfigName == m_pageNames[4] )
    {
        auto simGrp = uiOrdering.addNewGroup( "OPM Flow Simulation" );
        simGrp->add( &m_createSimulationJob );
        if ( m_createSimulationJob() )
        {
            if ( m_simulationJobName().isEmpty() ) m_simulationJobName = m_exportDeckName();
            if ( m_simulationJobFolder().path().isEmpty() ) m_simulationJobFolder = defaultFolder();
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

    if ( ( m_eclipseCase != nullptr ) && m_exportDeckName().isEmpty() ) m_exportDeckName = m_eclipseCase->caseUserDescription();

    m_createSimulationJob = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( ( m_eclipseCase == nullptr ) || ( m_eclipseCase->eclipseCaseData() == nullptr ) ||
         ( m_eclipseCase->eclipseCaseData()->mainGrid() == nullptr ) )
        return;

    const RigMainGrid* mainGrid       = m_eclipseCase->eclipseCaseData()->mainGrid();
    const cvf::Vec3st  gridDimensions = mainGrid->cellCounts();

    if ( field == &m_bcpropKeywords )
    {
        auto* tvAttr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttr )
        {
            tvAttr->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttr->alwaysEnforceResizePolicy = true;
        }
    }
    else if ( field == &m_visibleWellsPadding )
    {
        if ( auto lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            // Wells padding should be between 0 and 100 cells
            lineEditorAttr->validator = new QIntValidator( 0, 100, nullptr );
        }
    }
    else if ( field == &m_refinementCountI || field == &m_refinementCountJ || field == &m_refinementCountK )
    {
        if ( auto lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            auto* validator           = new QIntValidator( 1, 10, nullptr );
            lineEditorAttr->validator = validator;
        }
    }
    else if ( field == &m_minI || field == &m_maxI )
    {
        if ( auto lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            lineEditorAttr->validator = new QIntValidator( 1, (int)gridDimensions.x(), nullptr );
        }
    }
    else if ( field == &m_minJ || field == &m_maxJ )
    {
        if ( auto lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            lineEditorAttr->validator = new QIntValidator( 1, (int)gridDimensions.y(), nullptr );
        }
    }
    else if ( field == &m_minK || field == &m_maxK )
    {
        if ( auto lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            lineEditorAttr->validator = new QIntValidator( 1, (int)gridDimensions.z(), nullptr );
        }
    }

    if ( ( field == &m_exportFolder ) || ( field == &m_simulationJobFolder ) )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
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
    if ( !m_refineGrid() )
    {
        return cvf::Vec3st( 1, 1, 1 );
    }
    return cvf::Vec3st( m_refinementCountI(), m_refinementCountJ(), m_refinementCountK() );
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
    if ( changedField == &m_visibleWellsPadding )
    {
        applyBoundaryDefaults();
    }
    else if ( ( changedField == &m_boundaryCondition ) || ( changedField == &m_refineGrid ) )
    {
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
        auto [minActive, maxActive] = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->ijkBoundingBox();
        setMin( minActive );
        setMax( maxActive );
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
