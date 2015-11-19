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

#include "RimCrossSection.h"
#include "RivCrossSectionPartMgr.h"
#include "RiuMainWindow.h"
#include "RimView.h"


CAF_PDM_SOURCE_INIT(RimCrossSectionCollection, "CrossSectionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCrossSectionCollection::RimCrossSectionCollection()
{
    CAF_PDM_InitObject("Cross Sections", ":/undefined_image.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_crossSections, "CrossSections", "Cross Sections", "", "", "");
    m_crossSections.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);
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
        RimCrossSection* cs = m_crossSections[csIdx];
        cs->crossSectionPartMgr()->applySingleColorEffect();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors)
{
    for (size_t csIdx = 0; csIdx < m_crossSections.size(); ++csIdx)
    {
        RimCrossSection* cs = m_crossSections[csIdx];
        cs->crossSectionPartMgr()->updateCellResultColor(timeStepIndex, cellResultColors);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::appendPartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform)
{
    for (size_t csIdx = 0; csIdx < m_crossSections.size(); ++csIdx)
    {
        RimCrossSection* cs = m_crossSections[csIdx];
        if (cs->isActive)
        {
            cs->crossSectionPartMgr()->appendNativeCrossSectionFacesToModel(model, scaleTransform);
            cs->crossSectionPartMgr()->appendMeshLinePartsToModel(model, scaleTransform);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSectionCollection::appendCrossSection(RimCrossSection* crossSection)
{
    m_crossSections.push_back(crossSection);

    updateConnectedEditors();
    RiuMainWindow::instance()->setCurrentObjectInTreeView(crossSection);

    RimView* rimView = NULL;
    firstAnchestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}
