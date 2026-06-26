/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RivIjkIntersectionPartMgr.h"

#include "Rim3dView.h"
#include "RimIjkIntersection.h"

#include "RivIjkIntersectionSourceInfo.h"
#include "RivIntersectionGeometryGeneratorInterface.h"
#include "RivIntersectionHexGridInterface.h"
#include "RivIntersectionResultsColoringTools.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivPartPriority.h"
#include "RivScalarMapperUtils.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivIjkIntersectionPartMgr::RivIjkIntersectionPartMgr( RimIjkIntersection* intersection )
    : m_rimIntersection( intersection )
    , m_defaultColor( cvf::Color3::WHITE )
{
    CVF_ASSERT( m_rimIntersection );

    m_intersectionFacesTextureCoords = new cvf::Vec2fArray;

    cvf::ref<RivIntersectionHexGridInterface> hexGrid = intersection->createHexGridInterface();
    m_intersectionGenerator = new RivIjkIntersectionGeometryGenerator( m_rimIntersection, hexGrid.p(), intersection->mainGrid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionPartMgr::applySingleColorEffect()
{
    m_defaultColor = cvf::Color3f::OLIVE;
    updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionPartMgr::updateCellResultColor( int timeStepIndex )
{
    RivIntersectionResultsColoringTools::calculateIntersectionResultColors( timeStepIndex,
                                                                            true,
                                                                            m_rimIntersection,
                                                                            m_intersectionGenerator.p(),
                                                                            nullptr,
                                                                            nullptr,
                                                                            m_intersectionFaces.p(),
                                                                            m_intersectionFacesTextureCoords.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionPartMgr::generatePartGeometry( cvf::UByteArray* visibleCells )
{
    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_intersectionGenerator->generateSurface( visibleCells );
        if ( geo.notNull() )
        {
            geo->computeNormals();

            if ( useBufferObjects )
            {
                geo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Intersection IJK" );
            part->setDrawable( geo.p() );

            // Set mapping from triangle face index to cell index
            cvf::ref<RivIjkIntersectionSourceInfo> si = new RivIjkIntersectionSourceInfo( m_intersectionGenerator.p() );
            part->setSourceInfo( si.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellFaceBit );
            part->setPriority( RivPartPriority::PartType::Intersection );

            m_intersectionFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionGenerator->createMeshDrawable();
        if ( geoMesh.notNull() )
        {
            if ( useBufferObjects )
            {
                geoMesh->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Intersection IJK mesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellMeshBit );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_rimIntersection ) );

            m_intersectionGridLines = part;
        }
    }

    updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionPartMgr::updatePartEffect()
{
    caf::SurfaceEffectGenerator geometryEffgen( m_defaultColor, caf::PO_1 );

    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if ( m_intersectionFaces.notNull() )
    {
        m_intersectionFaces->setEffect( geometryOnlyEffect.p() );
    }

    cvf::ref<cvf::Effect>    eff;
    caf::MeshEffectGenerator meshEffGen( cvf::Color3::WHITE );
    eff = meshEffGen.generateCachedEffect();

    if ( m_intersectionGridLines.notNull() )
    {
        m_intersectionGridLines->setEffect( eff.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionPartMgr::appendNativeIntersectionFacesToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( m_intersectionFaces.notNull() )
    {
        m_intersectionFaces->setTransform( scaleTransform );
        model->addPart( m_intersectionFaces.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIjkIntersectionPartMgr::appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( m_intersectionGridLines.notNull() )
    {
        m_intersectionGridLines->setTransform( scaleTransform );
        model->addPart( m_intersectionGridLines.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RivIjkIntersectionPartMgr::intersectionGeometryGenerator() const
{
    if ( m_intersectionGenerator.notNull() ) return m_intersectionGenerator.p();

    return nullptr;
}
