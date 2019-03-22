/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicCreateSaturationPressurePlotsFeature.h"
#include "RicSaturationPressureUi.h"

#include "RiaApplication.h"

#include "RimEclipseResultCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"
#include "RimSaturationPressurePlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCreateSaturationPressurePlotsFeature, "RicCreateSaturationPressurePlotsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateSaturationPressurePlotsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSaturationPressurePlotsFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();

    bool                                 launchedFromPlotCollection = true;
    RimSaturationPressurePlotCollection* collection =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimSaturationPressurePlotCollection>();

    if (!collection)
    {
        collection                 = project->mainPlotCollection()->saturationPressurePlotCollection();
        launchedFromPlotCollection = false;
    }

    std::vector<RimEclipseResultCase*> eclipseCases;
    {
        RiaApplication*       app = RiaApplication::instance();
        std::vector<RimCase*> cases;
        app->project()->allCases(cases);
        for (auto* rimCase : cases)
        {
            auto erc = dynamic_cast<RimEclipseResultCase*>(rimCase);
            if (erc)
            {
                eclipseCases.push_back(erc);
            }
        }
    }

    RimEclipseResultCase* eclipseResultCase = nullptr;

    if (!eclipseCases.empty())
    {
        if (eclipseCases.size() == 1)
        {
            eclipseResultCase = eclipseCases[0];
        }
        else
        {
            RicSaturationPressureUi saturationPressureUi;
            saturationPressureUi.setSelectedCase(eclipseCases[0]);

            RiuPlotMainWindow* plotwindow = RiaApplication::instance()->mainPlotWindow();

            caf::PdmUiPropertyViewDialog propertyDialog(
                plotwindow, &saturationPressureUi, "Select Case to create Pressure Saturation plots", "");

            if (propertyDialog.exec() == QDialog::Accepted)
            {
                eclipseResultCase = dynamic_cast<RimEclipseResultCase*>(saturationPressureUi.selectedCase());
            }
        }
    }

    caf::PdmObject* objectToSelect = nullptr;

    if (eclipseResultCase)
    {
        eclipseResultCase->ensureReservoirCaseIsOpen();
        std::vector<RimSaturationPressurePlot*> plots = collection->createSaturationPressurePlots(eclipseResultCase);
        for (auto plot : plots)
        {
            plot->loadDataAndUpdate();
            plot->zoomAll();
            plot->updateConnectedEditors();
        }

        if (!plots.empty())
        {
            objectToSelect = plots.front();
        }
    }

    collection->updateAllRequiredEditors();
    RiaApplication::instance()->getOrCreateAndShowMainPlotWindow();

    if (objectToSelect)
    {
        RiuPlotMainWindowTools::selectAsCurrentItem(objectToSelect);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSaturationPressurePlotsFeature::setupActionLook(QAction* actionToSetup)
{
    RimSaturationPressurePlotCollection* collection =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimSaturationPressurePlotCollection>();
    if (!collection)
    {
        actionToSetup->setText("New Grid Cross Plot from 3d View");
    }
    else
    {
        actionToSetup->setText("Create Saturation Pressure Plots");
    }
    actionToSetup->setIcon(QIcon(":/SummaryXPlotsLight16x16.png"));
}
