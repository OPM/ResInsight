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

#include "RivTensorResultPartMgr.h"

#include "RiaColorTables.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimRegularLegendConfig.h"
#include "RimTensorResults.h"

#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemTypes.h"
#include "RigGeoMechCaseData.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivGeoMechPartMgr.h"
#include "RivGeoMechPartMgrCache.h"
#include "RivGeoMechVizLogic.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"
#include "cafTensor3.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfShaderProgram.h"
#include "cvfStructGridGeometryGenerator.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTensorResultPartMgr::RivTensorResultPartMgr( RimGeoMechView* reservoirView )
{
    m_rimReservoirView = reservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivTensorResultPartMgr::~RivTensorResultPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t frameIndex ) const
{
    CVF_ASSERT( model );

    if ( m_rimReservoirView.isNull() ) return;
    if ( !m_rimReservoirView->geoMechCase() ) return;
    if ( !m_rimReservoirView->geoMechCase()->geoMechData() ) return;

    if ( !m_rimReservoirView->tensorResults()->showTensors() ) return;

    RigFemPartCollection* femParts = m_rimReservoirView->femParts();
    if ( !femParts ) return;

    std::vector<TensorVisualization> tensorVisualizations;

    RigFemResultAddress address = m_rimReservoirView->tensorResults()->selectedTensorResult();
    if ( !isTensorAddress( address ) ) return;

    RigFemPartResultsCollection* resultCollection = m_rimReservoirView->geoMechCase()->geoMechData()->femPartResults();
    if ( !resultCollection ) return;

    for ( int partIdx = 0; partIdx < femParts->partCount(); partIdx++ )
    {
        std::vector<caf::Ten3f> vertexTensors = resultCollection->tensors( address, partIdx, (int)frameIndex );

        const RigFemPart*       part = femParts->part( partIdx );
        std::vector<caf::Ten3f> elmTensors;

        calculateElementTensors( *part, vertexTensors, &elmTensors );

        std::array<std::vector<float>, 3>      elmPrincipals;
        std::vector<std::array<cvf::Vec3f, 3>> elmPrincipalDirections;

        calculatePrincipalsAndDirections( elmTensors, &elmPrincipals, &elmPrincipalDirections );

        std::vector<RivGeoMechPartMgrCache::Key> partKeys =
            m_rimReservoirView->vizLogic()->keysToVisiblePartMgrs( (int)frameIndex );

        RigFemPartNodes nodes = part->nodes();

        float arrowConstantScaling =
            0.5 * m_rimReservoirView->tensorResults()->sizeScale() * part->characteristicElementSize();

        double min, max;
        m_rimReservoirView->tensorResults()->mappingRange( &min, &max );

        double maxAbsResult = 1.0;
        if ( min != cvf::UNDEFINED_DOUBLE && max != cvf::UNDEFINED_DOUBLE )
        {
            maxAbsResult = std::max( cvf::Math::abs( max ), cvf::Math::abs( min ) );
        }

        float arrowResultScaling = arrowConstantScaling / maxAbsResult;

        cvf::ref<RivGeoMechPartMgrCache> partMgrCache = m_rimReservoirView->vizLogic()->partMgrCache();

        for ( const RivGeoMechPartMgrCache::Key& partKey : partKeys )
        {
            const RivGeoMechPartMgr* partMgr = partMgrCache->partMgr( partKey );
            for ( auto mgr : partMgr->femPartMgrs() )
            {
                const RivFemPartGeometryGenerator* surfaceGenerator     = mgr->surfaceGenerator();
                const std::vector<size_t>& quadVerticesToNodeIdxMapping = surfaceGenerator->quadVerticesToNodeIdxMapping();
                const std::vector<size_t>& quadVerticesToElmIdx = surfaceGenerator->quadVerticesToGlobalElmIdx();

                for ( int quadVertex = 0; quadVertex < static_cast<int>( quadVerticesToNodeIdxMapping.size() );
                      quadVertex += 4 )
                {
                    cvf::Vec3f center = nodes.coordinates.at( quadVerticesToNodeIdxMapping[quadVertex] ) +
                                        nodes.coordinates.at( quadVerticesToNodeIdxMapping[quadVertex + 2] );

                    cvf::Vec3d displayCoord =
                        m_rimReservoirView->displayCoordTransform()->transformToDisplayCoord( cvf::Vec3d( center / 2 ) );

                    cvf::Vec3f faceNormal = calculateFaceNormal( nodes, quadVerticesToNodeIdxMapping, quadVertex );

                    size_t elmIdx = quadVerticesToElmIdx[quadVertex];

                    cvf::Vec3f result1, result2, result3;

                    if ( m_rimReservoirView->tensorResults()->scaleMethod() == RimTensorResults::RESULT )
                    {
                        result1.set( elmPrincipalDirections[elmIdx][0] * arrowResultScaling * elmPrincipals[0][elmIdx] );
                        result2.set( elmPrincipalDirections[elmIdx][1] * arrowResultScaling * elmPrincipals[1][elmIdx] );
                        result3.set( elmPrincipalDirections[elmIdx][2] * arrowResultScaling * elmPrincipals[2][elmIdx] );
                    }
                    else
                    {
                        result1.set( elmPrincipalDirections[elmIdx][0] * arrowConstantScaling );
                        result2.set( elmPrincipalDirections[elmIdx][1] * arrowConstantScaling );
                        result3.set( elmPrincipalDirections[elmIdx][2] * arrowConstantScaling );
                    }

                    if ( isDrawable( result1, m_rimReservoirView->tensorResults()->showPrincipal1() ) )
                    {
                        tensorVisualizations.push_back( TensorVisualization( cvf::Vec3f( displayCoord ),
                                                                             result1,
                                                                             faceNormal,
                                                                             isPressure( elmPrincipals[0][elmIdx] ),
                                                                             1,
                                                                             elmPrincipals[0][elmIdx] ) );
                        tensorVisualizations.push_back( TensorVisualization( cvf::Vec3f( displayCoord ),
                                                                             -result1,
                                                                             faceNormal,
                                                                             isPressure( elmPrincipals[0][elmIdx] ),
                                                                             1,
                                                                             elmPrincipals[0][elmIdx] ) );
                    }

                    if ( isDrawable( result2, m_rimReservoirView->tensorResults()->showPrincipal2() ) )
                    {
                        tensorVisualizations.push_back( TensorVisualization( cvf::Vec3f( displayCoord ),
                                                                             result2,
                                                                             faceNormal,
                                                                             isPressure( elmPrincipals[1][elmIdx] ),
                                                                             2,
                                                                             elmPrincipals[1][elmIdx] ) );
                        tensorVisualizations.push_back( TensorVisualization( cvf::Vec3f( displayCoord ),
                                                                             -result2,
                                                                             faceNormal,
                                                                             isPressure( elmPrincipals[1][elmIdx] ),
                                                                             2,
                                                                             elmPrincipals[1][elmIdx] ) );
                    }

                    if ( isDrawable( result3, m_rimReservoirView->tensorResults()->showPrincipal3() ) )
                    {
                        tensorVisualizations.push_back( TensorVisualization( cvf::Vec3f( displayCoord ),
                                                                             result3,
                                                                             faceNormal,
                                                                             isPressure( elmPrincipals[2][elmIdx] ),
                                                                             3,
                                                                             elmPrincipals[2][elmIdx] ) );
                        tensorVisualizations.push_back( TensorVisualization( cvf::Vec3f( displayCoord ),
                                                                             -result3,
                                                                             faceNormal,
                                                                             isPressure( elmPrincipals[2][elmIdx] ),
                                                                             3,
                                                                             elmPrincipals[2][elmIdx] ) );
                    }
                }
            }
        }
    }

    if ( !tensorVisualizations.empty() )
    {
        cvf::ref<cvf::Part> partIdx = createPart( tensorVisualizations );
        model->addPart( partIdx.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::calculateElementTensors( const RigFemPart&              part,
                                                      const std::vector<caf::Ten3f>& vertexTensors,
                                                      std::vector<caf::Ten3f>*       elmTensors )
{
    CVF_ASSERT( elmTensors );

    size_t elmCount = part.elementCount();
    elmTensors->resize( elmCount );

    for ( int elmIdx = 0; elmIdx < static_cast<int>( elmCount ); elmIdx++ )
    {
        if ( RigFemTypes::elementNodeCount( part.elementType( elmIdx ) ) == 8 )
        {
            caf::Ten3f tensorSumOfElmNodes = vertexTensors[part.elementNodeResultIdx( elmIdx, 0 )];
            for ( int i = 1; i < 8; i++ )
            {
                tensorSumOfElmNodes = tensorSumOfElmNodes + vertexTensors[part.elementNodeResultIdx( elmIdx, i )];
            }

            ( *elmTensors )[elmIdx] = tensorSumOfElmNodes * ( 1.0 / 8.0 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::calculatePrincipalsAndDirections( const std::vector<caf::Ten3f>&          tensors,
                                                               std::array<std::vector<float>, 3>*      principals,
                                                               std::vector<std::array<cvf::Vec3f, 3>>* principalDirections )
{
    CVF_ASSERT( principals );
    CVF_ASSERT( principalDirections );

    size_t elmCount = tensors.size();

    ( *principals )[0].resize( elmCount );
    ( *principals )[1].resize( elmCount );
    ( *principals )[2].resize( elmCount );

    ( *principalDirections ).resize( elmCount );

    for ( size_t nIdx = 0; nIdx < elmCount; ++nIdx )
    {
        cvf::Vec3f principalDirs[3];
        cvf::Vec3f principalValues = tensors[nIdx].calculatePrincipals( principalDirs );

        ( *principals )[0][nIdx] = principalValues[0];
        ( *principals )[1][nIdx] = principalValues[1];
        ( *principals )[2][nIdx] = principalValues[2];

        ( *principalDirections )[nIdx][0] = principalDirs[0];
        ( *principalDirections )[nIdx][1] = principalDirs[1];
        ( *principalDirections )[nIdx][2] = principalDirs[2];
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivTensorResultPartMgr::calculateFaceNormal( const RigFemPartNodes&     nodes,
                                                        const std::vector<size_t>& quadVerticesToNodeIdxMapping,
                                                        int                        quadVertex )
{
    cvf::Vec3f diag1 = nodes.coordinates.at( quadVerticesToNodeIdxMapping[quadVertex] ) -
                       nodes.coordinates.at( quadVerticesToNodeIdxMapping[quadVertex + 2] );

    cvf::Vec3f diag2 = nodes.coordinates.at( quadVerticesToNodeIdxMapping[quadVertex + 1] ) -
                       nodes.coordinates.at( quadVerticesToNodeIdxMapping[quadVertex + 3] );

    return ( diag1 ^ diag2 ).getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivTensorResultPartMgr::createPart( const std::vector<TensorVisualization>& tensorVisualizations ) const
{
    std::vector<uint> indices;
    indices.reserve( tensorVisualizations.size() * 5 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( tensorVisualizations.size() * 5 );

    uint counter = 0;
    for ( TensorVisualization tensor : tensorVisualizations )
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

    cvf::ScalarMapper* activeScalerMapper = nullptr;

    cvf::ref<cvf::ScalarMapperDiscreteLinear> discreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;
    auto                                      vectorColors = m_rimReservoirView->tensorResults()->vectorColors();
    if ( vectorColors == RimTensorResults::RESULT_COLORS )
    {
        activeScalerMapper = m_rimReservoirView->tensorResults()->arrowColorLegendConfig()->scalarMapper();

        createResultColorTextureCoords( lineTexCoords.p(), tensorVisualizations, activeScalerMapper );
    }
    else
    {
        activeScalerMapper = discreteScalarMapper.p();

        createOneColorPerPrincipalScalarMapper( vectorColors, discreteScalarMapper.p() );
        createOneColorPerPrincipalTextureCoords( lineTexCoords.p(), tensorVisualizations, discreteScalarMapper.p() );
    }

    caf::ScalarMapperEffectGenerator surfEffGen( activeScalerMapper, caf::PO_1 );

    if ( m_rimReservoirView && m_rimReservoirView->isLightingDisabled() )
    {
        surfEffGen.disableLighting( true );
    }

    caf::ScalarMapperMeshEffectGenerator meshEffGen( activeScalerMapper );
    cvf::ref<cvf::Effect>                scalarMapperMeshEffect = meshEffGen.generateUnCachedEffect();

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( scalarMapperMeshEffect.p() );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::createOneColorPerPrincipalScalarMapper( const RimTensorResults::TensorColors& colorSet,
                                                                     cvf::ScalarMapperDiscreteLinear* scalarMapper )
{
    CVF_ASSERT( scalarMapper );

    cvf::Color3ubArray arrowColors;
    arrowColors.resize( 3 );

    if ( colorSet == RimTensorResults::WHITE_GRAY_BLACK )
    {
        arrowColors = RiaColorTables::tensorWhiteGrayBlackPaletteColors().color3ubArray();
    }
    else if ( colorSet == RimTensorResults::ORANGE_BLUE_WHITE )
    {
        arrowColors = RiaColorTables::tensorOrangeBlueWhitePaletteColors().color3ubArray();
    }
    else if ( colorSet == RimTensorResults::MAGENTA_BROWN_GRAY )
    {
        arrowColors = RiaColorTables::tensorsMagentaBrownGrayPaletteColors().color3ubArray();
    }

    scalarMapper->setColors( arrowColors );

    // Using a linear color mapper to set colors for three discrete principal numbers (1, 2, 3)
    // by setting the 3 + 1 interval levels so the principal numbers match the center of the intervals.
    std::set<double> levelValues = {0.5, 1.5, 2.5, 3.5};
    scalarMapper->setLevelsFromValues( levelValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::createOneColorPerPrincipalTextureCoords( cvf::Vec2fArray* textureCoords,
                                                                      const std::vector<TensorVisualization>& tensorVisualizations,
                                                                      const cvf::ScalarMapper*                mapper )
{
    CVF_ASSERT( textureCoords );
    CVF_ASSERT( mapper );

    size_t vertexCount = tensorVisualizations.size() * 5;
    if ( textureCoords->size() != vertexCount ) textureCoords->reserve( vertexCount );

    for ( auto tensor : tensorVisualizations )
    {
        for ( size_t vxIdx = 0; vxIdx < 5; ++vxIdx )
        {
            cvf::Vec2f texCoord = mapper->mapToTextureCoord( tensor.principalNumber );
            textureCoords->add( texCoord );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivTensorResultPartMgr::createResultColorTextureCoords( cvf::Vec2fArray*                        textureCoords,
                                                             const std::vector<TensorVisualization>& tensorVisualizations,
                                                             const cvf::ScalarMapper*                mapper )
{
    CVF_ASSERT( textureCoords );
    CVF_ASSERT( mapper );

    size_t vertexCount = tensorVisualizations.size() * 5;
    if ( textureCoords->size() != vertexCount ) textureCoords->reserve( vertexCount );

    for ( auto tensor : tensorVisualizations )
    {
        for ( size_t vxIdx = 0; vxIdx < 5; ++vxIdx )
        {
            cvf::Vec2f texCoord = mapper->mapToTextureCoord( tensor.principalValue );
            textureCoords->add( texCoord );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isTensorAddress( RigFemResultAddress address )
{
    if ( !( address.resultPosType == RIG_ELEMENT_NODAL || address.resultPosType == RIG_INTEGRATION_POINT ) )
    {
        return false;
    }
    if ( !( address.fieldName == "SE" || address.fieldName == "ST" || address.fieldName == "NE" ) )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isValid( cvf::Vec3f resultVector )
{
    // nan
    if ( resultVector.x() != resultVector.x() || resultVector.y() != resultVector.y() ||
         resultVector.z() != resultVector.z() )
    {
        return false;
    }

    // inf
    if ( resultVector.x() == HUGE_VAL || resultVector.y() == HUGE_VAL || resultVector.z() == HUGE_VAL ||
         resultVector.x() == -HUGE_VAL || resultVector.y() == -HUGE_VAL || resultVector.z() == -HUGE_VAL )
    {
        return false;
    }

    // zero
    if ( resultVector == cvf::Vec3f::ZERO )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isPressure( float principalValue )
{
    if ( principalValue >= 0 )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivTensorResultPartMgr::isDrawable( cvf::Vec3f resultVector, bool showPrincipal ) const
{
    if ( !showPrincipal )
    {
        return false;
    }

    if ( !isValid( resultVector ) )
    {
        return false;
    }

    if ( resultVector.length() <= m_rimReservoirView->tensorResults()->threshold() )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 5> RivTensorResultPartMgr::createArrowVertices( const TensorVisualization& tensorVisualization ) const
{
    std::array<cvf::Vec3f, 5> vertices;

    cvf::Vec3f headTop;
    cvf::Vec3f shaftStart;

    if ( tensorVisualization.isPressure )
    {
        headTop    = tensorVisualization.vertex;
        shaftStart = tensorVisualization.vertex + tensorVisualization.result;
    }
    else
    {
        headTop    = tensorVisualization.vertex + tensorVisualization.result;
        shaftStart = tensorVisualization.vertex;
    }

    float headWidth = 0.05 * tensorVisualization.result.length();

    cvf::Vec3f headBottom = headTop - ( headTop - shaftStart ) * 0.2f;

    cvf::Vec3f headBottomDirection = tensorVisualization.result ^ tensorVisualization.faceNormal;
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
std::array<uint, 8> RivTensorResultPartMgr::createArrowIndices( uint startIndex ) const
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
