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

#include "RimIntersectionCollection.h"

#include "RimIntersection.h"
#include "RimIntersectionBox.h"
#include "RimSimWellInView.h"
#include "RimView.h"

#include "RiuMainWindow.h"

#include "RivIntersectionBoxPartMgr.h"
#include "RivIntersectionPartMgr.h"


CAF_PDM_SOURCE_INIT(RimIntersectionCollection, "CrossSectionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection::RimIntersectionCollection()
{
    CAF_PDM_InitObject("Intersections", ":/CrossSections16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_intersections, "CrossSections", "Intersections", "", "", "");
    m_intersections.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_intersectionBoxes, "IntersectionBoxes", "IntersectionBoxes", "", "", "");
    m_intersectionBoxes.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection::~RimIntersectionCollection()
{
    m_intersections.deleteAllChildObjects();
    m_intersectionBoxes.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::applySingleColorEffect()
{
    if(!this->isActive()) return;

    for (RimIntersection* cs : m_intersections)
    {
        if (cs->isActive)
        {
            cs->intersectionPartMgr()->applySingleColorEffect();
        }
    }

    for (RimIntersectionBox* cs : m_intersectionBoxes)
    {
        if(cs->isActive)
        {
            cs->intersectionBoxPartMgr()->applySingleColorEffect();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::updateCellResultColor(size_t timeStepIndex)
{
    if(!this->isActive()) return;

    for (RimIntersection* cs : m_intersections)
    {
        if(cs->isActive)
        {
            cs->intersectionPartMgr()->updateCellResultColor(timeStepIndex);
        }
    }

    for (RimIntersectionBox* cs : m_intersectionBoxes)
    {
        if(cs->isActive)
        {
            cs->intersectionBoxPartMgr()->updateCellResultColor(timeStepIndex);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendPartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (!isActive) return;

    for (RimIntersection* cs : m_intersections)
    {
        if (cs->isActive)
        {
            cs->intersectionPartMgr()->appendNativeCrossSectionFacesToModel(model, scaleTransform);
            cs->intersectionPartMgr()->appendMeshLinePartsToModel(model, scaleTransform);
            cs->intersectionPartMgr()->appendPolylinePartsToModel(model, scaleTransform);
        }
    }

    for (RimIntersectionBox* cs : m_intersectionBoxes)
    {
        if(cs->isActive)
        {
            cs->intersectionBoxPartMgr()->appendNativeCrossSectionFacesToModel(model, scaleTransform);
            cs->intersectionBoxPartMgr()->appendMeshLinePartsToModel(model, scaleTransform);

            if (cs->show3dManipulator())
            {
                cs->appendManipulatorPartsToModel(model);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersection(RimIntersection* intersection)
{
    m_intersections.push_back(intersection);

    updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(intersection);

    RimView* rimView = NULL;
    firstAncestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionBox(RimIntersectionBox* intersectionBox)
{
    m_intersectionBoxes.push_back(intersectionBox);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimIntersectionCollection::hasActiveIntersectionForSimulationWell(const RimSimWellInView* simWell) const
{
    if (!isActive) return false;

    for (RimIntersection* cs : m_intersections)
    {
        if (cs->isActive &&
            cs->type() == RimIntersection::CS_SIMULATION_WELL &&
            cs->simulationWell() == simWell)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::updateIntersectionBoxGeometry()
{
    for (RimIntersectionBox* intersectionBox : m_intersectionBoxes)
    {
        intersectionBox->updateBoxManipulatorGeometry();
    }
}
