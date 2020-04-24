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

#include "RivElementVectorResultPartMgr.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimElementVectorResult.h"
#include "RimRegularLegendConfig.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfShaderProgram.h"
#include "cvfStructGrid.h"
#include "cvfStructGridGeometryGenerator.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivElementVectorResultPartMgr::RivElementVectorResultPartMgr( RimEclipseView* reservoirView )
{
    m_rimReservoirView = reservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivElementVectorResultPartMgr::~RivElementVectorResultPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivElementVectorResultPartMgr::setTransform( cvf::Transform* scaleTransform )
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivElementVectorResultPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t timeStepIndex )
{
    CVF_ASSERT( model );

    if ( m_rimReservoirView.isNull() ) return;

    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    RimElementVectorResult* result = m_rimReservoirView->elementVectorResult();
    if ( !result ) return;

    if ( !result->showResult() ) return;

    std::vector<ElementVectorResultVisualization> tensorVisualizations;

    double characteristicCellSize = eclipseCase->characteristicCellSize();
    float  arrowConstantScaling   = 0.5 * result->sizeScale() * characteristicCellSize;

    double min, max;
    result->mappingRange( min, max );

    double maxAbsResult = 1.0;
    if ( min != cvf::UNDEFINED_DOUBLE && max != cvf::UNDEFINED_DOUBLE )
    {
        maxAbsResult = std::max( cvf::Math::abs( max ), cvf::Math::abs( min ) );
    }

    float arrowScaling = arrowConstantScaling;
    if ( result->scaleMethod() == RimElementVectorResult::RESULT )
    {
        arrowScaling = arrowConstantScaling / maxAbsResult;
    }

    std::vector<RigEclipseResultAddress> addresses;
    result->resultAddressIJK( addresses );

    std::vector<cvf::StructGridInterface::FaceType> directions;
    std::vector<RigEclipseResultAddress>            resultAddresses;
    if ( result->showVectorI() )
    {
        directions.push_back( cvf::StructGridInterface::POS_I );
        resultAddresses.push_back( addresses[0] );
    }
    if ( result->showVectorJ() )
    {
        directions.push_back( cvf::StructGridInterface::POS_J );
        resultAddresses.push_back( addresses[1] );
    }
    if ( result->showVectorK() )
    {
        directions.push_back( cvf::StructGridInterface::POS_K );
        resultAddresses.push_back( addresses[2] );
    }

    RigCaseCellResultsData* resultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const cvf::Vec3d offset = eclipseCase->mainGrid()->displayModelOffset();

    const std::vector<RigCell>& cells = eclipseCase->mainGrid()->globalCellArray();
    for ( int gcIdx = 0; gcIdx < static_cast<int>( cells.size() ); ++gcIdx )
    {
        if ( !cells[gcIdx].isInvalid() && activeCellInfo->isActive( gcIdx ) )
        {
            for ( int dir = 0; dir < static_cast<int>( directions.size() ); dir++ )
            {
                size_t resultIdx = activeCellInfo->cellResultIndex( gcIdx );
                double resultValue = resultsData->cellScalarResults( resultAddresses[dir], timeStepIndex ).at( resultIdx );

                if ( std::abs( resultValue ) >= result->threshold() )
                {
                    cvf::Vec3d faceCenter = cells[gcIdx].faceCenter( directions[dir] ) - offset;
                    cvf::Vec3d cellCenter = cells[gcIdx].center() - offset;
                    cvf::Vec3d faceNormal = ( faceCenter - cellCenter ).getNormalized() * arrowScaling;

                    if ( result->scaleMethod() == RimElementVectorResult::RESULT )
                    {
                        faceNormal *= std::abs( resultValue );
                    }

                    tensorVisualizations.push_back(
                        ElementVectorResultVisualization( faceCenter, faceNormal, resultValue ) );
                }
            }
        }
    }

    if ( !tensorVisualizations.empty() )
    {
        cvf::ref<cvf::Part> partIdx = createPart( *result, tensorVisualizations );
        partIdx->setTransform( m_scaleTransform.p() );
        partIdx->updateBoundingBox();
        model->addPart( partIdx.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>
    RivElementVectorResultPartMgr::createPart( const RimElementVectorResult&                        result,
                                               const std::vector<ElementVectorResultVisualization>& tensorVisualizations ) const
{
    std::vector<uint> indices;
    indices.reserve( tensorVisualizations.size() * 5 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( tensorVisualizations.size() * 5 );

    uint counter = 0;
    for ( ElementVectorResultVisualization tensor : tensorVisualizations )
    {
        for ( const cvf::Vec3f& vertex : createArrowVertices( tensor ) )
        {
            vertices.push_back( vertex );
        }

        for ( const uint& index : createArrowIndices( counter ) )
        {
            indices.push_back( index );
        }

        counter += 5;
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray( indices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUInt->setIndices( indexArray.p() );
    drawable->addPrimitiveSet( indexedUInt.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>( drawable->textureCoordArray() );

    if ( lineTexCoords.isNull() )
    {
        lineTexCoords = new cvf::Vec2fArray;
    }

    const cvf::ScalarMapper* activeScalerMapper = nullptr;

    cvf::ref<cvf::Effect> effect;

    auto vectorColors = result.vectorColors();
    if ( vectorColors == RimElementVectorResult::RESULT_COLORS )
    {
        activeScalerMapper = result.legendConfig()->scalarMapper();
        createResultColorTextureCoords( lineTexCoords.p(), tensorVisualizations, activeScalerMapper );

        caf::ScalarMapperMeshEffectGenerator meshEffGen( activeScalerMapper );
        effect = meshEffGen.generateCachedEffect();
    }
    else
    {
        caf::SurfaceEffectGenerator surfaceGen( result.getUniformVectorColor(), caf::PO_1 );
        surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
        effect = surfaceGen.generateCachedEffect();
    }

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivElementVectorResultPartMgr::createResultColorTextureCoords(
    cvf::Vec2fArray*                                     textureCoords,
    const std::vector<ElementVectorResultVisualization>& elementVectorResultVisualizations,
    const cvf::ScalarMapper*                             mapper )
{
    CVF_ASSERT( textureCoords );
    CVF_ASSERT( mapper );

    size_t vertexCount = elementVectorResultVisualizations.size() * 5;
    if ( textureCoords->size() != vertexCount ) textureCoords->reserve( vertexCount );

    for ( auto evrViz : elementVectorResultVisualizations )
    {
        for ( size_t vxIdx = 0; vxIdx < 5; ++vxIdx )
        {
            cvf::Vec2f texCoord = mapper->mapToTextureCoord( evrViz.result );
            textureCoords->add( texCoord );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 5>
    RivElementVectorResultPartMgr::createArrowVertices( const ElementVectorResultVisualization& evrViz ) const
{
    std::array<cvf::Vec3f, 5> vertices;

    cvf::Vec3f headTop    = evrViz.faceCenter + evrViz.faceNormal;
    cvf::Vec3f shaftStart = evrViz.faceCenter;

    // Flip arrow for negative results
    if ( evrViz.result < 0 )
    {
        std::swap( headTop, shaftStart );
    }

    float headWidth = 0.05 * evrViz.faceNormal.length();

    cvf::Vec3f headBottom = headTop - ( headTop - shaftStart ) * 0.2f;

    cvf::Vec3f headBottomDirection = evrViz.faceNormal ^ evrViz.faceCenter;
    cvf::Vec3f arrowBottomSegment  = headBottomDirection.getNormalized() * headWidth;

    vertices[0] = shaftStart;
    vertices[1] = headBottom;
    vertices[2] = headBottom + arrowBottomSegment;
    vertices[3] = headBottom - arrowBottomSegment;
    vertices[4] = headTop;

    return vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<uint, 8> RivElementVectorResultPartMgr::createArrowIndices( uint startIndex ) const
{
    std::array<uint, 8> indices;

    indices[0] = startIndex;
    indices[1] = startIndex + 1;
    indices[2] = startIndex + 2;
    indices[3] = startIndex + 3;
    indices[4] = startIndex + 3;
    indices[5] = startIndex + 4;
    indices[6] = startIndex + 4;
    indices[7] = startIndex + 2;

    return indices;
}
