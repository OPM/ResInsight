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

#include "RicNewWellTargetCandidatesGeneratorFeature.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimWellTargetCandidatesGenerator.h"

#include "RiuMainWindow.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellTargetCandidatesGeneratorFeature, "RicNewWellTargetCandidatesGeneratorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellTargetCandidatesGeneratorFeature::onActionTriggered( bool isChecked )
{
    if ( auto ensembles = caf::selectedObjectsByTypeStrict<RimEclipseCaseEnsemble*>(); !ensembles.empty() )
    {
        auto ensemble  = ensembles.front();
        auto generator = new RimWellTargetCandidatesGenerator();
        ensemble->addWellTargetsGenerator( generator );

        ensemble->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( generator );
    }
    else if ( auto eclipseCases = caf::selectedObjectsByTypeStrict<RimEclipseCase*>(); !eclipseCases.empty() )
    {
        auto eclipseCase = eclipseCases.front();
        auto generator   = new RimWellTargetCandidatesGenerator();
        eclipseCase->addWellTargetsGenerator( generator );

        eclipseCase->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( generator );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellTargetCandidatesGeneratorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Well Target Candidates Generator" );
    actionToSetup->setIcon( QIcon( ":/WellTargets.png" ) );
}
