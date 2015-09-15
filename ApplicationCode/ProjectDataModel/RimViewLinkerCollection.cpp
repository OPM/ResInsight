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

#include "RimViewLinkerCollection.h"

#include "RimViewLinker.h"
#include "RimViewLink.h"

#include "cafPdmUiTreeOrdering.h"




CAF_PDM_SOURCE_INIT(RimViewLinkerCollection, "RimViewLinkerCollection");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinkerCollection::RimViewLinkerCollection(void)
{
    CAF_PDM_InitObject("Linked Views", ":/chain.png", "", "");

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&viewLinkers, "ViewLinkers", "View Linkers", "", "", "");
    viewLinkers.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewLinkerCollection::~RimViewLinkerCollection(void)
{
    viewLinkers.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    for (size_t cIdx = 0; cIdx < viewLinkers.size(); ++cIdx)
    {
        RimViewLinker* childObject = viewLinkers[cIdx];
        if (childObject)
        {
            uiTreeOrdering.add(childObject);
            for (size_t j = 0; j < childObject->viewLinks.size(); j++)
            {
                uiTreeOrdering.add(childObject->viewLinks()[j]);
            }
        }
    }

    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&isActive == changedField)
    {
        if (isActive)
        {
            for (size_t cIdx = 0; cIdx < viewLinkers.size(); ++cIdx)
            {
                viewLinkers[cIdx]->applyAllOperations();
            }
        }
        else
        {
            for (size_t cIdx = 0; cIdx < viewLinkers.size(); ++cIdx)
            {
                viewLinkers[cIdx]->removeOverrides();
            }
        }
    }

    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewLinkerCollection::initAfterRead()
{
    this->updateUiIconFromToggleField();
}

