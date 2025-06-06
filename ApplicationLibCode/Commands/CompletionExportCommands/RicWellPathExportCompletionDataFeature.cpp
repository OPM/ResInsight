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

#include "RicWellPathExportCompletionDataFeature.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "RicExportFeatureImpl.h"

#include "RimDialogData.h"
#include "RimFishbones.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDir>

CAF_CMD_SOURCE_INIT( RicWellPathExportCompletionDataFeature, "RicWellPathExportCompletionDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions( const QString&                   dialogTitle,
                                                                                        const std::vector<RimWellPath*>& wellPaths )
{
    RiaApplication* app        = RiaApplication::instance();
    RimProject*     project    = app->project();
    QString         defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "COMPLETIONS" );

    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();

    if ( !exportSettings->caseToApply() )
    {
        std::vector<RimCase*> cases = app->project()->allGridCases();
        for ( auto c : cases )
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( c );
            if ( eclipseCase != nullptr )
            {
                exportSettings->caseToApply = eclipseCase;
                break;
            }
        }
    }

    if ( exportSettings->folder().isEmpty() ) exportSettings->folder = defaultDir;

    std::vector<RimWellPathFracture*>    wellPathFractures;
    std::vector<RimFishbones*>           wellPathFishbones;
    std::vector<RimPerforationInterval*> wellPathPerforations;

    std::vector<RimWellPath*> topLevelWells;
    {
        std::set<RimWellPath*> myWells;

        for ( auto w : wellPaths )
        {
            myWells.insert( w->topLevelWellPath() );
        }

        topLevelWells.assign( myWells.begin(), myWells.end() );
    }

    std::vector<RimWellPath*> allLaterals;
    {
        std::set<RimWellPath*> lateralSet;

        for ( auto t : topLevelWells )
        {
            auto laterals = t->allWellPathLaterals();
            for ( auto l : laterals )
            {
                lateralSet.insert( l );
            }
        }

        allLaterals.assign( lateralSet.begin(), lateralSet.end() );
    }

    for ( auto w : allLaterals )
    {
        auto fractures = w->descendantsIncludingThisOfType<RimWellPathFracture>();
        wellPathFractures.insert( wellPathFractures.end(), fractures.begin(), fractures.end() );

        auto fishbones = w->descendantsIncludingThisOfType<RimFishbones>();
        wellPathFishbones.insert( wellPathFishbones.end(), fishbones.begin(), fishbones.end() );

        auto perforations = w->descendantsIncludingThisOfType<RimPerforationInterval>();
        wellPathPerforations.insert( wellPathPerforations.end(), perforations.begin(), perforations.end() );
    }

    if ( !wellPathFractures.empty() )
    {
        exportSettings->showFractureInUi( true );
    }
    else
    {
        exportSettings->showFractureInUi( false );
    }

    if ( !wellPathFishbones.empty() )
    {
        exportSettings->showFishbonesInUi( true );
    }
    else
    {
        exportSettings->showFishbonesInUi( false );
    }

    if ( !wellPathPerforations.empty() )
    {
        exportSettings->showPerforationsInUi( true );

        std::vector<const RimWellPathValve*> perforationValves;
        for ( const auto& perf : wellPathPerforations )
        {
            auto other = perf->descendantsIncludingThisOfType<const RimWellPathValve>();
            perforationValves.insert( perforationValves.end(), other.begin(), other.end() );
        }

        if ( !perforationValves.empty() )
        {
            exportSettings->enableIncludeMsw();
        }
    }
    else
    {
        exportSettings->showPerforationsInUi( false );
    }

    caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(), exportSettings, dialogTitle, "" );
    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        {
            QDir folder( exportSettings->folder );
            if ( !folder.exists() )
            {
                QString txt = QString( "The path '%1' does not exist. Aborting export." ).arg( exportSettings->folder );
                RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Export", txt );

                return;
            }
        }

        RiaApplication::instance()->setLastUsedDialogDirectory( "COMPLETIONS", exportSettings->folder );

        RicWellPathExportCompletionDataFeatureImpl::exportCompletions( topLevelWells, *exportSettings );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::isCommandEnabled() const
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();

    return !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    CVF_ASSERT( !wellPaths.empty() );

    QString dialogTitle = "Export Completion Data for Selected Well Paths";
    RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions( dialogTitle, wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::setupActionLook( QAction* actionToSetup )
{
    std::vector<RimWellPath*> selected = selectedWellPaths();
    if ( selected.size() == 1u )
    {
        actionToSetup->setText( "Export Completion Data for Current Well Path" );
    }
    else
    {
        actionToSetup->setText( "Export Completion Data for Selected Well Paths" );
    }
    actionToSetup->setIcon( QIcon( ":/ExportCompletionsSymbol16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathExportCompletionDataFeature::selectedWellPaths()
{
    return caf::SelectionManager::instance()->objectsByType<RimWellPath>();
}
