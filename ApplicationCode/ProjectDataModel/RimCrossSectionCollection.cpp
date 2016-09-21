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

#include "RimCrossSectionCollection.h"

#include "RimIntersection.h"
#include "RimEclipseWell.h"
#include "RimView.h"

#include "RiuMainWindow.h"

#include "RivIntersectionPartMgr.h"


CAF_PDM_SOURCE_INIT(RimCrossSectionCollection, "CrossSectionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCrossSectionCollection::RimCrossSectionCollection()
{
    CAF_PDM_InitObject("Intersections", ":/CrossSections16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_crossSections, "CrossSections", "Intersections", "", "", "");
    m_crossSections.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCrossSectionCollection::~RimCrossSectionCollection()
{
    m_crossSections.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSectionCollection::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::applySingleColorEffect()
{
    for (size_t csIdx = 0; csIdx < m_crossSections.size(); ++csIdx)
    {
        RimIntersection* cs = m_crossSections[csIdx];
        if (cs->isActive)
        {
            cs->crossSectionPartMgr()->applySingleColorEffect();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::updateCellResultColor(size_t timeStepIndex)
{
    for (size_t csIdx = 0; csIdx < m_crossSections.size(); ++csIdx)
    {
        RimIntersection* cs = m_crossSections[csIdx];
        if (cs->isActive)
        {
            cs->crossSectionPartMgr()->updateCellResultColor(timeStepIndex);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::appendPartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    if (!isActive) return;

    for (size_t csIdx = 0; csIdx < m_crossSections.size(); ++csIdx)
    {
        RimIntersection* cs = m_crossSections[csIdx];
        if (cs->isActive)
        {
            cs->crossSectionPartMgr()->appendNativeCrossSectionFacesToModel(model, scaleTransform);
            cs->crossSectionPartMgr()->appendMeshLinePartsToModel(model, scaleTransform);

            if (cs->inputFromViewerEnabled)
            {
                cs->crossSectionPartMgr()->appendPolylinePartsToModel(model, scaleTransform);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::appendCrossSection(RimIntersection* crossSection)
{
    m_crossSections.push_back(crossSection);

    updateConnectedEditors();
    RiuMainWindow::instance()->selectAsCurrentItem(crossSection);

    RimView* rimView = NULL;
    firstAnchestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isActive)
    {
        RimView* rimView = NULL;
        firstAnchestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCrossSectionCollection::hasActiveCrossSectionForSimulationWell(RimEclipseWell* eclipseWell) const
{
    if (!isActive) return false;

    for (size_t csIdx = 0; csIdx < m_crossSections.size(); ++csIdx)
    {
        RimIntersection* cs = m_crossSections[csIdx];

        if (cs->isActive &&
            cs->type() == RimIntersection::CS_SIMULATION_WELL &&
            cs->simulationWell() == eclipseWell)
        {
            return true;
        }
    }

    return false;
}
