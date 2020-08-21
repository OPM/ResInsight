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

#include "RicfSetExportFolder.h"

#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfSetExportFolder, "setExportFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSetExportFolder::RicfSetExportFolder()
{
    // clang-format off
    CAF_PDM_InitScriptableField(&m_type,  "type",  RicfCommandFileExecutor::ExportTypeEnum(RicfCommandFileExecutor::ExportType::COMPLETIONS), "Type",  "", "", "");
    CAF_PDM_InitScriptableField(&m_path,  "path",  QString(),                                                                     "Path",  "", "", "");
    CAF_PDM_InitScriptableField(&m_createFolder, "createFolder", false, "Create Folder", "", "", "");
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSetExportFolder::execute()
{
    if ( m_createFolder )
    {
        QDir dir;

        if ( !dir.exists( m_path ) )
        {
            dir.mkpath( m_path );

            if ( !dir.exists( m_path ) )
            {
                QString error = QString( "Could not create folder : %1" ).arg( m_path );
                RiaLogging::error( error );
                return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
            }
        }
    }

    RicfCommandFileExecutor* executor = RicfCommandFileExecutor::instance();
    executor->setExportPath( m_type(), m_path );
    return caf::PdmScriptResponse();
}
