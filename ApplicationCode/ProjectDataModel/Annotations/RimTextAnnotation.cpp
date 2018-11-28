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

#include "RimTextAnnotation.h"

#include "RimAnnotationInViewCollection.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimAnnotationCollection.h"


CAF_PDM_SOURCE_INIT(RimTextAnnotation, "RimTextAnnotation");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTextAnnotation::RimTextAnnotation()
{
    CAF_PDM_InitObject("TextAnnotation", ":/TextAnnotation16x16.png", "", "");

    CAF_PDM_InitField(&m_anchorPointXyd, "AnchorPointXyd", Vec3d::ZERO, "Anchor Point", "", "", "");
    CAF_PDM_InitField(&m_labelPointXyd, "LabelPointXyd", Vec3d::ZERO, "Label Point", "", "", "");
    CAF_PDM_InitField(&m_text, "Text", QString("(New text)"), "Text", "", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Is Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTextAnnotation::~RimTextAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimTextAnnotation::anchorPoint() const
{
    auto pos = m_anchorPointXyd();
    pos.z() = -pos.z();
    return pos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimTextAnnotation::labelPoint() const
{
    auto pos = m_labelPointXyd();
    pos.z() = -pos.z();
    return pos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::setText(const QString& text)
{
    m_text = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RimTextAnnotation::text() const
{
    return m_text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_anchorPointXyd);
    uiOrdering.add(&m_labelPointXyd);
    uiOrdering.add(&m_text);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTextAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue)
{
    RimAnnotationCollectionBase* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);

    if(annColl) annColl->scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimTextAnnotation::userDescriptionField()
{
    return &m_text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimTextAnnotation::isActive()
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimTextAnnotation::objectToggleField()
{
    return &m_isActive;
}



