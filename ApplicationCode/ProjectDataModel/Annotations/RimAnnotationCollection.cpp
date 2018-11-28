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
    CAF_PDM_InitObject("Annotations", ":/WellCollection.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_reachCircleAnnotations, "ReachCircleAnnotations", "Reach Circle Annotations", "", "", "");
    m_reachCircleAnnotations.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_userDefinedPolylineAnnotations, "UserDefinedPolylineAnnotations", "User Defined Polyline Annotations", "", "", "");
    m_userDefinedPolylineAnnotations.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_polylineFromFileAnnotations, "PolylineFromFileAnnotations", "Polylines From File", "", "", "");
    m_polylineFromFileAnnotations.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_annotationPlaneDepth, "AnnotationPlaneDepth", 0.0,"Annotation Plane Depth", "", "", "");
    CAF_PDM_InitField(&m_snapAnnotations, "SnapAnnotations", false, "Snap Annotations to Plane", "", "", "");
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
    m_reachCircleAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimUserDefinedPolylinesAnnotation* annotation)
{

    m_userDefinedPolylineAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation(RimPolylinesFromFileAnnotation* annotation)
{
    m_polylineFromFileAnnotations.push_back(annotation);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimAnnotationCollection::annotationPlaneZ() const
{
    return -m_annotationPlaneDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationCollection::snapAnnotations() const
{
    return m_snapAnnotations;
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
std::vector<RimUserDefinedPolylinesAnnotation*> RimAnnotationCollection::userDefinedPolylineAnnotations() const
{
    return m_userDefinedPolylineAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimPolylinesFromFileAnnotation*> RimAnnotationCollection::polylinesFromFileAnnotations() const
{
    return m_polylineFromFileAnnotations.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolylinesFromFileAnnotation* RimAnnotationCollection::importOrUpdatePolylinesFromFile(const QStringList& fileNames)
{
    QStringList newFileNames;
    std::vector<RimPolylinesFromFileAnnotation*> polyLinesObjsToReload;
    size_t formationListBeforeImportCount = m_polylineFromFileAnnotations.size();

    for(const QString& newFileName : fileNames)
    {
        bool isFound = false;
        for(RimPolylinesFromFileAnnotation* polyLinesAnnot: m_polylineFromFileAnnotations)
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

        auto newColor = RiaColorTables::categoryPaletteColors().cycledColor3f(formationListBeforeImportCount + newLinesIdx);

        newPolyLinesAnnot->setFileName(newFileName);
        newPolyLinesAnnot->setDescriptionFromFileName();
        newPolyLinesAnnot->appearance()->setColor(newColor);

        m_polylineFromFileAnnotations.push_back(newPolyLinesAnnot);
        polyLinesObjsToReload.push_back(newPolyLinesAnnot);

        ++newLinesIdx;
    }

    reloadPolylinesFromFile(polyLinesObjsToReload);


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
size_t RimAnnotationCollection::lineBasedAnnotationsCount() const
{
    return m_reachCircleAnnotations.size() + m_userDefinedPolylineAnnotations.size() + m_polylineFromFileAnnotations.size();
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
        QMessageBox::warning(nullptr, "Import Formation Names", totalErrorMessage);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_annotationPlaneDepth);
    uiOrdering.add(&m_snapAnnotations);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                               const QVariant&            oldValue,
                                               const QVariant&            newValue)
{
    scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::loadDataAndUpdate()
{
    reloadPolylinesFromFile(m_polylineFromFileAnnotations.childObjects());
}
