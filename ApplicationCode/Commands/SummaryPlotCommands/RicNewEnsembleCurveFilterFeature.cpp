/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewEnsembleCurveFilterFeature.h"

#include "RiaApplication.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewEnsembleCurveFilterFeature, "RicNewEnsembleCurveFilterFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewEnsembleCurveFilterFeature::isCommandEnabled()
{
    std::vector<RimEnsembleCurveFilterCollection*> filterColls = caf::selectedObjectsByType<RimEnsembleCurveFilterCollection*>();

    return filterColls.size() == 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleCurveFilterFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimEnsembleCurveFilterCollection*> filterColls = caf::selectedObjectsByType<RimEnsembleCurveFilterCollection*>();

    if (filterColls.size() == 1)
    {
        auto newFilter = filterColls[0]->addFilter();
        filterColls[0]->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem(newFilter);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleCurveFilterFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Ensemble Curve Filter");
    actionToSetup->setIcon(QIcon(":/SummaryCurveFilter16x16.png"));
}

