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
#include "RivIntersectionBoxPartMgr.h"

#include "cafPdmUiSliderEditor.h"
#include "RimCase.h"
#include "cafPdmUiDoubleSliderEditor.h"


namespace caf
{
    template<>
    void AppEnum< RimIntersectionBox::SinglePlaneState >::setUp()
    {
        addItem(RimIntersectionBox::PLANE_STATE_NONE,   "PLANE_STATE_NONE", "None");
        addItem(RimIntersectionBox::PLANE_STATE_X,      "PLANE_STATE_X",    "X Plane");
        addItem(RimIntersectionBox::PLANE_STATE_Y,      "PLANE_STATE_Y",    "Y Plane");
        addItem(RimIntersectionBox::PLANE_STATE_Z,      "PLANE_STATE_Z",    "Z Plane");
        setDefault(RimIntersectionBox::PLANE_STATE_NONE);
    }
}


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

    CAF_PDM_InitField(&singlePlaneState, "singlePlaneState", caf::AppEnum<SinglePlaneState>(SinglePlaneState::PLANE_STATE_NONE), "Collapse box to plane", "", "", "");

    CAF_PDM_InitField(&minXCoord,    "MinXCoord",           0.0, "MinXCoord", "", "", "");
    minXCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&maxXCoord,    "MaxXCoord",           0.0, "MaxXCoord", "", "", "");
    maxXCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&minYCoord,    "MinYCoord",           0.0, "MinYCoord", "", "", "");
    minYCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&maxYCoord,    "MaxYCoord",           0.0, "MaxYCoord", "", "", "");
    maxYCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&minZCoord,    "MinZCoord",           0.0, "MinZCoord", "", "", "");
    minZCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&maxZCoord,    "MaxZCoord",           0.0, "MaxZCoord", "", "", "");
    maxZCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
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
cvf::Mat4d RimIntersectionBox::boxOrigin() const
{
    cvf::Mat4d mx(cvf::Mat4d::IDENTITY);
    mx.setTranslation(cvf::Vec3d(minXCoord, minYCoord, minZCoord));
    return mx;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimIntersectionBox::boxSize() const
{
    return cvf::Vec3d(maxXCoord, maxYCoord, maxZCoord) - cvf::Vec3d(minXCoord, minYCoord, minZCoord);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setModelBoundingBox(cvf::BoundingBox& boundingBox)
{
    m_boundingBox = boundingBox;

    minXCoord = boundingBox.min().x() + boundingBox.extent().x() / 4.0;
    minYCoord = boundingBox.min().y() + boundingBox.extent().y() / 4.0;
    minZCoord = boundingBox.min().z() + boundingBox.extent().z() / 4.0;

    maxXCoord = boundingBox.max().x() - boundingBox.extent().x() / 4.0;
    maxYCoord = boundingBox.max().y() - boundingBox.extent().y() / 4.0;
    maxZCoord = boundingBox.max().z() - boundingBox.extent().z() / 4.0;

    updateLabelsFromBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setXSlice(double xValue)
{
    singlePlaneState = PLANE_STATE_X;
    minXCoord = xValue;
    maxXCoord = xValue;

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setYSlice(double yValue)
{
    singlePlaneState = PLANE_STATE_Y;
    minYCoord = yValue;
    maxYCoord = yValue;

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setZSlice(double zValue)
{
    singlePlaneState = PLANE_STATE_Z;
    minZCoord = zValue;
    maxZCoord = zValue;

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::updateLabelsFromBoundingBox()
{
    {
        QString range = QString(" [%1 - %2]").arg(m_boundingBox.min().x()).arg(m_boundingBox.max().x());
        minXCoord.uiCapability()->setUiName(QString("Min X") + range);
        maxXCoord.uiCapability()->setUiName(QString("Max X") + range);
    }

    {
        QString range = QString(" [%1 - %2]").arg(m_boundingBox.min().y()).arg(m_boundingBox.max().y());
        minYCoord.uiCapability()->setUiName(QString("Min Y") + range);
        maxYCoord.uiCapability()->setUiName(QString("Max Y") + range);
    }

    {
        QString range = QString(" [%1 - %2]").arg(m_boundingBox.min().z()).arg(m_boundingBox.max().z());
        minZCoord.uiCapability()->setUiName(QString("Min Z") + range);
        maxZCoord.uiCapability()->setUiName(QString("Max Z") + range);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxPartMgr* RimIntersectionBox::intersectionBoxPartMgr()
{
    if (m_intersectionBoxPartMgr.isNull()) m_intersectionBoxPartMgr = new RivIntersectionBoxPartMgr(this);

    return m_intersectionBoxPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::initialize()
{
    RimCase* rimCase = NULL;
    firstAnchestorOrThisOfType(rimCase);
    if (rimCase)
    {
        m_boundingBox = rimCase->activeCellsBoundingBox();
    }

    updateLabelsFromBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &singlePlaneState)
    {
        updateVisibility();
        clampSinglePlaneValues();
    }
    else if (changedField == &minXCoord)
    {
        clampSinglePlaneValues();
        minXCoord = CVF_MIN(maxXCoord, minXCoord);
    }
    else if (changedField == &minYCoord)
    {
        clampSinglePlaneValues();
        minYCoord = CVF_MIN(maxYCoord, minYCoord);
    }
    else if (changedField == &minZCoord)
    {
        clampSinglePlaneValues();
        minZCoord = CVF_MIN(maxZCoord, minZCoord);
    }
    else if (changedField == &maxXCoord)
    {
        maxXCoord = CVF_MAX(maxXCoord, minXCoord);
    }
    else if (changedField == &maxYCoord)
    {
        maxYCoord = CVF_MAX(maxYCoord, minYCoord);
    }
    else if (changedField == &maxZCoord)
    {
        maxZCoord = CVF_MAX(maxZCoord, minZCoord);
    }


    if (changedField != &name)
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiDoubleSliderEditorAttribute* myAttr = static_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

    if (myAttr)
    {
        if (field == &minXCoord || field == &maxXCoord)
        {
            myAttr->m_minimum = m_boundingBox.min().x();
            myAttr->m_maximum = m_boundingBox.max().x();
        }
        else if (field == &minYCoord || field == &maxYCoord)
        {
            myAttr->m_minimum = m_boundingBox.min().y();
            myAttr->m_maximum = m_boundingBox.max().y();
        }
        else if (field == &minZCoord || field == &maxZCoord)
        {
            myAttr->m_minimum = m_boundingBox.min().z();
            myAttr->m_maximum = m_boundingBox.max().z();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);
    uiOrdering.add(&singlePlaneState);

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("X Coordinates");
        group->add(&minXCoord);
        group->add(&maxXCoord);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Y Coordinates");
        group->add(&minYCoord);
        group->add(&maxYCoord);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Z Coordinates");
        group->add(&minZCoord);
        group->add(&maxZCoord);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::initAfterRead()
{
    updateVisibility();
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
    m_intersectionBoxPartMgr = nullptr;

    RimView* rimView = NULL;
    this->firstAnchestorOrThisOfType(rimView);
    if (rimView)
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::updateVisibility()
{
    maxXCoord.uiCapability()->setUiReadOnly(false);
    maxYCoord.uiCapability()->setUiReadOnly(false);
    maxZCoord.uiCapability()->setUiReadOnly(false);

    if (singlePlaneState == PLANE_STATE_X)
    {
        maxXCoord.uiCapability()->setUiReadOnly(true);
    }
    else if (singlePlaneState == PLANE_STATE_Y)
    {
        maxYCoord.uiCapability()->setUiReadOnly(true);
    }
    else if (singlePlaneState == PLANE_STATE_Z)
    {
        maxZCoord.uiCapability()->setUiReadOnly(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::clampSinglePlaneValues()
{
    if (singlePlaneState == PLANE_STATE_X)
    {
        maxXCoord = minXCoord;
    }
    else if (singlePlaneState == PLANE_STATE_Y)
    {
        maxYCoord = minYCoord;
    }
    else if (singlePlaneState == PLANE_STATE_Z)
    {
        maxZCoord = minZCoord;
    }
}

