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

#include "RicNewWellPathFractureFeature.h"

#include "RiaApplication.h"

#include "RicFractureNameGenerator.h"

#include "RigWellPath.h"

#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "Riu3DMainWindowTools.h"

#include "WellPathCommands/RicWellPathsUnitSystemSettingsImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewWellPathFractureFeature, "RicNewWellPathFractureFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::addFracture(RimWellPath* wellPath, double measuredDepth)
{
    CVF_ASSERT(wellPath);

    if (!RicWellPathsUnitSystemSettingsImpl::ensureHasUnitSystem(wellPath)) return;

    RimWellPathFractureCollection* fractureCollection = wellPath->fractureCollection();
    CVF_ASSERT(fractureCollection);

    if (fractureCollection->fractures.empty())
    {
        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
        if (activeView)
        {
            activeView->fractureColors()->setDefaultResultName();
        }
    }

    RimWellPathFracture* fracture = new RimWellPathFracture();
    fractureCollection->fractures.push_back(fracture);

    fracture->setMeasuredDepth(measuredDepth);
    fracture->setFractureUnit(wellPath->unitSystem());

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    cvf::Vec3d positionAtWellpath = wellPathGeometry->interpolatedPointAlongWellPath(measuredDepth);
    fracture->setAnchorPosition(positionAtWellpath);

    RimOilField* oilfield = nullptr;
    fractureCollection->firstAncestorOrThisOfType(oilfield);
    if (!oilfield) return;

    fracture->setName(RicFractureNameGenerator::nameForNewFracture());

    auto unitSet = wellPath->unitSystem();
    fracture->setFractureUnit(unitSet);

    RimFractureTemplate* fracDef = oilfield->fractureDefinitionCollection->firstFractureOfUnit(unitSet);
    fracture->setFractureTemplate(fracDef);

    wellPath->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem(fracture);

    RimProject* project = nullptr;
    fractureCollection->firstAncestorOrThisOfType(project);
    if (project)
    {
        project->reloadCompletionTypeResultsInAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::onActionTriggered(bool isChecked)
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->allFractureTemplates().empty()) return;

    RimWellPathFractureCollection* fractureColl = RicNewWellPathFractureFeature::selectedWellPathFractureCollection();
    if (!fractureColl) return;

    RimWellPath* wellPath = nullptr;
    fractureColl->firstAncestorOrThisOfTypeAsserted(wellPath);

    double defaultMeasuredDepth = 0.0f;
    RicNewWellPathFractureFeature::addFracture(wellPath, defaultMeasuredDepth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellPathFractureFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathFractureFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj->allFractureTemplates().empty()) return false;

    if (selectedWellPathFractureCollection())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RicNewWellPathFractureFeature::selectedWellPathFractureCollection()
{
    RimWellPathFractureCollection* objToFind = nullptr;

    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    if (objToFind == nullptr)
    {
        std::vector<RimWellPath*> wellPaths;
        caf::SelectionManager::instance()->objectsByType(&wellPaths);
        if (!wellPaths.empty())
        {
            return wellPaths[0]->fractureCollection();
        }
    }

    return objToFind;
}
