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

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimIntersection.h"
#include "RimIntersectionBox.h"
#include "RimSimWellInView.h"

#include "Riu3DMainWindowTools.h"

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
void RimIntersectionCollection::updateCellResultColor(size_t timeStepIndex, 
                                                      const cvf::ScalarMapper* scalarColorMapper, 
                                                      const RivTernaryScalarMapper* ternaryColorMapper)
{
    if(!this->isActive()) return;

    for (RimIntersection* cs : m_intersections)
    {
        if(cs->isActive)
        {
            cs->intersectionPartMgr()->updateCellResultColor(timeStepIndex, scalarColorMapper, ternaryColorMapper);
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
void RimIntersectionCollection::appendPartsToModel(Rim3dView& view, cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (!isActive) return;

    for (RimIntersection* cs : m_intersections)
    {
        if (cs->isActive)
        {
            cs->intersectionPartMgr()->appendNativeCrossSectionFacesToModel(model, scaleTransform);
            cs->intersectionPartMgr()->appendMeshLinePartsToModel(model, scaleTransform);
            cs->intersectionPartMgr()->appendPolylinePartsToModel(view, model, scaleTransform);
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
std::vector<RimIntersection*> RimIntersectionCollection::intersections() const
{
    return m_intersections.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimIntersectionBox*> RimIntersectionCollection::intersectionBoxes() const
{
    return m_intersectionBoxes.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::recomputeSimWellBranchData()
{
    for (const auto& intersection : intersections())
    {
        intersection->recomputeSimulationWellBranchData();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionAndUpdate(RimIntersection* intersection)
{
    m_intersections.push_back(intersection);

    syncronize2dIntersectionViews();

    updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem(intersection);

    Rim3dView* rimView = nullptr;
    firstAncestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionNoUpdate(RimIntersection* intersection)
{
    m_intersections.push_back(intersection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::syncronize2dIntersectionViews()
{
    RimCase* ownerCase = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(ownerCase);
    ownerCase->intersectionViewCollection()->syncFromExistingIntersections(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::scheduleCreateDisplayModelAndRedraw2dIntersectionViews()
{
    for (RimIntersection* isection: m_intersections)
    {
        if (isection->correspondingIntersectionView())
        {
            isection->correspondingIntersectionView()->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionBoxAndUpdate(RimIntersectionBox* intersectionBox)
{
    m_intersectionBoxes.push_back(intersectionBox);

    updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem(intersectionBox);

    Rim3dView* rimView = nullptr;
    firstAncestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionCollection::appendIntersectionBoxNoUpdate(RimIntersectionBox* intersectionBox)
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
        Rim3dView* rimView = nullptr;
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
