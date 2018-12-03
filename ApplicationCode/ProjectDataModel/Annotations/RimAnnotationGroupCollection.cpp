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

#include "RimAnnotationGroupCollection.h"

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


CAF_PDM_SOURCE_INIT(RimAnnotationGroupCollection, "RimAnnotationGroupCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationGroupCollection::RimAnnotationGroupCollection()
{
    CAF_PDM_InitObject("Annotations", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Is Active", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_annotations, "Annotations", "Annotations", "", "", "");

    m_isActive.uiCapability()->setUiHidden(true);
    m_annotations.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationGroupCollection::~RimAnnotationGroupCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationGroupCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationGroupCollection::isVisible() const
{
    RimAnnotationCollectionBase* coll;
    firstAncestorOrThisOfType(coll);

    bool visible = true;
    if (coll) visible = coll->isActive();
    if (visible) visible = m_isActive;
    return visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationGroupCollection::addAnnotation(caf::PdmObject* annotation)
{
    m_annotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObject*> RimAnnotationGroupCollection::annotations() const
{
    return m_annotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationGroupCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue)
{
    if (changedField == &m_isActive)
    {
        updateUiIconFromToggleField();

        RimAnnotationCollectionBase* coll;
        firstAncestorOrThisOfType(coll);
        if(coll) coll->scheduleRedrawOfRelevantViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnnotationGroupCollection::objectToggleField()
{
    return &m_isActive;
}
