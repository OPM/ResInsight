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
    m_numSegments      = 100;
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
    m_parts.clear();
    CVF_ASSERT( model );
    if ( m_rimReservoirView.isNull() ) return;

    RimEclipseCase* eclipseCase = m_rimReservoirView->eclipseCase();
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    cvf::ref<caf::DisplayCoordTransform> displayCordXf = m_rimReservoirView->displayCoordTransform();

    std::vector<StreamlineVisualization> streamlineVisualizations;

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

    for ( const RigTracer& tracer : streamlineCollection->tracers() )
    {
        std::vector<cvf::Vec3d> tracerPoints;
        StreamlineVisualization visualization;
        for ( RigTracerPoint tracerPoint : tracer.tracerPoints() )
        {
            tracerPoints.push_back( displayCordXf->transformToDisplayCoord( tracerPoint.position() ) );
        }
        visualization.tracerPoints = tracerPoints;
        streamlineVisualizations.push_back( visualization );
    }
    */

    createExampleStreamline( streamlineVisualizations );

    for ( StreamlineVisualization visualization : streamlineVisualizations )
    {
        for ( size_t i = 0; i < m_numSegments; i++ )
        {
            double t = static_cast<double>( i ) / static_cast<double>( m_numSegments - 1 );
            CVF_ASSERT( t >= 0.0 && t <= 1.0 );
            cvf::ref<cvf::Part> partIdx = createPart( *streamlineCollection, visualization, t );
            m_parts.push_back( partIdx.p() );
            partIdx->updateBoundingBox();
            model->addPart( partIdx.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::updateAnimation()
{
    if ( m_parts.size() > 0 )
    {
        if ( m_count == 1 )
        {
            cvf::ref<cvf::Part> part;
            if ( m_currentT > 0 )
            {
                part = m_parts[m_currentT - 1];
            }
            else
            {
                part = m_parts[m_parts.size() - 1];
            }
            if ( part.notNull() )
            {
                caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 0, 0, 0 ), caf::PO_1 );
                surfaceGen.enableLighting( false );
                // surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
                cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();
                part->setEffect( effect.p() );
            }
            part = m_parts[m_currentT];
            if ( part.notNull() )
            {
                caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 0, 0, 1 ), caf::PO_1 );
                surfaceGen.enableLighting( false );
                // surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
                cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();
                part->setEffect( effect.p() );
            }
            m_count = 0;
            m_currentT++;
            if ( m_currentT >= m_parts.size() )
            {
                m_currentT = 0;
            }
            /*
            for ( size_t i = 0; i < m_parts.size(); i++ )
            {
                cvf::ref<cvf::Part> part = m_parts[i];
                if ( part.notNull() )
                {
                    if ( m_count == 1 )
                    {
                        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 0, 0, 1 ), caf::PO_1 );
                        surfaceGen.enableLighting( false );
                        // surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
                        cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();
                        part->setEffect( effect.p() );
                    }
                    else if ( m_count == 50 )
                    {
                        caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 0, 1, 0, 1 ), caf::PO_1 );
                        surfaceGen.enableLighting( false );
                        // surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
                        cvf::ref<cvf::Effect> effect = surfaceGen.generateCachedEffect();
                        part->setEffect( effect.p() );
                    }
                    else if ( m_count > 100 )
                    {
                        m_count = 0;
                    }
                }
            }
            */
        }
        m_count++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivStreamlinesPartMgr::createPart( const RimStreamlineInViewCollection& streamlineCollection,
                                                       const StreamlineVisualization&       streamlineVisualization,
                                                       const double                         t ) const
{
    std::vector<uint> shaftIndices;
    shaftIndices.reserve( streamlineVisualization.segmentsSize() * m_numSegments * 2 );

    std::vector<uint> headIndices;
    headIndices.reserve( streamlineVisualization.segmentsSize() * m_numSegments * 6 );

    std::vector<uint> lineIndices;
    lineIndices.reserve( streamlineVisualization.segmentsSize() * m_numSegments * 2 );

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve( streamlineVisualization.segmentsSize() * m_numSegments * 7 );

    // Better to reserve space already? - this would mean we need two for loops
    uint counter = 0;

    cvf::Vec3f anchorPoint = streamlineVisualization.getPointAt( t );
    cvf::Vec3f direction   = streamlineVisualization.getDirectionAt( t );

    for ( const cvf::Vec3f& vertex : createArrowVertices( anchorPoint, direction ) )
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

    if ( t < 1.0 )
    {
        for ( const uint& index : createLineIndices( counter ) )
        {
            lineIndices.push_back( index );
        }
    }

    counter += 7;

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntShaft =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexArrayShaft = new cvf::UIntArray( shaftIndices );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntHead =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_TRIANGLES );
    cvf::ref<cvf::UIntArray> indexArrayHead = new cvf::UIntArray( headIndices );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUIntLine =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
    cvf::ref<cvf::UIntArray> indexedArrayLine = new cvf::UIntArray( lineIndices );

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUIntShaft->setIndices( indexArrayShaft.p() );
    drawable->addPrimitiveSet( indexedUIntShaft.p() );

    indexedUIntHead->setIndices( indexArrayHead.p() );
    drawable->addPrimitiveSet( indexedUIntHead.p() );

    indexedUIntLine->setIndices( indexedArrayLine.p() );
    drawable->addPrimitiveSet( indexedUIntLine.p() );

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices );
    drawable->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>( drawable->textureCoordArray() );

    if ( lineTexCoords.isNull() )
    {
        lineTexCoords = new cvf::Vec2fArray;
    }

    const cvf::ScalarMapper* activeScalerMapper = nullptr;

    cvf::ref<cvf::Effect> effect;

    caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( 1, 0, 0, 1 ), caf::PO_1 );
    surfaceGen.enableLighting( !m_rimReservoirView->isLightingDisabled() );
    effect = surfaceGen.generateCachedEffect();

    drawable->setTextureCoordArray( lineTexCoords.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawable.p() );
    part->setEffect( effect.p() );

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
double RivStreamlinesPartMgr::StreamlineSegment::getChordLength() const
{
    return startPoint.pointDistance( endPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivStreamlinesPartMgr::StreamlineVisualization::computeTValues()
{
    double totalLength = 0.0;
    for ( StreamlineSegment& segment : segments )
    {
        totalLength += segment.getChordLength();
    }

    for ( StreamlineSegment& segment : segments )
    {
        segment.t = segment.getChordLength() / totalLength;
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
size_t RivStreamlinesPartMgr::StreamlineVisualization::segmentsSize() const
{
    return segments.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RivStreamlinesPartMgr::StreamlineSegment> RivStreamlinesPartMgr::StreamlineVisualization::getSegments() const
{
    return segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivStreamlinesPartMgr::StreamlineVisualization::getPointAt( double t ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( size_t i = 0; i < segments.size(); i++ )
    {
        if ( segments[i].t >= t )
        {
            double localT = 0.0;
            if ( i == 0 )
            {
                localT = t / segments[i].t;
            }
            else
            {
                localT = ( t - segments[i - 1].t ) / ( segments[i].t - segments[i - 1].t );
            }
            return segments[i].getPointAt( localT );
        }
    }
    return cvf::Vec3f();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivStreamlinesPartMgr::StreamlineVisualization::getDirectionAt( double t ) const
{
    CVF_ASSERT( areTValuesComputed );
    for ( size_t i = 0; i < segments.size(); i++ )
    {
        if ( segments[i].t >= t )
        {
            double localT = 0.0;
            if ( i == 0 )
            {
                localT = t / segments[i].t;
            }
            else
            {
                localT = ( t - segments[i - 1].t ) / ( segments[i].t - segments[i - 1].t );
            }
            return segments[i].getDirectionAt( localT );
        }
    }
    return cvf::Vec3f();
}
