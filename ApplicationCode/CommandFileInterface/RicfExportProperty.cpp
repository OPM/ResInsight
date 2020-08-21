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

#include "RiaLogging.h"

#include "ExportCommands/RicEclipseCellResultToFileImpl.h"
#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "RifEclipseInputFileTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafUtils.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RicfExportProperty, "exportProperty" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportProperty::RicfExportProperty()
{
    // clang-format off
    CAF_PDM_InitScriptableField(&m_caseId,           "caseId",           -1, "Case ID", "", "", "");
    CAF_PDM_InitScriptableField(&m_timeStepIndex,    "timeStep",         -1, "Time Step Index", "", "", "");
    CAF_PDM_InitScriptableField(&m_propertyName,     "property",         QString(), "Property Name", "", "", "");
    CAF_PDM_InitScriptableField(&m_eclipseKeyword,   "eclipseKeyword",   QString(), "Eclipse Keyword", "", "", "");
    CAF_PDM_InitScriptableField(&m_undefinedValue,   "undefinedValue",   0.0, "Undefined Value", "", "", "");
    CAF_PDM_InitScriptableField(&m_exportFileName,   "exportFile",       QString(), "Export FileName", "", "", "");
    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportProperty::execute()
{
    using TOOLS = RicfApplicationTools;

    RimEclipseCase* eclipseCase = TOOLS::caseFromId( m_caseId() );
    {
        if ( !eclipseCase )
        {
            QString error = QString( "exportProperty: Could not find case with ID %1" ).arg( m_caseId() );
            RiaLogging::error( error );
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
        }

        if ( !eclipseCase->eclipseCaseData() )
        {
            if ( !eclipseCase->openReserviorCase() )
            {
                QString error = QString( "exportProperty: Could not find eclipseCaseData with ID %1" ).arg( m_caseId() );
                RiaLogging::error( error );
                return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
            }
        }
    }

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    RigCaseCellResultsData* cellResultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    if ( !cellResultsData->ensureKnownResultLoaded( RigEclipseResultAddress( m_propertyName ) ) )
    {
        QString error = QString( "exportProperty: Could not find result property : %1" ).arg( m_propertyName() );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    QString filePath = m_exportFileName;
    if ( filePath.isNull() )
    {
        QDir propertiesDir(
            RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::PROPERTIES ) );
        QString fileName = QString( "%1-%2" ).arg( eclipseCase->caseUserDescription() ).arg( m_propertyName );
        fileName         = caf::Utils::makeValidFileBasename( fileName );
        filePath         = propertiesDir.filePath( fileName );
    }

    QString eclipseKeyword = m_eclipseKeyword;
    if ( eclipseKeyword.isNull() )
    {
        eclipseKeyword = m_propertyName;
    }

    QString errMsg;
    if ( !RicEclipseCellResultToFileImpl::writePropertyToTextFile( filePath,
                                                                   eclipseCase->eclipseCaseData(),
                                                                   m_timeStepIndex,
                                                                   m_propertyName,
                                                                   eclipseKeyword,
                                                                   m_undefinedValue,
                                                                   &errMsg ) )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
    }

    return caf::PdmScriptResponse();
}
