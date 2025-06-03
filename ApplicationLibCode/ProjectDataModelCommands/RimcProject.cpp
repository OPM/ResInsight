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

#include "Ensemble/RiaEnsembleImportTools.h"
#include "KeyValueStore/RiaKeyValueStoreUtil.h"
#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicImportSummaryCasesFeature.h"

#include "RimCornerPointCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFileSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSurfaceCollection.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_importSummaryCase, "importSummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_importSummaryCase::RimProject_importSummaryCase( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Import Summary Case", "", "", "Import Summary Case" );
    setNullptrValid( true );
    setResultPersistent( true );

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
std::unique_ptr<caf::PdmObjectHandle> RimProject_importSummaryCase::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFileSummaryCase );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_summaryCase, "summaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_summaryCase::RimProject_summaryCase( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Find Summary Case", "", "", "Find Summary Case" );
    setNullptrValid( true );
    setResultPersistent( true );

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
std::unique_ptr<caf::PdmObjectHandle> RimProject_summaryCase::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFileSummaryCase );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_surfaceFolder, "surfaceFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_surfaceFolder::RimProject_surfaceFolder( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Get Surface Folder", "", "", "Get Surface Folder" );
    setNullptrValid( true );
    setResultPersistent( true );

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
std::unique_ptr<caf::PdmObjectHandle> RimProject_surfaceFolder::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimSurfaceCollection );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_createGridFromKeyValues, "createGridFromKeyValues" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_createGridFromKeyValues::RimProject_createGridFromKeyValues( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create grid from key values", "", "", "Create Grid From Key Values" );
    setNullptrValid( true );
    setResultPersistent( true );

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

    RiaLogging::info( QString( "Grid dimensions: [%1 %2 %3]" ).arg( nx ).arg( ny ).arg( nz ) );
    RiaLogging::info( QString( "Coord: %1" ).arg( m_coordKey() ) );
    RiaLogging::info( QString( "Zcorn: %1" ).arg( m_zcornKey() ) );
    RiaLogging::info( QString( "Actnum: %1" ).arg( m_actnumKey() ) );

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
std::unique_ptr<caf::PdmObjectHandle> RimProject_createGridFromKeyValues::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimCornerPointCase );
}
