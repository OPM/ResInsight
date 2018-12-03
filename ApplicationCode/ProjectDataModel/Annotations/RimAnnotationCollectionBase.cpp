/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimAnnotationCollectionBase.h"

#include "RiaApplication.h"

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylinesAnnotation.h"

#include "RimProject.h"
#include "RimGridView.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationGroupCollection.h"

#include "QMessageBox"
#include <QString>
#include "RiaColorTables.h"


CAF_PDM_SOURCE_INIT(RimAnnotationCollectionBase, "RimAnnotationCollectionBase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase::RimAnnotationCollectionBase()
{
    CAF_PDM_InitObject("Annotations", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Is Active", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_textAnnotations, "TextAnnotations", "Text Annotations", "", "", "");

    m_textAnnotations.uiCapability()->setUiHidden(true);
    m_textAnnotations = new RimAnnotationGroupCollection("Text Annotations");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase::~RimAnnotationCollectionBase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationCollectionBase::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::addAnnotation(RimTextAnnotation* annotation)
{
    m_textAnnotations->addAnnotation(annotation);
    updateViewAnnotationCollections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTextAnnotation*> RimAnnotationCollectionBase::textAnnotations() const
{
    std::vector<RimTextAnnotation*> annotations;
    for (auto& a : m_textAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimTextAnnotation*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::updateViewAnnotationCollections()
{
    // Default implementation: No op
}

//--------------------------------------------------------------------------------------------------
/// At least one annotation have been deleted. Typically by the generic delete command
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::onAnnotationDeleted()
{
    scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::scheduleRedrawOfRelevantViews()
{
    // Todo: Do a Bounding Box check to see if this annotation actually is relevant for the view

    auto views = gridViewsContainingAnnotations();
    if ( !views.empty() )
    {
        for ( auto& view : views )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> RimAnnotationCollectionBase::gridViewsContainingAnnotations() const
{
    std::vector<RimGridView*> views;
    RimProject*               project = nullptr;
    this->firstAncestorOrThisOfType(project);

    if (!project) return views;

    std::vector<RimGridView*> visibleGridViews;
    project->allVisibleGridViews(visibleGridViews);

    for (auto& gridView : visibleGridViews)
    {
        /*if (gridView->annotationCollection()->annotationsCount() > 0)*/ views.push_back(gridView);
    }
    return views;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollectionBase::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue)
{
    if (changedField == &m_isActive)
    {
        updateUiIconFromToggleField();
        scheduleRedrawOfRelevantViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnnotationCollectionBase::objectToggleField()
{
    return &m_isActive;
}
