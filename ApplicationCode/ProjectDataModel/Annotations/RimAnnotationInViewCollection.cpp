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

#include "RimAnnotationInViewCollection.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimGridView.h"
#include "RimTextAnnotation.h"


CAF_PDM_SOURCE_INIT(RimAnnotationInViewCollection, "Annotations");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection::RimAnnotationInViewCollection()
{
    CAF_PDM_InitObject("Annotations", ":/Plus.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_textAnnotations, "TextAnnotations", "Text Annotations", "", "", "");
    m_textAnnotations.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection::~RimAnnotationInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::addAnnotation(RimTextAnnotation* annotation)
{
    m_textAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
/// At least one annotation have been deleted. Typically by the generic delete command
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::onAnnotationDeleted()
{
    auto                      project = RiaApplication::instance()->project();
    std::vector<RimGridView*> views;
    project->allVisibleGridViews(views);
    for (auto& view : views)
    {
        if (view->annotationCollection()->isActive()) view->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTextAnnotation*> RimAnnotationInViewCollection::textAnnotations() const
{
    return m_textAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationInViewCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&m_isActive == changedField)
    {
        this->updateUiIconFromToggleField();

        RimGridView* view;
        firstAncestorOrThisOfType(view);
        if (view)
        {
            //view->hasUserRequestedAnimation = true;
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnnotationInViewCollection::objectToggleField()
{
    return &m_isActive;
}
