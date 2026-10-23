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

#include "RicfExportProperty.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCommandForwarding.h"

#include "RimEclipseCase.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafUtils.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfExportProperty, "exportProperty" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportProperty::RicfExportProperty()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_timeStepIndex, "timeStep", -1, "Time Step Index" );
    CAF_PDM_InitScriptableField( &m_propertyName, "property", QString(), "Property Name" );
    CAF_PDM_InitScriptableField( &m_eclipseKeyword, "eclipseKeyword", QString(), "Eclipse Keyword" );
    CAF_PDM_InitScriptableField( &m_undefinedValue, "undefinedValue", 0.0, "Undefined Value" );
    CAF_PDM_InitScriptableField( &m_exportFileName, "exportFile", QString(), "Export FileName" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportProperty::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse case" ).arg( m_caseId() ), commandName );
    }

    // Resolve the default export file from the command file executor state. The Rimc method requires an explicit file.
    QString filePath = m_exportFileName();
    if ( filePath.isNull() )
    {
        QDir    propertiesDir( RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::PROPERTIES ) );
        QString fileName = QString( "%1-%2" ).arg( eclipseCase->caseUserDescription() ).arg( m_propertyName() );
        fileName         = caf::Utils::makeValidFileBasename( fileName );
        filePath         = propertiesDir.filePath( fileName );
    }

    RimEclipseCase_exportProperty method( eclipseCase );
    method.setTimeStep( m_timeStepIndex() );
    method.setPropertyName( m_propertyName() );
    method.setEclipseKeyword( m_eclipseKeyword() );
    method.setUndefinedValue( m_undefinedValue() );
    method.setExportFile( filePath );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
