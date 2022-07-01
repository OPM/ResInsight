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
#include "RicfImportFormationNames.h"

#include "RicImportFormationNamesFeature.h"

#include "RimCase.h"
#include "RimFormationNames.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicfImportFormationNames, "importFormationNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportFormationNames::RicfImportFormationNames()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_formationFiles, "formationFiles", "" );
    CAF_PDM_InitScriptableField( &m_applyToCaseId, "applyToCaseId", -1, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfImportFormationNames::execute()
{
    QStringList errorMessages, warningMessages;

    if ( !m_formationFiles().empty() )
    {
        QStringList formationFileList;
        for ( QString formationFile : m_formationFiles() )
        {
            if ( QFileInfo::exists( formationFile ) )
            {
                formationFileList.push_back( formationFile );
            }
            else
            {
                errorMessages.push_back( QString( "%1 does not exist" ).arg( formationFile ) );
            }
        }

        RimFormationNames* formationNames = RicImportFormationNamesFeature::importFormationFiles( formationFileList );
        if ( formationNames )
        {
            bool                  foundCase = false;
            std::vector<RimCase*> cases;
            RimProject::current()->allCases( cases );
            for ( RimCase* rimCase : cases )
            {
                if ( m_applyToCaseId() == -1 || ( rimCase->caseId() == m_applyToCaseId() ) )
                {
                    rimCase->setFormationNames( formationNames );
                    rimCase->updateFormationNamesData();
                    rimCase->updateConnectedEditors();
                    foundCase = true;
                }
            }
            if ( m_applyToCaseId() != -1 && !foundCase )
            {
                warningMessages << "Could not find the case to apply the formations to";
            }
        }
    }
    else
    {
        errorMessages << "No formation files provided";
    }

    caf::PdmScriptResponse response;
    for ( QString warningMessage : warningMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warningMessage );
    }
    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, errorMessage );
    }
    return response;
}
