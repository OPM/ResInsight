/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfSetStartDir.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfSetStartDir, "setStartDir" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetStartDir::RicfSetStartDir()
{
    CAF_PDM_InitScriptableField( &m_path, "path", QString(), "Path", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetStartDir::execute()
{
    QDir directory( m_path );
    if ( !directory.exists() )
    {
        QString error = QString( "Path does not exist: %1" ).arg( m_path );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }
    if ( !directory.isReadable() )
    {
        QString error = QString( "Path does not exist: %1" ).arg( m_path );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    RiaApplication::instance()->setStartDir( m_path );
    return caf::PdmScriptResponse();
}
