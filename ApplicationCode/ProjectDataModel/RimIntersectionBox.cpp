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

    CAF_PDM_InitField(&m_singlePlaneState, "singlePlaneState", caf::AppEnum<SinglePlaneState>(SinglePlaneState::PLANE_STATE_NONE), "Collapse box to plane", "", "", "");

    CAF_PDM_InitField(&m_minXCoord,    "MinXCoord",           0.0, "MinXCoord", "", "", "");
    m_minXCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_maxXCoord,    "MaxXCoord",           0.0, "MaxXCoord", "", "", "");
    m_maxXCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_minYCoord,    "MinYCoord",           0.0, "MinYCoord", "", "", "");
    m_minYCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_maxYCoord,    "MaxYCoord",           0.0, "MaxYCoord", "", "", "");
    m_maxYCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_minZCoord,    "MinZCoord",           0.0, "MinZCoord", "", "", "");
    m_minZCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_maxZCoord,    "MaxZCoord",           0.0, "MaxZCoord", "", "", "");
    m_maxZCoord.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
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
    mx.setTranslation(cvf::Vec3d(m_minXCoord, m_minYCoord, m_minZCoord));
    return mx;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimIntersectionBox::boxSize() const
{
    return cvf::Vec3d(m_maxXCoord, m_maxYCoord, m_maxZCoord) - cvf::Vec3d(m_minXCoord, m_minYCoord, m_minZCoord);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIntersectionBox::SinglePlaneState RimIntersectionBox::singlePlaneState() const
{
    return m_singlePlaneState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setModelBoundingBox(cvf::BoundingBox& boundingBox)
{
    m_boundingBox = boundingBox;

    m_minXCoord = boundingBox.min().x() + boundingBox.extent().x() / 4.0;
    m_minYCoord = boundingBox.min().y() + boundingBox.extent().y() / 4.0;
    m_minZCoord = boundingBox.min().z() + boundingBox.extent().z() / 4.0;

    m_maxXCoord = boundingBox.max().x() - boundingBox.extent().x() / 4.0;
    m_maxYCoord = boundingBox.max().y() - boundingBox.extent().y() / 4.0;
    m_maxZCoord = boundingBox.max().z() - boundingBox.extent().z() / 4.0;

    updateLabelsFromBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setXSlice(double xValue)
{
    m_singlePlaneState = PLANE_STATE_X;
    m_minXCoord = xValue;
    m_maxXCoord = xValue;

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setYSlice(double yValue)
{
    m_singlePlaneState = PLANE_STATE_Y;
    m_minYCoord = yValue;
    m_maxYCoord = yValue;

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setZSlice(double zValue)
{
    m_singlePlaneState = PLANE_STATE_Z;
    m_minZCoord = zValue;
    m_maxZCoord = zValue;

    updateVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::updateLabelsFromBoundingBox()
{
    {
        QString range = QString(" [%1 - %2]").arg(m_boundingBox.min().x()).arg(m_boundingBox.max().x());
        m_minXCoord.uiCapability()->setUiName(QString("Min X") + range);
        m_maxXCoord.uiCapability()->setUiName(QString("Max X") + range);
    }

    {
        QString range = QString(" [%1 - %2]").arg(m_boundingBox.min().y()).arg(m_boundingBox.max().y());
        m_minYCoord.uiCapability()->setUiName(QString("Min Y") + range);
        m_maxYCoord.uiCapability()->setUiName(QString("Max Y") + range);
    }

    {
        QString range = QString(" [%1 - %2]").arg(m_boundingBox.min().z()).arg(m_boundingBox.max().z());
        m_minZCoord.uiCapability()->setUiName(QString("Min Z") + range);
        m_maxZCoord.uiCapability()->setUiName(QString("Max Z") + range);
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
    if (changedField == &m_singlePlaneState)
    {
        updateVisibility();
        clampSinglePlaneValues();
    }
    else if (changedField == &m_minXCoord)
    {
        clampSinglePlaneValues();
        m_minXCoord = CVF_MIN(m_maxXCoord, m_minXCoord);
    }
    else if (changedField == &m_minYCoord)
    {
        clampSinglePlaneValues();
        m_minYCoord = CVF_MIN(m_maxYCoord, m_minYCoord);
    }
    else if (changedField == &m_minZCoord)
    {
        clampSinglePlaneValues();
        m_minZCoord = CVF_MIN(m_maxZCoord, m_minZCoord);
    }
    else if (changedField == &m_maxXCoord)
    {
        m_maxXCoord = CVF_MAX(m_maxXCoord, m_minXCoord);
    }
    else if (changedField == &m_maxYCoord)
    {
        m_maxYCoord = CVF_MAX(m_maxYCoord, m_minYCoord);
    }
    else if (changedField == &m_maxZCoord)
    {
        m_maxZCoord = CVF_MAX(m_maxZCoord, m_minZCoord);
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
        if (field == &m_minXCoord || field == &m_maxXCoord)
        {
            myAttr->m_minimum = m_boundingBox.min().x();
            myAttr->m_maximum = m_boundingBox.max().x();
        }
        else if (field == &m_minYCoord || field == &m_maxYCoord)
        {
            myAttr->m_minimum = m_boundingBox.min().y();
            myAttr->m_maximum = m_boundingBox.max().y();
        }
        else if (field == &m_minZCoord || field == &m_maxZCoord)
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
    uiOrdering.add(&m_singlePlaneState);

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("X Coordinates");
        group->add(&m_minXCoord);
        group->add(&m_maxXCoord);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Y Coordinates");
        group->add(&m_minYCoord);
        group->add(&m_maxYCoord);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Z Coordinates");
        group->add(&m_minZCoord);
        group->add(&m_maxZCoord);
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
    m_maxXCoord.uiCapability()->setUiReadOnly(false);
    m_maxYCoord.uiCapability()->setUiReadOnly(false);
    m_maxZCoord.uiCapability()->setUiReadOnly(false);

    if (m_singlePlaneState == PLANE_STATE_X)
    {
        m_maxXCoord.uiCapability()->setUiReadOnly(true);
    }
    else if (m_singlePlaneState == PLANE_STATE_Y)
    {
        m_maxYCoord.uiCapability()->setUiReadOnly(true);
    }
    else if (m_singlePlaneState == PLANE_STATE_Z)
    {
        m_maxZCoord.uiCapability()->setUiReadOnly(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::clampSinglePlaneValues()
{
    if (m_singlePlaneState == PLANE_STATE_X)
    {
        m_maxXCoord = m_minXCoord;
    }
    else if (m_singlePlaneState == PLANE_STATE_Y)
    {
        m_maxYCoord = m_minYCoord;
    }
    else if (m_singlePlaneState == PLANE_STATE_Z)
    {
        m_maxZCoord = m_minZCoord;
    }
}

