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

#include "cafDisplayCoordTransform.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryTools.h"
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

    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<ElementVectorResultVisualization> tensorVisualizations;

    double characteristicCellSize = eclipseCase->characteristicCellSize();
    float  arrowConstantScaling   = 10.0 * result->sizeScale() * characteristicCellSize;

    double min, max;
    result->mappingRange( min, max );

    double maxAbsResult = 1.0;
    if ( min != cvf::UNDEFINED_DOUBLE && max != cvf::UNDEFINED_DOUBLE )
    {
        maxAbsResult = std::max( cvf::Math::abs( max ), cvf::Math::abs( min ) );
    }

    float arrowScaling = arrowConstantScaling;
    if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT )
    {
        arrowScaling = arrowConstantScaling / maxAbsResult;
    }

    if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT_LOG )
    {
        arrowScaling = result->sizeScale() * scaleLogarithmically( maxAbsResult );
    }

    std::vector<RigEclipseResultAddress> addresses;
    result->resultAddressesIJK( addresses );

    std::vector<cvf::StructGridInterface::FaceType> directions;
    std::vector<RigEclipseResultAddress>            resultAddresses;

    for ( size_t fluidIndex = 0; fluidIndex < addresses.size(); fluidIndex += 3 )
    {
        if ( result->showVectorI() )
        {
            if ( fluidIndex == 0 ) directions.push_back( cvf::StructGridInterface::POS_I );
            resultAddresses.push_back( addresses[0 + fluidIndex] );
        }
        if ( result->showVectorJ() )
        {
            if ( fluidIndex == 0 ) directions.push_back( cvf::StructGridInterface::POS_J );
            resultAddresses.push_back( addresses[1 + fluidIndex] );
        }
        if ( result->showVectorK() )
        {
            if ( fluidIndex == 0 ) directions.push_back( cvf::StructGridInterface::POS_K );
            resultAddresses.push_back( addresses[2 + fluidIndex] );
        }
    }

    RigCaseCellResultsData* resultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const std::vector<RigCell>& cells = eclipseCase->mainGrid()->globalCellArray();

    auto getFaceCenterAndNormal =
        [activeCellInfo, cells, arrowScaling, displayCordXf]( size_t                             globalCellIdx,
                                                              cvf::StructGridInterface::FaceType faceType,
                                                              cvf::Vec3d&                        faceCenter,
                                                              cvf::Vec3d&                        faceNormal ) {
            faceCenter = displayCordXf->transformToDisplayCoord( cells[globalCellIdx].faceCenter( faceType ) );
            cvf::Vec3d cellCenter = displayCordXf->transformToDisplayCoord( cells[globalCellIdx].center() );
            faceNormal            = ( faceCenter - cellCenter ).getNormalized() * arrowScaling;
        };

    for ( size_t gcIdx = 0; gcIdx < cells.size(); ++gcIdx )
    {
        if ( !cells[gcIdx].isInvalid() && activeCellInfo->isActive( gcIdx ) )
        {
            size_t resultIdx = activeCellInfo->cellResultIndex( gcIdx );
            if ( result->vectorView() == RimElementVectorResult::VectorView::PER_FACE )
            {
                for ( int dir = 0; dir < static_cast<int>( directions.size() ); dir++ )
                {
                    double resultValue = 0.0;
                    for ( size_t flIdx = dir; flIdx < resultAddresses.size(); flIdx += directions.size() )
                    {
                        resultValue +=
                            resultsData->cellScalarResults( resultAddresses[flIdx], timeStepIndex ).at( resultIdx );
                    }

                    if ( std::abs( resultValue ) >= result->threshold() )
                    {
                        cvf::Vec3d faceCenter;
                        cvf::Vec3d faceNormal;
                        getFaceCenterAndNormal( gcIdx, directions[dir], faceCenter, faceNormal );

                        if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT )
                        {
                            faceNormal *= std::abs( resultValue );
                        }
                        else if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT_LOG )
                        {
                            faceNormal *= std::abs( scaleLogarithmically( std::abs( resultValue ) ) );
                        }

                        tensorVisualizations.push_back(
                            ElementVectorResultVisualization( faceCenter,
                                                              faceNormal,
                                                              resultValue,
                                                              std::cbrt( cells[gcIdx].volume() / 3.0 ) ) );
                    }
                }
            }
            else if ( result->vectorView() == RimElementVectorResult::VectorView::CELL_CENTER_TOTAL )
            {
                cvf::Vec3d aggregatedVector;
                cvf::Vec3d aggregatedResult;
                for ( int dir = 0; dir < static_cast<int>( directions.size() ); dir++ )
                {
                    double resultValue = 0.0;
                    for ( size_t flIdx = dir; flIdx < resultAddresses.size(); flIdx += directions.size() )
                    {
                        resultValue +=
                            resultsData->cellScalarResults( resultAddresses[flIdx], timeStepIndex ).at( resultIdx );
                    }

                    cvf::Vec3d faceCenter;
                    cvf::Vec3d faceNormal;
                    cvf::Vec3d faceNormalScaled;
                    getFaceCenterAndNormal( gcIdx, directions[dir], faceCenter, faceNormal );
                    faceNormalScaled = faceNormal;

                    if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT )
                    {
                        faceNormalScaled *= resultValue;
                    }
                    else if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT_LOG )
                    {
                        faceNormalScaled *= ( 1.0 - static_cast<double>( std::signbit( resultValue ) ) * 2.0 ) *
                                            scaleLogarithmically( std::abs( resultValue ) );
                    }

                    aggregatedVector += faceNormalScaled;

                    // If the vector is scaled in a logarithmic scale, the result should still be the same as before.
                    // Hence, we need to separate the result value from the vector.
                    aggregatedResult += faceNormal.getNormalized() * resultValue;
                }
                if ( aggregatedResult.length() >= result->threshold() )
                {
                    tensorVisualizations.push_back(
                        ElementVectorResultVisualization( displayCordXf->transformToDisplayCoord( cells[gcIdx].center() ),
                                                          aggregatedVector,
                                                          aggregatedResult.length(),
                                                          std::cbrt( cells[gcIdx].volume() / 3.0 ) ) );
                }
            }
        }
    }

    RigNNCData* nncData           = eclipseCaseData->mainGrid()->nncData();
    size_t      numNncConnections = nncData->connections().size();

    if ( result->showNncData() )
    {
        std::vector<const std::vector<std::vector<double>>*> nncResultVals;
        std::vector<RigEclipseResultAddress>                 combinedAddresses;
        result->resultAddressesCombined( combinedAddresses );

        for ( size_t flIdx = 0; flIdx < combinedAddresses.size(); flIdx++ )
        {
            if ( combinedAddresses[flIdx].m_resultCatType == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
            {
                nncResultVals.push_back( nncData->dynamicConnectionScalarResult( combinedAddresses[flIdx] ) );
            }
        }

        for ( size_t nIdx = 0; nIdx < numNncConnections; ++nIdx )
        {
            const RigConnection& conn = nncData->connections()[nIdx];
            if ( conn.polygon().size() )
            {
                double resultValue = 0.0;
                for ( size_t flIdx = 0; flIdx < nncResultVals.size(); flIdx++ )
                {
                    if ( nIdx < nncResultVals.at( flIdx )->at( timeStepIndex ).size() )
                    {
                        resultValue += nncResultVals.at( flIdx )->at( timeStepIndex )[nIdx];
                    }
                }
                cvf::Vec3d connCenter =
                    static_cast<cvf::Vec3d>( cvf::GeometryTools::computePolygonCenter<cvf::Vec3f>( conn.polygon() ) );

                cvf::Vec3d faceCenter;
                cvf::Vec3d connNormal;
                getFaceCenterAndNormal( conn.c1GlobIdx(), conn.face(), faceCenter, connNormal );

                if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT )
                {
                    connNormal *= std::abs( resultValue );
                }
                else if ( result->scaleMethod() == RimElementVectorResult::ScaleMethod::RESULT_LOG )
                {
                    connNormal *= scaleLogarithmically( std::abs( resultValue ) );
                }

                if ( std::abs( resultValue ) >= result->threshold() )
                {
                    tensorVisualizations.push_back(
                        ElementVectorResultVisualization( displayCordXf->transformToDisplayCoord( connCenter ),
                                                          connNormal,
                                                          resultValue,
                                                          std::cbrt( cells[conn.c1GlobIdx()].volume() / 3.0 ) ) );
                }
            }
        }
    }

    if ( !tensorVisualizations.empty() )
    {
        cvf::ref<cvf::Part> partIdx = createPart( *result, tensorVisualizations );
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
    std::vector<uint> shaftIndices;
    shaftIndices.reserve( tensorVisualizations.size() * 2 );

    std::vector<uint> headIndices;
    headIndices.reserve( tensorVisualizations.size() * 6 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( tensorVisualizations.size() * 7 );

    uint counter = 0;
    for ( ElementVectorResultVisualization tensor : tensorVisualizations )
    {
        for ( const cvf::Vec3f& vertex : createArrowVertices( tensor ) )
        {
            vertices.push_back( vertex );
        }

        for ( const uint& index : createArrowShaftIndices( counter ) )
        {
            shaftIndices.push_back( index );
        }

        for ( const uint& index : createArrowHeadIndices( counter ) )
        {
            headIndices.push_back( index );
        }

        counter += 7;
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntShaft =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexArrayShaft = new cvf::UIntArray( shaftIndices );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntHead =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_TRIANGLES );
    cvf::ref<cvf::UIntArray> indexArrayHead = new cvf::UIntArray( headIndices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntShaft->setIndices( indexArrayShaft.p() );
    drawable->addPrimitiveSet( indexedUIntShaft.p() );

    indexedUIntHead->setIndices( indexArrayHead.p() );
    drawable->addPrimitiveSet( indexedUIntHead.p() );

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
    if ( vectorColors == RimElementVectorResult::TensorColors::RESULT_COLORS )
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

    size_t vertexCount = elementVectorResultVisualizations.size() * 7;
    if ( textureCoords->size() != vertexCount ) textureCoords->reserve( vertexCount );

    for ( auto evrViz : elementVectorResultVisualizations )
    {
        for ( size_t vxIdx = 0; vxIdx < 7; ++vxIdx )
        {
            cvf::Vec2f texCoord = mapper->mapToTextureCoord( std::abs( evrViz.result ) );
            textureCoords->add( texCoord );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 7>
    RivElementVectorResultPartMgr::createArrowVertices( const ElementVectorResultVisualization& evrViz ) const
{
    std::array<cvf::Vec3f, 7> vertices;

    RimElementVectorResult* result = m_rimReservoirView->elementVectorResult();
    if ( !result ) return vertices;

    cvf::Vec3f headTop    = evrViz.faceCenter + evrViz.faceNormal;
    cvf::Vec3f shaftStart = evrViz.faceCenter;
    if ( result->vectorSuraceCrossingLocation() == RimElementVectorResult::VectorSurfaceCrossingLocation::VECTOR_CENTER &&
         result->vectorView() == RimElementVectorResult::VectorView::PER_FACE )
    {
        headTop    = evrViz.faceCenter + evrViz.faceNormal / 2.0;
        shaftStart = evrViz.faceCenter - evrViz.faceNormal / 2.0;
    }

    // Flip arrow for negative results and if the vector is not aggregated (in which case we do not have any negative
    // result)
    if ( evrViz.result < 0 && result->vectorView() != RimElementVectorResult::VectorView::CELL_CENTER_TOTAL )
    {
        std::swap( headTop, shaftStart );
    }

    float headLength = std::min<float>( evrViz.approximateCellLength / 5.0f, ( headTop - shaftStart ).length() / 2.0 );

    // A fixed size is preferred here
    cvf::Vec3f headBottom = headTop - ( headTop - shaftStart ).getNormalized() * headLength;

    cvf::Vec3f headBottomDirection1 = evrViz.faceNormal ^ evrViz.faceCenter;
    cvf::Vec3f headBottomDirection2 = headBottomDirection1 ^ evrViz.faceNormal;
    cvf::Vec3f arrowBottomSegment1  = headBottomDirection1.getNormalized() * headLength / 8.0f;
    cvf::Vec3f arrowBottomSegment2  = headBottomDirection2.getNormalized() * headLength / 8.0f;

    vertices[0] = shaftStart;
    vertices[1] = headBottom;
    vertices[2] = headBottom + arrowBottomSegment1;
    vertices[3] = headBottom - arrowBottomSegment1;
    vertices[4] = headTop;
    vertices[5] = headBottom + arrowBottomSegment2;
    vertices[6] = headBottom - arrowBottomSegment2;

    return vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<uint, 2> RivElementVectorResultPartMgr::createArrowShaftIndices( uint startIndex ) const
{
    std::array<uint, 2> indices;

    indices[0] = startIndex;
    indices[1] = startIndex + 1;

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<uint, 6> RivElementVectorResultPartMgr::createArrowHeadIndices( uint startIndex ) const
{
    std::array<uint, 6> indices;

    indices[0] = startIndex + 2;
    indices[1] = startIndex + 3;
    indices[2] = startIndex + 4;

    indices[3] = startIndex + 5;
    indices[4] = startIndex + 6;
    indices[5] = startIndex + 4;
    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivElementVectorResultPartMgr::scaleLogarithmically( double value ) const
{
    // If values are smaller than one, the logarithm would return
    // increasing negative values the smaller the number is. However, small
    // numbers shall remain small and not be scaled up. In order to achieve this,
    // add 1.0 to small values in order to still obtain small positive numbers after scaling.
    if ( value <= 1.0 )
    {
        value += 1.0;
    }
    return std::log10( value );
}
