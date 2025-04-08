/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicComputeStatisticsFeature.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseStatisticsCaseCollection.h"
#include "RimIdenticalGridCaseGroup.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicComputeStatisticsFeature, "RicComputeStatisticsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicComputeStatisticsFeature::isCommandEnabled() const
{
    std::vector<RimEclipseStatisticsCase*> selection = selectedCases();
    if ( !selection.empty() )
    {
        RimEclipseStatisticsCase* statisticsCase = selection[0];
        if ( statisticsCase )
        {
            RimIdenticalGridCaseGroup* gridCaseGroup = statisticsCase->firstAncestorOrThisOfType<RimIdenticalGridCaseGroup>();

            RimCaseCollection* caseCollection = gridCaseGroup ? gridCaseGroup->caseCollection() : nullptr;
            return caseCollection ? !caseCollection->reservoirs.empty() : false;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicComputeStatisticsFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEclipseStatisticsCase*> selection = selectedCases();
    if ( !selection.empty() )
    {
        RimEclipseStatisticsCase* statisticsCase = selection[0];

        statisticsCase->computeStatisticsAndUpdateViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicComputeStatisticsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Compute" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseStatisticsCase*> RicComputeStatisticsFeature::selectedCases()
{
    return caf::SelectionManager::instance()->objectsByType<RimEclipseStatisticsCase>();
}
