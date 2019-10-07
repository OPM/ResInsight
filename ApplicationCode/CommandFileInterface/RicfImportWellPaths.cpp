/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RicfImportWellPaths.h"

#include "RiaApplication.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RicfImportWellPathResults, "importWellPathResults" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellPathResults::RicfImportWellPathResults( const std::vector<QString>& wellPathNames )
{
    CAF_PDM_InitObject( "well_path_results", "", "", "" );
    CAF_PDM_InitField( &importedWellPathNames, "importedWellPathNames", wellPathNames, "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfImportWellPaths, "importWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellPaths::RicfImportWellPaths()
{
    RICF_InitFieldNoDefault( &m_wellFilePaths, "wellFilePaths", "", "", "", "" );
}

RicfCommandResponse RicfImportWellPaths::execute()
{
    return RicfCommandResponse();
}
