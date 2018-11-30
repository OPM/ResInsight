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

#include "RimCase.h"
#include "RimProject.h"
#include "RimGridView.h"
#include "RimTextAnnotation.h"

#include <cvfBoundingBox.h>

#include <cafPdmUiDoubleSliderEditor.h>


CAF_PDM_SOURCE_INIT(RimAnnotationInViewCollection, "Annotations");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection::RimAnnotationInViewCollection()
{
    CAF_PDM_InitObject("Annotations", ":/Annotations16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_annotationPlaneDepth, "AnnotationPlaneDepth", 0.0, "Annotation Plane Depth", "", "", "");
    CAF_PDM_InitField(&m_snapAnnotations, "SnapAnnotations", false, "Snap Annotations to Plane", "", "", "");

    m_annotationPlaneDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    m_annotationPlaneDepth.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LabelPosType::TOP);
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
bool RimAnnotationInViewCollection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimAnnotationInViewCollection::annotationPlaneZ() const
{
    return -m_annotationPlaneDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationInViewCollection::snapAnnotations() const
{
    return m_snapAnnotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_snapAnnotations);
    if(m_snapAnnotations())
        uiOrdering.add(&m_annotationPlaneDepth);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&m_isActive == changedField)
    {
        this->updateUiIconFromToggleField();
    }
    scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimAnnotationInViewCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_annotationPlaneDepth)
    {
        auto* attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (attr)
        {
            RimCase* rimCase;
            firstAncestorOrThisOfType(rimCase);

            if (rimCase)
            {
                auto bb = rimCase->allCellsBoundingBox();
                attr->m_minimum = -bb.max().z();
                attr->m_maximum = -bb.min().z();
            }
            else
            {
                attr->m_minimum = 0;
                attr->m_maximum = 10000;
            }
        }
    }
}
