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

#include "RimAnnotationCollection.h"

#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RimAnnotationGroupCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationLineAppearance.h"
#include "RimGridView.h"
#include "RimOilField.h"
#include "RimPolylineTarget.h"
#include "RimPolylinesFromFileAnnotation.h"
#include "RimProject.h"
#include "RimReachCircleAnnotation.h"
#include "RimTextAnnotation.h"
#include "RimUserDefinedPolylinesAnnotation.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonFile.h"

#include <QString>

CAF_PDM_SOURCE_INIT( RimAnnotationCollection, "RimAnnotationCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection::RimAnnotationCollection()
{
    CAF_PDM_InitObject( "Annotations", ":/Annotations16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_reachCircleAnnotations, "ReachCircleAnnotations", "Reach Circle Annotations" );
    m_reachCircleAnnotations = new RimAnnotationGroupCollection();
    m_reachCircleAnnotations->uiCapability()->setUiName( RimAnnotationGroupCollection::REACH_CIRCLE_ANNOTATION_UI_NAME );
    m_reachCircleAnnotations->uiCapability()->setUiIconFromResourceString( ":/ReachCircle16x16.png" );

    // obsolete things
    CAF_PDM_InitFieldNoDefault( &m_userDefinedPolylineAnnotations_OBSOLETE,
                                "UserDefinedPolylineAnnotations",
                                "User Defined Polyline Annotations" );
    m_userDefinedPolylineAnnotations_OBSOLETE = new RimAnnotationGroupCollection();
    m_userDefinedPolylineAnnotations_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_userDefinedPolylineAnnotations_OBSOLETE->uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_polylineFromFileAnnotations_OBSOLETE, "PolylineFromFileAnnotations", "Polylines From File" );
    m_polylineFromFileAnnotations_OBSOLETE = new RimAnnotationGroupCollection();
    m_polylineFromFileAnnotations_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_polylineFromFileAnnotations_OBSOLETE->uiCapability()->setUiTreeHidden( true );
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
void RimAnnotationCollection::initAfterRead()
{
    auto polycoll = RimProject::current()->activeOilField()->polygonCollection();

    for ( auto oldPolyObj : m_userDefinedPolylineAnnotations_OBSOLETE->annotations() )
    {
        auto oldPoly = dynamic_cast<RimUserDefinedPolylinesAnnotation*>( oldPolyObj );
        if ( oldPoly == nullptr ) continue;

        RimPolygon* newPoly = new RimPolygon();
        newPoly->setName( oldPoly->uiName() );
        newPoly->setIsClosed( oldPoly->closePolyline() );

        std::vector<cvf::Vec3d> points;

        for ( auto target : oldPoly->activeTargets() )
        {
            points.push_back( target->targetPointXYZ() );
        }

        newPoly->setPointsInDomainCoords( points );
        newPoly->setColor( oldPoly->appearance()->color() );

        polycoll->addUserDefinedPolygon( newPoly );
    }

    for ( auto oldPolyObj : m_polylineFromFileAnnotations_OBSOLETE->annotations() )
    {
        auto oldPoly = dynamic_cast<RimPolylinesFromFileAnnotation*>( oldPolyObj );
        if ( oldPoly == nullptr ) continue;

        RimPolygonFile* newPoly = new RimPolygonFile();
        newPoly->setName( oldPoly->uiName() );

        newPoly->setFileName( oldPoly->fileName() );

        polycoll->addPolygonFile( newPoly );
    }

    m_userDefinedPolylineAnnotations_OBSOLETE.children().clear();
    m_polylineFromFileAnnotations_OBSOLETE.children().clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::addAnnotation( RimReachCircleAnnotation* annotation )
{
    m_reachCircleAnnotations->addAnnotation( annotation );
    updateViewAnnotationCollections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimReachCircleAnnotation*> RimAnnotationCollection::reachCircleAnnotations() const
{
    std::vector<RimReachCircleAnnotation*> annotations;
    for ( auto& a : m_reachCircleAnnotations->annotations() )
    {
        annotations.push_back( dynamic_cast<RimReachCircleAnnotation*>( a ) );
    }
    return annotations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimAnnotationCollection::lineBasedAnnotationsCount() const
{
    return m_reachCircleAnnotations->annotations().size();
}

//--------------------------------------------------------------------------------------------------
/// Update view-local annotation collections, to mirror the state in the global collection (this collection)
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::updateViewAnnotationCollections()
{
    auto views = viewsContainingAnnotations();

    for ( const auto* view : views )
    {
        if ( view->annotationCollection() )
        {
            view->annotationCollection()->onGlobalCollectionChanged( this );
        }
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
    all.insert( all.end(), m_textAnnotations->m_annotations.begin(), m_textAnnotations->m_annotations.end() );
    all.insert( all.end(), m_reachCircleAnnotations->m_annotations.begin(), m_reachCircleAnnotations->m_annotations.end() );
    return all;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    onAnnotationDeleted();
}
