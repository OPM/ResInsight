/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimcIdenticalGridCaseGroup.h"

#include "RiaApplication.h"
#include "RiaProjectModifier.h"

#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimIdenticalGridCaseGroup, RimcIdenticalGridCaseGroup_createStatisticsCase, "create_statistics_case" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcIdenticalGridCaseGroup_createStatisticsCase::RimcIdenticalGridCaseGroup_createStatisticsCase( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )

{
    CAF_PDM_InitObject( "Create Statistics Case", "", "", "Create a new statistics case in the grid case group" );

    CAF_PDM_InitScriptableField( &m_populateResultSelection,
                                 "PopulateResultSelection",
                                 false,
                                 "Populate Result Selection",
                                 "",
                                 "",
                                 "Select all available source properties for statistics. When false, no properties are selected "
                                 "and set_source_properties() must be called before compute_statistics()." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimcIdenticalGridCaseGroup_createStatisticsCase::setPopulateResultSelection( bool populate )
{
    m_populateResultSelection = populate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcIdenticalGridCaseGroup_createStatisticsCase::execute()
{
    auto gridCaseGroup = self<RimIdenticalGridCaseGroup>();
    if ( !gridCaseGroup ) return std::unexpected( "No grid case group is available." );

    RimEclipseStatisticsCase* statCase = m_populateResultSelection() ? gridCaseGroup->createAndAppendStatisticsCase()
                                                                     : gridCaseGroup->createAndAppendEmptyStatisticsCase();
    if ( !statCase ) return std::unexpected( "Could not create statistics case." );

    RimProject::current()->assignCaseIdToCase( statCase );
    gridCaseGroup->updateConnectedEditors();

    return statCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcIdenticalGridCaseGroup_createStatisticsCase::classKeywordReturnedType() const
{
    return RimEclipseStatisticsCase::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimIdenticalGridCaseGroup, RimIdenticalGridCaseGroup_replaceSourceCases, "replaceSourceCases" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup_replaceSourceCases::RimIdenticalGridCaseGroup_replaceSourceCases( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Replace Source Cases", "", "", "Replace all source cases of the group with the given grid files and reload the project" );

    CAF_PDM_InitScriptableField( &m_gridFiles, "GridFiles", std::vector<QString>(), "Grid Files", "", "", "Paths to the new grid files" );
    CAF_PDM_InitScriptableField( &m_projectFile,
                                 "ProjectFile",
                                 QString(),
                                 "Project File",
                                 "",
                                 "",
                                 "Optional project file to reload. Defaults to the current project file." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup_replaceSourceCases::setGridFiles( const std::vector<QString>& gridFiles )
{
    m_gridFiles = gridFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup_replaceSourceCases::setProjectFile( const QString& projectFile )
{
    m_projectFile = projectFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimIdenticalGridCaseGroup_replaceSourceCases::execute()
{
    auto* caseGroup = self<RimIdenticalGridCaseGroup>();
    if ( !caseGroup ) return std::unexpected( "No grid case group is available." );

    if ( m_gridFiles().empty() ) return std::unexpected( "No grid files specified." );

    QString projectPath = m_projectFile();
    if ( projectPath.isEmpty() )
    {
        RimProject* project = RimProject::current();
        if ( project ) projectPath = project->fileName();
    }

    if ( projectPath.isEmpty() )
    {
        return std::unexpected( "The project must be saved as a file before replacing the source cases of a group." );
    }

    cvf::ref<RiaProjectModifier> projectModifier = cvf::make_ref<RiaProjectModifier>();
    projectModifier->setReplaceSourceCasesById( caseGroup->groupId(), m_gridFiles() );

    if ( !RiaApplication::instance()->loadProject( projectPath, RiaApplication::ProjectLoadAction::PLA_CALCULATE_STATISTICS, projectModifier.p() ) )
    {
        return std::unexpected( "Could not reload project" );
    }

    return nullptr;
}
