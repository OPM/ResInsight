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

#include "RivStreamlinesPartMgr.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimStreamlineInViewCollection.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigTracer.h"
#include "RigTracerPoint.h"
#include "RigWellResultPoint.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUIntScoped.h"
#include "cvfShaderProgram.h"

#include "cafPdmPointer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivStreamlinesPartMgr::RivStreamlinesPartMgr( RimEclipseView* reservoirView )
{
    m_rimReservoirView = reservoirView;
    m_count            = 0;
    m_currentT         = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivStreamlinesPartMgr::~RivStreamlinesPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t timeStepIndex )
{
    m_streamlines.clear();

    CVF_ASSERT( model );
    if ( m_rimReservoirView.isNull() ) return;

    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    RimSimWellInViewCollection* wellCollection = m_rimReservoirView->wellCollection();

    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();

    /*
    for ( RimSimWellInView* well : wellCollection->wells() )
    {
        std::vector<std::vector<cvf::Vec3d>>         pipeBranchesCLCoords;
        std::vector<std::vector<RigWellResultPoint>> pipeBranchesCellIds;

        auto noConst = const_cast<RimSimWellInView*>( well );
        RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline( noConst,
                                                                                  pipeBranchesCLCoords,
                                                                                  pipeBranchesCellIds );
        for ( size_t i = 0; i < pipeBranchesCLCoords.size(); i++ )
        {
            std::vector<cvf::Vec3d> tracerPoints;
            StreamlineVisualization visualization;
            for ( size_t j = 0; j < pipeBranchesCLCoords[i].size(); j++ )
            {
                tracerPoints.push_back( displayCordXf->transformToDisplayCoord( pipeBranchesCLCoords[i][j] ) );
            }
            visualization.tracerPoints = tracerPoints;
            streamlineVisualizations.push_back( visualization );
        }
    }
    */
    /*
    for ( const RigTracer& tracer : streamlineCollection->tracers() )
    {
        StreamlineVisualization visualization;

        for ( size_t i = 0; i < tracer.tracerPoints().size() - 1; i++ )
        {
            double            pointDistance = 99999999.0;
            StreamlineSegment segment( tracer.tracerPoints()[i].position(),
                                       tracer.tracerPoints()[i + 1].position(),
                                       tracer.tracerPoints()[i].direction(),
                                       tracer.tracerPoints()[i + 1].direction(),
                                       tracer.tracerPoints()[i].absValue(),
                                       tracer.tracerPoints()[i + 1].absValue() );
            if ( i > 0 )
            {
                cvf::Vec3d directionVector =
                    ( tracer.tracerPoints()[i].position() - visualization.getSegments().back().startPoint
        ).getNormalized(); double dotProduct = directionVector.getNormalized().dot(
        tracer.tracerPoints()[i].direction().getNormalized() ); if ( dotProduct >= 0 )
                {
                    pointDistance =
                        tracer.tracerPoints()[i].position().pointDistance( visualization.getSegments().back().startPoint
        );
                }
                else
                {
                    pointDistance = tracer.tracerPoints()[i].position().pointDistance(
                        visualization.getSegments().front().startPoint );
                }
                if ( pointDistance >= streamlineCollection->distanceBetweenTracerPoints() )
                {
                    if ( dotProduct >= 0 )
                    {
                        ( *std::prev( visualization.getSegments().end() ) ).endPoint =
        tracer.tracerPoints()[i].position(); visualization.appendSegment( segment );
                    }
                    else
                    {
                        ( *visualization.getSegments().begin() ).endPoint = tracer.tracerPoints()[i].position();
                        visualization.prependSegment( segment );
                    }
                }
            }
            else
            {
                visualization.appendSegment( segment );
            }
        }

        visualization.computeTValues();
        m_streamlines.push_back( visualization );
    }

    // createExampleStreamline( streamlineVisualizations );

    for ( StreamlineVisualization& visualization : m_streamlines )
    {
        if ( streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::VECTORS )
        {
            for ( RivStreamlinesPartMgr::StreamlineSegment segment : visualization.getSegments() )
            {
                model->addPart( createVectorPart( *streamlineCollection, segment ).p() );
            }
        }
        else if ( visualization.getApproximatedTotalLength() >= streamlineCollection->tracerLength() )
        {
            double currentPos = 0.0;
            while ( currentPos < 1.0 )
            {
                double t1 = currentPos;
                currentPos += streamlineCollection->tracerLength() / visualization.getApproximatedTotalLength();
                double t2 = std::min<double>( currentPos, 1.0 );
                CVF_ASSERT( t1 >= 0.0 && t1 <= 1.0 && t2 >= 0.0 && t2 <= 1.0 );
                if ( streamlineCollection->visualizationMode() ==
    RimStreamlineInViewCollection::VisualizationMode::CURVES )
                {
                    model->addPart( createCurvePart( *streamlineCollection, visualization, t1, t2 ).p() );
                }
                else
                {
                    model->addPart( createPointsPart( *streamlineCollection, visualization, t1, t2 ).p() );
                }
            }
        }
    }
    */
    for ( const RigTracer& tracer : streamlineCollection->tracers() )
    {
        Streamline streamline;
        for ( size_t i = 0; i < tracer.tracerPoints().size() - 1; i++ )
        {
            if ( i > 0 && tracer.tracerPoints()[i].position().pointDistance(
                              streamline.getTracerPoint( streamline.countTracerPoints() - 1 ) ) >=
                              streamlineCollection->distanceBetweenTracerPoints() )
            {
                streamline.appendTracerPoint( tracer.tracerPoints()[i].position() );
                streamline.appendAbsVelocity( tracer.tracerPoints()[i].absValue() );
                streamline.appendDirection( tracer.tracerPoints()[i].direction() );
            }
            else if ( i == 0 )
            {
                streamline.appendTracerPoint( tracer.tracerPoints()[i].position() );
                streamline.appendAbsVelocity( tracer.tracerPoints()[i].absValue() );
                streamline.appendDirection( tracer.tracerPoints()[i].direction() );
            }
        }
        m_streamlines.push_back( streamline );
    }
    for ( Streamline& streamline : m_streamlines )
    {
        if ( streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::ANIMATION ||
             streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::MANUAL )
        {
            model->addPart( createPart( *streamlineCollection, streamline ).p() );
        }
        else if ( streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::VECTORS )
        {
            model->addPart( createVectorPart( *streamlineCollection, streamline ).p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::updateAnimation()
{
    /*
    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();
    if ( streamlineCollection &&
         streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::CURVES )
    {
        for ( StreamlineVisualization& streamline : m_streamlines )
        {
            double              t     = streamline.currentAnimationGlobalT;
            cvf::ref<cvf::Part> part1 = streamline.getPartAtGlobalT( streamline.currentAnimationGlobalT );
            streamline.updateAnimationGlobalT( 50.0 * streamlineCollection->animationSpeed() );
            cvf::ref<cvf::Part> part2 = streamline.getPartAtGlobalT( streamline.currentAnimationGlobalT );
            if ( part1.p() != part2.p() || t == 0 )
            {
                setAlpha( part1, 0.2 );
                setAlpha( part2, 1.0 );
            }
        }
    }
    */
    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();
    if ( streamlineCollection &&
         ( streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::ANIMATION ||
           streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::MANUAL ) )
    {
        for ( Streamline& streamline : m_streamlines )
        {
            if ( streamline.getPart().notNull() && dynamic_cast<cvf::DrawableGeo*>( streamline.getPart()->drawable() ) )
            {
                if ( dynamic_cast<cvf::PrimitiveSetIndexedUIntScoped*>(
                         static_cast<cvf::DrawableGeo*>( streamline.getPart()->drawable() )->primitiveSet( 0 ) ) )
                {
                    if ( streamlineCollection->visualizationMode() ==
                         RimStreamlineInViewCollection::VisualizationMode::ANIMATION )
                    {
                        streamline.incrementAnimationIndex( streamlineCollection->animationSpeed() );
                    }
                    else
                    {
                        streamline.setAnimationIndex( streamlineCollection->animationIndex() * 2 );
                    }
                    // Each part should have exactly one primitive set.
                    static_cast<cvf::PrimitiveSetIndexedUIntScoped*>(
                        static_cast<cvf::DrawableGeo*>( streamline.getPart()->drawable() )->primitiveSet( 0 ) )
                        ->setScope( 0, streamline.getAnimationIndex() );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createPart( const RimStreamlineInViewCollection& streamlineCollection,
                                                       Streamline&                          streamline )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> indices;
    // Each point is used twice except the first and the last one
    indices.reserve( streamline.countTracerPoints() * 2 - 2 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( streamline.countTracerPoints() * 2 - 2 );

    uint count = 0;
    for ( size_t i = 0; i < streamline.countTracerPoints(); i++ )
    {
        vertices.push_back( cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamline.getTracerPoint( i ) ) ) );
        indices.push_back( count++ );
        if ( i > 0 && i < streamline.countTracerPoints() - 1 )
        {
            vertices.push_back( cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamline.getTracerPoint( i ) ) ) );
            indices.push_back( count++ );
        }
    }

    cvf::ref<cvf::PrimitiveSetIndexedUIntScoped> indexedUIntLine =
        new cvf::PrimitiveSetIndexedUIntScoped( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexedArrayLine = new cvf::UIntArray( indices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntLine->setIndices( indexedArrayLine.p(), 0, 0 );
    drawable->addPrimitiveSet( indexedUIntLine.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>( drawable->textureCoordArray() );

    if ( lineTexCoords.isNull() )
    {
        lineTexCoords = new cvf::Vec2fArray;
    }

    cvf::ref<cvf::Effect> effect;

    const cvf::ScalarMapper* activeScalarMapper = streamlineCollection.legendConfig()->scalarMapper();
    createResultColorTextureCoords( lineTexCoords.p(), streamline, activeScalarMapper );

    caf::ScalarMapperMeshEffectGenerator meshEffGen( activeScalarMapper );
    effect = meshEffGen.generateCachedEffect();

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );
    part->updateBoundingBox();
    streamline.setPart( part );

    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::createResultColorTextureCoords( cvf::Vec2fArray*         textureCoords,
                                                            const Streamline&        streamline,
                                                            const cvf::ScalarMapper* mapper )
{
    CVF_ASSERT( textureCoords );
    CVF_ASSERT( mapper );

    RimStreamlineInViewCollection* streamlineCollection = m_rimReservoirView->streamlineCollection();

    size_t vertexCount = streamline.countTracerPoints() * 2 - 2;
    if ( streamlineCollection &&
         streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::VECTORS )
    {
        vertexCount = streamline.countTracerPoints() * 7;
    }
    if ( textureCoords->size() != vertexCount ) textureCoords->reserve( vertexCount );

    size_t count = 0;
    for ( size_t i = 0; i < streamline.countTracerPoints(); i++ )
    {
        cvf::Vec2f texCoord = mapper->mapToTextureCoord( streamline.getAbsVelocity( i ) );
        if ( streamlineCollection &&
             streamlineCollection->visualizationMode() == RimStreamlineInViewCollection::VisualizationMode::VECTORS )
        {
            for ( size_t vxIdx = 0; vxIdx < 7; ++vxIdx )
            {
                textureCoords->add( texCoord );
            }
        }
        else
        {
            textureCoords->add( texCoord );
            if ( i > 0 && i < streamline.countTracerPoints() - 1 )
            {
                textureCoords->add( texCoord );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createCurvePart( const RimStreamlineInViewCollection& streamlineCollection,
                                                            StreamlineVisualization& streamlineVisualization,
                                                            const double             t1,
                                                            const double             t2 )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> lineIndices;
    lineIndices.reserve( 2 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( 2 );

    cvf::Vec3f anchorPoint =
        cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamlineVisualization.getPointAt( t1 ) ) );
    cvf::Vec3f endPoint = cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamlineVisualization.getPointAt( t2 ) ) );

    vertices.push_back( anchorPoint );
    vertices.push_back( endPoint );

    lineIndices.push_back( 0 );
    lineIndices.push_back( 1 );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntLine =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexedArrayLine = new cvf::UIntArray( lineIndices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntLine->setIndices( indexedArrayLine.p() );
    drawable->addPrimitiveSet( indexedUIntLine.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Effect> effect;

    caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 0.2f, 0.2f, 0.2f, 1 ), caf::PO_1 );
    surfaceGen.enableLighting( false );
    effect = surfaceGen.generateCachedEffect();

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );
    part->updateBoundingBox();
    streamlineVisualization.appendPart( part.p(), t1 );
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createVectorPart( const RimStreamlineInViewCollection& streamlineCollection,
                                                             Streamline&                          streamline )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> shaftIndices;
    shaftIndices.reserve( 2 * streamline.countTracerPoints() );

    std::vector<uint> headIndices;
    headIndices.reserve( 6 * streamline.countTracerPoints() );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( 7 * streamline.countTracerPoints() );

    uint count = 0;
    for ( size_t i = 0; i < streamline.countTracerPoints(); i++ )
    {
        cvf::Vec3f anchorPoint = cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamline.getTracerPoint( i ) ) );
        cvf::Vec3f direction   = cvf::Vec3f( streamline.getDirection( i ) ) * streamlineCollection.scaleFactor();

        for ( const cvf::Vec3f& vertex : createArrowVertices( anchorPoint, direction ) )
        {
            vertices.push_back( vertex );
        }

        for ( const uint& index : createArrowShaftIndices( 0 ) )
        {
            shaftIndices.push_back( index );
        }

        for ( const uint& index : createArrowHeadIndices( 0 ) )
        {
            headIndices.push_back( index );
        }
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

    cvf::ref<cvf::Effect> effect;

    const cvf::ScalarMapper* activeScalarMapper = streamlineCollection.legendConfig()->scalarMapper();
    createResultColorTextureCoords( lineTexCoords.p(), streamline, activeScalarMapper );

    caf::ScalarMapperMeshEffectGenerator meshEffGen( activeScalarMapper );
    effect = meshEffGen.generateCachedEffect();

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );
    part->updateBoundingBox();
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createPointsPart( const RimStreamlineInViewCollection& streamlineCollection,
                                                             StreamlineVisualization& streamlineVisualization,
                                                             const double             t1,
                                                             const double             t2 )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> lineIndices;
    lineIndices.reserve( 2 );

    std::vector<uint> pointIndices;
    pointIndices.reserve( 4 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( 6 );

    cvf::Vec3f anchorPoint =
        cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamlineVisualization.getPointAt( t1 ) ) );
    cvf::Vec3f endPoint = cvf::Vec3f( displayCordXf->transformToDisplayCoord( streamlineVisualization.getPointAt( t2 ) ) );

    vertices.push_back( anchorPoint );
    vertices.push_back( endPoint );

    cvf::Vec3f lineDirection = cvf::Vec3f( streamlineVisualization.getDirectionAt( t1 ) ) ^ anchorPoint;

    vertices.push_back( anchorPoint + lineDirection.getNormalized() * 1.5 );
    vertices.push_back( anchorPoint - lineDirection.getNormalized() * 1.5 );
    vertices.push_back( endPoint + lineDirection.getNormalized() * 1.5 );
    vertices.push_back( endPoint - lineDirection.getNormalized() * 1.5 );

    lineIndices.push_back( 0 );
    lineIndices.push_back( 1 );

    pointIndices.push_back( 2 );
    pointIndices.push_back( 3 );
    pointIndices.push_back( 4 );
    pointIndices.push_back( 5 );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntLine =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexedArrayLine = new cvf::UIntArray( lineIndices );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntPoints =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexedArrayPoints = new cvf::UIntArray( pointIndices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntLine->setIndices( indexedArrayLine.p() );
    drawable->addPrimitiveSet( indexedUIntLine.p() );

    indexedUIntPoints->setIndices( indexedArrayPoints.p() );
    drawable->addPrimitiveSet( indexedUIntPoints.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Effect> effect;

    caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 1, 1, 1 ), caf::PO_1 );
    surfaceGen.enableLighting( false );
    effect = surfaceGen.generateCachedEffect();

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->updateBoundingBox();
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<cvf::Vec3f, 7> RivStreamlinesPartMgr::createArrowVertices( const cvf::Vec3f anchorPoint,
                                                                      const cvf::Vec3f direction ) const
{
    std::array<cvf::Vec3f, 7> vertices;
    cvf::Vec3f                headTop = anchorPoint + direction;

    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return vertices;

    float headLength =
        std::min<float>( eclipseCase->characteristicCellSize() / 3.0f, ( headTop - anchorPoint ).length() / 2.0 );

    // A fixed size is preferred here
    cvf::Vec3f headBottom = headTop - ( headTop - anchorPoint ).getNormalized() * headLength;

    float arrowWidth = headLength / 2.0f;

    cvf::Vec3f headBottomDirection1 = direction ^ anchorPoint;
    cvf::Vec3f headBottomDirection2 = headBottomDirection1 ^ direction;
    cvf::Vec3f arrowBottomSegment1  = headBottomDirection1.getNormalized() * arrowWidth;
    cvf::Vec3f arrowBottomSegment2  = headBottomDirection2.getNormalized() * arrowWidth;

    vertices[0] = anchorPoint;
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
std::array<uint, 2> RivStreamlinesPartMgr::createArrowShaftIndices( uint startIndex ) const
{
    std::array<uint, 2> indices;

    indices[0] = startIndex;
    indices[1] = startIndex + 1;

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<uint, 6> RivStreamlinesPartMgr::createArrowHeadIndices( uint startIndex ) const
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

/*
//--------------------------------------------------------------------------------------------------
/// This is just a test function creating a visible streamline.
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::createExampleStreamline( std::vector<StreamlineVisualization>& streamlineVisualizations
)
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    // Take one random cell
    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return;
    const std::vector<RigCell>& cells  = eclipseCase->mainGrid()->globalCellArray();
    size_t                      index  = cells.size() / 2;
    const RigCell               cell   = cells[index];
    double                      length = eclipseCase->characteristicCellSize();

    StreamlineVisualization streamline;

    cvf::Vec3d startPoint = displayCordXf->transformToDisplayCoord( cell.center() );
    cvf::Vec3d endPoint   = displayCordXf->transformToDisplayCoord( cell.center() + cvf::Vec3d::X_AXIS * length
* 10.0 + cvf::Vec3d::Y_AXIS * length * 5.0 ); cvf::Vec3d startDirection = cvf::Vec3d::X_AXIS * length * 5.0;
cvf::Vec3d endDirection   = ( cvf::Vec3d::X_AXIS + cvf::Vec3d::Y_AXIS ) * length * 10.0; streamline.appendSegment(
StreamlineSegment( startPoint, endPoint, startDirection, endDirection ) );

    startPoint     = endPoint;
    endPoint       = startPoint + cvf::Vec3d::X_AXIS * length * 10.0 + cvf::Vec3d::Y_AXIS * length * 8.0;
    startDirection = endDirection;
    endDirection   = cvf::Vec3d::Y_AXIS * length * 5.0;

    streamline.appendSegment( StreamlineSegment( startPoint, endPoint, startDirection, endDirection ) );

    streamline.computeTValues();

    streamlineVisualizations.push_back( streamline );
}
*/

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::setAlpha( cvf::ref<cvf::Part> part, float alpha )
{
    if ( part.notNull() )
    {
        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( alpha, alpha, alpha, 1 ), caf::PO_1 );
        surfaceGen.enableLighting( false );
        cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();
        part->setEffect( effect.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineSegment::computeSegments()
{
    a = startPoint;
    b = startDirection;
    c = 3.0 * ( endPoint - startPoint ) - 2.0 * startDirection - endDirection;
    d = 2.0 * ( startPoint - endPoint ) + endDirection + startDirection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::StreamlineSegment::getPointAt( double t ) const
{
    return a + b * t + c * t * t + d * t * t * t;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::StreamlineSegment::getDirectionAt( double t ) const
{
    return b + c * t + d * t * t;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::StreamlineSegment::getVelocityAt( double localT ) const
{
    if ( localT == 0 )
    {
        return startVelocity;
    }
    return startVelocity + ( endVelocity - startVelocity ) / localT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::StreamlineSegment::getChordLength() const
{
    return startPoint.pointDistance( endPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::computeTValues()
{
    double totalLength   = getApproximatedTotalLength();
    double currentLength = 0.0;
    for ( StreamlineSegment& segment : segments )
    {
        segment.globalTStart = currentLength / totalLength;
        currentLength += segment.getChordLength();
        segment.globalTEnd = currentLength / totalLength;
    }
    areTValuesComputed = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::appendSegment( StreamlineSegment segment )
{
    segments.push_back( segment );
    areTValuesComputed = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::prependSegment( StreamlineSegment segment )
{
    segments.push_front( segment );
    areTValuesComputed = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::appendPart( cvf::ref<cvf::Part> part, double globalT )
{
    parts.push_back( part.p() );
    partTValues.push_back( globalT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivStreamlinesPartMgr::StreamlineVisualization::segmentsSize() const
{
    return segments.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<RivStreamlinesPartMgr::StreamlineSegment> RivStreamlinesPartMgr::StreamlineVisualization::getSegments()
{
    return segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::clear()
{
    segments.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::updateAnimationGlobalT( double timeMs )
{
    double totalLength = getApproximatedTotalLength(); // m
    double velocity    = getVelocityAt( currentAnimationGlobalT ); // m/s
    currentAnimationGlobalT += velocity * timeMs / 1000.0 / totalLength;

    if ( currentAnimationGlobalT > 1.0 )
    {
        currentAnimationGlobalT = 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::StreamlineVisualization::getApproximatedTotalLength()
{
    if ( areTValuesComputed ) return approximatedTotalLength;
    double totalLength = 0.0;
    for ( auto segment : segments )
    {
        totalLength += segment.getChordLength();
    }
    approximatedTotalLength = totalLength;
    return approximatedTotalLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::StreamlineVisualization::getPointAt( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( std::list<StreamlineSegment>::const_iterator it = segments.begin(); it != segments.end(); ++it )
    {
        if ( it->globalTStart <= globalT && it->globalTEnd >= globalT )
        {
            double localT = ( globalT - it->globalTStart ) / ( it->globalTEnd - it->globalTStart );
            return it->getPointAt( localT );
        }
    }
    return cvf::Vec3d();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::StreamlineVisualization::getDirectionAt( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( std::list<StreamlineSegment>::const_iterator it = segments.begin(); it != segments.end(); ++it )
    {
        if ( it->globalTStart <= globalT && it->globalTEnd >= globalT )
        {
            double localT = ( globalT - it->globalTStart ) / ( it->globalTEnd - it->globalTStart );
            return it->getDirectionAt( localT );
        }
    }
    return cvf::Vec3d();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::StreamlineVisualization::getVelocityAt( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( std::list<StreamlineSegment>::const_iterator it = segments.begin(); it != segments.end(); ++it )
    {
        if ( it->globalTStart <= globalT && it->globalTEnd >= globalT )
        {
            double localT = ( globalT - it->globalTStart ) / ( it->globalTEnd - it->globalTStart );
            return it->getVelocityAt( localT );
        }
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Collection<cvf::Part> RivStreamlinesPartMgr::StreamlineVisualization::getParts()
{
    return parts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::StreamlineVisualization::getPartAtGlobalT( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    double t = 0.0;
    for ( size_t index = 0; index < parts.size(); index++ )
    {
        t = partTValues[index];
        if ( t >= globalT )
        {
            return parts[index];
        }
    }
    return cvf::ref<cvf::Part>( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::appendTracerPoint( cvf::Vec3d point )
{
    tracerPoints.push_back( point );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::appendAbsVelocity( double velocity )
{
    absVelocities.push_back( velocity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::appendDirection( cvf::Vec3d direction )
{
    directions.push_back( direction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::clear()
{
    tracerPoints.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::Streamline::getPart()
{
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::Streamline::getTracerPoint( size_t index ) const
{
    return tracerPoints[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::Streamline::getAbsVelocity( size_t index ) const
{
    return absVelocities[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivStreamlinesPartMgr::Streamline::getDirection( size_t index ) const
{
    return directions[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivStreamlinesPartMgr::Streamline::countTracerPoints() const
{
    return tracerPoints.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::setPart( cvf::ref<cvf::Part> part )
{
    this->part = part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivStreamlinesPartMgr::Streamline::getAnimationIndex() const
{
    return animIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::incrementAnimationIndex( size_t increment )
{
    animIndex += increment;
    if ( animIndex >= tracerPoints.size() * 2 - 2 )
    {
        animIndex = 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::Streamline::setAnimationIndex( size_t index )
{
    animIndex = index;
    if ( animIndex >= tracerPoints.size() * 2 - 2 )
    {
        animIndex = tracerPoints.size() * 2 - 2;
    }
}
