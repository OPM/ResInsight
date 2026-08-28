/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicNewWellTargetMappingFeature.h"

#include "RimDataAnalyticsCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimReservoirGridEnsemble.h"
#include "RimWellTargetMapping.h"

#include "RiuMainWindow.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellTargetMappingFeature, "RicNewWellTargetMappingFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellTargetMappingFeature::isCommandEnabled() const
{
    if ( auto gridEnsembles = caf::selectedObjectsByTypeStrict<RimReservoirGridEnsemble*>(); !gridEnsembles.empty() )
    {
        return true;
    }
    if ( auto cases = caf::selectedObjectsByType<RimEclipseCase*>(); !cases.empty() )
    {
        return ( cases.front()->ensemble() == nullptr );
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellTargetMappingFeature::onActionTriggered( bool isChecked )
{
    if ( auto ensembles = caf::selectedObjectsByTypeStrict<RimEclipseCaseEnsemble*>(); !ensembles.empty() )
    {
        auto ensemble          = ensembles.front();
        auto wellTargetMapping = new RimWellTargetMapping();
        ensemble->addWellTargetMapping( wellTargetMapping );
        wellTargetMapping->setDefaults();

        ensemble->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( wellTargetMapping );
    }
    else if ( auto gridEnsembles = caf::selectedObjectsByTypeStrict<RimReservoirGridEnsemble*>(); !gridEnsembles.empty() )
    {
        auto ensemble          = gridEnsembles.front();
        auto wellTargetMapping = new RimWellTargetMapping();
        ensemble->addWellTargetMapping( wellTargetMapping );
        wellTargetMapping->setDefaults();

        ensemble->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( wellTargetMapping );
    }
    else if ( auto eclipseCases = caf::selectedObjectsByTypeStrict<RimEclipseCase*>(); !eclipseCases.empty() )
    {
        auto eclipseCase       = eclipseCases.front();
        auto wellTargetMapping = new RimWellTargetMapping();
        eclipseCase->addWellTargetMapping( wellTargetMapping );
        wellTargetMapping->setDefaults();

        eclipseCase->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( wellTargetMapping );
    }
    else if ( auto collections = caf::selectedObjectsByTypeStrict<RimDataAnalyticsCollection*>(); !collections.empty() )
    {
        auto eclipseCase = collections.front()->firstAncestorOrThisOfType<RimEclipseCase>();
        if ( !eclipseCase ) return;

        auto wellTargetMapping = new RimWellTargetMapping();
        eclipseCase->addWellTargetMapping( wellTargetMapping );
        wellTargetMapping->setDefaults();

        eclipseCase->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( wellTargetMapping );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellTargetMappingFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Well Target Mapping" );
    actionToSetup->setIcon( QIcon( ":/WellTargets.png" ) );
}
