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
#include "cvfShaderProgram.h"

#include "cafPdmPointer.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivStreamlinesPartMgr::RivStreamlinesPartMgr( RimEclipseView* reservoirView )
{
    m_rimReservoirView = reservoirView;
    m_count            = 0;
    m_numSegments      = 200;
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
    for ( const RigTracer& tracer : streamlineCollection->tracers() )
    {
        StreamlineVisualization visualization;

        for ( size_t i = 0; i < tracer.tracerPoints().size() - 1; i++ )
        {
            if ( i == 0 || tracer.tracerPoints()[i].position().pointDistance(
                               cvf::Vec3d( visualization.getSegments().back().startPoint ) ) >=
                               streamlineCollection->distanceBetweenTracerPoints() )
            {
                StreamlineSegment segment( tracer.tracerPoints()[i].position(),
                                           tracer.tracerPoints()[i + 1].position(),
                                           tracer.tracerPoints()[i].direction(),
                                           tracer.tracerPoints()[i + 1].direction(),
                                           tracer.tracerPoints()[i].absValue(),
                                           tracer.tracerPoints()[i + 1].absValue() );
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
        else
        {
            for ( size_t i = 0; i < m_numSegments; i++ )
            {
                double t = static_cast<double>( i ) / static_cast<double>( m_numSegments - 1 );
                CVF_ASSERT( t >= 0.0 && t <= 1.0 );
                createCurvePart( *streamlineCollection, visualization, t );
            }
            for ( auto segmentPart : visualization.getParts() )
            {
                model->addPart( segmentPart.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::updateAnimation()
{
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
                setAlpha( part1, 0.0 );
                setAlpha( part2, 1.0 );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::createCurvePart( const RimStreamlineInViewCollection& streamlineCollection,
                                             StreamlineVisualization&             streamlineVisualization,
                                             const double                         t )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> lineIndices;
    lineIndices.reserve( 2 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( 2 );

    cvf::Vec3f anchorPoint =
        cvf::Vec3f( displayCordXf->transformToDisplayCoord( cvf::Vec3d( streamlineVisualization.getPointAt( t ) ) ) );
    cvf::Vec3f endPoint = cvf::Vec3f( displayCordXf->transformToDisplayCoord(
        cvf::Vec3d( streamlineVisualization.getPointAt( t ) +
                    streamlineVisualization.getDirectionAt( t ) * streamlineCollection.scaleFactor() ) ) );

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

    caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 1, 1, 0 ), caf::PO_1 );
    surfaceGen.enableLighting( false );
    effect = surfaceGen.generateCachedEffect();

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );
    part->updateBoundingBox();
    streamlineVisualization.appendPart( part.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createVectorPart( const RimStreamlineInViewCollection& streamlineCollection,
                                                             StreamlineSegment&                   streamlineSegment )
{
    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<uint> shaftIndices;
    shaftIndices.reserve( 2 );

    std::vector<uint> headIndices;
    headIndices.reserve( 6 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( 7 );

    cvf::Vec3f anchorPoint =
        cvf::Vec3f( displayCordXf->transformToDisplayCoord( cvf::Vec3d( streamlineSegment.startPoint ) ) );
    cvf::Vec3f direction = streamlineSegment.startDirection * streamlineCollection.scaleFactor();

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

    cvf::ref<cvf::Effect> effect;

    caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 1, 1, 1 ), caf::PO_1 );
    surfaceGen.enableLighting( false );
    effect = surfaceGen.generateCachedEffect();

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<uint, 2> RivStreamlinesPartMgr::createLineIndices( uint startIndex ) const
{
    std::array<uint, 2> indices;

    indices[0] = startIndex;
    indices[1] = startIndex + 7;
    return indices;
}

/*
//--------------------------------------------------------------------------------------------------
/// This is just a test function creating a visible streamline.
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::createExampleStreamline( std::vector<StreamlineVisualization>& streamlineVisualizations )
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
    cvf::Vec3d endPoint   = displayCordXf->transformToDisplayCoord( cell.center() + cvf::Vec3d::X_AXIS * length * 10.0 +
                                                                  cvf::Vec3d::Y_AXIS * length * 5.0 );
    cvf::Vec3d startDirection = cvf::Vec3d::X_AXIS * length * 5.0;
    cvf::Vec3d endDirection   = ( cvf::Vec3d::X_AXIS + cvf::Vec3d::Y_AXIS ) * length * 10.0;
    streamline.appendSegment( StreamlineSegment( startPoint, endPoint, startDirection, endDirection ) );

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
        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 1, 1, alpha ), caf::PO_1 );
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
    c = 3.0f * ( endPoint - startPoint ) - 2.0f * startDirection - endDirection;
    d = 2.0f * ( startPoint - endPoint ) + endDirection + startDirection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivStreamlinesPartMgr::StreamlineSegment::getPointAt( double t ) const
{
    return a + b * t + c * t * t + d * t * t * t;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivStreamlinesPartMgr::StreamlineSegment::getDirectionAt( double t ) const
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
        currentLength += segment.getChordLength();
        segment.globalT = currentLength / totalLength;
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
void RivStreamlinesPartMgr::StreamlineVisualization::appendPart( cvf::ref<cvf::Part> part )
{
    parts.push_back( part.p() );
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
std::list<RivStreamlinesPartMgr::StreamlineSegment> RivStreamlinesPartMgr::StreamlineVisualization::getSegments() const
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
cvf::Vec3f RivStreamlinesPartMgr::StreamlineVisualization::getPointAt( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( std::list<StreamlineSegment>::const_iterator it = segments.begin(); it != segments.end(); ++it )
    {
        if ( it->globalT >= globalT )
        {
            double localT = 0.0;
            if ( it == segments.begin() )
            {
                localT = globalT / it->globalT;
                if ( it->globalT == 0 )
                {
                    localT = 0.0;
                }
            }
            else
            {
                localT = ( globalT - it->globalT ) / ( it->globalT - std::prev( it )->globalT );
            }
            return it->getPointAt( localT );
        }
    }
    return cvf::Vec3f();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivStreamlinesPartMgr::StreamlineVisualization::getDirectionAt( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( std::list<StreamlineSegment>::const_iterator it = segments.begin(); it != segments.end(); ++it )
    {
        if ( it->globalT >= globalT )
        {
            double localT = 0.0;
            if ( it == segments.begin() )
            {
                localT = globalT / it->globalT;
                if ( it->globalT == 0 )
                {
                    localT = 0.0;
                }
            }
            else
            {
                localT = ( globalT - it->globalT ) / ( it->globalT - std::prev( it )->globalT );
            }
            return it->getDirectionAt( localT );
        }
    }
    return cvf::Vec3f();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivStreamlinesPartMgr::StreamlineVisualization::getVelocityAt( double globalT ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( std::list<StreamlineSegment>::const_iterator it = segments.begin(); it != segments.end(); ++it )
    {
        if ( it->globalT >= globalT )
        {
            double localT = 0.0;
            if ( it == segments.begin() )
            {
                localT = globalT / it->globalT;
                if ( it->globalT == 0 )
                {
                    localT = 0.0;
                }
            }
            else
            {
                localT = ( globalT - it->globalT ) / ( it->globalT - std::prev( it )->globalT );
            }
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
        t = static_cast<double>( index ) / static_cast<double>( parts.size() - 1 );
        if ( t >= globalT )
        {
            return parts[index];
        }
    }
    return cvf::ref<cvf::Part>( nullptr );
}
