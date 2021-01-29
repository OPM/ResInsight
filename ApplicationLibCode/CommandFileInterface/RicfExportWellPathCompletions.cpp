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

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfExportWellPathCompletions, "exportWellPathCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellPathCompletions::RicfExportWellPathCompletions()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_timeStep, "timeStep", -1, "Time Step Index", "", "", "" );
    CAF_PDM_InitScriptableField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names", "", "", "" );

    CAF_PDM_InitScriptableField( &m_fileSplit,
                                 "fileSplit",
                                 RicExportCompletionDataSettingsUi::ExportSplitType(),
                                 "File Split",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_compdatExport,
                                 "compdatExport",
                                 RicExportCompletionDataSettingsUi::CompdatExportType(),
                                 "Compdat Export",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_combinationMode,
                                 "combinationMode",
                                 RicExportCompletionDataSettingsUi::CombinationModeType(),
                                 "Combination Mode",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_useLateralNTG, "useNtgHorizontally", false, "Use NTG Horizontally", "", "", "" );
    CAF_PDM_InitScriptableField( &m_includePerforations, "includePerforations", true, "Include Perforations", "", "", "" );
    CAF_PDM_InitScriptableField( &m_includeFishbones, "includeFishbones", true, "Include Fishbones", "", "", "" );
    CAF_PDM_InitScriptableField( &m_includeFractures, "includeFractures", true, "Include Fractures", "", "", "" );

    CAF_PDM_InitScriptableField( &m_excludeMainBoreForFishbones,
                                 "excludeMainBoreForFishbones",
                                 false,
                                 "Exclude Main Bore for Fishbones",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_performTransScaling,
                                 "performTransScaling",
                                 false,
                                 "Perform Transmissibility Scaling",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_transScalingTimeStep,
                                 "transScalingTimeStep",
                                 0,
                                 "Transmissibility Scaling Pressure Time Step",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_transScalingInitialWBHP,
                                 "transScalingWBHPFromSummary",
                                 RicExportCompletionDataSettingsUi::TransScalingWBHPSource(),
                                 "Transmissibility Scaling WBHP from summary",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_transScalingWBHP,
                                 "transScalingWBHP",
                                 200.0,
                                 "Transmissibility Scaling Constant WBHP Value",
                                 "",
                                 "",
                                 "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportWellPathCompletions::execute()
{
    using TOOLS = RicfApplicationTools;

    RicExportCompletionDataSettingsUi exportSettings;

    if ( m_timeStep < 0 )
    {
        exportSettings.timeStep = 0;
    }
    else
    {
        exportSettings.timeStep = m_timeStep;
    }

    exportSettings.fileSplit     = m_fileSplit;
    exportSettings.compdatExport = m_compdatExport;
    exportSettings.setExportDataSourceAsComment( RicfCommandFileExecutor::instance()->exportDataSouceAsComment() );

    exportSettings.performTransScaling    = m_performTransScaling;
    exportSettings.transScalingTimeStep   = m_transScalingTimeStep;
    exportSettings.transScalingWBHPSource = m_transScalingInitialWBHP;
    exportSettings.transScalingWBHP       = m_transScalingWBHP;

    exportSettings.useLateralNTG               = m_useLateralNTG;
    exportSettings.includePerforations         = m_includePerforations;
    exportSettings.includeFishbones            = m_includeFishbones;
    exportSettings.excludeMainBoreForFishbones = m_excludeMainBoreForFishbones;
    exportSettings.includeFractures            = m_includeFractures;

    exportSettings.setCombinationMode( m_combinationMode() );

    {
        auto eclipseCase = TOOLS::caseFromId( m_caseId() );
        if ( !eclipseCase )
        {
            QString error = QString( "exportWellPathCompletions: Could not find case with ID %1" ).arg( m_caseId() );
            RiaLogging::error( error );
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
        }
        exportSettings.caseToApply = eclipseCase;
    }

    QString exportFolder =
        RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::COMPLETIONS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "completions" );
    }
    exportSettings.folder = exportFolder;

    caf::PdmScriptResponse response;

    std::vector<RimWellPath*> wellPaths;
    if ( m_wellPathNames().empty() )
    {
        for ( auto wellPath : RimProject::current()->activeOilField()->wellPathCollection->allWellPaths() )
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
                RimProject::current()->activeOilField()->wellPathCollection->wellPathByName( wellPathName );
            if ( wellPath )
            {
                wellPaths.push_back( wellPath );
            }
            else
            {
                QString warning =
                    QString( "exportWellPathCompletions: Could not find well path with name %1" ).arg( wellPathName );
                RiaLogging::warning( warning );
                response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
            }
        }
    }

    std::vector<RimSimWellInView*> simWells;

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( wellPaths, simWells, exportSettings );

    return response;
}
