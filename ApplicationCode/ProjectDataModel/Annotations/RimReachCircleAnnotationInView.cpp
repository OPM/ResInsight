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

#include "RimReachCircleAnnotationInView.h"
#include "RimReachCircleAnnotation.h"
#include "RimAnnotationCollectionBase.h"
#include "RimAnnotationGroupCollection.h"


CAF_PDM_SOURCE_INIT(RimReachCircleAnnotationInView, "RimReachCircleAnnotationInView");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleAnnotationInView::RimReachCircleAnnotationInView()
{
    CAF_PDM_InitObject("ReachCircleAnnotationInView", ":/ReachCircle16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Is Active", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_sourceAnnotation, "SourceAnnotation", "Source Annotation", "", "", "");

    m_isActive.uiCapability()->setUiHidden(true);
    m_sourceAnnotation = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleAnnotationInView::RimReachCircleAnnotationInView(RimReachCircleAnnotation* sourceAnnotation)
    : RimReachCircleAnnotationInView()
{
    CVF_ASSERT(sourceAnnotation);

    m_isActive = sourceAnnotation->isActive();
    m_sourceAnnotation = sourceAnnotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReachCircleAnnotationInView::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotationInView::setSourceAnnotation(RimReachCircleAnnotation* annotation)
{
    m_sourceAnnotation = annotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleAnnotation* RimReachCircleAnnotationInView::sourceAnnotation() const
{
    return m_sourceAnnotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReachCircleAnnotationInView::isVisible() const
{
    RimAnnotationGroupCollection* coll;
    firstAncestorOrThisOfType(coll);

    bool visible = true;
    if (coll) visible = coll->isVisible();
    if (visible && m_sourceAnnotation)
    {
        visible = m_sourceAnnotation->isVisible();

        if (visible)
        {
            RimAnnotationGroupCollection* globalColl;
            m_sourceAnnotation->firstAncestorOrThisOfType(globalColl);
            if (globalColl) visible = globalColl->isVisible();
        }
    }
    if (visible) visible = m_isActive;
    return visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReachCircleAnnotationInView::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue)
{
    if (changedField == &m_isActive)
    {
        RimAnnotationCollectionBase* coll;
        firstAncestorOrThisOfType(coll);

        if (coll) coll->scheduleRedrawOfRelevantViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimReachCircleAnnotationInView::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimReachCircleAnnotationInView::userDescriptionField()
{
    return m_sourceAnnotation ? m_sourceAnnotation->userDescriptionField() : nullptr;
}

