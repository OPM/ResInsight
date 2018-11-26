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

#include "RimTextAnnotation.h"
#include "RimReachCircleAnnotation.h"
#include "RimPolylinesAnnotation.h"

#include "RimProject.h"
#include "RimGridView.h"
#include "RimAnnotationInViewCollection.h"

#include "QMessageBox"
#include <QString>


CAF_PDM_SOURCE_INIT(RimAnnotationCollection, "RimAnnotationCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection::RimAnnotationCollection()
{
    CAF_PDM_InitObject("Annotations", ":/WellCollection.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_textAnnotations, "TextAnnotations", "Text Annotations", "", "", "");
    m_textAnnotations.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_reachCircleAnnotations, "ReachCircleAnnotations", "Reach Circle Annotations", "", "", "");
    m_reachCircleAnnotations.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_polylineAnnotations, "PolylineAnnotations", "Polyline Annotations", "", "", "");
    m_polylineAnnotations.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_polylineFromFileAnnotations, "PolylineFromFileAnnotations", "Polylines From File", "", "", "");
    m_polylineFromFileAnnotations.uiCapability()->setUiHidden(true);

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
void RimAnnotationCollection::addAnnotation(RimPolylinesAnnotation* annotation)
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
std::vector<RimPolylinesAnnotation*> RimAnnotationCollection::polylineAnnotations() const
{
    return m_polylineAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimPolyLinesFromFileAnnotation*> RimAnnotationCollection::polylinesFromFileAnnotations() const
{
    return m_polylineFromFileAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolyLinesFromFileAnnotation* RimAnnotationCollection::importOrUpdatePolylinesFromFile(const QStringList& fileNames)
{
    QStringList newFileNames;
    std::vector<RimPolyLinesFromFileAnnotation*> polyLinesObjsToReload;
    size_t formationListBeforeImportCount = m_polylineFromFileAnnotations.size();

    for(const QString& newFileName : fileNames)
    {
        bool isFound = false;
        for(RimPolyLinesFromFileAnnotation* polyLinesAnnot: m_polylineFromFileAnnotations)
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

    for(const QString& newFileName :  newFileNames)
    {
        RimPolyLinesFromFileAnnotation* newPolyLinesAnnot = new RimPolyLinesFromFileAnnotation;
        newPolyLinesAnnot->setFileName(newFileName);
        m_polylineFromFileAnnotations.push_back(newPolyLinesAnnot);
        polyLinesObjsToReload.push_back(newPolyLinesAnnot);
        newPolyLinesAnnot->setDescriptionFromFileName();
    }

    QString totalErrorMessage;

    for (RimPolyLinesFromFileAnnotation* polyLinesAnnot: polyLinesObjsToReload)
    {
        QString errormessage;

        polyLinesAnnot->readPolyLinesFile(&errormessage);
        if (!errormessage.isEmpty())
        {
            totalErrorMessage += "\nError in: " + polyLinesAnnot->fileName() 
                + "\n\t" + errormessage;
        }
    }

    if (!totalErrorMessage.isEmpty())
    {
        QMessageBox::warning(nullptr, "Import Formation Names", totalErrorMessage);
    }

    if (m_polylineFromFileAnnotations.size() > formationListBeforeImportCount)
    {
        return m_polylineFromFileAnnotations[m_polylineFromFileAnnotations.size() - 1];
    }
    else
    {
        return nullptr;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::scheduleRedrawOfRelevantViews()
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
std::vector<RimGridView*> RimAnnotationCollection::gridViewsContainingAnnotations() const
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
