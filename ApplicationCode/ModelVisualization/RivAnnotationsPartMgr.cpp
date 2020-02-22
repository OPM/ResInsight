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

#include "RivAnnotationsPartMgr.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimAnnotationInViewCollection.h"
#include "RimProject.h"

#include "RimPolylinesFromFileAnnotationInView.h"
#include "RimUserDefinedPolylinesAnnotationInView.h"

#include "RivPolylineAnnotationPartMgr.h"
#include "RivReachCircleAnnotationPartMgr.h"
#include "RivTextAnnotationPartMgr.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivAnnotationsPartMgr::RivAnnotationsPartMgr( Rim3dView* view )
    : m_rimView( view )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivAnnotationsPartMgr::~RivAnnotationsPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationsPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const cvf::BoundingBox&           boundingBox )
{
    createAnnotationPartManagers();

    for ( auto& partMgr : m_textAnnotationPartMgrs )
    {
        partMgr->appendDynamicGeometryPartsToModel( model, displayCoordTransform, boundingBox );
    }
    for ( auto& partMgr : m_reachCircleAnnotationPartMgrs )
    {
        partMgr->appendDynamicGeometryPartsToModel( model, displayCoordTransform, boundingBox );
    }
    for ( auto& partMgr : m_polylineAnnotationPartMgrs )
    {
        partMgr->appendDynamicGeometryPartsToModel( model, displayCoordTransform, boundingBox );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationsPartMgr::createAnnotationPartManagers()
{
    std::vector<RimAnnotationInViewCollection*> colls;
    m_rimView->descendantsIncludingThisOfType( colls );

    if ( colls.empty() ) return;
    auto coll = colls.front();

    auto localTextAnnotations           = coll->textAnnotations();
    auto textAnnotations                = coll->globalTextAnnotations();
    auto reachCircleAnnotations         = coll->globalReachCircleAnnotations();
    auto userDefinedPolylineAnnotations = coll->globalUserDefinedPolylineAnnotations();
    auto polylineFromFileAnnotations    = coll->globalPolylineFromFileAnnotations();

    clearGeometryCache();

    if ( m_textAnnotationPartMgrs.size() != localTextAnnotations.size() + textAnnotations.size() )
    {
        for ( auto annotation : localTextAnnotations )
        {
            auto* apm = new RivTextAnnotationPartMgr( m_rimView, annotation );
            m_textAnnotationPartMgrs.push_back( apm );
        }
        for ( auto annotation : textAnnotations )
        {
            auto* apm = new RivTextAnnotationPartMgr( m_rimView, annotation );
            m_textAnnotationPartMgrs.push_back( apm );
        }
    }
    if ( m_reachCircleAnnotationPartMgrs.size() != reachCircleAnnotations.size() )
    {
        for ( auto annotation : reachCircleAnnotations )
        {
            auto* apm = new RivReachCircleAnnotationPartMgr( m_rimView, annotation );
            m_reachCircleAnnotationPartMgrs.push_back( apm );
        }
    }
    if ( m_polylineAnnotationPartMgrs.size() != userDefinedPolylineAnnotations.size() + polylineFromFileAnnotations.size() )
    {
        for ( auto annotation : userDefinedPolylineAnnotations )
        {
            auto* apm = new RivPolylineAnnotationPartMgr( m_rimView, annotation );
            m_polylineAnnotationPartMgrs.push_back( apm );
        }
        for ( auto annotation : polylineFromFileAnnotations )
        {
            auto* apm = new RivPolylineAnnotationPartMgr( m_rimView, annotation );
            m_polylineAnnotationPartMgrs.push_back( apm );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationsPartMgr::clearGeometryCache()
{
    m_textAnnotationPartMgrs.clear();
    m_reachCircleAnnotationPartMgrs.clear();
    m_polylineAnnotationPartMgrs.clear();
}
