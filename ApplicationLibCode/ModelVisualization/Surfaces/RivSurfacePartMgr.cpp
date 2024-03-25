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

#include "RiaPreferences.h"

#include "ProjectDataModel/RimCase.h"
#include "RigSurface.h"
#include "Rim3dView.h"
#include "RimRegularLegendConfig.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "RivIntersectionGeometryGeneratorInterface.h"
#include "RivIntersectionHexGridInterface.h"
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
RivSurfacePartMgr::RivSurfacePartMgr( RimSurfaceInView* surface, bool nativeOnly )
    : m_surfaceInView( surface )
    , m_useNativePartsOnly( nativeOnly )
{
    CVF_ASSERT( surface );

    if ( !nativeOnly )
    {
        cvf::ref<RivIntersectionHexGridInterface> hexGrid = m_surfaceInView->createHexGridInterface();
        m_intersectionGenerator                           = new RivSurfaceIntersectionGeometryGenerator( m_surfaceInView, hexGrid.p() );
    }

    m_intersectionFacesTextureCoords = new cvf::Vec2fArray;
    m_nativeTrianglesTextureCoords   = new cvf::Vec2fArray;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivSurfacePartMgr::isNativePartMgr() const
{
    return m_useNativePartsOnly;
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
        updateNativeSurfaceColors();

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
void RivSurfacePartMgr::updateCellResultColor( int timeStepIndex )
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
        caf::SurfaceEffectGenerator surfaceGenBehind( cvf::Color4f( m_surfaceInView->surface()->color() ), caf::PO_POS_LARGE );

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

    RiaPreferences* prefs = RiaPreferences::current();

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
QString RivSurfacePartMgr::resultInfoText( Rim3dView* view, uint hitPart, cvf::Vec3d hitPoint )
{
    QString retval;

    if ( m_surfaceInView->surfaceResultDefinition()->isChecked() )
    {
        const auto& values = m_usedSurfaceData->propertyValues( m_surfaceInView->surfaceResultDefinition()->propertyName() );
        if ( values.empty() ) return "";

        const auto& ind  = m_usedSurfaceData->triangleIndices();
        const auto& vert = m_usedSurfaceData->vertices();

        size_t indIndex = (size_t)( hitPart * 3 );

        // find closest triangle corner point to hit point and show that value
        if ( ind.size() > ( indIndex + 2 ) )
        {
            uint vertIndex1 = ind[indIndex];
            uint vertIndex2 = ind[indIndex + 1];
            uint vertIndex3 = ind[indIndex + 2];

            double dist1 = vert[vertIndex1].pointDistance( hitPoint );
            double dist2 = vert[vertIndex2].pointDistance( hitPoint );
            double dist3 = vert[vertIndex3].pointDistance( hitPoint );

            double resultValue = -1.0;
            if ( vertIndex1 < values.size() ) resultValue = values[vertIndex1];
            if ( dist2 < dist1 && vertIndex2 < values.size() ) resultValue = values[vertIndex2];
            if ( ( dist3 < dist1 ) && ( dist3 < dist2 ) && vertIndex3 < values.size() ) resultValue = values[vertIndex3];

            retval += QString( "%1 : %2\n\n" ).arg( m_surfaceInView->surfaceResultDefinition()->propertyName() ).arg( resultValue );
        }
    }
    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RivSurfacePartMgr::intersectionGeometryGenerator() const
{
    if ( m_intersectionGenerator.notNull() ) return m_intersectionGenerator.p();

    return nullptr;
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
    cvf::Vec3d displayModOffset( 0, 0, 0 );

    auto view = m_surfaceInView->firstAncestorOrThisOfType<Rim3dView>();
    if ( view )
    {
        auto ownerCase = view->ownerCase();
        if ( ownerCase ) displayModOffset = ownerCase->displayModelOffset();
    }

    m_usedSurfaceData = m_surfaceInView->surface()->surfaceData();
    if ( m_usedSurfaceData.isNull() ) return;

    const std::vector<cvf::Vec3d>& vertices = m_usedSurfaceData->vertices();
    if ( vertices.empty() ) return;

    cvf::ref<cvf::Vec3fArray> cvfVertices = new cvf::Vec3fArray( vertices.size() );
    for ( size_t i = 0; i < vertices.size(); ++i )
    {
        ( *cvfVertices )[i] = cvf::Vec3f( vertices[i] - displayModOffset );
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
