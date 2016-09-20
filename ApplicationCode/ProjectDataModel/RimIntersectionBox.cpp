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

#include "RimIntersectionBox.h"

#include "RimView.h"

#include "cafPdmUiSliderEditor.h"



CAF_PDM_SOURCE_INIT(RimIntersectionBox, "IntersectionBox");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBox::RimIntersectionBox()
{
    CAF_PDM_InitObject("Intersection Box", ":/IntersectionBox16x16.png", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Intersection Name"), "Name", "", "", "");
    CAF_PDM_InitField(&isActive,    "Active",           true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&minXCoord,    "MinXCoord",           0.0, "MinXCoord", "", "", "");
    minXCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&maxXCoord,    "MaxXCoord",           0.0, "MaxXCoord", "", "", "");
    maxXCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&minYCoord,    "MinYCoord",           0.0, "MinYCoord", "", "", "");
    minYCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&maxYCoord,    "MaxYCoord",           0.0, "MaxYCoord", "", "", "");
    maxYCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&minZCoord,    "MinZCoord",           0.0, "MinZCoord", "", "", "");
    minZCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&maxZCoord,    "MaxZCoord",           0.0, "MaxZCoord", "", "", "");
    maxZCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBox::~RimIntersectionBox()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setModelBoundingBox(cvf::BoundingBox& boundingBox)
{
    m_boundingBox = boundingBox;

    minXCoord = cvf::Math::floor(boundingBox.min().x());
    minYCoord = cvf::Math::floor(boundingBox.min().y());
    minZCoord = cvf::Math::floor(boundingBox.min().z());

    maxXCoord = cvf::Math::ceil(boundingBox.max().x());
    maxYCoord = cvf::Math::ceil(boundingBox.max().y());
    maxZCoord = cvf::Math::ceil(boundingBox.max().z());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isActive)
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiSliderEditorAttribute* myAttr = static_cast<caf::PdmUiSliderEditorAttribute*>(attribute);

    if (myAttr)
    {
        if (field == &minXCoord || field == &maxXCoord)
        {
            myAttr->m_minimum = cvf::Math::floor(m_boundingBox.min().x());
            myAttr->m_maximum = cvf::Math::ceil(m_boundingBox.max().x());
        }
        else if (field == &minYCoord || field == &maxYCoord)
        {
            myAttr->m_minimum = cvf::Math::floor(m_boundingBox.min().y());
            myAttr->m_maximum = cvf::Math::ceil(m_boundingBox.max().y());
        }
        else if (field == &minZCoord || field == &maxZCoord)
        {
            myAttr->m_minimum = cvf::Math::floor(m_boundingBox.min().z());
            myAttr->m_maximum = cvf::Math::ceil(m_boundingBox.max().z());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

/*
    uiOrdering.add(&minXCoord);
    uiOrdering.add(&maxXCoord);

    uiOrdering.setForgetRemainingFields(true);
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionBox::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersectionBox::objectToggleField()
{
    return &isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::rebuildGeometryAndScheduleCreateDisplayModel()
{
    RimView* rimView = NULL;
    this->firstAnchestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

