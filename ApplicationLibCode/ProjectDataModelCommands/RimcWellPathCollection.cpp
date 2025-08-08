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

#include "RimWellPath.h"
#include "RimWellPathCollection.h"

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
