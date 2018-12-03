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

    CAF_PDM_InitFieldNoDefault(&m_textAnnotations, "TextAnnotations", "Text Annotations", "", "", "");
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
void RimAnnotationCollectionBase::addAnnotation(RimTextAnnotation* annotation)
{
    m_textAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTextAnnotation*> RimAnnotationCollectionBase::textAnnotations() const
{
    return m_textAnnotations.childObjects();
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
        if (gridView->annotationCollection()->isActive()) views.push_back(gridView);
    }
    return views;
}
