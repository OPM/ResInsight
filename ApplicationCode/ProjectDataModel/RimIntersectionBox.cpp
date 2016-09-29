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
#include "RimEclipseView.h"


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
void RimIntersectionBox::setToDefaultSizeBox()
{
    cvf::BoundingBox boundingBox = currentCellBoundingBox();
    cvf::Vec3d center = boundingBox.center();
    
    double defaultWidthFactor = 0.5;

    m_minXCoord = center.x() - 0.5 * boundingBox.extent().x() *  defaultWidthFactor;
    m_minYCoord = center.y() - 0.5 * boundingBox.extent().y() *  defaultWidthFactor;
    m_minZCoord = center.z() - 0.5 * boundingBox.extent().z() *  defaultWidthFactor;
    m_maxXCoord = center.x() + 0.5 * boundingBox.extent().x() *  defaultWidthFactor;
    m_maxYCoord = center.y() + 0.5 * boundingBox.extent().y() *  defaultWidthFactor;
    m_maxZCoord = center.z() + 0.5 * boundingBox.extent().z() *  defaultWidthFactor;
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::setToDefaultSizeSlice(SinglePlaneState plane, const cvf::Vec3d& position)
{
    m_singlePlaneState = plane;

    cvf::BoundingBox boundingBox = currentCellBoundingBox();
    cvf::Vec3d center = position;
    
    if (center.isUndefined()) center = boundingBox.center();

    double defaultWidthFactor = 0.5;

    m_minXCoord = center[0] - 0.5 * boundingBox.extent().x() * defaultWidthFactor;
    m_minYCoord = center[1] - 0.5 * boundingBox.extent().y() * defaultWidthFactor;
    m_minZCoord = center[2] - 0.5 * boundingBox.extent().z() * defaultWidthFactor;
    m_maxXCoord = center[0] + 0.5 * boundingBox.extent().x() * defaultWidthFactor;
    m_maxYCoord = center[1] + 0.5 * boundingBox.extent().y() * defaultWidthFactor;
    m_maxZCoord = center[2] + 0.5 * boundingBox.extent().z() * defaultWidthFactor;

    switch (plane) 
    {
        case PLANE_STATE_X: m_minXCoord = m_maxXCoord = center[0]; break;
        case PLANE_STATE_Y: m_minYCoord = m_maxYCoord = center[1]; break;
        case PLANE_STATE_Z: m_minZCoord = m_maxZCoord = center[2]; break;
    }

    updateVisibility();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::updateLabelsFromBoundingBox()
{
    cvf::BoundingBox cellsBoundingBox = currentCellBoundingBox();
    {
        QString range = QString(" [%1 - %2]").arg(cellsBoundingBox.min().x()).arg(cellsBoundingBox.max().x());
        m_minXCoord.uiCapability()->setUiName(QString("Min X") + range);
        m_maxXCoord.uiCapability()->setUiName(QString("Max X") + range);
    }

    {
        QString range = QString(" [%1 - %2]").arg(cellsBoundingBox.min().y()).arg(cellsBoundingBox.max().y());
        m_minYCoord.uiCapability()->setUiName(QString("Min Y") + range);
        m_maxYCoord.uiCapability()->setUiName(QString("Max Y") + range);
    }

    {
        QString range = QString(" [%1 - %2]").arg(cellsBoundingBox.min().z()).arg(cellsBoundingBox.max().z());
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
void RimIntersectionBox::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_singlePlaneState)
    {
        switchSingelPlaneState();
        updateVisibility();
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
        cvf::BoundingBox cellsBoundingBox = currentCellBoundingBox();
        if (field == &m_minXCoord || field == &m_maxXCoord)
        {
            myAttr->m_minimum = cellsBoundingBox.min().x();
            myAttr->m_maximum = cellsBoundingBox.max().x();
        }
        else if (field == &m_minYCoord || field == &m_maxYCoord)
        {
            myAttr->m_minimum = cellsBoundingBox.min().y();
            myAttr->m_maximum = cellsBoundingBox.max().y();
        }
        else if (field == &m_minZCoord || field == &m_maxZCoord)
        {
            myAttr->m_minimum = cellsBoundingBox.min().z();
            myAttr->m_maximum = cellsBoundingBox.max().z();
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

    updateLabelsFromBoundingBox();
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
    this->firstAncestorOrThisOfType(rimView);
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
        m_maxXCoord = m_minXCoord = 0.5*(m_minXCoord + m_maxXCoord);
    }
    else if (m_singlePlaneState == PLANE_STATE_Y)
    {
        m_maxYCoord = m_minYCoord = 0.5*(m_minYCoord + m_maxYCoord);
    }
    else if (m_singlePlaneState == PLANE_STATE_Z)
    {
        m_maxZCoord = m_minZCoord = 0.5*(m_minZCoord + m_maxZCoord);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimIntersectionBox::switchSingelPlaneState()
{
    cvf::Vec3d orgSize = boxSize();
    double orgWidth = orgSize.length();

    switch( m_singlePlaneState()) // New collapsed direction
    {
        case PLANE_STATE_X:
        orgWidth =  orgSize[0];
        break;
        case PLANE_STATE_Y:
        orgWidth =  orgSize[1];
        break;
        case PLANE_STATE_Z:
        orgWidth =  orgSize[2];
        break;
        case PLANE_STATE_NONE:
        orgWidth =  orgSize.length() *0.3;
        break;
    }

    // For the originally collapsed direction, set a new width

    if(m_minXCoord() == m_maxXCoord()) 
    {
        double center = m_minXCoord;
        m_minXCoord =  center - 0.5*orgWidth;
        m_maxXCoord =  center + 0.5*orgWidth;
    }

    if(m_minYCoord() == m_maxYCoord()) 
    {
        double center = m_minYCoord;
        m_minYCoord =  center - 0.5*orgWidth;
        m_maxYCoord =  center + 0.5*orgWidth;
    }

    if(m_minZCoord() == m_maxZCoord()) 
    {
        double center = m_minZCoord;
        m_minZCoord =  center - 0.5*orgWidth;
        m_maxZCoord =  center + 0.5*orgWidth;
    }

    clampSinglePlaneValues();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimIntersectionBox::currentCellBoundingBox() 
{
    RimCase* rimCase = NULL;
    this->firstAncestorOrThisOfType(rimCase);
    
    CVF_ASSERT(rimCase);

    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType(eclView);

    bool useAllCells = true;
    if (eclView)
    {
        useAllCells = eclView->showInactiveCells();
    }

    if(false)//useAllCells) // For now, only use the active CellsBBox. 
        return rimCase->allCellsBoundingBox();
    else
        return rimCase->activeCellsBoundingBox();

}
