/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RicfEnableDataSourceAsComment.h"

#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfEnableDataSourceAsComment, "enableDataSourceAsComment" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfEnableDataSourceAsComment::RicfEnableDataSourceAsComment()
{
    CAF_PDM_InitScriptableField( &m_enableDataSourceAsComment,
                                 "enableDataSourceAsComment",
                                 true,
                                 "Enable Data Source as Comment",
                                 "",
                                 "",
                                 "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfEnableDataSourceAsComment::execute()
{
    RicfCommandFileExecutor* executor = RicfCommandFileExecutor::instance();
    executor->setExportDataSouceAsComment( m_enableDataSourceAsComment );

    return caf::PdmScriptResponse();
}
