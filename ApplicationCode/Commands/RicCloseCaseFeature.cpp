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

#include "RicCloseCaseFeature.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimEclipseCaseCollection.h"
#include "RimCaseCollection.h"
#include "RimProject.h"
#include "RimOilField.h"
#include "RimGeoMechModels.h"

#include "RiaApplication.h"

#include "cafSelectionManager.h"
#include "cafPdmFieldHandle.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicCloseCaseFeature, "RicCloseCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::isCommandEnabled()
{
    return selectedEclipseCase() != NULL || selectedGeoMechCase() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::onActionTriggered(bool isChecked)
{
    RimEclipseCase* eclipseCase = selectedEclipseCase();
    RimGeoMechCase* geoMechCase = selectedGeoMechCase();
    if (eclipseCase)
    {
        deleteEclipseCase(eclipseCase);
    }
    else if (geoMechCase)
    {
        deleteGeoMechCase(geoMechCase);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Close");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicCloseCaseFeature::selectedEclipseCase() const
{
    std::vector<RimEclipseCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RicCloseCaseFeature::selectedGeoMechCase() const
{
    std::vector<RimGeoMechCase*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::removeCaseFromAllGroups(RimEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    RimProject* proj = RiaApplication::instance()->project();
    RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
    RimEclipseCaseCollection* analysisModels = (activeOilField) ? activeOilField->analysisModels() : NULL;
    if (analysisModels)
    {
        analysisModels->removeCaseFromAllGroups(eclipseCase);
        analysisModels->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::deleteEclipseCase(RimEclipseCase* eclipseCase)
{
    CVF_ASSERT(eclipseCase);

    RimCaseCollection* caseCollection = eclipseCase->parentCaseCollection();
    if (caseCollection)
    {
        if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(caseCollection))
        {
            RimIdenticalGridCaseGroup* caseGroup = caseCollection->parentCaseGroup();
            CVF_ASSERT(caseGroup);

            caseGroup->statisticsCaseCollection()->reservoirs.removeChildObject(eclipseCase);
            caseGroup->updateConnectedEditors();
        }
        else
        {
            removeCaseFromAllGroups(eclipseCase);
        }
    }
    else
    {
        removeCaseFromAllGroups(eclipseCase);
    }

    delete eclipseCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::deleteGeoMechCase(RimGeoMechCase* geoMechCase)
{
    CVF_ASSERT(geoMechCase);

    RimProject* proj = RiaApplication::instance()->project();
    RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
    RimGeoMechModels* models = (activeOilField) ? activeOilField->geoMechModels() : NULL;
    if (models)
    {
        models->cases.removeChildObject(geoMechCase);
        models->updateConnectedEditors();
    }

    delete geoMechCase;
}
