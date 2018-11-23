/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimAnnotationCollection.h"

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylineAnnotation.h"

#include <QString>


namespace caf
{
    // template<>
    // void RimWellPathCollection::WellVisibilityEnum::setUp()
    // {
        // addItem(RimWellPathCollection::FORCE_ALL_OFF,       "FORCE_ALL_OFF",      "Off");
        // addItem(RimWellPathCollection::ALL_ON,              "ALL_ON",             "Individual");
        // addItem(RimWellPathCollection::FORCE_ALL_ON,        "FORCE_ALL_ON",       "On");
    // }
}


CAF_PDM_SOURCE_INIT(RimAnnotationCollection, "RimAnnotationCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection::RimAnnotationCollection()
{
    CAF_PDM_InitObject("Annotations", ":/WellCollection.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_textAnnotations, "TextAnnotations", "Text Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_reachCircleAnnotations, "ReachCircleAnnotations", "Reach Circle Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_polylineAnnotations, "PolylineAnnotations", "Polyline Annotations", "", "", "");
    m_textAnnotations.uiCapability()->setUiHidden(true);
    m_reachCircleAnnotations.uiCapability()->setUiHidden(true);
    m_polylineAnnotations.uiCapability()->setUiHidden(true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection::~RimAnnotationCollection()
{
   // wellPaths.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimTextAnnotation* annotation)
{
    m_textAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimReachCircleAnnotation* annotation)
{
    m_reachCircleAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimPolylineAnnotation* annotation)
{
    m_polylineAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimTextAnnotation*> RimAnnotationCollection::textAnnotations() const
{
    return m_textAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimReachCircleAnnotation*> RimAnnotationCollection::reachCircleAnnotations() const
{
    return m_reachCircleAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylineAnnotation*> RimAnnotationCollection::polylineAnnotations() const
{
    return m_polylineAnnotations.childObjects();
}
