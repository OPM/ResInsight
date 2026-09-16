/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcCase.h"

#include "RiaApplication.h"
#include "RiaProjectModifier.h"

#include "RimCase.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimCase, RimCase_replaceGrid, "replaceGrid" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase_replaceGrid::RimCase_replaceGrid( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Replace Grid", "", "", "Replace the grid file of the case and reload the project" );

    CAF_PDM_InitScriptableField( &m_newGridFile, "NewGridFile", QString(), "New Grid File", "", "Path to the new grid file (EGRID, GRID, GRDECL or ODB)" );
    CAF_PDM_InitScriptableField( &m_projectFile,
                                 "ProjectFile",
                                 QString(),
                                 "Project File",
                                 "",
                                 "Optional project file to reload. Defaults to the current project file." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase_replaceGrid::setNewGridFile( const QString& newGridFile )
{
    m_newGridFile = newGridFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCase_replaceGrid::setProjectFile( const QString& projectFile )
{
    m_projectFile = projectFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimCase_replaceGrid::execute()
{
    auto* rimCase = self<RimCase>();
    if ( !rimCase ) return std::unexpected( "No case is available." );

    if ( m_newGridFile().isEmpty() ) return std::unexpected( "No new grid file specified." );

    QString projectPath = m_projectFile();
    if ( projectPath.isEmpty() )
    {
        RimProject* project = RimProject::current();
        if ( project ) projectPath = project->fileName();
    }

    if ( projectPath.isEmpty() )
    {
        return std::unexpected( "The project must be saved as a file before replacing the grid of a case." );
    }

    QString   filePath = m_newGridFile();
    QFileInfo casePathInfo( filePath );
    if ( !casePathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        filePath = startDir.absoluteFilePath( m_newGridFile() );
    }

    cvf::ref<RiaProjectModifier> projectModifier = cvf::make_ref<RiaProjectModifier>();
    projectModifier->setReplaceCase( rimCase->caseId(), filePath );

    if ( !RiaApplication::instance()->loadProject( projectPath, RiaApplication::ProjectLoadAction::PLA_NONE, projectModifier.p() ) )
    {
        return std::unexpected( "Could not reload project" );
    }

    return nullptr;
}
