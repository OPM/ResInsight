/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicNewEmptySummaryMultiPlotFeature.h"

#include "RicSummaryPlotBuilder.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RicSummaryPlotBuilder.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewEmptySummaryMultiPlotFeature, "RicNewEmptySummaryMultiPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEmptySummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;

    bool skipCreationOfPlotBasedOnPreferences = false;
    RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( selectedIndividualSummaryCases,
                                                                   selectedEnsembles,
                                                                   skipCreationOfPlotBasedOnPreferences );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEmptySummaryMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}
