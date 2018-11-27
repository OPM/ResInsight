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
#include "RimGridView.h"
#include "RimProject.h"
#include "RimAnnotationCollection.h"


CAF_PDM_SOURCE_INIT(RimReachCircleAnnotation, "RimReachCircleAnnotation");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReachCircleAnnotation::RimReachCircleAnnotation()
{
    CAF_PDM_InitObject("CircleAnnotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_centerPoint, "CenterPoint", Vec3d::ZERO, "Center Point", "", "", "");
    CAF_PDM_InitField(&m_radius, "Radius", 0.0, "Radius", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimReachCircleAnnotation::centerPoint() const
{
    return m_centerPoint;
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
void RimReachCircleAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_centerPoint);
    uiOrdering.add(&m_radius);

    auto appearanceGroup = uiOrdering.addNewGroup("Line Appearance");
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
std::vector<RimGridView*> RimReachCircleAnnotation::gridViewsContainingAnnotations() const
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
