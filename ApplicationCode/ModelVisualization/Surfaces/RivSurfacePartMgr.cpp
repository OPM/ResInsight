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

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimRegularLegendConfig.h"
#include "RimSurface.h"
#include "RimSurfaceInView.h"
#include "RimTernaryLegendConfig.h"

#include "RigHexIntersectionTools.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigSurface.h"

#include "RivHexGridIntersectionTools.h"
#include "RivIntersectionResultsColoringTools.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivPartPriority.h"
#include "RivReservoirSurfaceIntersectionSourceInfo.h"
#include "RivScalarMapperUtils.h"
#include "RivSurfaceIntersectionGeometryGenerator.h"
#include "RivTernaryScalarMapper.h"

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
    CVF_ASSERT( surface );

    m_intersectionFacesTextureCoords = new cvf::Vec2fArray;

    cvf::ref<RivIntersectionHexGridInterface> hexGrid = m_surfaceInView->createHexGridInterface();
    m_intersectionGenerator = new RivSurfaceIntersectionGeometryGenerator( m_surfaceInView, hexGrid.p() );
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
        this->applySingleColor();

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

    if ( false && m_nativeTrianglesPart.notNull() )
    {
        if ( !m_nativeVertexToCellIndexMap.size() )
        {
            generateNativeVertexToCellIndexMap();
        }

        RimGridView* gridView = nullptr;
        m_surfaceInView->firstAncestorOrThisOfType( gridView );

        if ( !gridView ) return;

        bool isLightingDisabled = gridView->isLightingDisabled();

        RimEclipseResultDefinition*   eclipseResDef      = nullptr;
        RimGeoMechResultDefinition*   geomResultDef      = nullptr;
        const cvf::ScalarMapper*      scalarColorMapper  = nullptr;
        const RivTernaryScalarMapper* ternaryColorMapper = nullptr;

        // Ordinary result

        if ( !eclipseResDef && !geomResultDef )
        {
            RimEclipseView* eclipseView = nullptr;
            m_surfaceInView->firstAncestorOrThisOfType( eclipseView );

            if ( eclipseView )
            {
                eclipseResDef = eclipseView->cellResult();
                if ( !scalarColorMapper ) scalarColorMapper = eclipseView->cellResult()->legendConfig()->scalarMapper();
                if ( !ternaryColorMapper )
                    ternaryColorMapper = eclipseView->cellResult()->ternaryLegendConfig()->scalarMapper();
            }

            RimGeoMechView* geoView;
            m_surfaceInView->firstAncestorOrThisOfType( geoView );

            if ( geoView )
            {
                geomResultDef = geoView->cellResult();
                if ( !scalarColorMapper ) scalarColorMapper = geoView->cellResult()->legendConfig()->scalarMapper();
            }
        }

        cvf::ref<cvf::Vec2fArray> nativeFacesTextureCoords = new cvf::Vec2fArray();

        if ( eclipseResDef )
        {
            if ( !eclipseResDef->isTernarySaturationSelected() )
            {
                RigEclipseCaseData* eclipseCaseData = eclipseResDef->eclipseCase()->eclipseCaseData();

                cvf::ref<RigResultAccessor> resultAccessor;

                if ( !RiaDefines::isPerCellFaceResult( eclipseResDef->resultVariable() ) )

                {
                    resultAccessor = RigResultAccessorFactory::createFromResultDefinition( eclipseCaseData,
                                                                                           0,
                                                                                           timeStepIndex,
                                                                                           eclipseResDef );
                }

                if ( resultAccessor.isNull() )
                {
                    resultAccessor = new RigHugeValResultAccessor;
                }

                RivSurfacePartMgr::calculateVertexTextureCoordinates( nativeFacesTextureCoords.p(),
                                                                      m_nativeVertexToCellIndexMap,
                                                                      resultAccessor.p(),
                                                                      scalarColorMapper );

                RivScalarMapperUtils::applyTextureResultsToPart( m_nativeTrianglesPart.p(),
                                                                 nativeFacesTextureCoords.p(),
                                                                 scalarColorMapper,
                                                                 1.0,
                                                                 caf::FC_NONE,
                                                                 isLightingDisabled );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::appendIntersectionGeometryPartsToModel( cvf::ModelBasicList* model,
                                                                cvf::Transform*      scaleTransform )
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

    appendNativeGeometryPartsToModel( model, scaleTransform );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::applySingleColor()
{
    {
        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( m_surfaceInView->surface()->color() ), caf::PO_1 );
        cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();

        caf::SurfaceEffectGenerator surfaceGenBehind( cvf::Color4f( m_surfaceInView->surface()->color() ), caf::PO_2 );
        cvf::ref<cvf::Effect>       effBehind = surfaceGenBehind.generateCachedEffect();

        if ( m_nativeTrianglesPart.notNull() )
        {
            m_nativeTrianglesPart->setEffect( effBehind.p() );
        }

        if ( m_intersectionFaces.notNull() )
        {
            m_intersectionFaces->setEffect( eff.p() );
        }
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
            part->setName( "Reservoir Surface" );
            part->setDrawable( geo.p() );

            // Set mapping from triangle face index to cell index
            cvf::ref<RivReservoirSurfaceIntersectionSourceInfo> si = new RivReservoirSurfaceIntersectionSourceInfo(
                m_intersectionGenerator.p() );
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
            part->setName( "Surface faultmesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionFaultMeshBit );
            part->setPriority( RivPartPriority::PartType::FaultMeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_surfaceInView ) );

            m_intersectionFaultGridLines = part;
        }
    }

    applySingleColor();
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
    m_nativeTrianglesPart->setName( "Native Reservoir Surface" );
    m_nativeTrianglesPart->setDrawable( drawGeo.p() );

    m_nativeVertexToCellIndexMap.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::generateNativeVertexToCellIndexMap()
{
    cvf::ref<RivIntersectionHexGridInterface> hexGrid = m_surfaceInView->createHexGridInterface();

    const std::vector<cvf::Vec3d>& vertices = m_usedSurfaceData->vertices();
    m_nativeVertexToCellIndexMap.resize( vertices.size(), -1 );

    for ( size_t vxIdx = 0; vxIdx < vertices.size(); ++vxIdx )
    {
        cvf::BoundingBox box;
        box.add( vertices[vxIdx] );
        std::vector<size_t> cellCandidates;
        hexGrid->findIntersectingCells( box, &cellCandidates );

        for ( size_t cellIdx : cellCandidates )
        {
            cvf::Vec3d cellCorners[8];
            hexGrid->cellCornerVertices( cellIdx, cellCorners );

            if ( RigHexIntersectionTools::isPointInCell( vertices[vxIdx], cellCorners ) )
            {
                m_nativeVertexToCellIndexMap[vxIdx] = cellIdx;
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimensional texture.
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivSurfacePartMgr::calculateVertexTextureCoordinates( cvf::Vec2fArray*           textureCoords,
                                                           const std::vector<size_t>& vertexToCellIdxMap,
                                                           const RigResultAccessor*   resultAccessor,
                                                           const cvf::ScalarMapper*   mapper )
{
    if ( !resultAccessor ) return;

    size_t numVertices = vertexToCellIdxMap.size();

    textureCoords->resize( numVertices );
    cvf::Vec2f* rawPtr = textureCoords->ptr();

#pragma omp parallel for
    for ( int vxIdx = 0; vxIdx < numVertices; vxIdx++ )
    {
        double     cellScalarValue = resultAccessor->cellScalarGlobIdx( vertexToCellIdxMap[vxIdx] );
        cvf::Vec2f texCoord        = mapper->mapToTextureCoord( cellScalarValue );
        if ( cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue ) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        rawPtr[vxIdx] = texCoord;
    }
}
