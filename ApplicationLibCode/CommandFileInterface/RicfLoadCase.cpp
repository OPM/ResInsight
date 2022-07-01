/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2019 Statoil ASA
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicfLoadCase.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RicImportGeneralDataFeature.h"

#include "RifReaderSettings.h"
#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RicfLoadCaseResult, "loadCaseResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfLoadCaseResult::RicfLoadCaseResult( int caseId )
{
    CAF_PDM_InitObject( "case_result" );
    CAF_PDM_InitField( &this->caseId, "id", caseId, "" );
}

CAF_PDM_SOURCE_INIT( RicfLoadCase, "loadCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfLoadCase::RicfLoadCase()
{
    CAF_PDM_InitScriptableField( &m_path, "path", QString(), "Path to Case File" );
    CAF_PDM_InitScriptableField( &m_gridOnly, "gridOnly", false, "Load Grid Data Only" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfLoadCase::execute()
{
    QString   absolutePath = m_path;
    QFileInfo projectPathInfo( absolutePath );
    if ( !projectPathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        absolutePath = startDir.absoluteFilePath( m_path );
    }

    std::shared_ptr<RifReaderSettings> readerSettings;
    if ( m_gridOnly ) readerSettings = RifReaderSettings::createGridOnlyReaderSettings();

    bool createPlot       = false;
    bool createView       = false;
    auto fileOpenMetaData = RicImportGeneralDataFeature::openEclipseFilesFromFileNames( QStringList{ absolutePath },
                                                                                        createPlot,
                                                                                        createView,
                                                                                        readerSettings );

    if ( fileOpenMetaData.createdCaseIds.empty() )
    {
        QString error = QString( "loadCase: Unable to load case from %1" ).arg( absolutePath );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    caf::PdmScriptResponse response;
    response.setResult( new RicfLoadCaseResult( fileOpenMetaData.createdCaseIds.front() ) );
    return response;
}
