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

#include "RicfExportSimWellFractureCompletions.h"

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfExportSimWellFractureCompletions, "exportSimWellFractureCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportSimWellFractureCompletions::RicfExportSimWellFractureCompletions()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewId, "viewId", -1, "View ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_viewName, "viewName", QString( "" ), "View Name", "", "", "" );
    CAF_PDM_InitScriptableField( &m_timeStep, "timeStep", -1, "Time Step Index", "", "", "" );
    CAF_PDM_InitScriptableField( &m_simWellNames,
                                 "simulationWellNames",
                                 std::vector<QString>(),
                                 "Simulation Well Names",
                                 "",
                                 "",
                                 "" );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportSimWellFractureCompletions::execute()
{
    using TOOLS = RicfApplicationTools;

    RicExportCompletionDataSettingsUi exportSettings;

    exportSettings.timeStep      = m_timeStep;
    exportSettings.fileSplit     = m_fileSplit;
    exportSettings.compdatExport = m_compdatExport;
    exportSettings.setExportDataSourceAsComment( RicfCommandFileExecutor::instance()->exportDataSouceAsComment() );

    {
        auto eclipseCase = TOOLS::caseFromId( m_caseId() );
        if ( !eclipseCase )
        {
            QString error = QString( "exportSimWellCompletions: Could not find case with ID %1" ).arg( m_caseId() );
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

    std::vector<RimEclipseView*> views;
    for ( Rim3dView* v : exportSettings.caseToApply->views() )
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( v );
        if ( eclipseView && ( eclipseView->id() == m_viewId() || eclipseView->name() == m_viewName() ) )
        {
            views.push_back( eclipseView );
        }
    }
    if ( views.empty() )
    {
        QString error = QString( "exportSimWellCompletions: Could not find any views with id %1 or named \"%2\" in the "
                                 "case with ID %3" )
                            .arg( m_viewId )
                            .arg( m_viewName )
                            .arg( m_caseId() );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    caf::PdmScriptResponse response;

    std::vector<RimSimWellInView*> simWells;
    if ( m_simWellNames().empty() )
    {
        for ( RimEclipseView* view : views )
        {
            for ( auto simWell : view->wellCollection()->wells )
            {
                if ( simWell->showWell() )
                {
                    simWells.push_back( simWell );
                }
            }
        }
    }
    else
    {
        for ( const QString& wellPathName : m_simWellNames() )
        {
            for ( RimEclipseView* view : views )
            {
                RimSimWellInView* simWell = view->wellCollection()->findWell( wellPathName );
                if ( simWell )
                {
                    simWells.push_back( simWell );
                }
                else
                {
                    QString warning = QString( "exportSimWellCompletions: Could not find well with name %1 in view "
                                               "\"%2\" on case with ID %2" )
                                          .arg( wellPathName )
                                          .arg( m_viewName )
                                          .arg( m_caseId() );
                    RiaLogging::warning( warning );
                    response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warning );
                }
            }
        }
    }

    std::vector<RimWellPath*> wellPaths;

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( wellPaths, simWells, exportSettings );

    return response;
}
