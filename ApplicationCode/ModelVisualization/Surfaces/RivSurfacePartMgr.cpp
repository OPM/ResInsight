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

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "ProjectDataModel/RimCase.h"
#include "RigSurface.h"
#include "Rim3dView.h"
#include "RimRegularLegendConfig.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "RivIntersectionResultsColoringTools.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivPartPriority.h"
#include "RivReservoirSurfaceIntersectionSourceInfo.h"
#include "RivScalarMapperUtils.h"
#include "RivSurfaceIntersectionGeometryGenerator.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSurfacePartMgr::RivSurfacePartMgr( RimSurfaceInView* surface )
    : m_surfaceInView( surface )
{
    CVF_ASSERT( surface );

    cvf::ref<RivIntersectionHexGridInterface> hexGrid = m_surfaceInView->createHexGridInterface();
    m_intersectionGenerator = new RivSurfaceIntersectionGeometryGenerator( m_surfaceInView, hexGrid.p() );

    m_intersectionFacesTextureCoords = new cvf::Vec2fArray;
    m_nativeTrianglesTextureCoords   = new cvf::Vec2fArray;
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
        this->updateNativeSurfaceColors();

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
void RivSurfacePartMgr::updateCellResultColor( size_t timeStepIndex )
{
    if ( m_intersectionFaces.notNull() )
    {
        RivIntersectionResultsColoringTools::calculateIntersectionResultColors( timeStepIndex,
                                                                                true,
                                                                                m_surfaceInView,
                                                                                m_intersectionGenerator.p(),
                                                                                nullptr,
                                                                                nullptr,
                                                                                m_intersectionFaces.p(),
                                                                                m_intersectionFacesTextureCoords.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::appendIntersectionGeometryPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( !m_surfaceInView->surfaceResultDefinition()->isChecked() )
    {
        if ( m_intersectionFaces.isNull() )
        {
            generatePartGeometry();
        }

        if ( m_intersectionFaces.notNull() )
        {
            m_intersectionFaces->setTransform( scaleTransform );
            model->addPart( m_intersectionFaces.p() );
        }

        // Mesh Lines

        if ( m_intersectionGridLines.isNull() )
        {
            generatePartGeometry();
        }

        if ( m_intersectionGridLines.notNull() )
        {
            m_intersectionGridLines->setTransform( scaleTransform );
            model->addPart( m_intersectionGridLines.p() );
        }

        if ( m_intersectionFaultGridLines.notNull() )
        {
            m_intersectionFaultGridLines->setTransform( scaleTransform );
            model->addPart( m_intersectionFaultGridLines.p() );
        }
    }

    appendNativeGeometryPartsToModel( model, scaleTransform );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::updateNativeSurfaceColors()
{
    if ( m_surfaceInView->surfaceResultDefinition()->isChecked() )
    {
        if ( m_usedSurfaceData.isNull() ) generateNativePartGeometry();

        auto mapper = m_surfaceInView->surfaceResultDefinition()->legendConfig()->scalarMapper();

        if ( m_usedSurfaceData.notNull() )
        {
            QString propertyName = m_surfaceInView->surfaceResultDefinition()->propertyName();
            auto    values       = m_usedSurfaceData->propertyValues( propertyName );

            const std::vector<cvf::Vec3d>& vertices = m_usedSurfaceData->vertices();

            m_nativeTrianglesTextureCoords->resize( vertices.size() );
            m_nativeTrianglesTextureCoords->setAll( cvf::Vec2f( 0.5f, 1.0f ) );
            for ( size_t i = 0; i < values.size(); i++ )
            {
                const double val = values[i];
                if ( val < std::numeric_limits<double>::infinity() && val == val )
                {
                    m_nativeTrianglesTextureCoords->set( i, mapper->mapToTextureCoord( val ) );
                }
            }

            float effectiveOpacityLevel = 1.0;
            bool  disableLighting       = true; // always disable lighting for now, as it doesn't look good

            RivScalarMapperUtils::applyTextureResultsToPart( m_nativeTrianglesPart.p(),
                                                             m_nativeTrianglesTextureCoords.p(),
                                                             mapper,
                                                             effectiveOpacityLevel,
                                                             caf::FC_NONE,
                                                             disableLighting );
        }
    }
    else
    {
        caf::SurfaceEffectGenerator surfaceGenBehind( cvf::Color4f( m_surfaceInView->surface()->color() ),
                                                      caf::PO_POS_LARGE );

        cvf::ref<cvf::Effect> effBehind = surfaceGenBehind.generateCachedEffect();
        if ( m_nativeTrianglesPart.notNull() )
        {
            m_nativeTrianglesPart->setEffect( effBehind.p() );
        }
    }

    if ( m_intersectionFaces.notNull() )
    {
        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( m_surfaceInView->surface()->color() ), caf::PO_1 );
        cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();

        m_intersectionFaces->setEffect( eff.p() );
    }

    // Update mesh colors as well, in case of change

    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    if ( m_intersectionGridLines.notNull() )
    {
        cvf::ref<cvf::Effect>    eff;
        caf::MeshEffectGenerator CrossSectionEffGen( prefs->defaultGridLineColors() );
        eff = CrossSectionEffGen.generateCachedEffect();

        m_intersectionGridLines->setEffect( eff.p() );
    }

    if ( m_intersectionFaultGridLines.notNull() )
    {
        cvf::ref<cvf::Effect>    eff;
        caf::MeshEffectGenerator CrossSectionEffGen( prefs->defaultFaultGridLineColors() );
        eff = CrossSectionEffGen.generateCachedEffect();

        m_intersectionFaultGridLines->setEffect( eff.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::generatePartGeometry()
{
    if ( m_intersectionGenerator.isNull() ) return;

    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_intersectionGenerator->generateSurface();
        if ( geo.notNull() )
        {
            geo->computeNormals();

            if ( useBufferObjects )
            {
                geo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Intersected Reservoir Surface" );
            part->setDrawable( geo.p() );

            // Set mapping from triangle face index to cell index
            cvf::ref<RivReservoirSurfaceIntersectionSourceInfo> si =
                new RivReservoirSurfaceIntersectionSourceInfo( m_intersectionGenerator.p() );
            part->setSourceInfo( si.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellFaceBit );
            part->setPriority( RivPartPriority::PartType::Intersection );

            m_intersectionFaces = part;
        }
    }

    // Cell Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionGenerator->createMeshDrawable();
        if ( geoMesh.notNull() )
        {
            if ( useBufferObjects )
            {
                geoMesh->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Surface intersection mesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellMeshBit );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_surfaceInView ) );

            m_intersectionGridLines = part;
        }
    }

    // Fault Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionGenerator->createFaultMeshDrawable();
        if ( geoMesh.notNull() )
        {
            if ( useBufferObjects )
            {
                geoMesh->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Surface fault mesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionFaultMeshBit );
            part->setPriority( RivPartPriority::PartType::FaultMeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_surfaceInView ) );

            m_intersectionFaultGridLines = part;
        }
    }

    updateNativeSurfaceColors();
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
    if ( m_usedSurfaceData.isNull() ) return;

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

    cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( m_surfaceInView );

    m_nativeTrianglesPart = new cvf::Part();
    m_nativeTrianglesPart->setName( "Native Reservoir Surface" );
    m_nativeTrianglesPart->setDrawable( drawGeo.p() );
    m_nativeTrianglesPart->setSourceInfo( objectSourceInfo.p() );
}
