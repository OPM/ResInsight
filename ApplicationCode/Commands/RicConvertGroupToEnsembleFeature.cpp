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

#include "RicConvertGroupToEnsembleFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicImportEnsembleFeature.h"
#include "RicCreateSummaryCaseCollectionFeature.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>


CAF_CMD_SOURCE_INIT(RicConvertGroupToEnsembleFeature, "RicConvertGroupToEnsembleFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicConvertGroupToEnsembleFeature::isCommandEnabled()
{
    const auto& selGroups = caf::selectedObjectsByTypeStrict<RimSummaryCaseCollection*>();
    if (selGroups.empty()) return false;

    for (const auto& group : selGroups)
    {
        if (!group->isEnsemble()) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicConvertGroupToEnsembleFeature::onActionTriggered(bool isChecked)
{
    const auto& selGroups = caf::selectedObjectsByTypeStrict<RimSummaryCaseCollection*>();

    for (const auto& group : selGroups)
    {
        if (group->isEnsemble()) continue;

        group->setAsEnsemble(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicConvertGroupToEnsembleFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryEnsemble16x16.png"));
    actionToSetup->setText("Convert to Ensemble");
}
