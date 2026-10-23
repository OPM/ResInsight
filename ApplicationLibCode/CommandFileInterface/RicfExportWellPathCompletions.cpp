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

#include "RicfExportWellPathCompletions.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCommandForwarding.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimEclipseCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicfExportWellPathCompletions, "exportWellPathCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellPathCompletions::RicfExportWellPathCompletions()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_timeStep, "timeStep", -1, "Time Step Index" );
    CAF_PDM_InitScriptableField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names" );

    CAF_PDM_InitScriptableField( &m_fileSplit, "fileSplit", RicExportCompletionDataSettingsUi::ExportSplitType(), "File Split" );
    CAF_PDM_InitScriptableField( &m_compdatExport, "compdatExport", RicExportCompletionDataSettingsUi::CompdatExportType(), "Compdat Export" );

    CAF_PDM_InitScriptableField( &m_includeMsw, "includeMsw", true, "Export Multi Segment Well Model" );
    CAF_PDM_InitScriptableField( &m_useLateralNTG, "useNtgHorizontally", false, "Use NTG Horizontally" );
    CAF_PDM_InitScriptableField( &m_includePerforations, "includePerforations", true, "Include Perforations" );
    CAF_PDM_InitScriptableField( &m_includeFishbones, "includeFishbones", true, "Include Fishbones" );
    CAF_PDM_InitScriptableField( &m_includeFractures, "includeFractures", true, "Include Fractures" );

    CAF_PDM_InitScriptableField( &m_excludeMainBoreForFishbones, "excludeMainBoreForFishbones", false, "Exclude Main Bore for Fishbones" );

    CAF_PDM_InitScriptableField( &m_performTransScaling, "performTransScaling", false, "Perform Transmissibility Scaling" );
    CAF_PDM_InitScriptableField( &m_transScalingTimeStep, "transScalingTimeStep", 0, "Transmissibility Scaling Pressure Time Step" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_transScalingInitialWBHP,
                                                  "transScalingWBHPFromSummary",
                                                  "transScalingWbhpFromSummary",
                                                  RicExportCompletionDataSettingsUi::TransScalingWBHPSource(),
                                                  "Transmissibility Scaling WBHP from summary" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_transScalingWBHP,
                                                  "transScalingWBHP",
                                                  "transScalingWbhp",
                                                  200.0,
                                                  "Transmissibility Scaling Constant WBHP Value" );

    CAF_PDM_InitScriptableField( &m_exportDataSourceAsComments, "exportComments", true, "Export Data Source as Comments" );
    CAF_PDM_InitScriptableField( &m_exportWelspec, "exportWelspec", true, "Export WELSPEC keyword" );
    CAF_PDM_InitScriptableField( &m_customFileNameIncludingPath, "customFileName", QString(), "Custom Filename" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportWellPathCompletions::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse case" ).arg( m_caseId() ), commandName );
    }

    // Resolve the export folder and custom file name. The Rimc method requires an explicit folder.
    QString exportFolderPath;
    QString customFileName;
    if ( m_customFileNameIncludingPath().isEmpty() )
    {
        exportFolderPath = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::COMPLETIONS );
        if ( exportFolderPath.isNull() )
        {
            exportFolderPath = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "completions" );
        }
    }
    else
    {
        QFileInfo fi( m_customFileNameIncludingPath() );
        customFileName = fi.fileName();

        auto pathCandidate = fi.path();
        if ( pathCandidate.size() > 2 )
        {
            exportFolderPath = pathCandidate;
        }
    }

    caf::PdmScriptResponse response;

    // Resolve well path names to objects. Unknown names are warnings, matching the legacy behavior.
    std::vector<RimWellPath*> wellPaths;
    for ( const QString& wellPathName : m_wellPathNames() )
    {
        RimWellPath* wellPath = RimProject::current()->activeOilField()->wellPathCollection->wellPathByName( wellPathName );
        if ( wellPath )
        {
            wellPaths.push_back( wellPath );
        }
        else
        {
            QString warning = QString( "%1: Could not find well path with name %2" ).arg( commandName ).arg( wellPathName );
            RiaLogging::warning( warning.toStdString() );
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
        }
    }

    // Legacy behavior: if names were given but none resolved, nothing is exported (not all visible wells)
    if ( !m_wellPathNames().empty() && wellPaths.empty() ) return response;

    RimEclipseCase_exportCompletions method( eclipseCase );
    method.setWellPaths( wellPaths );
    method.setTimeStep( m_timeStep() );
    method.setExportFolder( exportFolderPath );
    method.setCustomFileName( customFileName );
    method.setFileSplit( m_fileSplit() );
    method.setCompdatExport( m_compdatExport() );
    method.setIncludeMsw( m_includeMsw() );
    method.setUseNtgHorizontally( m_useLateralNTG() );
    method.setIncludePerforations( m_includePerforations() );
    method.setIncludeFishbones( m_includeFishbones() );
    method.setIncludeFractures( m_includeFractures() );
    method.setExcludeMainBoreForFishbones( m_excludeMainBoreForFishbones() );
    method.setPerformTransScaling( m_performTransScaling() );
    method.setTransScalingTimeStep( m_transScalingTimeStep() );
    method.setTransScalingWbhpSource( m_transScalingInitialWBHP() );
    method.setTransScalingWbhp( m_transScalingWBHP() );
    method.setExportComments( m_exportDataSourceAsComments() );
    method.setExportWelspec( m_exportWelspec() );

    auto result = method.execute();
    if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );

    return response;
}
