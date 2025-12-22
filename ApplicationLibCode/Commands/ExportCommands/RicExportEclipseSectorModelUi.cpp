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
#include "RigEclipseResultTools.h"
#include "RigMainGrid.h"

#include "ProjectDataModel/Jobs/RimKeywordBcprop.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "Tools/RimEclipseViewTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QDir>
#include <QFileInfo>

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

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportEclipseSectorModelUi::RicExportEclipseSectorModelUi()
    : m_visibleMin( caf::VecIjk0::ZERO )
    , m_visibleMax( caf::VecIjk0::ZERO )
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
    m_visibleWellsPadding.setRange( 0, 100 );

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
    refinementCountI.setRange( 1, 10 );
    CAF_PDM_InitField( &refinementCountJ, "RefinementCountJ", 1, "" );
    refinementCountJ.setRange( 1, 10 );
    CAF_PDM_InitField( &refinementCountK, "RefinementCountK", 1, "" );
    refinementCountK.setRange( 1, 10 );

    CAF_PDM_InitFieldNoDefault( &exportParameters, "ExportParams", "Export Parameters" );
    CAF_PDM_InitField( &m_exportParametersFilename, "ExportParamsFilename", QString(), "File Name" );

    CAF_PDM_InitFieldNoDefault( &selectedKeywords, "ExportMainKeywords", "Keywords to Export" );

    CAF_PDM_InitField( &m_writeEchoInGrdeclFiles,
                       "WriteEchoInGrdeclFiles",
                       RiaPreferences::current()->writeEchoInGrdeclFiles(),
                       "Write NOECHO and ECHO" );

    CAF_PDM_InitFieldNoDefault( &m_exportFolder, "ExportFolder", "Export Folder" );
    m_exportFolder = defaultFolder();

    m_exportGridFilename       = defaultGridFileName();
    m_exportParametersFilename = defaultResultsFileName();
    m_exportFaultsFilename     = defaultFaultsFileName();

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
void RicExportEclipseSectorModelUi::setCaseData( RigEclipseCaseData* caseData,
                                                 RimEclipseView*     eclipseView,
                                                 const caf::VecIjk0& visibleMin,
                                                 const caf::VecIjk0& visibleMax )
{
    m_caseData    = caseData;
    m_eclipseView = eclipseView;
    m_visibleMin  = visibleMin;
    m_visibleMax  = visibleMax;

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
caf::VecIjk0 RicExportEclipseSectorModelUi::min() const
{
    return caf::VecIjk0( minI() - 1, minJ() - 1, minK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::VecIjk0 RicExportEclipseSectorModelUi::max() const
{
    return caf::VecIjk0( maxI() - 1, maxJ() - 1, maxK() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setMin( const caf::VecIjk0& min )
{
    minI = static_cast<int>( min.x() ) + 1;
    minJ = static_cast<int>( min.y() ) + 1;
    minK = static_cast<int>( min.z() ) + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelUi::setMax( const caf::VecIjk0& max )
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
    if ( field == &m_exportParametersFilename || field == &m_exportGridFilename || field == &m_exportFaultsFilename )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
        {
            myAttr->m_selectSaveFileName  = true;
            myAttr->m_fileSelectionFilter = "GRDECL files (*.grdecl *.GRDECL);;All files (*.*)";
        }
    }
    else if ( field == &selectedKeywords )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute ) )
        {
            myAttr->heightHint = 280;
        }
    }

    if ( field == &m_exportFolder )
    {
        if ( auto* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
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
    // Update dynamic range limits before defining UI ordering
    if ( m_caseData && m_caseData->mainGrid() )
    {
        const cvf::Vec3st gridDimensions = m_caseData->mainGrid()->cellCounts();

        minI.setRange( 1, (int)gridDimensions.x() );
        minJ.setRange( 1, (int)gridDimensions.y() );
        minK.setRange( 1, (int)gridDimensions.z() );

        maxI.setRange( 1, (int)gridDimensions.x() );
        maxJ.setRange( 1, (int)gridDimensions.y() );
        maxK.setRange( 1, (int)gridDimensions.z() );
    }

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

        if ( exportGridBox() == RiaModelExportDefines::VISIBLE_WELLS_BOX )
        {
            gridBoxGroup->add( &m_visibleWellsPadding, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        }

        gridBoxGroup->add( &makeInvisibleCellsInactive, { .newRow = true, .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );

        const bool boxReadOnly = ( exportGridBox() != RiaModelExportDefines::MANUAL_SELECTION );
        minI.uiCapability()->setUiReadOnly( boxReadOnly );
        minJ.uiCapability()->setUiReadOnly( boxReadOnly );
        minK.uiCapability()->setUiReadOnly( boxReadOnly );
        maxI.uiCapability()->setUiReadOnly( boxReadOnly );
        maxJ.uiCapability()->setUiReadOnly( boxReadOnly );
        maxK.uiCapability()->setUiReadOnly( boxReadOnly );

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
    if ( exportGridBox == RiaModelExportDefines::ACTIVE_CELLS_BOX )
    {
        auto [minActive, maxActive] = m_caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->ijkBoundingBox();
        setMin( minActive );
        setMax( maxActive );
    }
    else if ( exportGridBox == RiaModelExportDefines::VISIBLE_CELLS_BOX )
    {
        setMin( m_visibleMin );
        setMax( m_visibleMax );
    }
    else if ( exportGridBox == RiaModelExportDefines::VISIBLE_WELLS_BOX )
    {
        auto [minWellCells, maxWellCells] = RimEclipseViewTools::computeVisibleWellCells( m_eclipseView, m_caseData, m_visibleWellsPadding() );
        setMin( minWellCells );
        setMax( maxWellCells );
    }
    else if ( exportGridBox == RiaModelExportDefines::FULL_GRID_BOX )
    {
        const RigMainGrid* mainGrid = m_caseData->mainGrid();
        setMin( caf::VecIjk0::ZERO );
        cvf::Vec3st maxCounts = mainGrid->cellCounts() - cvf::Vec3st( 1, 1, 1 );
        setMax( caf::VecIjk0( maxCounts.x(), maxCounts.y(), maxCounts.z() ) );
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
