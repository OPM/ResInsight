/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicNewEnsembleJobFeature.h"

#include "RiaApplication.h"
#include "RiaPreferencesOpm.h"

#include "RimReservoirGridEnsemble.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "Jobs/RimEnsembleJob.h"
#include "Jobs/RimJobCollection.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QIcon>

CAF_CMD_SOURCE_INIT( RicNewEnsembleJobFeature, "RicNewEnsembleJobFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEnsembleJobFeature::isCommandEnabled() const
{
    std::vector<RimReservoirGridEnsemble*> selectedEnsembles = caf::SelectionManager::instance()->objectsByType<RimReservoirGridEnsemble>();
    return RiaPreferencesOpm::current()->validateFlowSettings() && selectedEnsembles.size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleJobFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimReservoirGridEnsemble*> selectedEnsembles = caf::SelectionManager::instance()->objectsByType<RimReservoirGridEnsemble>();

    if ( selectedEnsembles.empty() ) return;

    auto job = new RimEnsembleJob();
    job->setName( selectedEnsembles[0]->name() );
    job->setEnsemble( selectedEnsembles[0] );
    auto jobColl = RimTools::jobCollection();
    jobColl->addNewJob( job );

    Riu3DMainWindowTools::selectAsCurrentItem( job );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleJobFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/opm.png" ) );
    actionToSetup->setText( "New Ensemble Job... " );
}
