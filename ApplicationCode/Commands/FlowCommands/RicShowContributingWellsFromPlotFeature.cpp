/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicShowContributingWellsFromPlotFeature.h"

#include "RiaApplication.h"

#include "RicSelectViewUI.h"
#include "RicShowContributingWellsFeatureImpl.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimWellAllocationPlot.h"

#include "RiuMainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowContributingWellsFromPlotFeature, "RicShowContributingWellsFromPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowContributingWellsFromPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::onActionTriggered(bool isChecked)
{
    RimWellAllocationPlot* wellAllocationPlot = RiaApplication::instance()->activeWellAllocationPlot();
    if (!wellAllocationPlot) return;

    RimEclipseResultCase* parentEclipseCase = nullptr;
    wellAllocationPlot->flowDiagSolution()->firstAncestorOrThisOfTypeAsserted(parentEclipseCase);

    RimEclipseView* viewToManipulate = nullptr;

    {
        RimEclipseView* viewForSameResultCase = nullptr;

        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
        if (activeView)
        {
            RimEclipseResultCase* activeViewParent = nullptr;
            activeView->firstAncestorOrThisOfTypeAsserted(activeViewParent);

            if (activeViewParent == parentEclipseCase)
            {
                viewForSameResultCase = activeView;
            }
            else
            {
                if (parentEclipseCase->views().size() > 0)
                {
                    viewForSameResultCase = dynamic_cast<RimEclipseView*>(parentEclipseCase->views()[0]);
                }
            }
        }
        
        RicSelectViewUI featureUi;
        if (viewForSameResultCase)
        {
            featureUi.setView(viewForSameResultCase);
        }
        else
        {
            featureUi.setCase(parentEclipseCase);
        }

        caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "Show Contributing Wells in View", "");
        propertyDialog.resize(QSize(400, 200));
        
        if (propertyDialog.exec() != QDialog::Accepted) return;

        if (featureUi.createNewView())
        {
            RimEclipseView* createdView = parentEclipseCase->createAndAddReservoirView();
            createdView->name = featureUi.newViewName();

            // Must be run before buildViewItems, as wells are created in this function
            createdView->loadDataAndUpdate();
            parentEclipseCase->updateConnectedEditors();

            viewToManipulate = createdView;
        }
        else
        {
            viewToManipulate = featureUi.selectedView();
        }
    }

    CAF_ASSERT(viewToManipulate);

    int timeStep = wellAllocationPlot->timeStep();
    QString wellName = wellAllocationPlot->wellName();

    RicShowContributingWellsFeatureImpl::modifyViewToShowContributingWells(viewToManipulate, wellName, timeStep);

    auto* feature = caf::CmdFeatureManager::instance()->getCommandFeature("RicShowMainWindowFeature");
    feature->actionTriggered(false);

    RiuMainWindow::instance()->setExpanded(viewToManipulate, true);
    RiuMainWindow::instance()->selectAsCurrentItem(viewToManipulate);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowContributingWellsFromPlotFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/new_icon16x16.png"));
    actionToSetup->setText("Show Contributing Wells");
}
