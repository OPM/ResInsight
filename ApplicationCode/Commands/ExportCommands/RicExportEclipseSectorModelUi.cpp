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
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseResultDefinition.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"

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
    addItem( RicExportEclipseSectorModelUi::FULL_GRID_BOX, "FULL_GRID", "Full Grid" );
    addItem( RicExportEclipseSectorModelUi::MANUAL_SELECTION, "MANUAL_SELECTION", "User Defined Selection" );

    setDefault( RicExportEclipseSectorModelUi::VISIBLE_CELLS_BOX );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportEclipseSectorModelUi::RicExportEclipseSectorModelUi()
{
    CAF_PDM_InitObject( "Export Visible Cells as Eclipse Input Grid", "", "", "" );

    CAF_PDM_InitField( &exportGrid, "ExportGrid", true, "Export Grid Data", "", "Includes COORD, ZCORN and ACTNUM", "" );
    CAF_PDM_InitField( &exportGridFilename, "ExportGridFilename", QString(), "Grid File Name", "", "", "" );
    exportGridFilename.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &exportInLocalCoordinates,
                       "ExportInLocalCoords",
                       false,
                       "Export in Local Coordinates",
                       "",
                       "Remove UTM location on export",
                       "" );
    CAF_PDM_InitField( &makeInvisibleCellsInactive, "InvisibleCellActnum", false, "Make Invisible Cells Inactive", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &exportGridBox, "GridBoxSelection", "Cells to Export", "", "", "" );

    QString minIJKLabel = "Min I, J, K";
    CAF_PDM_InitField( &minI, "MinI", std::numeric_limits<int>::max(), minIJKLabel, "", "", "" );
    CAF_PDM_InitField( &minJ, "MinJ", std::numeric_limits<int>::max(), "", "", "", "" );
    minJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &minK, "MinK", std::numeric_limits<int>::max(), "", "", "", "" );
    minK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    QString maxIJKLabel = "Max I, J, K";
    CAF_PDM_InitField( &maxI, "MaxI", -std::numeric_limits<int>::max(), maxIJKLabel, "", "", "" );
    CAF_PDM_InitField( &maxJ, "MaxJ", -std::numeric_limits<int>::max(), "", "", "", "" );
    maxJ.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    CAF_PDM_InitField( &maxK, "MaxK", -std::numeric_limits<int>::max(), "", "", "", "" );
    maxK.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &exportFaults, "ExportFaults", "Export Fault Data", "", "", "" );
    exportFaults = EXPORT_TO_SINGLE_SEPARATE_FILE;

    CAF_PDM_InitField( &exportFaultsFilename, "ExportFaultsFilename", QString(), "Faults File Name", "", "", "" );
    exportFaultsFilename.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    QString ijkLabel = "Cell Count I, J, K";
    CAF_PDM_InitField( &refinementCountI, "RefinementCountI", 1, ijkLabel, "", "", "" );
    CAF_PDM_InitField( &refinementCountJ, "RefinementCountJ", 1, "", "", "", "" );
    CAF_PDM_InitField( &refinementCountK, "RefinementCountK", 1, "", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &exportParameters, "ExportParams", "Export Parameters", "", "", "" );
    CAF_PDM_InitField( &exportParametersFilename, "ExportParamsFilename", QString(), "File Name", "", "", "" );
    exportParametersFilename.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &selectedKeywords, "ExportMainKeywords", "Keywords to Export", "", "", "" );

    exportGridFilename       = defaultGridFileName();
    exportParametersFilename = defaultResultsFileName();
    exportFaultsFilename     = defaultFaultsFileName();

    m_tabNames << "Grid Data";
    m_tabNames << "Parameters";
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
                                                 const cvf::Vec3i&   visibleMin /*= cvf::Vec3i::ZERO*/,
                                                 const cvf::Vec3i&   visibleMax /*= cvf::Vec3i::ZERO*/ )
{
    m_caseData   = caseData;
    m_visibleMin = visibleMin;
    m_visibleMax = visibleMax;

    if ( minI == std::numeric_limits<int>::max() ) minI = m_visibleMin.x() + 1;
    if ( minJ == std::numeric_limits<int>::max() ) minJ = m_visibleMin.y() + 1;
    if ( minK == std::numeric_limits<int>::max() ) minK = m_visibleMin.z() + 1;

    if ( maxI == -std::numeric_limits<int>::max() ) maxI = m_visibleMax.x() + 1;
    if ( maxJ == std::numeric_limits<int>::max() ) maxJ = m_visibleMax.y() + 1;
    if ( maxK == std::numeric_limits<int>::max() ) maxK = m_visibleMax.z() + 1;

    if ( selectedKeywords.v().empty() )
    {
        for ( QString keyword : mainKeywords() )
        {
            if ( caseData &&
                 caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                     ->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, keyword ) ) )
            {
                selectedKeywords.v().push_back( keyword );
            }
        }
    }
    else
    {
        std::vector<QString> validSelectedKeywords;
        for ( QString keyword : selectedKeywords() )
        {
            if ( caseData &&
                 caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
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
cvf::Vec3i RicExportEclipseSectorModelUi::min() const
{
    return cvf::Vec3i( minI() - 1, minJ() - 1, minK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RicExportEclipseSectorModelUi::max() const
{
    return cvf::Vec3i( maxI() - 1, maxJ() - 1, maxK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setMin( const cvf::Vec3i& min )
{
    minI = min.x() + 1;
    minJ = min.y() + 1;
    minK = min.z() + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setMax( const cvf::Vec3i& max )
{
    maxI = max.x() + 1;
    maxJ = max.y() + 1;
    maxK = max.z() + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    if ( !m_caseData ) return;

    const RigMainGrid* mainGrid = m_caseData->mainGrid();
    cvf::Vec3i gridDimensions( int( mainGrid->cellCountI() ), int( mainGrid->cellCountJ() ), int( mainGrid->cellCountK() ) );

    caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );

    if ( field == &exportParametersFilename || field == &exportGridFilename || field == &exportFaultsFilename )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectSaveFileName = true;
        }
    }
    else if ( field == &selectedKeywords )
    {
        caf::PdmUiListEditorAttribute* myAttr = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_heightHint = 280;
        }
    }
    else if ( field == &refinementCountI || field == &refinementCountJ || field == &refinementCountK )
    {
        if ( lineEditorAttr )
        {
            QIntValidator* validator  = new QIntValidator( 1, 10, nullptr );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( uiConfigName == m_tabNames[0] )
    {
        caf::PdmUiGroup* gridGroup = uiOrdering.addNewGroup( "Grid Export" );
        gridGroup->add( &exportGrid );
        gridGroup->add( &exportGridFilename );
        exportGridFilename.uiCapability()->setUiReadOnly( !exportGrid() );
        gridGroup->add( &exportInLocalCoordinates );
        exportInLocalCoordinates.uiCapability()->setUiReadOnly( !exportGrid() );

        caf::PdmUiGroup* gridBoxGroup = uiOrdering.addNewGroup( "Grid Box Selection" );
        gridBoxGroup->add( &exportGridBox, {true, 4, 1} );

        gridBoxGroup->add( &minI, {true, 2, 1} );
        gridBoxGroup->add( &minJ, false );
        gridBoxGroup->add( &minK, false );

        gridBoxGroup->add( &maxI, {true, 2, 1} );
        gridBoxGroup->add( &maxJ, false );
        gridBoxGroup->add( &maxK, false );
        gridBoxGroup->add( &makeInvisibleCellsInactive, {true, 2, 1} );

        minI.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        minJ.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        minK.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        maxI.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        maxJ.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );
        maxK.uiCapability()->setUiReadOnly( exportGridBox() != MANUAL_SELECTION );

        caf::PdmUiGroup* gridRefinement = uiOrdering.addNewGroup( "Grid Refinement" );
        gridRefinement->add( &refinementCountI, {true, 2, 1} );
        gridRefinement->add( &refinementCountJ, {false} );
        gridRefinement->add( &refinementCountK, {false} );
        refinementCountI.uiCapability()->setUiReadOnly( !exportGrid() );
        refinementCountJ.uiCapability()->setUiReadOnly( !exportGrid() );
        refinementCountK.uiCapability()->setUiReadOnly( !exportGrid() );

        caf::PdmUiGroup* faultsGroup = uiOrdering.addNewGroup( "Faults" );
        faultsGroup->add( &exportFaults );
        if ( exportFaults() != EXPORT_NO_RESULTS )
        {
            if ( exportFaults() == EXPORT_TO_SINGLE_SEPARATE_FILE )
            {
                faultsGroup->add( &exportFaultsFilename );
            }
        }
    }
    else if ( uiConfigName == m_tabNames[1] )
    {
        caf::PdmUiGroup* resultsGroup = uiOrdering.addNewGroup( "Parameter Export" );

        resultsGroup->add( &exportParameters );
        if ( exportParameters() != EXPORT_NO_RESULTS )
        {
            if ( exportParameters() == EXPORT_TO_SINGLE_SEPARATE_FILE )
            {
                resultsGroup->add( &exportParametersFilename );
            }
        }

        if ( exportParameters() != EXPORT_NO_RESULTS )
        {
            resultsGroup->add( &selectedKeywords );
        }
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
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
            this->updateConnectedEditors();
        }
    }
    else if ( changedField == &exportGridBox )
    {
        applyBoundaryDefaults();
        this->updateConnectedEditors();
    }
    else if ( changedField == &exportGridFilename )
    {
        QFileInfo info( exportGridFilename() );
        QDir      dirPath = info.absoluteDir();

        if ( exportParametersFilename() == defaultResultsFileName() )
        {
            exportParametersFilename = dirPath.absoluteFilePath( "RESULTS.GRDECL" );
        }
        if ( exportFaultsFilename() == defaultFaultsFileName() )
        {
            exportFaultsFilename = dirPath.absoluteFilePath( "FAULTS.GRDECL" );
        }
    }
    else if ( changedField == &exportParametersFilename )
    {
        QFileInfo info( exportParametersFilename() );
        QDir      dirPath = info.absoluteDir();

        if ( exportGridFilename() == defaultGridFileName() )
        {
            exportGridFilename = dirPath.absoluteFilePath( "GRID.GRDECL" );
        }
        if ( exportFaultsFilename() == defaultFaultsFileName() )
        {
            exportFaultsFilename = dirPath.absoluteFilePath( "FAULTS.GRDECL" );
        }
    }
    else if ( changedField == &exportFaultsFilename )
    {
        QFileInfo info( exportFaultsFilename() );
        QDir      dirPath = info.absoluteDir();

        if ( exportGridFilename() == defaultGridFileName() )
        {
            exportGridFilename = dirPath.absoluteFilePath( "GRID.GRDECL" );
        }
        if ( exportParametersFilename() == defaultResultsFileName() )
        {
            exportParametersFilename = dirPath.absoluteFilePath( "RESULTS.GRDECL" );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicExportEclipseSectorModelUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                          bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &selectedKeywords )
    {
        RigCaseCellResultsData*       resultData = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        QList<caf::PdmOptionItemInfo> allOptions =
            RimEclipseResultDefinition::calcOptionsForVariableUiFieldStandard( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                               resultData );

        std::set<QString> mainKeywords = this->mainKeywords();
        for ( caf::PdmOptionItemInfo option : allOptions )
        {
            if ( mainKeywords.count( option.optionUiText() ) )
            {
                if ( resultData->hasResultEntry(
                         RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, option.optionUiText() ) ) )
                {
                    options.push_back( option );
                }
            }
        }
        for ( caf::PdmOptionItemInfo option : allOptions )
        {
            if ( !mainKeywords.count( option.optionUiText() ) && option.optionUiText() != "None" )
            {
                if ( resultData->hasResultEntry(
                         RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, option.optionUiText() ) ) )
                {
                    if ( option.optionUiText() == "ACTNUM" && exportGrid() )
                    {
                        if ( exportParameters() != EXPORT_TO_GRID_FILE )
                            options.push_back( caf::PdmOptionItemInfo( "ACTNUM (included in Grid File)", "ACTNUM" ) );
                    }
                    else
                    {
                        options.push_back( option );
                    }
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &exportFaults )
    {
        std::set<ResultExportOptions> validFaultOptions = {EXPORT_NO_RESULTS,
                                                           EXPORT_TO_GRID_FILE,
                                                           EXPORT_TO_SINGLE_SEPARATE_FILE};
        if ( !exportGrid() ) validFaultOptions.erase( EXPORT_TO_GRID_FILE );
        for ( ResultExportOptions option : validFaultOptions )
        {
            options.push_back( caf::PdmOptionItemInfo( ResultExportOptionsEnum::uiText( option ), option ) );
        }
    }
    else if ( fieldNeedingOptions == &exportParameters )
    {
        std::set<ResultExportOptions> validFaultOptions = {EXPORT_NO_RESULTS,
                                                           EXPORT_TO_GRID_FILE,
                                                           EXPORT_TO_SINGLE_SEPARATE_FILE,
                                                           EXPORT_TO_SEPARATE_FILE_PER_RESULT};
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
    return {RiaDefines::eqlnumResultName(), "FIPNUM", "NTG", "PERMX", "PERMY", "PERMZ", "PORO", "PVTNUM", "SATNUM", "SWATINIT"};
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
        fallbackDirectory =
            RiaApplication::instance()->lastUsedDialogDirectoryWithFallback( "BINARY_GRID", generalFallback );
    }
    return RiaApplication::instance()->lastUsedDialogDirectoryWithFallback( "EXPORT_INPUT_GRID", fallbackDirectory );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::defaultGridFileName() const
{
    QDir baseDir( defaultFolder() );
    return baseDir.absoluteFilePath( "GRID.GRDECL" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::defaultResultsFileName() const
{
    QDir baseDir( defaultFolder() );
    return baseDir.absoluteFilePath( "RESULTS.GRDECL" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportEclipseSectorModelUi::defaultFaultsFileName() const
{
    QDir baseDir( defaultFolder() );
    return baseDir.absoluteFilePath( "FAULTS.GRDECL" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::applyBoundaryDefaults()
{
    if ( exportGridBox == ACTIVE_CELLS_BOX )
    {
        cvf::Vec3st minActive, maxActive;
        m_caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->IJKBoundingBox( minActive, maxActive );
        setMin( cvf::Vec3i( minActive ) );
        setMax( cvf::Vec3i( maxActive ) );
    }
    else if ( exportGridBox == VISIBLE_CELLS_BOX )
    {
        setMin( m_visibleMin );
        setMax( m_visibleMax );
    }
    else if ( exportGridBox == FULL_GRID_BOX )
    {
        const RigMainGrid* mainGrid = m_caseData->mainGrid();
        cvf::Vec3i         gridDimensions( int( mainGrid->cellCountI() - 1 ),
                                           int( mainGrid->cellCountJ() - 1 ),
                                           int( mainGrid->cellCountK() - 1 ) );

        setMin( cvf::Vec3i( 0, 0, 0 ) );
        setMax( gridDimensions );
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
    for ( QString keyword : selectedKeywords() )
    {
        if ( resultData->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, keyword ) ) )
        {
            validKeywords.push_back( keyword );
        }
    }
    selectedKeywords.v().swap( validKeywords );
}
