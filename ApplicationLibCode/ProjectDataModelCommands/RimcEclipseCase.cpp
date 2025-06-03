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

#include "RimcEclipseCase.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"

#include "RicImportSummaryCasesFeature.h"

#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSurfaceCollection.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_importProperties, "import_properties" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_importProperties::RimcEclipseCase_importProperties( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Import Properties", "", "", "Import Properties" );
    setNullptrValid( true );
    setResultPersistent( false );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileNames, "FileNames", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcEclipseCase_importProperties::execute()
{
    std::vector<QString> absolutePaths = m_fileNames;
    for ( auto& path : absolutePaths )
    {
        QFileInfo projectPathInfo( path );
        if ( !projectPathInfo.exists() )
        {
            QDir startDir( RiaApplication::instance()->startDir() );
            path = startDir.absoluteFilePath( path );
        }
    }

    QStringList propertyFileNames;
    std::copy( absolutePaths.begin(), absolutePaths.end(), std::back_inserter( propertyFileNames ) );

    auto eclipseCase = self<RimEclipseCase>();
    eclipseCase->importAsciiInputProperties( propertyFileNames );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseCase_importProperties::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcEclipseCase_importProperties::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcEclipseCase_importProperties::isNullptrValidResult_obsolete() const
{
    return true;
}
