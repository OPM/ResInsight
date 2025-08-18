/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimcWellPathCollection.h"

#include "CompletionData/RimCompletionData.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "Tools/RimTemporaryObjectCollection.h"

#include "WellPathCommands/RicImportWellPaths.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimcWellPathCollection_importWellPath, "ImportWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCollection_importWellPath::RimcWellPathCollection_importWellPath( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Import Well Path" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "File Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCollection_importWellPath::execute()
{
    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Well path collection is null. Cannot add well path." ) );
    }

    if ( m_fileName().isEmpty() )
    {
        return std::unexpected( QString( "File name is empty. Cannot add well path." ) );
    }

    QStringList               errorMessages;
    std::vector<RimWellPath*> importedWellPaths = wellPathCollection->addWellPaths( { m_fileName() }, &errorMessages );
    if ( importedWellPaths.empty() )
    {
        if ( !errorMessages.empty() )
        {
            return std::unexpected( errorMessages.join( "\n" ) );
        }
        else
        {
            return std::unexpected( QString( "No well paths were imported from file '%1'." ).arg( m_fileName() ) );
        }
    }

    return importedWellPaths.front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPathCollection_importWellPath::classKeywordReturnedType() const
{
    return RimWellPath::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPathCollection, RimcWellPathCollection_wellCompletions, "WellCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPathCollection_wellCompletions::RimcWellPathCollection_wellCompletions( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "Well Completions" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellName, "WellName", "Name of Well" );
    CAF_PDM_InitScriptableField( &m_caseId, "eclipseCaseId", -1, "Eclipse Case to extract data from" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPathCollection_wellCompletions::execute()
{
    auto wellPathCollection = self<RimWellPathCollection>();
    if ( !wellPathCollection )
    {
        return std::unexpected( QString( "Well path collection is missing. Cannot get completion data." ) );
    }

    if ( m_wellName().isEmpty() )
    {
        return std::unexpected( QString( "Well name is empty. Nothing to do." ) );
    }

    if ( m_caseId() < 0 )
    {
        return std::unexpected( QString( "Eclipse case is not set. Cannot get completion data." ) );
    }

    RimEclipseCase* eCase = RimProject::current()->eclipseCaseFromCaseId( m_caseId() );
    if ( eCase == nullptr )
    {
        return std::unexpected( QString( "Eclipse case is not set. Cannot get completion data." ) );
    }

    if ( auto wellPath = wellPathCollection->wellPathByName( m_wellName ) )
    {
        if ( auto modWellPath = dynamic_cast<RimModeledWellPath*>( wellPath ) )
        {
            if ( auto completionData = modWellPath->completionData( eCase ) )
            {
                RimTemporaryObjectCollection::instance()->addTemporaryObject( completionData );

                return completionData;
            }
            else
            {
                return std::unexpected( QString( "Well path '%1' does not have any completion data." ).arg( m_wellName ) );
            }
        }
    }
    return std::unexpected( QString( "Modeled Well path with name '%1' does not exist. Cannot get completion data." ).arg( m_wellName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPathCollection_wellCompletions::classKeywordReturnedType() const
{
    return RimCompletionData::classKeywordStatic();
}
