#include "RivContourMapProjectionPartMgr.h"

#include "RiaColorTools.h"
#include "RiaFontCache.h"
#include "RiaWeightedMeanCalculator.h"
#include "RigCellGeometryTools.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivPartPriority.h"
#include "RivScalarMapperUtils.h"

#include "RimContourMapProjection.h"
#include "RimGridView.h"

#include "cafCategoryMapper.h"
#include "cafEffectGenerator.h"
#include "cafFixedAtlasFont.h"

#include "cvfCamera.h"
#include "cvfDrawableText.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryTools.h"
#include "cvfGeometryUtils.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"
#include "cvfScalarMapper.h"

#include <cmath>

#include <QDebug>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapProjectionPartMgr::RivContourMapProjectionPartMgr( RimContourMapProjection* contourMapProjection,
                                                                RimGridView*             contourMap )
{
    m_contourMapProjection = contourMapProjection;
    m_parentContourMap     = contourMap;

    m_labelEffect = new cvf::Effect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::createProjectionGeometry()
{
    m_contourMapProjection->generateGeometryIfNecessary();

    m_contourLinePolygons = m_contourMapProjection->contourPolygons();
    m_contourMapTriangles = m_contourMapProjection->trianglesWithVertexValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendProjectionToModel( cvf::ModelBasicList*              model,
                                                              const caf::DisplayCoordTransform* displayCoordTransform ) const
{
    cvf::ref<cvf::Part> mapPart = createProjectionMapPart( displayCoordTransform );
    if ( mapPart.notNull() )
    {
        model->addPart( mapPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendPickPointVisToModel( cvf::ModelBasicList*              model,
                                                                const caf::DisplayCoordTransform* displayCoordTransform ) const
{
    cvf::ref<cvf::DrawableGeo> drawable = createPickPointVisDrawable( displayCoordTransform );
    if ( drawable.notNull() && drawable->boundingBox().isValid() )
    {
        caf::MeshEffectGenerator meshEffectGen( cvf::Color3::MAGENTA );
        meshEffectGen.setLineWidth( 1.0f );
        meshEffectGen.createAndConfigurePolygonOffsetRenderState( caf::PO_2 );
        cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable( drawable.p() );
        part->setEffect( effect.p() );
        part->setSourceInfo( new RivMeshLinesSourceInfo( m_contourMapProjection.p() ) );

        model->addPart( part.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec2fArray> RivContourMapProjectionPartMgr::createTextureCoords( const std::vector<double>& values ) const
{
    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray( values.size() );

#pragma omp parallel for
    for ( int i = 0; i < (int)values.size(); ++i )
    {
        if ( values[i] != std::numeric_limits<double>::infinity() )
        {
            cvf::Vec2f textureCoord =
                m_contourMapProjection->legendConfig()->scalarMapper()->mapToTextureCoord( values[i] );
            textureCoord.y()      = 0.0;
            ( *textureCoords )[i] = textureCoord;
        }
        else
        {
            ( *textureCoords )[i] = cvf::Vec2f( 0.0, 1.0 );
        }
    }
    return textureCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendContourLinesToModel( const cvf::Camera*                camera,
                                                                cvf::ModelBasicList*              model,
                                                                const caf::DisplayCoordTransform* displayCoordTransform )
{
    if ( m_contourMapProjection->showContourLines() )
    {
        cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();

        std::vector<std::vector<cvf::BoundingBox>> labelBBoxes;
        std::vector<cvf::ref<cvf::Drawable>>       labelDrawables =
            createContourLabels( camera, displayCoordTransform, &labelBBoxes );

        std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels =
            createContourPolygons( displayCoordTransform, labelBBoxes );

        std::vector<double> tickValues;
        mapper->majorTickValues( &tickValues );

        for ( size_t i = 0; i < contourDrawablesForAllLevels.size(); ++i )
        {
            std::vector<cvf::ref<cvf::Drawable>> contourDrawables = contourDrawablesForAllLevels[i];

            cvf::Color3f backgroundColor( mapper->mapToColor( tickValues[i] ) );
            cvf::Color3f lineColor = RiaColorTools::contrastColor( backgroundColor );

            for ( cvf::ref<cvf::Drawable> contourDrawable : contourDrawables )
            {
                if ( contourDrawable.notNull() && contourDrawable->boundingBox().isValid() )
                {
                    caf::MeshEffectGenerator meshEffectGen( lineColor );
                    meshEffectGen.setLineWidth( 1.0f );
                    meshEffectGen.createAndConfigurePolygonOffsetRenderState( caf::PO_1 );

                    cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

                    cvf::ref<cvf::Part> part = new cvf::Part;
                    part->setDrawable( contourDrawable.p() );
                    part->setEffect( effect.p() );
                    part->setPriority( RivPartPriority::MeshLines );
                    part->setSourceInfo( new RivMeshLinesSourceInfo( m_contourMapProjection.p() ) );

                    model->addPart( part.p() );
                }
            }
        }
        for ( auto labelDrawableRef : labelDrawables )
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setDrawable( labelDrawableRef.p() );
            part->setEffect( m_labelEffect.p() );
            part->setPriority( RivPartPriority::Text );
            part->setSourceInfo( new RivMeshLinesSourceInfo( m_contourMapProjection.p() ) );
            model->addPart( part.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableText> RivContourMapProjectionPartMgr::createTextLabel( const cvf::Color3f& textColor,
                                                                             const cvf::Color3f& backgroundColor )
{
    auto font = RiaFontCache::getFont( RiaFontCache::FONT_SIZE_10 );

    cvf::ref<cvf::DrawableText> labelDrawable = new cvf::DrawableText();
    labelDrawable->setFont( font.p() );
    labelDrawable->setCheckPosVisible( false );
    labelDrawable->setUseDepthBuffer( true );
    labelDrawable->setDrawBorder( false );
    labelDrawable->setDrawBackground( false );
    labelDrawable->setBackgroundColor( backgroundColor );
    labelDrawable->setVerticalAlignment( cvf::TextDrawer::CENTER );
    labelDrawable->setTextColor( textColor );
    labelDrawable->setBorderColor( textColor );

    return labelDrawable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part>
    RivContourMapProjectionPartMgr::createProjectionMapPart( const caf::DisplayCoordTransform* displayCoordTransform ) const
{
    const std::vector<cvf::Vec4d>& vertices = m_contourMapTriangles;
    if ( vertices.size() < 3u )
    {
        return cvf::ref<cvf::Part>();
    }
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices.size() );
    cvf::ref<cvf::UIntArray>  faceList    = new cvf::UIntArray( vertices.size() );
    std::vector<double>       values( vertices.size() );
    for ( uint i = 0; i < vertices.size(); ++i )
    {
        cvf::Vec3d globalVertex = cvf::Vec3d( vertices[i].x(), vertices[i].y(), vertices[i].z() ) +
                                  m_contourMapProjection->origin3d();
        cvf::Vec3f displayVertexPos( displayCoordTransform->transformToDisplayCoord( globalVertex ) );
        ( *vertexArray )[i] = displayVertexPos;
        ( *faceList )[i]    = i;
        values[i]           = vertices[i].w();
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexUInt =
        new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_TRIANGLES, faceList.p() );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->addPrimitiveSet( indexUInt.p() );
    geo->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( geo.p() );

    cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();

    cvf::ref<cvf::Vec2fArray> textureCoords = createTextureCoords( values );
    RivScalarMapperUtils::applyTextureResultsToPart( part.p(),
                                                     textureCoords.p(),
                                                     mapper,
                                                     1.0f,
                                                     caf::FC_NONE,
                                                     true,
                                                     m_parentContourMap->backgroundColor() );

    part->setSourceInfo( new RivObjectSourceInfo( m_contourMapProjection.p() ) );
    part->setPriority( RivPartPriority::BaseLevel );
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::ref<cvf::Drawable>>>
    RivContourMapProjectionPartMgr::createContourPolygons( const caf::DisplayCoordTransform* displayCoordTransform,
                                                           const std::vector<std::vector<cvf::BoundingBox>>& labelBBoxes ) const
{
    const cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();
    std::vector<double>      tickValues;
    mapper->majorTickValues( &tickValues );

    std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels;
    contourDrawablesForAllLevels.resize( tickValues.size() );

    for ( size_t i = 1; i < m_contourLinePolygons.size(); ++i )
    {
        std::vector<cvf::ref<cvf::Drawable>> contourDrawables;

        for ( size_t j = 0; j < m_contourLinePolygons[i].size(); ++j )
        {
            if ( m_contourLinePolygons[i][j].vertices.empty() ) continue;

            cvf::String labelText( m_contourLinePolygons[i][j].value );

            size_t nVertices = m_contourLinePolygons[i][j].vertices.size();

            std::vector<cvf::Vec3f> displayLines;
            displayLines.reserve( nVertices * 2 );
            for ( size_t v = 0; v < nVertices; ++v )
            {
                cvf::Vec3d globalVertex1 = m_contourLinePolygons[i][j].vertices[v] + m_contourMapProjection->origin3d();
                cvf::Vec3d displayVertex1 = displayCoordTransform->transformToDisplayCoord( globalVertex1 );

                cvf::Vec3d globalVertex2;
                if ( v < nVertices - 1 )
                    globalVertex2 = m_contourLinePolygons[i][j].vertices[v + 1] + m_contourMapProjection->origin3d();
                else
                    globalVertex2 = m_contourLinePolygons[i][j].vertices[0] + m_contourMapProjection->origin3d();

                cvf::Vec3d displayVertex2 = displayCoordTransform->transformToDisplayCoord( globalVertex2 );

                cvf::BoundingBox lineBBox;
                lineBBox.add( displayVertex1 );
                lineBBox.add( displayVertex2 );

                bool addOriginalSegment = true;
                for ( const cvf::BoundingBox& existingBBox : labelBBoxes[i] )
                {
                    if ( lineBBox.intersects( existingBBox ) )
                    {
                        if ( existingBBox.contains( displayVertex1 ) && existingBBox.contains( displayVertex2 ) )
                        {
                            addOriginalSegment = false;
                        }
                        else
                        {
                            cvf::Vec3d dir = displayVertex2 - displayVertex1;

                            cvf::Ray ray;
                            ray.setOrigin( displayVertex1 );
                            ray.setDirection( dir.getNormalized() );
                            ray.setMaximumDistance( dir.length() );

                            if ( !existingBBox.contains( displayVertex1 ) )
                            {
                                cvf::Vec3d intersection;
                                bool       hit = ray.boxIntersect( existingBBox, &intersection );
                                if ( hit )
                                {
                                    displayLines.push_back( cvf::Vec3f( displayVertex1 ) );
                                    displayLines.push_back( cvf::Vec3f( intersection ) );
                                    addOriginalSegment = false;
                                }
                            }

                            if ( !existingBBox.contains( displayVertex2 ) )
                            {
                                ray.setOrigin( displayVertex2 );
                                ray.setDirection( -ray.direction() );
                                cvf::Vec3d intersection;
                                bool       hit = ray.boxIntersect( existingBBox, &intersection );
                                if ( hit )
                                {
                                    displayLines.push_back( cvf::Vec3f( intersection ) );
                                    displayLines.push_back( cvf::Vec3f( displayVertex2 ) );
                                    addOriginalSegment = false;
                                }
                            }
                        }
                    }
                }
                if ( addOriginalSegment )
                {
                    displayLines.push_back( cvf::Vec3f( displayVertex1 ) );
                    displayLines.push_back( cvf::Vec3f( displayVertex2 ) );
                }
            }

            cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( displayLines );

            std::vector<cvf::uint> indices;
            indices.reserve( vertexArray->size() );
            for ( cvf::uint k = 0; k < vertexArray->size(); ++k )
            {
                indices.push_back( k );
            }

            cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt =
                new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
            cvf::ref<cvf::UIntArray> indexArray = new cvf::UIntArray( indices );
            indexedUInt->setIndices( indexArray.p() );

            cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

            geo->addPrimitiveSet( indexedUInt.p() );
            geo->setVertexArray( vertexArray.p() );
            contourDrawables.push_back( geo );
        }
        if ( !contourDrawables.empty() )
        {
            contourDrawablesForAllLevels[i] = contourDrawables;
        }
    }
    return contourDrawablesForAllLevels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::ref<cvf::Drawable>>
    RivContourMapProjectionPartMgr::createContourLabels( const cvf::Camera*                camera,
                                                         const caf::DisplayCoordTransform* displayCoordTransform,
                                                         std::vector<std::vector<cvf::BoundingBox>>* labelBBoxes ) const
{
    CVF_ASSERT( camera && displayCoordTransform && labelBBoxes );

    std::vector<cvf::ref<cvf::Drawable>> labelDrawables;
    labelBBoxes->clear();
    labelBBoxes->resize( m_contourLinePolygons.size() );

    const cvf::ScalarMapper* mapper = m_contourMapProjection->legendConfig()->scalarMapper();
    if ( dynamic_cast<const caf::CategoryMapper*>( mapper ) != nullptr ) return labelDrawables;

    std::vector<double> tickValues;
    mapper->majorTickValues( &tickValues );

    const RimContourMapProjection::ContourPolygons* previousLevel = nullptr;
    for ( int64_t i = (int64_t)m_contourLinePolygons.size() - 1; i > 0; --i )
    {
        cvf::Color3f                backgroundColor( mapper->mapToColor( tickValues[i] ) );
        cvf::Color3f                textColor = RiaColorTools::contrastColor( backgroundColor );
        cvf::ref<cvf::DrawableText> label     = createTextLabel( textColor, backgroundColor );

        for ( size_t j = 0; j < m_contourLinePolygons[i].size(); ++j )
        {
            if ( m_contourLinePolygons[i][j].vertices.empty() ) continue;

            cvf::String labelText( m_contourLinePolygons[i][j].value );

            size_t nVertices              = m_contourLinePolygons[i][j].vertices.size();
            size_t nLabels                = nVertices;
            double distanceSinceLastLabel = std::numeric_limits<double>::infinity();
            for ( size_t l = 0; l < nLabels; ++l )
            {
                size_t nVertex    = ( nVertices * l ) / nLabels;
                size_t nextVertex = ( nVertex + 1 ) % nVertices;

                const cvf::Vec3d& localVertex1 = m_contourLinePolygons[i][j].vertices[nVertex];
                const cvf::Vec3d& localVertex2 = m_contourLinePolygons[i][j].vertices[nextVertex];

                cvf::Vec3d lineCenter = ( localVertex1 + localVertex2 ) * 0.5;
                if ( previousLevel && lineOverlapsWithPreviousContourLevel( lineCenter, previousLevel ) )
                {
                    continue;
                }

                cvf::Vec3d globalVertex1 = localVertex1 + m_contourMapProjection->origin3d();
                cvf::Vec3d globalVertex2 = localVertex2 + m_contourMapProjection->origin3d();

                cvf::Vec3d globalVertex = 0.5 * ( globalVertex1 + globalVertex2 );

                cvf::Vec3d segment       = globalVertex2 - globalVertex1;
                cvf::Vec3d displayVertex = displayCoordTransform->transformToDisplayCoord( globalVertex );
                cvf::Vec3d windowVertex;
                camera->project( displayVertex, &windowVertex );
                CVF_ASSERT( !windowVertex.isUndefined() );
                displayVertex.z() += 10.0f;
                cvf::BoundingBox windowBBox =
                    label->textBoundingBox( labelText, cvf::Vec3f::ZERO, cvf::Vec3f( segment.getNormalized() ) );
                cvf::Vec3d displayBBoxMin, displayBBoxMax;
                camera->unproject( windowBBox.min() + windowVertex, &displayBBoxMin );
                camera->unproject( windowBBox.max() + windowVertex, &displayBBoxMax );

                CVF_ASSERT( !displayBBoxMin.isUndefined() );
                CVF_ASSERT( !displayBBoxMax.isUndefined() );

                cvf::BoundingBox displayBBox( displayBBoxMin - cvf::Vec3d::Z_AXIS * 20.0,
                                              displayBBoxMax + cvf::Vec3d::Z_AXIS * 20.0 );

                cvf::Vec3d currentExtent = displayBBoxMax - displayBBoxMin;

                bool overlaps = false;
                if ( distanceSinceLastLabel < currentExtent.length() * 10.0 )
                {
                    overlaps = true;
                }

                if ( !overlaps )
                {
                    for ( auto boxVector : *labelBBoxes )
                    {
                        for ( const cvf::BoundingBox& existingBBox : boxVector )
                        {
                            double dist = ( displayBBox.center() - existingBBox.center() ).length();
                            if ( dist < segment.length() || existingBBox.intersects( displayBBox ) )
                            {
                                overlaps = true;
                                break;
                            }
                        }
                    }
                }

                if ( !overlaps )
                {
                    cvf::Vec3f displayVertexV( displayVertex );
                    CVF_ASSERT( !displayVertex.isUndefined() );
                    label->addText( labelText, displayVertexV, cvf::Vec3f( segment.getNormalized() ) );
                    labelBBoxes->at( i ).push_back( displayBBox );
                    distanceSinceLastLabel = 0.0;
                }
                else
                {
                    distanceSinceLastLabel += segment.length();
                }
            }
        }
        if ( label->numberOfTexts() != 0u )
        {
            labelDrawables.push_back( label );
        }

        previousLevel = &m_contourLinePolygons[i];
    }
    return labelDrawables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivContourMapProjectionPartMgr::createPickPointVisDrawable( const caf::DisplayCoordTransform* displayCoordTransform ) const
{
    std::vector<cvf::Vec3d> pickPointPolygon = m_contourMapProjection->generatePickPointPolygon();
    if ( pickPointPolygon.empty() )
    {
        return nullptr;
    }
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( pickPointPolygon.size() );

    for ( size_t i = 0; i < pickPointPolygon.size(); ++i )
    {
        cvf::Vec3d globalPoint = pickPointPolygon[i] + m_contourMapProjection->origin3d();
        cvf::Vec3f displayPoint( displayCoordTransform->transformToDisplayCoord( globalPoint ) );
        ( *vertexArray )[i] = displayPoint;
    }

    cvf::ref<cvf::DrawableGeo> geo = nullptr;
    if ( vertexArray->size() > 0u )
    {
        std::vector<cvf::uint> indices;
        indices.reserve( vertexArray->size() );
        for ( cvf::uint j = 0; j < vertexArray->size(); ++j )
        {
            indices.push_back( j );
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt =
            new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
        cvf::ref<cvf::UIntArray> indexArray = new cvf::UIntArray( indices );
        indexedUInt->setIndices( indexArray.p() );

        geo = new cvf::DrawableGeo;

        geo->addPrimitiveSet( indexedUInt.p() );
        geo->setVertexArray( vertexArray.p() );
    }
    return geo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RivContourMapProjectionPartMgr::lineOverlapsWithPreviousContourLevel(
    const cvf::Vec3d&                               lineCenter,
    const RimContourMapProjection::ContourPolygons* previousLevel ) const
{
    const int64_t jump = 50;
    CVF_ASSERT( previousLevel );
    double tolerance = 1.0e-2 * m_contourMapProjection->sampleSpacing();
    for ( const RimContourMapProjection::ContourPolygon& edgePolygon : *previousLevel )
    {
        std::pair<int64_t, double> closestIndex( 0, std::numeric_limits<double>::infinity() );
        for ( int64_t i = 0; i < (int64_t)edgePolygon.vertices.size(); i += jump )
        {
            const cvf::Vec3d& edgeVertex1 = edgePolygon.vertices[i];
            const cvf::Vec3d& edgeVertex2 = edgePolygon.vertices[( i + 1 ) % edgePolygon.vertices.size()];
            double            dist1 = cvf::GeometryTools::linePointSquareDist( edgeVertex1, edgeVertex2, lineCenter );
            if ( dist1 < tolerance )
            {
                return true;
            }
            if ( dist1 < closestIndex.second )
            {
                closestIndex = std::make_pair( i, dist1 );
            }
        }
        for ( int64_t i = std::max( (int64_t)1, closestIndex.first - jump + 1 );
              i < std::min( (int64_t)edgePolygon.vertices.size(), closestIndex.first + jump );
              ++i )
        {
            const cvf::Vec3d& edgeVertex1 = edgePolygon.vertices[i];
            const cvf::Vec3d& edgeVertex2 = edgePolygon.vertices[( i + 1 ) % edgePolygon.vertices.size()];
            double            dist1 = cvf::GeometryTools::linePointSquareDist( edgeVertex1, edgeVertex2, lineCenter );
            if ( dist1 < tolerance )
            {
                return true;
            }
        }
    }
    return false;
}
