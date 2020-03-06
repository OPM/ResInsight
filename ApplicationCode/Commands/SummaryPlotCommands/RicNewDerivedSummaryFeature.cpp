/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicNewDerivedSummaryFeature.h"

#include "RiaApplication.h"

#include "RimDerivedSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDerivedSummaryFeature, "RicNewDerivedSummaryFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDerivedSummaryFeature::isCommandEnabled()
{
    std::vector<RimSummaryCaseMainCollection*> mainColls =
        caf::selectedObjectsByTypeStrict<RimSummaryCaseMainCollection*>();

    return mainColls.size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedSummaryFeature::onActionTriggered( bool isChecked )
{
    if ( isCommandEnabled() )
    {
        auto project  = RiaApplication::instance()->project();
        auto mainColl = project->firstSummaryCaseMainCollection();

        auto derivedCase = new RimDerivedSummaryCase;
        mainColl->addCase( derivedCase );

        mainColl->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( derivedCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedSummaryFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Delta Summary Case" );
    // actionToSetup->setIcon( QIcon( ":/SummaryEnsemble16x16.png" ) );
}
