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

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

CAF_PDM_SOURCE_INIT( RicfExportWellPathCompletions, "exportWellPathCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellPathCompletions::RicfExportWellPathCompletions()
{
    RICF_InitField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    RICF_InitField( &m_timeStep, "timeStep", -1, "Time Step Index", "", "", "" );
    RICF_InitField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names", "", "", "" );

    RICF_InitField( &m_fileSplit, "fileSplit", RicExportCompletionDataSettingsUi::ExportSplitType(), "File Split", "", "", "" );
    RICF_InitField( &m_compdatExport,
                    "compdatExport",
                    RicExportCompletionDataSettingsUi::CompdatExportType(),
                    "Compdat Export",
                    "",
                    "",
                    "" );
    RICF_InitField( &m_combinationMode,
                    "combinationMode",
                    RicExportCompletionDataSettingsUi::CombinationModeType(),
                    "Combination Mode",
                    "",
                    "",
                    "" );

    RICF_InitField( &m_useLateralNTG, "useNtgHorizontally", false, "Use NTG Horizontally", "", "", "" );
    RICF_InitField( &m_includePerforations, "includePerforations", true, "Include Perforations", "", "", "" );
    RICF_InitField( &m_includeFishbones, "includeFishbones", true, "Include Fishbones", "", "", "" );
    RICF_InitField( &m_includeFractures, "includeFractures", true, "Include Fractures", "", "", "" );

    RICF_InitField( &m_excludeMainBoreForFishbones,
                    "excludeMainBoreForFishbones",
                    false,
                    "Exclude Main Bore for Fishbones",
                    "",
                    "",
                    "" );

    RICF_InitField( &m_performTransScaling, "performTransScaling", false, "Perform Transmissibility Scaling", "", "", "" );
    RICF_InitField( &m_transScalingTimeStep, "transScalingTimeStep", 0, "Transmissibility Scaling Pressure Time Step", "", "", "" );
    RICF_InitField( &m_transScalingInitialWBHP,
                    "transScalingWBHPFromSummary",
                    RicExportCompletionDataSettingsUi::TransScalingWBHPSource(),
                    "Transmissibility Scaling WBHP from summary",
                    "",
                    "",
                    "" );
    RICF_InitField( &m_transScalingWBHP, "transScalingWBHP", 200.0, "Transmissibility Scaling Constant WBHP Value", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfExportWellPathCompletions::execute()
{
    using TOOLS = RicfApplicationTools;

    RimProject*                        project        = RiaApplication::instance()->project();
    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();

    if ( m_timeStep < 0 )
    {
        exportSettings->timeStep = 0;
    }
    else
    {
        exportSettings->timeStep = m_timeStep;
    }

    exportSettings->fileSplit     = m_fileSplit;
    exportSettings->compdatExport = m_compdatExport;

    exportSettings->performTransScaling    = m_performTransScaling;
    exportSettings->transScalingTimeStep   = m_transScalingTimeStep;
    exportSettings->transScalingWBHPSource = m_transScalingInitialWBHP;
    exportSettings->transScalingWBHP       = m_transScalingWBHP;

    exportSettings->useLateralNTG               = m_useLateralNTG;
    exportSettings->includePerforations         = m_includePerforations;
    exportSettings->includeFishbones            = m_includeFishbones;
    exportSettings->excludeMainBoreForFishbones = m_excludeMainBoreForFishbones;
    exportSettings->includeFractures            = m_includeFractures;

    exportSettings->setCombinationMode( m_combinationMode() );

    {
        auto eclipseCase = TOOLS::caseFromId( m_caseId() );
        if ( !eclipseCase )
        {
            QString error = QString( "exportWellPathCompletions: Could not find case with ID %1" ).arg( m_caseId() );
            RiaLogging::error( error );
            return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
        }
        exportSettings->caseToApply = eclipseCase;
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::COMPLETIONS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "completions" );
    }
    exportSettings->folder = exportFolder;

    RicfCommandResponse response;

    std::vector<RimWellPath*> wellPaths;
    if ( m_wellPathNames().empty() )
    {
        for ( auto wellPath : RiaApplication::instance()->project()->activeOilField()->wellPathCollection->wellPaths() )
        {
            if ( wellPath->showWellPath() )
            {
                wellPaths.push_back( wellPath );
            }
        }
    }
    else
    {
        for ( const QString& wellPathName : m_wellPathNames() )
        {
            RimWellPath* wellPath =
                RiaApplication::instance()->project()->activeOilField()->wellPathCollection->wellPathByName( wellPathName );
            if ( wellPath )
            {
                wellPaths.push_back( wellPath );
            }
            else
            {
                QString warning =
                    QString( "exportWellPathCompletions: Could not find well path with name %1" ).arg( wellPathName );
                RiaLogging::warning( warning );
                response.updateStatus( RicfCommandResponse::COMMAND_WARNING, warning );
            }
        }
    }

    std::vector<RimSimWellInView*> simWells;

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( wellPaths, simWells, *exportSettings );

    return response;
}
