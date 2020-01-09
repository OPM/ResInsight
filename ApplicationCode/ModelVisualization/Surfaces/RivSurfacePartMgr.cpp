/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RivSurfacePartMgr.h"

#include "RigSurface.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"

#include "RimCase.h"
#include "cafEffectGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSurfacePartMgr::RivSurfacePartMgr( RimSurfaceInView* surface )
    : m_surfaceInView( surface )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::appendNativeGeometryPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( m_nativeTrianglesPart.isNull() || m_surfaceInView->surface()->surfaceData() != m_usedSurfaceData.p() )
    {
        generateNativePartGeometry();
    }

    if ( m_nativeTrianglesPart.notNull() )
    {
        m_nativeTrianglesPart->setTransform( scaleTransform );

        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( m_surfaceInView->surface()->color() ), caf::PO_1 );
        cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();
        m_nativeTrianglesPart->setEffect( eff.p() );

        model->addPart( m_nativeTrianglesPart.p() );

        if ( m_nativeMeshLinesPart.notNull() )
        {
            m_nativeMeshLinesPart->setTransform( scaleTransform );
            model->addPart( m_nativeMeshLinesPart.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::generateNativePartGeometry()
{
    RimCase* ownerCase;
    m_surfaceInView->firstAncestorOrThisOfTypeAsserted( ownerCase );
    cvf::Vec3d displayModOffsett = ownerCase->displayModelOffset();

    m_usedSurfaceData = m_surfaceInView->surface()->surfaceData();

    const std::vector<cvf::Vec3d>& vertices    = m_usedSurfaceData->vertices();
    cvf::ref<cvf::Vec3fArray>      cvfVertices = new cvf::Vec3fArray( vertices.size() );
    for ( size_t i = 0; i < vertices.size(); ++i )
    {
        ( *cvfVertices )[i] = cvf::Vec3f( vertices[i] - displayModOffsett );
    }

    const std::vector<unsigned>&           triangleIndices = m_usedSurfaceData->triangleIndices();
    cvf::ref<cvf::UIntArray>               cvfIndices      = new cvf::UIntArray( triangleIndices );
    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexSet        = new cvf::PrimitiveSetIndexedUInt( cvf::PT_TRIANGLES );
    indexSet->setIndices( cvfIndices.p() );

    cvf::ref<cvf::DrawableGeo> drawGeo = new cvf::DrawableGeo;
    drawGeo->addPrimitiveSet( indexSet.p() );
    drawGeo->setVertexArray( cvfVertices.p() );
    drawGeo->computeNormals();

    m_nativeTrianglesPart = new cvf::Part();
    m_nativeTrianglesPart->setDrawable( drawGeo.p() );
}
