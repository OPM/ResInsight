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

#include "RicImportSummaryCasesFeature.h"

#include "RimFileSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSurfaceCollection.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_importSummaryCase, "importSummaryCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_importSummaryCase::RimProject_importSummaryCase( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Import Summary Case", "", "", "Import Summary Case" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimProject_importSummaryCase::execute()
{
    QStringList                  summaryFileNames{m_fileName};
    std::vector<RimSummaryCase*> newCases;

    if ( RicImportSummaryCasesFeature::createSummaryCasesFromFiles( summaryFileNames, &newCases ) )
    {
        RicImportSummaryCasesFeature::addSummaryCases( newCases );

        if ( RiaGuiApplication::isRunning() )
        {
            RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
            if ( mainPlotWindow && !newCases.empty() )
            {
                mainPlotWindow->updateSummaryPlotToolBar();
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
bool RimProject_importSummaryCase::resultIsPersistent() const
{
    return true;
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
    CAF_PDM_InitScriptableFieldNoDefault( &m_caseId, "CaseId", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimProject_summaryCase::execute()
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
bool RimProject_summaryCase::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimProject_summaryCase::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFileSummaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject_summaryCase::isNullptrValidResult() const
{
    return true;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimProject, RimProject_surfaceFolder, "surfaceFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimProject_surfaceFolder::RimProject_surfaceFolder( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Get Surface Folder", "", "", "Get Surface Folder" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_folderName, "FolderName", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimProject_surfaceFolder::execute()
{
    auto                  proj     = RimProject::current();
    RimSurfaceCollection* surfcoll = proj->activeOilField()->surfaceCollection();

    // Blank foldername parameter should return the topmost folder
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
bool RimProject_surfaceFolder::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimProject_surfaceFolder::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimSurfaceCollection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimProject_surfaceFolder::isNullptrValidResult() const
{
    return true;
}
