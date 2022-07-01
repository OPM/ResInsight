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

#include "RicfReplaceCase.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

#include "RicfCommandFileExecutor.h"

#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicfSingleCaseReplace, "replaceCase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfSingleCaseReplace::RicfSingleCaseReplace()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_newGridFile, "newGridFile", QString(), "New Grid File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicfSingleCaseReplace::caseId() const
{
    return m_caseId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfSingleCaseReplace::filePath() const
{
    return m_newGridFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfSingleCaseReplace::execute()
{
    QString projectPath = RimProject::current()->fileName();
    if ( projectPath.isEmpty() )
    {
        QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
        if ( lastProjectPath.isNull() )
        {
            QString errMsg( "replaceCase: The project must be saved as a file before calling 'replaceCase'." );
            RiaLogging::error( errMsg );
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
        }
        projectPath = lastProjectPath;
    }

    cvf::ref<RiaProjectModifier> projectModifier = cvf::make_ref<RiaProjectModifier>();

    QString   filePath = m_newGridFile();
    QFileInfo casePathInfo( filePath );
    if ( !casePathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        filePath = startDir.absoluteFilePath( m_newGridFile() );
    }

    if ( m_caseId() < 0 )
    {
        projectModifier->setReplaceCaseFirstOccurrence( filePath );
    }
    else
    {
        projectModifier->setReplaceCase( m_caseId(), filePath );
    }

    if ( !RiaApplication::instance()->loadProject( projectPath,
                                                   RiaApplication::ProjectLoadAction::PLA_NONE,
                                                   projectModifier.p() ) )
    {
        QString errMsg( "Could not reload project" );
        RiaLogging::error( errMsg );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
    }

    return caf::PdmScriptResponse();
}

CAF_PDM_SOURCE_INIT( RicfMultiCaseReplace, "replaceMultipleCases" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfMultiCaseReplace::RicfMultiCaseReplace()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfMultiCaseReplace::setCaseReplacePairs( const std::map<int, QString>& caseIdToGridFileNameMap )
{
    m_caseIdToGridFileNameMap = caseIdToGridFileNameMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfMultiCaseReplace::execute()
{
    if ( m_caseIdToGridFileNameMap.empty() )
    {
        QString errMsg( "replaceCaseImpl: No replacements available." );
        RiaLogging::error( errMsg );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if ( lastProjectPath.isNull() )
    {
        QString errMsg( "replaceCase: 'openProject' must be called before 'replaceCase' to specify project file to "
                        "replace case in." );
        RiaLogging::error( errMsg );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
    }

    cvf::ref<RiaProjectModifier> projectModifier = cvf::make_ref<RiaProjectModifier>();
    for ( const auto& a : m_caseIdToGridFileNameMap )
    {
        const int caseId   = a.first;
        QString   filePath = a.second;
        QFileInfo casePathInfo( filePath );
        if ( !casePathInfo.exists() )
        {
            QDir startDir( RiaApplication::instance()->startDir() );
            filePath = startDir.absoluteFilePath( filePath );
        }

        if ( caseId < 0 )
        {
            projectModifier->setReplaceCaseFirstOccurrence( filePath );
        }
        else
        {
            projectModifier->setReplaceCase( caseId, filePath );
        }
    }

    if ( !RiaApplication::instance()->loadProject( lastProjectPath,
                                                   RiaApplication::ProjectLoadAction::PLA_NONE,
                                                   projectModifier.p() ) )
    {
        QString errMsg( "Could not reload project" );
        RiaLogging::error( errMsg );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
    }

    return caf::PdmScriptResponse();
}
