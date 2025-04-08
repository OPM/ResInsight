/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicCreateSummaryEnsembleFeature.h"

#include "RicImportEnsembleFeature.h"

#include "RimSummaryCase.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSummaryEnsembleFeature, "RicCreateSummaryEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateSummaryEnsembleFeature::isCommandEnabled() const
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
    return !selection.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSummaryEnsembleFeature::onActionTriggered( bool isChecked )
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
    if ( selection.empty() ) return;

    RicImportEnsembleFeature::createSummaryEnsemble( selection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSummaryEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Summary Ensemble" );
    actionToSetup->setIcon( QIcon( ":/SummaryGroup16x16.png" ) );
}
