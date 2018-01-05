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

#include "RicEclipseWellShowFeatures.h"

#include "RicEclipseWellFeatureImpl.h"

#include "RimSimWellInViewCollection.h"
#include "RimSimWellInView.h"

#include <QAction>



CAF_CMD_SOURCE_INIT(RicEclipseWellShowLabelFeature, "RicEclipseWellShowLabelFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowLabelFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    for (RimSimWellInView* w : selection)
    {
        w->showWellLabel.setValueWithFieldChanged(isChecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowLabelFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Well Label");
    actionToSetup->setCheckable(true);
    actionToSetup->setChecked(isCommandChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowLabelFeature::isCommandEnabled()
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowLabelFeature::isCommandChecked()
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* well = selection[0];

        return well->showWellLabel();
    }

    return false;
}








CAF_CMD_SOURCE_INIT(RicEclipseWellShowHeadFeature, "RicEclipseWellShowHeadFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowHeadFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    for (RimSimWellInView* w : selection)
    {
        w->showWellHead.setValueWithFieldChanged(isChecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowHeadFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Well Head");
    actionToSetup->setCheckable(true);
    actionToSetup->setChecked(isCommandChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowHeadFeature::isCommandEnabled()
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowHeadFeature::isCommandChecked()
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* well = selection[0];

        return well->showWellHead();
    }

    return false;
}









CAF_CMD_SOURCE_INIT(RicEclipseWellShowPipeFeature, "RicEclipseWellShowPipeFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowPipeFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    for (RimSimWellInView* w : selection)
    {
        w->showWellPipe.setValueWithFieldChanged(isChecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowPipeFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Well Pipes");
    actionToSetup->setCheckable(true);
    actionToSetup->setChecked(isCommandChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowPipeFeature::isCommandEnabled()
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowPipeFeature::isCommandChecked()
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* well = selection[0];

        return well->showWellPipe();
    }

    return false;
}








CAF_CMD_SOURCE_INIT(RicEclipseWellShowSpheresFeature, "RicEclipseWellShowSpheresFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowSpheresFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    for (RimSimWellInView* w : selection)
    {
        w->showWellSpheres.setValueWithFieldChanged(isChecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowSpheresFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Well Spheres");
    actionToSetup->setCheckable(true);
    actionToSetup->setChecked(isCommandChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowSpheresFeature::isCommandEnabled()
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowSpheresFeature::isCommandChecked()
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* well = selection[0];

        return well->showWellSpheres();
    }

    return false;
}








CAF_CMD_SOURCE_INIT(RicEclipseWellShowWellCellsFeature, "RicEclipseWellShowWellCellsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowWellCellsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    for (RimSimWellInView* w : selection)
    {
        w->showWellCells.setValueWithFieldChanged(isChecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowWellCellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Well Cells");
    actionToSetup->setCheckable(true);
    actionToSetup->setChecked(isCommandChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowWellCellsFeature::isCommandEnabled()
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowWellCellsFeature::isCommandChecked()
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* well = selection[0];

        return well->showWellCells();
    }

    return false;
}







CAF_CMD_SOURCE_INIT(RicEclipseWellShowWellCellFenceFeature, "RicEclipseWellShowWellCellFenceFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowWellCellFenceFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    for (RimSimWellInView* w : selection)
    {
        w->showWellCellFence.setValueWithFieldChanged(isChecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseWellShowWellCellFenceFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show Well Cell Fence");
    actionToSetup->setCheckable(true);
    actionToSetup->setChecked(isCommandChecked());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowWellCellFenceFeature::isCommandEnabled()
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipseWellShowWellCellFenceFeature::isCommandChecked()
{
    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();
    if (selection.size() > 0)
    {
        RimSimWellInView* well = selection[0];

        return well->showWellCellFence();
    }

    return false;
}
