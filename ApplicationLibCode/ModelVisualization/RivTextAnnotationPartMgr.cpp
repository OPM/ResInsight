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

#include "RivTextAnnotationPartMgr.h"

#include "RiaBoundingBoxTools.h"
#include "RiaColorTools.h"
#include "RiaFontCache.h"
#include "RiaPreferences.h"

#include "Rim3dView.h"
#include "RimAnnotationInViewCollection.h"
#include "RimAnnotationTextAppearance.h"
#include "RimTextAnnotation.h"
#include "RimTextAnnotationInView.h"

#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafFixedAtlasFont.h"

#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfqtUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::RivTextAnnotationPartMgr( Rim3dView* view, RimTextAnnotation* annotationLocal )
    : m_rimView( view )
    , m_rimAnnotationLocal( annotationLocal )
    , m_rimAnnotationInView( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::RivTextAnnotationPartMgr( Rim3dView* view, RimTextAnnotationInView* annotationInView )
    : m_rimView( view )
    , m_rimAnnotationLocal( nullptr )
    , m_rimAnnotationInView( annotationInView )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::~RivTextAnnotationPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::buildParts( const caf::DisplayCoordTransform* displayXf, bool doFlatten, double xOffset )
{
    clearAllGeometry();

    auto collection = annotationCollection();
    if ( !collection ) return;

    cvf::Vec3d anchorPositionInDomain = getAnchorPointInDomain( collection->snapAnnotations(), collection->annotationPlaneZ() );
    cvf::Vec3d labelPositionInDomain  = getLabelPointInDomain( collection->snapAnnotations(), collection->annotationPlaneZ() );

    cvf::Vec3d anchorPosition = displayXf->transformToDisplayCoord( anchorPositionInDomain );
    cvf::Vec3d labelPosition  = displayXf->transformToDisplayCoord( labelPositionInDomain );
    QString    text           = rimAnnotation()->text();

    auto fontSize        = rimAnnotation()->appearance()->fontSize();
    auto fontColor       = rimAnnotation()->appearance()->fontColor();
    auto backgroundColor = rimAnnotation()->appearance()->backgroundColor();
    auto anchorLineColor = rimAnnotation()->appearance()->anchorLineColor();

    // Line part
    {
        std::vector<cvf::Vec3d> points = { anchorPosition, labelPosition };

        auto part = RivAnnotationTools::createPartFromPolyline( anchorLineColor, points );
        part->setName( "RivTextAnnotationPartMgr" );
        part->setSourceInfo( new RivObjectSourceInfo( rimAnnotation() ) );

        m_linePart = part;
    }

    // Text part
    {
        auto font = RiaFontCache::getFont( fontSize );

        auto drawableText =
            RivAnnotationTools::createDrawableText( font.p(), fontColor, backgroundColor, text.toStdString(), cvf::Vec3f( labelPosition ) );

        auto        part      = RivAnnotationTools::createPart( drawableText.p() );
        cvf::String cvfString = cvfqt::Utils::toString( text );
        part->setName( "RivTextAnnotationPartMgr: " + cvfString );

        m_labelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::Vec3d RivTextAnnotationPartMgr::getAnchorPointInDomain( bool snapToPlaneZ, double planeZ )
{
    auto pt = rimAnnotation()->anchorPoint();

    if ( snapToPlaneZ )
    {
        pt.z() = planeZ;
    }
    return pt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTextAnnotationPartMgr::Vec3d RivTextAnnotationPartMgr::getLabelPointInDomain( bool snapToPlaneZ, double planeZ )
{
    auto pt = rimAnnotation()->labelPoint();

    if ( snapToPlaneZ )
    {
        pt.z() = planeZ;
    }
    return pt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTextAnnotationPartMgr::isTextInBoundingBox( const cvf::BoundingBox& boundingBox )
{
    auto coll = annotationCollection();
    if ( !coll ) return false;

    auto effectiveBoundingBox = RiaBoundingBoxTools::inflate( boundingBox, 3 );
    if ( effectiveBoundingBox.contains( getAnchorPointInDomain( coll->snapAnnotations(), coll->annotationPlaneZ() ) ) ||
         effectiveBoundingBox.contains( getLabelPointInDomain( coll->snapAnnotations(), coll->annotationPlaneZ() ) ) )
        return true;
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::clearAllGeometry()
{
    m_linePart  = nullptr;
    m_labelPart = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTextAnnotationPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                  const caf::DisplayCoordTransform* displayXf,
                                                                  const cvf::BoundingBox&           boundingBox )
{
    if ( !rimAnnotation() || !isAnnotationVisible() ) return;

    // Check bounding box
    if ( !isTextInBoundingBox( boundingBox ) ) return;

    if ( !validateAnnotation( rimAnnotation() ) ) return;

    buildParts( displayXf, false, 0.0 );
    model->addPart( m_linePart.p() );
    model->addPart( m_labelPart.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTextAnnotationPartMgr::validateAnnotation( const RimTextAnnotation* annotation ) const
{
    return rimAnnotation()->anchorPoint() != cvf::Vec3d::ZERO && !rimAnnotation()->text().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationInViewCollection* RivTextAnnotationPartMgr::annotationCollection() const
{
    auto colls = m_rimView->descendantsIncludingThisOfType<RimAnnotationInViewCollection>();
    return !colls.empty() ? colls.front() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTextAnnotation* RivTextAnnotationPartMgr::rimAnnotation() const
{
    if ( m_rimAnnotationLocal ) return m_rimAnnotationLocal;

    return m_rimAnnotationInView->sourceAnnotation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTextAnnotationPartMgr::isAnnotationVisible() const
{
    if ( m_rimAnnotationLocal ) return m_rimAnnotationLocal->isVisible();
    if ( m_rimAnnotationInView ) return m_rimAnnotationInView->isVisible();
    return false;
}
