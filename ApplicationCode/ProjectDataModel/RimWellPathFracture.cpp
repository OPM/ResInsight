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

#include "RimWellPathFracture.h"

#include "RigWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmUiDoubleSliderEditor.h"



CAF_PDM_SOURCE_INIT(RimWellPathFracture, "WellPathFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::RimWellPathFracture(void)
{
    CAF_PDM_InitObject("Fracture", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(         &measuredDepth,          "MeasuredDepth",        0.0f, "Measured Depth Location", "", "", "");
    measuredDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::~RimWellPathFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFracture::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &measuredDepth)
    {
        cvf::Vec3d positionAtWellpath = cvf::Vec3d::ZERO;
        
        caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(this);
        if (!objHandle) return;

        RimWellPath* wellPath = nullptr;
        objHandle->firstAncestorOrThisOfType(wellPath);
        if (!wellPath) return;

        RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
        positionAtWellpath = wellPathGeometry->interpolatedPointAlongWellPath(measuredDepth);

        this->setAnchorPosition(positionAtWellpath);

        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj) proj->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    uiOrdering.add(&measuredDepth);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Properties");
    geometryGroup->add(&azimuth);
    geometryGroup->add(&m_fractureTemplate);

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup("Fracture Center Info");
    fractureCenterGroup->add(&m_uiAnchorPosition);
    fractureCenterGroup->add(&m_displayIJK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &measuredDepth)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (myAttr)
        {
            RimWellPath* rimWellPath = nullptr;
            this->firstAncestorOrThisOfType(rimWellPath);
            CVF_ASSERT(rimWellPath);

            RigWellPath* wellPathGeo = rimWellPath->wellPathGeometry();
            CVF_ASSERT(wellPathGeo);
            {
                if (wellPathGeo->m_measuredDepths.size() > 2)
                {
                    myAttr->m_minimum = wellPathGeo->m_measuredDepths.front();
                    myAttr->m_maximum = wellPathGeo->m_measuredDepths.back();
                }
            }
        }
    }
}

