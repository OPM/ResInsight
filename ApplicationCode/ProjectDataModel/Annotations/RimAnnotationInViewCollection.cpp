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

#include "RimAnnotationCollection.h"
#include "RimAnnotationGroupCollection.h"
#include "RimCase.h"
#include "RimProject.h"
#include "RimGridView.h"
#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimPolylinesFromFileAnnotation.h"
#include "RimTextAnnotationInView.h"
#include "RimReachCircleAnnotationInView.h"
#include "RimUserDefinedPolylinesAnnotationInView.h"
#include "RimPolylinesFromFileAnnotationInView.h"

#include <cvfBoundingBox.h>

#include <cafPdmUiDoubleSliderEditor.h>

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
caf::PdmObject* sourcePdmAnnotation(const caf::PdmObject* annotationInView)
{
    auto t = dynamic_cast<const RimTextAnnotationInView*>(annotationInView);
    if (t)
    {
        return t->sourceAnnotation();
    }
    
    auto c = dynamic_cast<const RimReachCircleAnnotationInView*>(annotationInView);
    if (c)
    {
        return c->sourceAnnotation();
    }
    
    auto up = dynamic_cast<const RimUserDefinedPolylinesAnnotationInView*>(annotationInView);
    if (up)
    {
        return up->sourceAnnotation();
    }
    
    auto pf = dynamic_cast<const RimPolylinesFromFileAnnotationInView*>(annotationInView);
    if (pf)
    {
        return pf->sourceAnnotation();
    }

    return nullptr;
}


