/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimReachCircleAnnotation.h"

#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationLineAppearance.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimAnnotationCollection.h"

#include "RicVec3dPickEventHandler.h"

#include "cafPdmUiVec3dEditor.h"

CAF_PDM_SOURCE_INIT(RimReachCircleAnnotation, "RimReachCircleAnnotation");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleAnnotation::RimReachCircleAnnotation()
{
    CAF_PDM_InitObject("CircleAnnotation", ":/ReachCircle16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Is Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_centerPointXyd, "CenterPointXyd", Vec3d::ZERO, "Center Point", "", "", "");
    m_centerPointXyd.uiCapability()->setUiEditorTypeName(caf::PdmUiVec3dEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_radius, "Radius", 0.0, "Radius", "", "", "");
    CAF_PDM_InitField(&m_name, "Name", QString("Circle Annotation"), "Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_appearance, "Appearance", "Appearance", "", "", "");

    m_appearance = new RimReachCircleLineAppearance();
    m_appearance.uiCapability()->setUiTreeHidden(true);
    m_appearance.uiCapability()->setUiTreeChildrenHidden(true);

    m_centerPointEventHandler.reset(new RicVec3dPickEventHandler(&m_centerPointXyd));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReachCircleAnnotation::isActive()
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReachCircleAnnotation::isVisible()
{
    RimAnnotationCollectionBase* coll;
    firstAncestorOrThisOfType(coll);

    return coll && coll->isActive() && m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimReachCircleAnnotation::centerPoint() const
{
    auto pos = m_centerPointXyd();
    pos.z() = -pos.z();
    return pos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimReachCircleAnnotation::radius() const
{
    return m_radius;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimReachCircleAnnotation::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleLineAppearance* RimReachCircleAnnotation::appearance() const
{
    return m_appearance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_name);
    uiOrdering.add(&m_centerPointXyd);
    uiOrdering.add(&m_radius);

    auto appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearance()->uiOrdering(uiConfigName, *appearanceGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue)
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);

    annColl->scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimReachCircleAnnotation::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimReachCircleAnnotation::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotation::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                     QString                    uiConfigName,
                                                     caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_centerPointXyd)
    {
        caf::PdmUiVec3dEditorAttribute* attr = dynamic_cast<caf::PdmUiVec3dEditorAttribute*>(attribute);
        if (attr)
        {
            attr->pickEventHandler = m_centerPointEventHandler;
            if (m_centerPointXyd().isZero())
            {
                attr->startPicking = true;
            }
        }
    }

}

