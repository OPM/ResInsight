/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimcProject.h"
#include "RimcViewWindow.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "KeyValueStore/RiaKeyValueStoreUtil.h"
#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"
#include "RiaQStringFormatter.h"
#include "RiaRegressionTestRunner.h"
#include "RiaResultNames.h"

#include "ExportCommands/RicSnapshotAllPlotsToFileFeature.h"
#include "ExportCommands/RicSnapshotAllViewsToFileFeature.h"
#include "RicImportFormationNamesFeature.h"
#include "RicImportGeneralDataFeature.h"
#include "RicImportSummaryCasesFeature.h"
#include "ViewLink/RicLinkVisibleViewsFeature.h"
#include "ViewLink/RicUnLinkViewFeature.h"

#include "RifReaderSettings.h"

#include "Formations/RimFormationNames.h"
#include "Rim3dView.h"
#include "RimCalcScript.h"
#include "RimCase.h"
#include "RimCornerPointCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFileSummaryCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSurfaceCollection.h"
#include "RimTools.h"
#include "RimValveTemplateCollection.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

#include <memory>
#include <set>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_importSummaryCase, "importSummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_importSummaryCase::RimProject_importSummaryCase( caf::PdmObjectHandle* self )
    : PdmObjectCreationMethod( self )

{
    CAF_PDM_InitObject( "Import Summary Case", "", "", "Import Summary Case" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_importSummaryCase::execute()
{
    QString   absolutePath = m_fileName;
    QFileInfo projectPathInfo( absolutePath );
    if ( !projectPathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        absolutePath = startDir.absoluteFilePath( m_fileName );
    }

    QStringList summaryFileNames{ absolutePath };

    RiaEnsembleImportTools::CreateConfig createConfig{ .fileType = RiaDefines::FileType::SMSPEC, .ensembleOrGroup = false, .allowDialogs = false };
    auto newCases = RiaEnsembleImportTools::createSummaryCasesFromFiles( summaryFileNames, createConfig );
    if ( !newCases.empty() )
    {
        RicImportSummaryCasesFeature::addSummaryCases( newCases );

        if ( RiaGuiApplication::isRunning() )
        {
            RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
            if ( mainPlotWindow && !newCases.empty() )
            {
                mainPlotWindow->updateMultiPlotToolBar();
            }
        }

        if ( newCases.size() == 1 )
        {
            return newCases[0];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_importSummaryCase::classKeywordReturnedType() const
{
    return RimFileSummaryCase::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_summaryCase, "summaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_summaryCase::RimProject_summaryCase( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_VALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )

{
    CAF_PDM_InitObject( "Find Summary Case", "", "", "Find Summary Case" );

    CAF_PDM_InitScriptableField( &m_caseId, "CaseId", -1, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_summaryCase::execute()
{
    auto proj     = RimProject::current();
    auto sumCases = proj->allSummaryCases();

    for ( auto s : sumCases )
    {
        if ( s->caseId() == m_caseId ) return s;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_summaryCase::classKeywordReturnedType() const
{
    return RimFileSummaryCase::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_surfaceFolder, "surfaceFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_surfaceFolder::RimProject_surfaceFolder( caf::PdmObjectHandle* self )
    : PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Get Surface Folder", "", "", "Get Surface Folder" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_folderName, "FolderName", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_surfaceFolder::execute()
{
    auto                  proj     = RimProject::current();
    RimSurfaceCollection* surfcoll = proj->activeOilField()->surfaceCollection();

    // Blank folder name parameter should return the topmost folder
    if ( m_folderName().isEmpty() ) return surfcoll;

    for ( auto s : surfcoll->subCollections() )
    {
        if ( s->collectionName() == m_folderName() ) return s;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_surfaceFolder::classKeywordReturnedType() const
{
    return RimSurfaceCollection::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_createGridFromKeyValues, "createGridFromKeyValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_createGridFromKeyValues::RimProject_createGridFromKeyValues( caf::PdmObjectHandle* self )
    : PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create grid from key values", "", "", "Create Grid From Key Values" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nx, "Nx", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_ny, "Ny", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_nz, "Nz", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_coordKey, "CoordKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_zcornKey, "ZcornKey", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_actnumKey, "ActnumKey", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_createGridFromKeyValues::execute()
{
    RiaLogging::info( "Creating grid from key values" );

    QString name = m_name();
    if ( name.isEmpty() ) return std::unexpected( "Empty name not allowed" );

    int nx = m_nx();
    int ny = m_ny();
    int nz = m_nz();
    if ( nx <= 0 || ny <= 0 || nz <= 0 ) return std::unexpected( "Invalid grid size. nx, ny and nz must be positive." );

    RiaLogging::info( std::format( "Grid dimensions: [{} {} {}]", nx, ny, nz ) );
    RiaLogging::info( std::format( "Coord: {}", m_coordKey() ) );
    RiaLogging::info( std::format( "Zcorn: {}", m_zcornKey() ) );
    RiaLogging::info( std::format( "Actnum: {}", m_actnumKey() ) );

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    std::vector<float> coord  = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_coordKey().toStdString() ) );
    std::vector<float> zcorn  = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_zcornKey().toStdString() ) );
    std::vector<float> actnum = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_actnumKey().toStdString() ) );
    if ( coord.empty() || zcorn.empty() || actnum.empty() )
    {
        return std::unexpected( "Found unexcepted empty coord, zcorn or actnum array." );
    }

    RimProject* project = RimProject::current();
    if ( !project ) return std::unexpected( "Invalid project." );

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if ( !analysisModels ) return std::unexpected( "Missing analysis models." );

    auto result = RimCornerPointCase::createFromCoordinatesArray( nx, ny, nz, coord, zcorn, actnum );
    if ( !result.has_value() ) return result;

    RimCornerPointCase* grid = result.value();
    grid->setCustomCaseName( name );
    project->assignCaseIdToCase( grid );

    analysisModels->cases.push_back( grid );

    RimMainPlotCollection::current()->ensureDefaultFlowPlotsAreCreated();

    if ( RiaGuiApplication::isRunning() )
    {
        if ( RimEclipseView* riv = grid->createAndAddReservoirView() )
        {
            riv->loadDataAndUpdate();

            if ( !riv->cellResult()->hasResult() )
            {
                riv->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
            }

            analysisModels->updateConnectedEditors();

            if ( RiuMainWindow::instance() ) RiuMainWindow::instance()->selectAsCurrentItem( riv->cellResult() );
        }
    }

    keyValueStore->remove( m_coordKey().toStdString() );
    keyValueStore->remove( m_zcornKey().toStdString() );
    keyValueStore->remove( m_actnumKey().toStdString() );

    return grid;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_createGridFromKeyValues::classKeywordReturnedType() const
{
    return RimCornerPointCase::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_wellPathCollection, "wellPathCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_wellPathCollection::RimProject_wellPathCollection( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "Get Well Path Collection", "", "", "Get Well Path Collection" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_wellPathCollection::execute()
{
    auto wellPathCollection = RimWellPathCollection::instance();
    if ( !wellPathCollection )
    {
        return std::unexpected( "No well path collection found." );
    }

    return wellPathCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_wellPathCollection::classKeywordReturnedType() const
{
    return RimWellPathCollection::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_valveTemplates, "valveTemplates" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_valveTemplates::RimProject_valveTemplates( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "Get Valve Template Collection", "", "", "Get Valve Template Collection" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_valveTemplates::execute()
{
    auto valveTemplateCollection = RimTools::valveTemplateCollection();
    if ( !valveTemplateCollection )
    {
        return std::unexpected( "No valve template collection found." );
    }

    return valveTemplateCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_valveTemplates::classKeywordReturnedType() const
{
    return RimValveTemplateCollection::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_tileViews, "tileViews" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_tileViews::RimProject_tileViews( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Tile Views", "", "", "Tile all visible 3D view windows" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_tileViews::execute()
{
    if ( !RiaGuiApplication::isRunning() )
    {
        return std::unexpected( "Tiling views requires ResInsight to run with the graphical user interface." );
    }

    RiuMainWindow* mainWindow = RiuMainWindow::instance();
    if ( !mainWindow ) return std::unexpected( "No 3D main window is available." );

    mainWindow->tileViewWindows();
    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_linkViews, "linkViews" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_linkViews::RimProject_linkViews( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Link Views", "", "", "Link the specified 3D views" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_views, "Views", "Views to link" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_linkViews::execute()
{
    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    std::set<Rim3dView*>    seenViews;
    std::vector<Rim3dView*> uniqueViews;

    // Use the set only for duplicate detection. The vector preserves input order because the first
    // unlinked view becomes the master view when a new view linker is created.
    for ( Rim3dView* view : m_views.ptrReferencedObjectsByType() )
    {
        auto insertResult = seenViews.insert( view );
        if ( insertResult.second ) uniqueViews.push_back( view );
    }

    if ( uniqueViews.size() < 2 ) return std::unexpected( "At least two unique views are required." );

    std::vector<Rim3dView*> linkableViews;
    for ( Rim3dView* view : uniqueViews )
    {
        if ( !view->assosiatedViewLinker() ) linkableViews.push_back( view );
    }

    RicLinkVisibleViewsFeature::linkViews( linkableViews );
    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_unlinkViews, "unlinkViews" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_unlinkViews::RimProject_unlinkViews( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Unlink Views", "", "", "Unlink the specified 3D views" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_views, "Views", "Views to unlink" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_unlinkViews::execute()
{
    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    std::set<Rim3dView*>    seenViews;
    std::vector<Rim3dView*> uniqueViews;
    for ( Rim3dView* view : m_views.ptrReferencedObjectsByType() )
    {
        auto insertResult = seenViews.insert( view );
        if ( insertResult.second ) uniqueViews.push_back( view );
    }

    if ( uniqueViews.empty() ) return std::unexpected( "At least one view is required." );

    RicUnLinkViewFeature::unlinkViews( uniqueViews );
    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_exportSnapshots, "exportSnapshots" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_exportSnapshots::RimProject_exportSnapshots( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Snapshots", "", "", "Export snapshots of all 3D views and/or plots in the project" );

    CAF_PDM_InitScriptableField( &m_contentType,
                                 "ContentType",
                                 RiaDefines::SnapshotContentType::ALL,
                                 "Content Type",
                                 "",
                                 "",
                                 "Export 3D views, plots or both" );
    CAF_PDM_InitScriptableField( &m_exportFolder,
                                 "ExportFolder",
                                 QString(),
                                 "Export Folder",
                                 "",
                                 "",
                                 "Folder to export to. Defaults to the 'snapshots' folder next to the project file." );
    CAF_PDM_InitScriptableField( &m_prefix, "Prefix", QString(), "Prefix", "", "", "Prefix for the generated file names" );
    CAF_PDM_InitScriptableField( &m_width, "Width", -1, "Width", "", "", "Image width in pixels. Use -1 for the current size." );
    CAF_PDM_InitScriptableField( &m_height, "Height", -1, "Height", "", "", "Image height in pixels. Use -1 for the current size." );
    CAF_PDM_InitScriptableField( &m_plotFileFormat,
                                 "PlotFileFormat",
                                 RiaDefines::SnapshotFileFormat::PNG,
                                 "Plot File Format",
                                 "",
                                 "",
                                 "Output file format for plots. 3D views are always exported as PNG." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_exportSnapshots::setContentType( RiaDefines::SnapshotContentType contentType )
{
    m_contentType = contentType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_exportSnapshots::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_exportSnapshots::setPrefix( const QString& prefix )
{
    m_prefix = prefix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_exportSnapshots::setWidth( int width )
{
    m_width = width;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_exportSnapshots::setHeight( int height )
{
    m_height = height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_exportSnapshots::setPlotFileFormat( RiaDefines::SnapshotFileFormat fileFormat )
{
    m_plotFileFormat = fileFormat;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_exportSnapshots::execute()
{
    if ( !RiaGuiApplication::isRunning() ) return std::unexpected( "Snapshots cannot be exported without a GUI." );

    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    const QString exportFolder = RimViewWindow_exportSnapshot::resolveExportFolder( m_exportFolder() );

    int width  = m_width();
    int height = m_height();
    if ( RiaRegressionTestRunner::instance()->isRunningRegressionTests() )
    {
        QSize defaultSize = RiaRegressionTestRunner::regressionDefaultImageSize();
        width             = defaultSize.width();
        height            = defaultSize.height();
    }

    const bool exportViews = m_contentType() != RiaDefines::SnapshotContentType::PLOTS;
    const bool exportPlots = m_contentType() != RiaDefines::SnapshotContentType::VIEWS;

    if ( exportViews )
    {
        RicSnapshotAllViewsToFileFeature::exportSnapshotOfViewsIntoFolder( exportFolder, width, height, m_prefix() );
    }

    if ( exportPlots )
    {
        const bool    activateWidget = !RiaRegressionTestRunner::instance()->isRunningRegressionTests();
        const QString fileSuffix     = m_plotFileFormat() == RiaDefines::SnapshotFileFormat::PDF ? ".pdf" : ".png";

        RicSnapshotAllPlotsToFileFeature::exportSnapshotOfPlotsIntoFolder( exportFolder, width, height, activateWidget, m_prefix(), -1, fileSuffix );
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_loadCase, "loadCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_loadCase::RimProject_loadCase( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Load Case", "", "", "Load a grid case from file and add it to the project" );

    CAF_PDM_InitScriptableField( &m_path, "Path", QString(), "Path", "", "", "Path to the grid file (EGRID, GRID, GRDECL or ROFF)" );
    CAF_PDM_InitScriptableField( &m_gridOnly, "GridOnly", false, "Grid Only", "", "", "Load the grid geometry only, without results" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_loadCase::setPath( const QString& path )
{
    m_path = path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_loadCase::setGridOnly( bool gridOnly )
{
    m_gridOnly = gridOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_loadCase::execute()
{
    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    if ( m_path().isEmpty() ) return std::unexpected( "No path specified." );

    QString   absolutePath = m_path();
    QFileInfo pathInfo( absolutePath );
    if ( !pathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        absolutePath = startDir.absoluteFilePath( m_path() );
    }

    RifReaderSettings readerSettings = m_gridOnly() ? RiaPreferencesGrid::gridOnlyReaderSettings()
                                                    : RiaPreferencesGrid::current()->readerSettings();

    const bool createPlot = false;
    const bool createView = false;
    auto       fileOpenMetaData =
        RicImportGeneralDataFeature::openEclipseFilesFromFileNames( QStringList{ absolutePath }, createPlot, createView, readerSettings );

    if ( fileOpenMetaData.createdCaseIds.empty() )
    {
        return std::unexpected( QString( "Unable to load case from %1" ).arg( absolutePath ) );
    }

    const int caseId = fileOpenMetaData.createdCaseIds.front();
    for ( RimCase* rimCase : project->allGridCases() )
    {
        if ( rimCase && rimCase->caseId() == caseId ) return rimCase;
    }

    return std::unexpected( QString( "Case loaded from %1 was not found in the project" ).arg( absolutePath ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_loadCase::classKeywordReturnedType() const
{
    return RimCase::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_createGridCaseGroup, "createGridCaseGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_createGridCaseGroup::RimProject_createGridCaseGroup( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Grid Case Group", "", "", "Create a grid case group from a list of grid files with identical grids" );

    CAF_PDM_InitScriptableField( &m_casePaths, "CasePaths", std::vector<QString>(), "Case Paths", "", "", "Paths to the grid files" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_createGridCaseGroup::setCasePaths( const std::vector<QString>& casePaths )
{
    m_casePaths = casePaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_createGridCaseGroup::execute()
{
    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    if ( m_casePaths().empty() ) return std::unexpected( "No case paths specified." );

    QStringList casePaths;
    for ( QString casePath : m_casePaths() )
    {
        QFileInfo casePathInfo( casePath );
        if ( !casePathInfo.exists() )
        {
            QDir startDir( RiaApplication::instance()->startDir() );
            casePath = startDir.absoluteFilePath( casePath );
        }
        casePaths.push_back( casePath );
    }

    RimIdenticalGridCaseGroup* caseGroup = nullptr;
    if ( !RiaImportEclipseCaseTools::addEclipseCases( casePaths, &caseGroup ) || !caseGroup )
    {
        return std::unexpected( "Could not load grid case group" );
    }

    return caseGroup;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_createGridCaseGroup::classKeywordReturnedType() const
{
    return RimIdenticalGridCaseGroup::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_importFormationNames, "importFormationNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_importFormationNames::RimProject_importFormationNames( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Import Formation Names", "", "", "Import formation names from files and return the formation names object" );

    CAF_PDM_InitScriptableField( &m_formationFiles,
                                 "FormationFiles",
                                 std::vector<QString>(),
                                 "Formation Files",
                                 "",
                                 "",
                                 "Formation files to import (.lyr, .fmu, ...)" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_importFormationNames::setFormationFiles( const std::vector<QString>& formationFiles )
{
    m_formationFiles = formationFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_importFormationNames::execute()
{
    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    if ( m_formationFiles().empty() ) return std::unexpected( "No formation files provided" );

    QStringList formationFileList;
    QStringList missingFiles;
    for ( const QString& formationFile : m_formationFiles() )
    {
        if ( QFileInfo::exists( formationFile ) )
        {
            formationFileList.push_back( formationFile );
        }
        else
        {
            missingFiles.push_back( formationFile );
        }
    }

    if ( !missingFiles.empty() ) return std::unexpected( QString( "%1 does not exist" ).arg( missingFiles.join( ", " ) ) );

    RimFormationNames* formationNames = RicImportFormationNamesFeature::importFormationFiles( formationFileList );
    if ( !formationNames ) return std::unexpected( "Could not import formation names" );

    return formationNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimProject_importFormationNames::classKeywordReturnedType() const
{
    return RimFormationNames::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_runOctaveScript, "runOctaveScript" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_runOctaveScript::RimProject_runOctaveScript( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Run Octave Script", "", "", "Run an Octave script once per case. Empty case list means all Eclipse cases." );

    CAF_PDM_InitScriptableField( &m_path, "Path", QString(), "Path", "", "", "Path to the Octave script" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_cases, "Cases", "Cases", "", "", "Cases to run the script for. Empty means all Eclipse cases." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_runOctaveScript::setPath( const QString& path )
{
    m_path = path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimProject_runOctaveScript::setCases( const std::vector<RimCase*>& cases )
{
    m_cases.setValue( cases );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimProject_runOctaveScript::execute()
{
    auto* project = self<RimProject>();
    if ( !project ) return std::unexpected( "No project is available." );

    if ( m_path().isEmpty() ) return std::unexpected( "No script path specified." );
    if ( !QFileInfo::exists( m_path() ) ) return std::unexpected( QString( "Script %1 does not exist" ).arg( m_path() ) );

    std::vector<int> caseIds;
    for ( RimCase* rimCase : m_cases.ptrReferencedObjectsByType() )
    {
        if ( rimCase ) caseIds.push_back( rimCase->caseId() );
    }
    if ( caseIds.empty() )
    {
        for ( RimEclipseCase* eclipseCase : project->eclipseCases() )
        {
            caseIds.push_back( eclipseCase->caseId() );
        }
    }

    RiaApplication* app              = RiaApplication::instance();
    QString         octavePath       = app->octavePath();
    QStringList     processArguments = RimCalcScript::createCommandLineArguments( m_path() );

    bool ok = false;
    if ( caseIds.empty() )
    {
        ok = app->launchProcess( octavePath, processArguments, app->octaveProcessEnvironment() );
    }
    else
    {
        ok = app->launchProcessForMultipleCases( octavePath, processArguments, caseIds, app->octaveProcessEnvironment() );
    }

    if ( !ok ) return std::unexpected( QString( "Could not execute script %1" ).arg( m_path() ) );

    app->waitForProcess();
    return nullptr;
}
