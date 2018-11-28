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

#include "RicNewPltPlotFeature.h"

#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"

#include "RimProject.h"
#include "RimSimWellInView.h"
#include "Rim3dView.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellPltPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimPltPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimEclipseResultCase.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuSelectionManager.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <vector>


CAF_CMD_SOURCE_INIT(RicNewPltPlotFeature, "RicNewPltPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPltPlotFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;

    RimSimWellInView* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView*>();
    RimWellPath* selectedWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>();

    bool enable = true;

    if (selectedWellPath)
    {
        if (selectedWellPath->wellPathGeometry() == nullptr && !RimWellPlotTools::hasFlowData(selectedWellPath))
        {
            return false;
        }
    }

    if (simWell != nullptr)
    {
        RimProject* proj = RiaApplication::instance()->project();
        QString simWellName = simWell->name();

        RimWellPath* wellPath = proj->wellPathFromSimWellName(simWellName);
        enable = wellPath != nullptr;
    }
    return enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPltPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();

    RimPltPlotCollection* pltPlotColl = proj->mainPlotCollection()->pltPlotCollection();
    if (pltPlotColl)
    {
        QString wellPathName;
        RimWellPath* wellPath = nullptr;
        RimSimWellInView* eclipseWell = nullptr;

        if ((wellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>()) != nullptr)
        {
            wellPathName = wellPath->name();
        }
        else if ((eclipseWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView*>()) != nullptr)
        {
            wellPath = proj->wellPathFromSimWellName(eclipseWell->name());
            if (!wellPath ) return;

            wellPathName = wellPath->name();
        }

        QString plotName = QString(RimWellPltPlot::plotNameFormatString()).arg(wellPathName);

        RimWellPltPlot* pltPlot = new RimWellPltPlot();
        pltPlot->setCurrentWellName(wellPathName);

        RimWellLogTrack* plotTrack = new RimWellLogTrack();
        pltPlot->wellLogPlot()->addTrack(plotTrack);
        plotTrack->setDescription(QString("Track %1").arg(pltPlot->wellLogPlot()->trackCount()));

        pltPlotColl->addPlot(pltPlot);
        pltPlot->setDescription(plotName);

        //pltPlot->applyInitialSelections();
        pltPlot->loadDataAndUpdate();
        pltPlotColl->updateConnectedEditors();

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::setExpanded(plotTrack);
        RiuPlotMainWindowTools::selectAsCurrentItem(pltPlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPltPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New PLT Plot");
    actionToSetup->setIcon(QIcon(":/WellFlowPlot16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicNewPltPlotFeature::selectedWellPath() const
{
    auto selection = caf::selectedObjectsByType<RimWellPath*>();
    return selection.size() > 0 ? selection[0] : nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RicNewPltPlotFeature::selectedSimulationWell(int * branchIndex) const
{
    RiuSelectionItem* selItem = RiuSelectionManager::instance()->selectedItem(RiuSelectionManager::RUI_TEMPORARY);
    RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>(selItem);
    if (simWellSelItem)
    {
        (*branchIndex) = static_cast<int>(simWellSelItem->m_branchIndex);
        return simWellSelItem->m_simWell;
    }
    else
    {
        std::vector<RimSimWellInView*> selection;
        caf::SelectionManager::instance()->objectsByType(&selection);
        (*branchIndex) = 0;
        return selection.size() > 0 ? selection[0] : nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPltPlotFeature::caseAvailable() const
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    return cases.size() > 0;
}

