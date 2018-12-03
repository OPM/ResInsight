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

#include "RimAnnotationCollection.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"

#include "RimAnnotationGroupCollection.h"
#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylinesFromFileAnnotation.h"
#include "RimUserDefinedPolylinesAnnotation.h"

#include "RimProject.h"
#include "RimGridView.h"
#include "RimAnnotationInViewCollection.h"

#include "QMessageBox"
#include <QString>
#include "RiaColorTables.h"


CAF_PDM_SOURCE_INIT(RimAnnotationCollection, "RimAnnotationCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection::RimAnnotationCollection()
{
    CAF_PDM_InitObject("Annotations", ":/Annotations16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_reachCircleAnnotations, "ReachCircleAnnotations", "Reach Circle Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_userDefinedPolylineAnnotations, "UserDefinedPolylineAnnotations", "User Defined Polyline Annotations", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_polylineFromFileAnnotations, "PolylineFromFileAnnotations", "Polylines From File", "", "", "");

    m_reachCircleAnnotations.uiCapability()->setUiHidden(true);
    m_userDefinedPolylineAnnotations.uiCapability()->setUiHidden(true);
    m_polylineFromFileAnnotations.uiCapability()->setUiHidden(true);

    m_reachCircleAnnotations = new RimAnnotationGroupCollection("Reach Circle Annotations");
    m_userDefinedPolylineAnnotations = new RimAnnotationGroupCollection("User Defined Polyline Annotations");
    m_polylineFromFileAnnotations = new RimAnnotationGroupCollection("Polylines From File");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection::~RimAnnotationCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimReachCircleAnnotation* annotation)
{
    m_reachCircleAnnotations->addAnnotation(annotation);
    updateViewAnnotationCollections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimUserDefinedPolylinesAnnotation* annotation)
{

    m_userDefinedPolylineAnnotations->addAnnotation(annotation);
    updateViewAnnotationCollections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimPolylinesFromFileAnnotation* annotation)
{
    m_polylineFromFileAnnotations->addAnnotation(annotation);
    updateViewAnnotationCollections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimReachCircleAnnotation*> RimAnnotationCollection::reachCircleAnnotations() const
{
    std::vector<RimReachCircleAnnotation*> annotations;
    for (auto& a : m_reachCircleAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimReachCircleAnnotation*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimUserDefinedPolylinesAnnotation*> RimAnnotationCollection::userDefinedPolylineAnnotations() const
{
    std::vector<RimUserDefinedPolylinesAnnotation*> annotations;
    for (auto& a : m_userDefinedPolylineAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimUserDefinedPolylinesAnnotation*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylinesFromFileAnnotation*> RimAnnotationCollection::polylinesFromFileAnnotations() const
{
    std::vector<RimPolylinesFromFileAnnotation*> annotations;
    for (auto& a : m_polylineFromFileAnnotations->annotations())
    {
        annotations.push_back(dynamic_cast<RimPolylinesFromFileAnnotation*>(a));
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolylinesFromFileAnnotation* RimAnnotationCollection::importOrUpdatePolylinesFromFile(const QStringList& fileNames)
{
    QStringList newFileNames;
    std::vector<RimPolylinesFromFileAnnotation*> polyLinesObjsToReload;

    for(const QString& newFileName : fileNames)
    {
        bool isFound = false;
        for(RimPolylinesFromFileAnnotation* polyLinesAnnot: polylinesFromFileAnnotations())
        {
            if(polyLinesAnnot->fileName() == newFileName)
            {
                polyLinesObjsToReload.push_back(polyLinesAnnot);
                isFound = true;
                break;
            }
        }

        if(!isFound)
        {
            newFileNames.push_back(newFileName);
        }
    }

    size_t newLinesIdx = 0;
    for(const QString& newFileName :  newFileNames)
    {
        RimPolylinesFromFileAnnotation* newPolyLinesAnnot = new RimPolylinesFromFileAnnotation;

        auto newColor = RiaColorTables::categoryPaletteColors().cycledColor3f(lineBasedAnnotationsCount());

        newPolyLinesAnnot->setFileName(newFileName);
        newPolyLinesAnnot->setDescriptionFromFileName();
        newPolyLinesAnnot->appearance()->setColor(newColor);

        m_polylineFromFileAnnotations->addAnnotation(newPolyLinesAnnot);
        polyLinesObjsToReload.push_back(newPolyLinesAnnot);

        ++newLinesIdx;
    }

    reloadPolylinesFromFile(polyLinesObjsToReload);

    if (!newFileNames.empty())
    {
        return polylinesFromFileAnnotations().back();
    }
    else
    {
        return nullptr;
    }

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimAnnotationCollection::lineBasedAnnotationsCount() const
{
    return m_reachCircleAnnotations->annotations().size() + 
        m_userDefinedPolylineAnnotations->annotations().size() + 
        m_polylineFromFileAnnotations->annotations().size();
}

//--------------------------------------------------------------------------------------------------
/// Update view-local annotation collections, to mirror the state in the global collection (this collection)
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::updateViewAnnotationCollections()
{
    auto views = gridViewsContainingAnnotations();

    for (const auto* view : views)
    {
        view->annotationCollection()->onGlobalCollectionChanged(this);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::onAnnotationDeleted()
{
    updateViewAnnotationCollections();
    RimAnnotationCollectionBase::onAnnotationDeleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmObject*> RimAnnotationCollection::allPdmAnnotations() const
{
    std::vector<caf::PdmObject*> all;
    all.insert(all.end(), m_textAnnotations->m_annotations.begin(), m_textAnnotations->m_annotations.end());
    all.insert(all.end(), m_reachCircleAnnotations->m_annotations.begin(), m_reachCircleAnnotations->m_annotations.end());
    all.insert(all.end(), m_userDefinedPolylineAnnotations->m_annotations.begin(), m_userDefinedPolylineAnnotations->m_annotations.end());
    all.insert(all.end(), m_polylineFromFileAnnotations->m_annotations.begin(), m_polylineFromFileAnnotations->m_annotations.end());
    return all;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::reloadPolylinesFromFile(const std::vector<RimPolylinesFromFileAnnotation *>& polyLinesObjsToReload)
{
    QString totalErrorMessage;

    for ( RimPolylinesFromFileAnnotation* polyLinesAnnot: polyLinesObjsToReload )
    {
        QString errormessage;

        polyLinesAnnot->readPolyLinesFile(&errormessage);
        if ( !errormessage.isEmpty() )
        {
            totalErrorMessage += "\nError in: " + polyLinesAnnot->fileName()
                + "\n\t" + errormessage;
        }
    }

    if ( !totalErrorMessage.isEmpty() )
    {
        QMessageBox::warning(nullptr, "Import Polylines", totalErrorMessage);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::loadDataAndUpdate()
{
    reloadPolylinesFromFile(polylinesFromFileAnnotations());
}
