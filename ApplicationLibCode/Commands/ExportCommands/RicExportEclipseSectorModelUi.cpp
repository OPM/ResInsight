/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicExportEclipseSectorModelUi.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "ProjectDataModel/Jobs/RimKeywordBcprop.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QDir>
#include <QFileInfo>
#include <QIntValidator>

#include <set>

CAF_PDM_SOURCE_INIT( RicExportEclipseSectorModelUi, "RicExportEclipseInputGridUi" );

namespace caf
{
template <>
void RicExportEclipseSectorModelUi::ResultExportOptionsEnum::setUp()
{
    addItem( RicExportEclipseSectorModelUi::EXPORT_NO_RESULTS, "NO_RESULTS", "Do not export" );
    addItem( RicExportEclipseSectorModelUi::EXPORT_TO_GRID_FILE, "TO_GRID_FILE", "Append to grid file" );
    addItem( RicExportEclipseSectorModelUi::EXPORT_TO_SINGLE_SEPARATE_FILE, "TO_SINGLE_RESULT_FILE", "Export to single file" );
    addItem( RicExportEclipseSectorModelUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT,
             "TO_SEPARATE_RESULT_FILES",
             "Export to a separate file per parameter" );

    setDefault( RicExportEclipseSectorModelUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT );
}

template <>
void RicExportEclipseSectorModelUi::GridBoxSelectionEnum::setUp()
{
    addItem( RicExportEclipseSectorModelUi::VISIBLE_CELLS_BOX, "VISIBLE_CELLS", "Box Containing all Visible Cells" );
    addItem( RicExportEclipseSectorModelUi::ACTIVE_CELLS_BOX, "ACTIVE_CELLS", "Box Containing all Active Cells" );
    addItem( RicExportEclipseSectorModelUi::VISIBLE_WELLS_BOX, "VISIBLE_WELLS", "Box Containing all Visible Simulation Wells" );
    addItem( RicExportEclipseSectorModelUi::FULL_GRID_BOX, "FULL_GRID", "Full Grid" );
    addItem( RicExportEclipseSectorModelUi::MANUAL_SELECTION, "MANUAL_SELECTION", "User Defined Selection" );

    setDefault( RicExportEclipseSectorModelUi::VISIBLE_CELLS_BOX );
}

template <>
void RicExportEclipseSectorModelUi::BoundaryConditionEnum::setUp()
{
    addItem( RicExportEclipseSectorModelUi::OPERNUM_OPERATER, "OPERNUM_OPERATER", "OPERNUM + OPERATER" );
    addItem( RicExportEclipseSectorModelUi::BCCON_BCPROP, "BCCON_BCPROP", "BCCON + BCPROP" );

    setDefault( RicExportEclipseSectorModelUi::OPERNUM_OPERATER );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportEclipseSectorModelUi::RicExportEclipseSectorModelUi()
{
    CAF_PDM_InitObject( "Export Visible Cells as Eclipse Input Grid" );

    CAF_PDM_InitField( &exportGrid, "ExportGrid", true, "Export Grid Data", "", "Includes COORD, ZCORN and ACTNUM", "" );
    CAF_PDM_InitField( &m_exportGridFilename, "ExportGridFilename", QString(), "Grid File Name" );
    CAF_PDM_InitField( &exportInLocalCoordinates, "ExportInLocalCoords", false, "Export in Local Coordinates", "", "Remove UTM location on export", "" );
    CAF_PDM_InitField( &makeInvisibleCellsInactive, "InvisibleCellActnum", false, "Make Invisible Cells Inactive" );

    CAF_PDM_InitFieldNoDefault( &exportGridBox, "GridBoxSelection", "Cells to Export" );

    CAF_PDM_InitField( &m_visibleWellsPadding,
                       "VisibleWellsPadding",
                       2,
                       "Wells Padding (cells)",
                       "",
                       "Number of cells to add around visible wells",
                       "" );

    QString minIJKLabel = "Min I, J, K";
    CAF_PDM_InitField( &minI, "MinI", std::numeric_limits<int>::max(), minIJKLabel );
    CAF_PDM_InitField( &minJ, "MinJ", std::numeric_limits<int>::max(), "" );
    minJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &minK, "MinK", std::numeric_limits<int>::max(), "" );
    minK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    QString maxIJKLabel = "Max I, J, K";
    CAF_PDM_InitField( &maxI, "MaxI", -std::numeric_limits<int>::max(), maxIJKLabel );
    CAF_PDM_InitField( &maxJ, "MaxJ", -std::numeric_limits<int>::max(), "" );
    maxJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &maxK, "MaxK", -std::numeric_limits<int>::max(), "" );
    maxK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &exportFaults, "ExportFaults", "Export Fault Data" );
    exportFaults = EXPORT_TO_SINGLE_SEPARATE_FILE;

    CAF_PDM_InitField( &m_exportFaultsFilename, "ExportFaultsFilename", QString(), "Faults File Name" );

    QString ijkLabel = "Cell Count I, J, K";
    CAF_PDM_InitField( &refinementCountI, "RefinementCountI", 1, ijkLabel );
    CAF_PDM_InitField( &refinementCountJ, "RefinementCountJ", 1, "" );
    CAF_PDM_InitField( &refinementCountK, "RefinementCountK", 1, "" );

    CAF_PDM_InitFieldNoDefault( &exportParameters, "ExportParams", "Export Parameters" );
    CAF_PDM_InitField( &m_exportParametersFilename, "ExportParamsFilename", QString(), "File Name" );

    CAF_PDM_InitFieldNoDefault( &selectedKeywords, "ExportMainKeywords", "Keywords to Export" );

    CAF_PDM_InitField( &m_writeEchoInGrdeclFiles,
                       "WriteEchoInGrdeclFiles",
                       RiaPreferences::current()->writeEchoInGrdeclFiles(),
                       "Write NOECHO and ECHO" );

    CAF_PDM_InitFieldNoDefault( &m_exportFolder, "ExportFolder", "Export Folder" );
    m_exportFolder = defaultFolder();

    CAF_PDM_InitFieldNoDefault( &m_bcpropKeywords, "BcpropKeywords", "BCPROP Keywords" );

    CAF_PDM_InitField( &m_exportSimulationInput, "ExportSimulationInput", false, "Export Simulation Input" );

    CAF_PDM_InitFieldNoDefault( &m_boundaryCondition, "BoundaryCondition", "Boundary Condition Type" );

    CAF_PDM_InitField( &m_porvMultiplier, "PorvMultiplier", 1.0e6, "PORV Multiplier" );

    m_exportGridFilename       = defaultGridFileName();
    m_exportParametersFilename = defaultResultsFileName();
    m_exportFaultsFilename     = defaultFaultsFileName();

    // Add 10 default BCPROP keywords
    for ( int i = 0; i < 10; ++i )
    {
        m_bcpropKeywords.push_back( new RimKeywordBcprop() );
    }

    m_tabNames << "Grid Data";
    m_tabNames << "Parameters";
    m_tabNames << "Simulation Input";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportEclipseSectorModelUi::~RicExportEclipseSectorModelUi()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList& RicExportEclipseSectorModelUi::tabNames() const
{
    return m_tabNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setCaseData( RigEclipseCaseData* caseData /*= nullptr*/,
                                                 RimEclipseView*     eclipseView /*= nullptr*/,
                                                 const cvf::Vec3st&  visibleMin /*= cvf::Vec3st::ZERO*/,
                                                 const cvf::Vec3st&  visibleMax /*= cvf::Vec3st::ZERO*/ )
{
    m_caseData    = caseData;
    m_eclipseView = eclipseView;
    m_visibleMin  = visibleMin;
    m_visibleMax  = visibleMax;

    // Check if a .DATA file exists next to the grid file
    m_exportSimulationInput = false;
    if ( eclipseView && eclipseView->eclipseCase() )
    {
        QFileInfo fi( eclipseView->eclipseCase()->gridFileName() );
        QString   dataFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".DATA";
        if ( QFile::exists( dataFileName ) )
        {
            m_exportSimulationInput = true;
        }
    }

    if ( minI == std::numeric_limits<int>::max() ) minI = static_cast<int>( m_visibleMin.x() ) + 1;
    if ( minJ == std::numeric_limits<int>::max() ) minJ = static_cast<int>( m_visibleMin.y() ) + 1;
    if ( minK == std::numeric_limits<int>::max() ) minK = static_cast<int>( m_visibleMin.z() ) + 1;

    if ( maxI == std::numeric_limits<int>::max() ) maxI = static_cast<int>( m_visibleMax.x() ) + 1;
    if ( maxJ == std::numeric_limits<int>::max() ) maxJ = static_cast<int>( m_visibleMax.y() ) + 1;
    if ( maxK == std::numeric_limits<int>::max() ) maxK = static_cast<int>( m_visibleMax.z() ) + 1;

    if ( selectedKeywords.v().empty() )
    {
        for ( const QString& keyword : mainKeywords() )
        {
            if ( caseData && caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                 ->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, keyword ) ) )
            {
                selectedKeywords.v().push_back( keyword );
            }
        }
    }
    else
    {
        std::vector<QString> validSelectedKeywords;
        for ( const QString& keyword : selectedKeywords() )
        {
            if ( caseData && caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                 ->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, keyword ) ) )
            {
                validSelectedKeywords.push_back( keyword );
            }
        }
        selectedKeywords.v() = validSelectedKeywords;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicExportEclipseSectorModelUi::min() const
{
    return cvf::Vec3st( minI() - 1, minJ() - 1, minK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3st RicExportEclipseSectorModelUi::max() const
{
    return cvf::Vec3st( maxI() - 1, maxJ() - 1, maxK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setMin( const cvf::Vec3st& min )
{
    minI = static_cast<int>( min.x() ) + 1;
    minJ = static_cast<int>( min.y() ) + 1;
    minK = static_cast<int>( min.z() ) + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setMax( const cvf::Vec3st& max )
{
    maxI = static_cast<int>( max.x() ) + 1;
    maxJ = static_cast<int>( max.y() ) + 1;
    maxK = static_cast<int>( max.z() ) + 1;
}

cvf::Vec3st RicExportEclipseSectorModelUi::refinement() const
{
    return cvf::Vec3st( refinementCountI(), refinementCountJ(), refinementCountK() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    if ( !m_caseData ) return;

    const RigMainGrid* mainGrid       = m_caseData->mainGrid();
    const cvf::Vec3st  gridDimensions = mainGrid->cellCounts();

    auto* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );

    if ( field == &m_exportParametersFilename || field == &m_exportGridFilename || field == &m_exportFaultsFilename )
    {
        auto* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectSaveFileName  = true;
            myAttr->m_fileSelectionFilter = "GRDECL files (*.grdecl *.GRDECL);;All files (*.*)";
        }
    }
    else if ( field == &selectedKeywords )
    {
        auto myAttr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->heightHint = 280;
        }
    }
    else if ( field == &m_bcpropKeywords )
    {
        auto* tvAttr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttr )
        {
            tvAttr->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FIT_CONTENT;
            tvAttr->alwaysEnforceResizePolicy = true;
        }
    }
    else if ( field == &m_visibleWellsPadding )
    {
        if ( lineEditorAttr )
        {
            // Wells padding should be between 0 and 100 cells
            lineEditorAttr->validator = new QIntValidator( 0, 100, nullptr );
        }
    }
    else if ( field == &refinementCountI || field == &refinementCountJ || field == &refinementCountK )
    {
        if ( lineEditorAttr )
        {
            auto* validator           = new QIntValidator( 1, 10, nullptr );
            lineEditorAttr->validator = validator;
        }
    }
    else if ( field == &minI || field == &maxI )
    {
        if ( lineEditorAttr )
        {
            lineEditorAttr->validator = new QIntValidator( 1, (int)gridDimensions.x(), nullptr );
        }
    }
    else if ( field == &minJ || field == &maxJ )
    {
        if ( lineEditorAttr )
        {
            lineEditorAttr->validator = new QIntValidator( 1, (int)gridDimensions.y(), nullptr );
        }
    }
    else if ( field == &minK || field == &maxK )
    {
        if ( lineEditorAttr )
        {
            lineEditorAttr->validator = new QIntValidator( 1, (int)gridDimensions.z(), nullptr );
        }
    }

    if ( field == &m_exportFolder )
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
void RicExportEclipseSectorModelUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( uiConfigName == m_tabNames[0] )
    {
        uiOrdering.add( &m_exportFolder );

        caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "Grid Export" );
        gridGroup->add( &exportGrid );
        gridGroup->add( &m_exportGridFilename );
        m_exportGridFilename.uiCapability()->setUiReadOnly( !exportGrid() );
        gridGroup->add( &exportInLocalCoordinates );
        exportInLocalCoordinates.uiCapability()->setUiReadOnly( !exportGrid() );

        caf::PdmUiGroup* gridBoxGroup = uiOrdering.addNewGroup( "Grid Box Selection" );
        gridBoxGroup->add( &exportGridBox, { .newRow = true, .totalColumnSpan = 4, .leftLabelColumnSpan = 1 } );

        gridBoxGroup->add( &minI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        gridBoxGroup->appendToRow( &minJ );
        gridBoxGroup->appendToRow( &minK );

        gridBoxGroup->add( &maxI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        gridBoxGroup->appendToRow( &maxJ );
        gridBoxGroup->appendToRow( &maxK );

        if ( exportGridBox() == VISIBLE_WELLS_BOX )
        {
            gridBoxGroup->add( &m_visibleWellsPadding, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        }

        gridBoxGroup->add( &makeInvisibleCellsInactive, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );

        minI.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        minJ.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        minK.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        maxI.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        maxJ.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        maxK.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );

        caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup( "Grid Refinement" );
        gridRefinement->add( &refinementCountI, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        gridRefinement->appendToRow( &refinementCountJ );
        gridRefinement->appendToRow( &refinementCountK );
        refinementCountI.uiCapability()->setUiReadOnly( !exportGrid() );
        refinementCountJ.uiCapability()->setUiReadOnly( !exportGrid() );
        refinementCountK.uiCapability()->setUiReadOnly( !exportGrid() );

        caf::PdmUiGroup* faultsGroup = uiOrdering.addNewGroup( "Faults" );
        faultsGroup->add( &exportFaults );
        if ( exportFaults() != EXPORT_NO_RESULTS )
        {
            if ( exportFaults() == EXPORT_TO_SINGLE_SEPARATE_FILE )
            {
                faultsGroup->add( &m_exportFaultsFilename );
            }
        }
    }
    else if ( uiConfigName == m_tabNames[1] )
    {
        caf::PdmUiGroup* resultsGroup = uiOrdering.addNewGroup( "Parameter Export" );

        resultsGroup->add( &m_writeEchoInGrdeclFiles );

        resultsGroup->add( &exportParameters );
        if ( exportParameters() != EXPORT_NO_RESULTS )
        {
            if ( exportParameters() == EXPORT_TO_SINGLE_SEPARATE_FILE )
            {
                resultsGroup->add( &m_exportParametersFilename );
            }
        }

        if ( exportParameters() != EXPORT_NO_RESULTS )
        {
            resultsGroup->add( &selectedKeywords );
        }
    }
    else if ( uiConfigName == m_tabNames[2] )
    {
        caf::PdmUiGroup* bcGroup = uiOrdering.addNewGroup( "Boundary Conditions" );
        bcGroup->add( &m_boundaryCondition );

        if ( m_boundaryCondition() == OPERNUM_OPERATER )
        {
            bcGroup->add( &m_porvMultiplier );
        }
        else if ( m_boundaryCondition() == BCCON_BCPROP )
        {
            m_bcpropKeywords.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
            bcGroup->add( &m_bcpropKeywords );
        }
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &exportGrid )
    {
        if ( !exportGrid() )
        {
            if ( exportFaults() == EXPORT_TO_GRID_FILE )
            {
                exportFaults = EXPORT_TO_SINGLE_SEPARATE_FILE;
            }
            if ( exportParameters() == EXPORT_TO_GRID_FILE )
            {
                exportParameters = EXPORT_TO_SEPARATE_FILE_PER_RESULT;
            }
            updateConnectedEditors();
        }
    }
    else if ( changedField == &exportGridBox || changedField == &m_visibleWellsPadding )
    {
        applyBoundaryDefaults();
        updateConnectedEditors();
    }
    else if ( changedField == &m_boundaryCondition )
    {
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportEclipseSectorModelUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &selectedKeywords )
    {
        RigCaseCellResultsData* resultData = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

        std::vector<RiaDefines::ResultCatType> exportTypes = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                               RiaDefines::ResultCatType::GENERATED,
                                                               RiaDefines::ResultCatType::INPUT_PROPERTY };

        QList<caf::PdmOptionItemInfo> allOptions;

        for ( const auto resultCategory : exportTypes )
        {
            auto options = RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard( resultCategory, resultData );
            allOptions.append( options );
        }

        std::set<QString> mainKeywords = RicExportEclipseSectorModelUi::mainKeywords();
        for ( const caf::PdmOptionItemInfo& option : allOptions )
        {
            if ( mainKeywords.count( option.optionUiText() ) )
            {
                options.push_back( option );
            }
        }
        for ( const caf::PdmOptionItemInfo& option : allOptions )
        {
            if ( !mainKeywords.count( option.optionUiText() ) && option.optionUiText() != "None" )
            {
                options.push_back( option );
            }
        }
    }
    else if ( fieldNeedingOptions == &exportFaults )
    {
        std::set<ResultExportOptions> validFaultOptions = { EXPORT_NO_RESULTS, EXPORT_TO_GRID_FILE, EXPORT_TO_SINGLE_SEPARATE_FILE };
        if ( !exportGrid() ) validFaultOptions.erase( EXPORT_TO_GRID_FILE );
        for ( ResultExportOptions option : validFaultOptions )
        {
            options.push_back( caf::PdmOptionItemInfo( ResultExportOptionsEnum::uiText( option ), option ) );
        }
    }
    else if ( fieldNeedingOptions == &exportParameters )
    {
        std::set<ResultExportOptions> validFaultOptions = { EXPORT_NO_RESULTS,
                                                            EXPORT_TO_GRID_FILE,
                                                            EXPORT_TO_SINGLE_SEPARATE_FILE,
                                                            EXPORT_TO_SEPARATE_FILE_PER_RESULT };
        if ( !exportGrid() ) validFaultOptions.erase( EXPORT_TO_GRID_FILE );
        for ( ResultExportOptions option : validFaultOptions )
        {
            options.push_back( caf::PdmOptionItemInfo( ResultExportOptionsEnum::uiText( option ), option ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RicExportEclipseSectorModelUi::mainKeywords()
{
    return { RiaResultNames::eqlnumResultName(), "FIPNUM", "NTG", "PERMX", "PERMY", "PERMZ", "PORO", "PVTNUM", "SATNUM", "SWATINIT" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::defaultFolder() const
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
QString RicExportEclipseSectorModelUi::defaultGridFileName() const
{
    return "GRID.GRDECL";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::defaultResultsFileName() const
{
    return "RESULTS.GRDECL";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::defaultFaultsFileName() const
{
    return "FAULTS.GRDECL";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::applyBoundaryDefaults()
{
    if ( exportGridBox == ACTIVE_CELLS_BOX )
    {
        auto [minActive, maxActive] = m_caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->ijkBoundingBox();
        setMin( minActive );
        setMax( maxActive );
    }
    else if ( exportGridBox == VISIBLE_CELLS_BOX )
    {
        setMin( m_visibleMin );
        setMax( m_visibleMax );
    }
    else if ( exportGridBox == VISIBLE_WELLS_BOX )
    {
        auto [minWellCells, maxWellCells] = computeVisibleWellCells( m_eclipseView, m_caseData, m_visibleWellsPadding() );
        setMin( minWellCells );
        setMax( maxWellCells );
    }
    else if ( exportGridBox == FULL_GRID_BOX )
    {
        const RigMainGrid* mainGrid = m_caseData->mainGrid();
        setMin( cvf::Vec3st::ZERO );
        setMax( mainGrid->cellCounts() - cvf::Vec3st( 1, 1, 1 ) );
    }
    else
    {
        const RigMainGrid* mainGrid = m_caseData->mainGrid();

        if ( maxI() > (int)mainGrid->cellCountI() )
        {
            maxI = (int)mainGrid->cellCountI();
        }
        if ( maxJ() > (int)mainGrid->cellCountJ() )
        {
            maxJ = (int)mainGrid->cellCountJ();
        }
        if ( maxK() > (int)mainGrid->cellCountK() )
        {
            maxK = (int)mainGrid->cellCountK();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::removeInvalidKeywords()
{
    RigCaseCellResultsData* resultData = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    std::vector<QString> validKeywords;
    for ( const QString& keyword : selectedKeywords() )
    {
        if ( resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, keyword ) ) )
        {
            validKeywords.push_back( keyword );
        }
    }
    selectedKeywords.v().swap( validKeywords );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::exportFaultsFilename() const
{
    return m_exportFolder().path() + "/" + m_exportFaultsFilename();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::exportGridFilename() const
{
    return m_exportFolder().path() + "/" + m_exportGridFilename();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::exportParametersFilename() const
{
    return m_exportFolder().path() + "/" + m_exportParametersFilename();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportEclipseSectorModelUi::writeEchoKeywords() const
{
    return m_writeEchoInGrdeclFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigSimWellData*> RicExportEclipseSectorModelUi::getVisibleSimulationWells( RimEclipseView* view )
{
    std::vector<const RigSimWellData*> visibleWells;

    if ( !view ) return visibleWells;

    // Get well collection from view
    RimSimWellInViewCollection* wellCollection = view->wellCollection();
    if ( !wellCollection ) return visibleWells;

    // Iterate through visible wells in the collection
    for ( RimSimWellInView* rimWell : wellCollection->wells() )
    {
        if ( rimWell && rimWell->showWell() && rimWell->simWellData() )
        {
            visibleWells.push_back( rimWell->simWellData() );
        }
    }

    return visibleWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st>
    RicExportEclipseSectorModelUi::computeVisibleWellCells( RimEclipseView* view, RigEclipseCaseData* caseData, int visibleWellsPadding )
{
    if ( view )
    {
        // Get visible simulation wells from the view
        std::vector<const RigSimWellData*> visibleWells = getVisibleSimulationWells( view );

        if ( !visibleWells.empty() )
        {
            // Get current time step
            int currentTimeStep = view->currentTimeStep();

            // Calculate wells bounding box IJK
            auto [minIjk, maxIjk] = RigEclipseCaseDataTools::wellsBoundingBoxIjk( caseData, visibleWells, currentTimeStep, true, true );
            if ( !minIjk.isUndefined() && !maxIjk.isUndefined() )
            {
                // Apply user-defined padding
                size_t padding                  = static_cast<size_t>( std::max( 0, visibleWellsPadding ) );
                auto [expandedMin, expandedMax] = RigEclipseCaseDataTools::expandBoundingBoxIjk( caseData, minIjk, maxIjk, padding );

                if ( !expandedMin.isUndefined() && !expandedMax.isUndefined() )
                {
                    // Use 0-based indexing as expected by setMin/setMax
                    return { expandedMin, expandedMax };
                }
                else
                {
                    // Fallback without padding
                    return { minIjk, maxIjk };
                }
            }
        }
    }

    // No view available, fallback to full grid
    const RigMainGrid* mainGrid = caseData->mainGrid();
    return { cvf::Vec3st::ZERO, mainGrid->cellCounts() - cvf::Vec3st( 1, 1, 1 ) };
}