CAF_PDM_SOURCE_INIT(RimAnnotationInViewCollection, "Annotations");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection::RimAnnotationInViewCollection()
{
    CAF_PDM_InitObject("Annotations", ":/Annotations16x16.png", "", "");

    CAF_PDM_InitField(&m_annotationPlaneDepth, "AnnotationPlaneDepth", 0.0, "Annotation Plane Depth", "", "", "");
    CAF_PDM_InitField(&m_snapAnnotations, "SnapAnnotations", false, "Snap Annotations to Plane", "", "", "");

    m_annotationPlaneDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    m_annotationPlaneDepth.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::LabelPosType::TOP);

    CAF_PDM_InitFieldNoDefault(&m_globalTextAnnotations, "TextAnnotationsInView", "Global Text Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_globalReachCircleAnnotations, "ReachCircleAnnotationsInView", "Global Reach Circle Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_globalUserDefinedPolylineAnnotations, "UserDefinedPolylinesAnnotationsInView", "Global User Defined Polylines Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_globalPolylineFromFileAnnotations, "PolylinesFromFileAnnotationsInView", "Global Polylines From File Annotations", "", "", "");

    m_globalTextAnnotations.uiCapability()->setUiHidden(true);
    m_globalReachCircleAnnotations.uiCapability()->setUiHidden(true);
    m_globalUserDefinedPolylineAnnotations.uiCapability()->setUiHidden(true);
    m_globalPolylineFromFileAnnotations.uiCapability()->setUiHidden(true);

    m_globalTextAnnotations = new RimAnnotationGroupCollection("Global Text Annotations");
    m_globalReachCircleAnnotations = new RimAnnotationGroupCollection("Global Reach Circle Annotations");
    m_globalUserDefinedPolylineAnnotations = new RimAnnotationGroupCollection("Global User Defined Polylines Annotations");
    m_globalPolylineFromFileAnnotations = new RimAnnotationGroupCollection("Global Polylines From File Annotations");
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
std::vector<RimTextAnnotationInView*> RimAnnotationInViewCollection::globalTextAnnotations() const
{
    std::vector<RimTextAnnotationInView*> annotations;
    for (auto& a : m_globalTextAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimTextAnnotationInView*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimReachCircleAnnotationInView*> RimAnnotationInViewCollection::globalReachCircleAnnotations() const
{
    std::vector<RimReachCircleAnnotationInView*> annotations;
    for (auto& a : m_globalReachCircleAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimReachCircleAnnotationInView*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimUserDefinedPolylinesAnnotationInView*> RimAnnotationInViewCollection::globalUserDefinedPolylineAnnotations() const
{
    std::vector<RimUserDefinedPolylinesAnnotationInView*> annotations;
    for (auto& a : m_globalUserDefinedPolylineAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimUserDefinedPolylinesAnnotationInView*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylinesFromFileAnnotationInView*> RimAnnotationInViewCollection::globalPolylineFromFileAnnotations() const
{
    std::vector<RimPolylinesFromFileAnnotationInView*> annotations;
    for (auto& a : m_globalPolylineFromFileAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimPolylinesFromFileAnnotationInView*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
/// Called when the global annotation collection has changed
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::onGlobalCollectionChanged(const RimAnnotationCollection* globalCollection)
{
    // Sync annotations from global annotation collection
    auto globals = globalCollection->allPdmAnnotations();
    auto locals = allGlobalPdmAnnotations();
    std::vector<const caf::PdmObject*> globalAnnotationsToDelete;
    std::set<caf::PdmObject*> globalsSet(globals.begin(), globals.end());

    for(const auto local : locals)
    {
        auto sourceAnnotation = sourcePdmAnnotation(local);
        if (globalsSet.count(sourceAnnotation) > 0)
        {
            globalsSet.erase(sourceAnnotation);
        }
        else
        {
            globalAnnotationsToDelete.push_back(local);
        }
    }

    // Remove deleted global annotations
    for(auto a : globalAnnotationsToDelete)
    {
        deleteGlobalAnnotation(a);
    }

    // Add newly added global annotations
    for (const auto& global : globalsSet)
    {
        addGlobalAnnotation(global);
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimAnnotationInViewCollection::annotationsCount() const
{
    return m_textAnnotations->m_annotations.size() + allGlobalPdmAnnotations().size();
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
    scheduleRedrawOfRelevantViews();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObject*> RimAnnotationInViewCollection::allGlobalPdmAnnotations() const
{
    std::vector<caf::PdmObject*> all;
    all.insert(all.end(), m_globalTextAnnotations->m_annotations.begin(), m_globalTextAnnotations->m_annotations.end());
    all.insert(all.end(), m_globalReachCircleAnnotations->m_annotations.begin(), m_globalReachCircleAnnotations->m_annotations.end());
    all.insert(all.end(), m_globalUserDefinedPolylineAnnotations->m_annotations.begin(), m_globalUserDefinedPolylineAnnotations->m_annotations.end());
    all.insert(all.end(), m_globalPolylineFromFileAnnotations->m_annotations.begin(), m_globalPolylineFromFileAnnotations->m_annotations.end());
    return all;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::addGlobalAnnotation(caf::PdmObject* annotation)
{
    auto t = dynamic_cast<RimTextAnnotation*>(annotation);
    if (t)
    {
        m_globalTextAnnotations->addAnnotation(new RimTextAnnotationInView(t));
        return;
    }

    auto c = dynamic_cast< RimReachCircleAnnotation*>(annotation);
    if (c)
    {
        m_globalReachCircleAnnotations->addAnnotation(new RimReachCircleAnnotationInView(c));
        return;
    }

    auto up = dynamic_cast<RimUserDefinedPolylinesAnnotation*>(annotation);
    if (up)
    {
        m_globalUserDefinedPolylineAnnotations->addAnnotation(new RimUserDefinedPolylinesAnnotationInView(up));
        return;
    }

    auto pf = dynamic_cast<RimPolylinesFromFileAnnotation*>(annotation);
    if (pf)
    {
        m_globalPolylineFromFileAnnotations->addAnnotation(new RimPolylinesFromFileAnnotationInView(pf));
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationInViewCollection::deleteGlobalAnnotation(const caf::PdmObject* annotation)
{
    for(size_t i = 0; i < m_globalTextAnnotations->m_annotations.size(); i++)
    {
        if (m_globalTextAnnotations->m_annotations[i] == annotation)
        {
            m_globalTextAnnotations->m_annotations.erase(i);
            return;
        }
    }

    for (size_t i = 0; i < m_globalReachCircleAnnotations->m_annotations.size(); i++)
    {
        if (m_globalReachCircleAnnotations->m_annotations[i] == annotation)
        {
            m_globalReachCircleAnnotations->m_annotations.erase(i);
            return;
        }
    }

    for (size_t i = 0; i < m_globalUserDefinedPolylineAnnotations->m_annotations.size(); i++)
    {
        if (m_globalUserDefinedPolylineAnnotations->m_annotations[i] == annotation)
        {
            m_globalUserDefinedPolylineAnnotations->m_annotations.erase(i);
            return;
        }
    }

    for (size_t i = 0; i < m_globalPolylineFromFileAnnotations->m_annotations.size(); i++)
    {
        if (m_globalPolylineFromFileAnnotations->m_annotations[i] == annotation)
        {
            m_globalPolylineFromFileAnnotations->m_annotations.erase(i);
            return;
        }
    }
}
