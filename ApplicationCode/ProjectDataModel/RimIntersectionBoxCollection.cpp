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

#include "RimIntersectionBoxCollection.h"

#include "RimCase.h"
#include "RimIntersectionBox.h"
#include "RimView.h"

#include "RivIntersectionBoxPartMgr.h"

#include "RiuMainWindow.h"

#include "cvfBoundingBox.h"


CAF_PDM_SOURCE_INIT(RimIntersectionBoxCollection, "IntersectionBoxCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBoxCollection::RimIntersectionBoxCollection()
{
    CAF_PDM_InitObject("Intersection Boxes", ":/IntersectionBoxes16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_intersectionBoxes, "IntersectionBoxes", "IntersectionBoxes", "", "", "");
    m_intersectionBoxes.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBoxCollection::~RimIntersectionBoxCollection()
{
    m_intersectionBoxes.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionBoxCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBoxCollection::applySingleColorEffect()
{
    if (!isActive) return;

    for (size_t csIdx = 0; csIdx < m_intersectionBoxes.size(); ++csIdx)
    {
        RimIntersectionBox* cs = m_intersectionBoxes[csIdx];
        if (cs->isActive)
        {
            cs->intersectionBoxPartMgr()->applySingleColorEffect();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBoxCollection::updateCellResultColor(size_t timeStepIndex)
{
    if (!isActive) return;

    for (size_t csIdx = 0; csIdx < m_intersectionBoxes.size(); ++csIdx)
    {
        RimIntersectionBox* cs = m_intersectionBoxes[csIdx];
        if (cs->isActive)
        {
            cs->intersectionBoxPartMgr()->updateCellResultColor(timeStepIndex);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBoxCollection::appendPartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (!isActive) return;

    for (size_t csIdx = 0; csIdx < m_intersectionBoxes.size(); ++csIdx)
    {
        RimIntersectionBox* cs = m_intersectionBoxes[csIdx];
        if (cs->isActive)
        {
             cs->intersectionBoxPartMgr()->appendNativeCrossSectionFacesToModel(model, scaleTransform);
             cs->intersectionBoxPartMgr()->appendMeshLinePartsToModel(model, scaleTransform);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBoxCollection::appendIntersectionBox(RimIntersectionBox* intersectionBox)
{
    m_intersectionBoxes.push_back(intersectionBox);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBoxCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isActive)
    {
        RimView* rimView = NULL;
        firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}
