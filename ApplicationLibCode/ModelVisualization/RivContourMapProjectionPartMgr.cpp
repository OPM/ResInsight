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

#include "RivContourMapProjectionPartMgr.h"

#include "RiaColorTools.h"
#include "RiaFontCache.h"

#include "ContourMap/RigContourMapGrid.h"
#include "ContourMap/RigContourPolygonsTools.h"

#include "RivContourMapElevationProvider.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivPartPriority.h"
#include "RivScalarMapperUtils.h"

#include "cafCategoryMapper.h"
#include "cafEffectGenerator.h"
#include "cafFixedAtlasFont.h"

#include "cvfCamera.h"
#include "cvfColor3.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfRay.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfScalarMapper.h"
#include "cvfViewport.h"
#include "cvfqtUtils.h"

#include <algorithm>
#include <array>
#include <cmath>

#include <QDebug>
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivContourMapProjectionPartMgr::RivContourMapProjectionPartMgr( caf::PdmObject* pdmObject )
{
    m_pdmObject = pdmObject;

    m_labelEffect = new cvf::Effect;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RivContourLineAppearance::colorForLevel( const cvf::Color3f& levelColor ) const
{
    return color.value_or( RiaColorTools::contrastColor( levelColor ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<cvf::Vec3d> RivContourMapProjectionPartMgr::toDomainCoord( const cvf::Vec3d&                     localVertex,
                                                                         const RigContourMapGrid&              contourMapGrid,
                                                                         const RivContourMapElevationProvider* elevationProvider )
{
    cvf::Vec3d domainVertex = localVertex + contourMapGrid.origin3d();

    if ( elevationProvider )
    {
        auto elevation = elevationProvider->domainElevation( cvf::Vec2d( localVertex.x(), localVertex.y() ) );
        if ( !elevation ) return {};

        domainVertex.z() = *elevation;
    }

    return domainVertex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<cvf::Vec3d> RivContourMapProjectionPartMgr::toDisplayCoord( const cvf::Vec3d&                     localVertex,
                                                                          const RigContourMapGrid&              contourMapGrid,
                                                                          const caf::DisplayCoordTransform*     displayCoordTransform,
                                                                          const RivContourMapElevationProvider* elevationProvider )
{
    auto domainVertex = toDomainCoord( localVertex, contourMapGrid, elevationProvider );
    if ( !domainVertex ) return {};

    return displayCoordTransform->transformToDisplayCoord( *domainVertex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>>
    RivContourMapProjectionPartMgr::resampledDisplaySegments( const std::vector<cvf::Vec3d>&        localVertices,
                                                              const RigContourMapGrid&              contourMapGrid,
                                                              const caf::DisplayCoordTransform*     displayCoordTransform,
                                                              const RivContourMapElevationProvider* elevationProvider )
{
    std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>> segments;

    const size_t nVertices = localVertices.size();
    if ( nVertices < 2 ) return segments;

    const double resamplingDistance = elevationProvider ? elevationProvider->resamplingDistance() : 0.0;

    // Keeps a pathologically long segment from producing an unbounded number of samples
    const int maxSamplesPerSegment = 200;

    segments.reserve( nVertices );

    for ( size_t v = 0; v < nVertices; ++v )
    {
        const cvf::Vec3d& localVertex1 = localVertices[v];
        const cvf::Vec3d& localVertex2 = ( v < nVertices - 1 ) ? localVertices[v + 1] : localVertices[0];

        int sampleCount = 1;
        if ( resamplingDistance > 0.0 )
        {
            const double segmentLength = ( localVertex2 - localVertex1 ).length();

            sampleCount = static_cast<int>( std::ceil( segmentLength / resamplingDistance ) );
            sampleCount = std::clamp( sampleCount, 1, maxSamplesPerSegment );
        }

        auto previous = toDisplayCoord( localVertex1, contourMapGrid, displayCoordTransform, elevationProvider );

        for ( int sample = 1; sample <= sampleCount; ++sample )
        {
            const double     fraction   = static_cast<double>( sample ) / sampleCount;
            const cvf::Vec3d localPoint = localVertex1 + ( localVertex2 - localVertex1 ) * fraction;

            auto current = toDisplayCoord( localPoint, contourMapGrid, displayCoordTransform, elevationProvider );

            // A gap is left wherever there is no elevation, which splits a draped polyline into pieces
            if ( previous && current ) segments.emplace_back( *previous, *current );

            previous = current;
        }
    }

    return segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendProjectionToModel( cvf::ModelBasicList*                  model,
                                                              const caf::DisplayCoordTransform*     displayCoordTransform,
                                                              const std::vector<cvf::Vec4d>&        vertices,
                                                              const RigContourMapGrid&              contourMapGrid,
                                                              const cvf::Color3f&                   backgroundColor,
                                                              cvf::ScalarMapper*                    scalarMapper,
                                                              const RivContourMapElevationProvider* elevationProvider ) const
{
    cvf::ref<cvf::Part> mapPart =
        createProjectionMapPart( displayCoordTransform, vertices, contourMapGrid, backgroundColor, scalarMapper, elevationProvider );
    if ( mapPart.notNull() )
    {
        model->addPart( mapPart.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivContourMapProjectionPartMgr::appendPickPointVisToModel( cvf::ModelBasicList*              model,
                                                                const caf::DisplayCoordTransform* displayCoordTransform,
                                                                const cvf::Vec2d&                 pickPoint,
                                                                const RigContourMapGrid&          contourMapGrid ) const

{
    cvf::ref<cvf::DrawableGeo> drawable = createPickPointVisDrawable( displayCoordTransform, pickPoint, contourMapGrid );
    if ( drawable.notNull() && drawable->boundingBox().isValid() )
    {
        caf::MeshEffectGenerator meshEffectGen( cvf::Color3::MAGENTA );
        meshEffectGen.setLineWidth( 1.0f );
        caf::MeshEffectGenerator::createAndConfigurePolygonOffsetRenderState( caf::PO_2 );
        cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName( "RivContourMapProjectionPartMgr::appendPickPointVisToModel" );
        part->setDrawable( drawable.p() );
        part->setEffect( effect.p() );
        part->setSourceInfo( new RivMeshLinesSourceInfo( m_pdmObject.p() ) );

        model->addPart( part.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Vec2fArray> RivContourMapProjectionPartMgr::createTextureCoords( const std::vector<double>& values,
                                                                               cvf::ScalarMapper*         scalarMapper ) const
{
    cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray( values.size() );

#pragma omp parallel for
    for ( int i = 0; i < (int)values.size(); ++i )
    {
        if ( values[i] != std::numeric_limits<double>::infinity() )
        {
            cvf::Vec2f textureCoord = scalarMapper->mapToTextureCoord( values[i] );
            textureCoord.y()        = 0.0;
            ( *textureCoords )[i]   = textureCoord;
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
                                                                const caf::DisplayCoordTransform* displayCoordTransform,
                                                                const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                                                const RigContourMapGrid&              contourMapGrid,
                                                                cvf::ScalarMapper*                    mapper,
                                                                bool                                  showContourLines,
                                                                bool                                  showContourLabels,
                                                                caf::NumberFormatType                 numberFormat,
                                                                int                                   precision,
                                                                const RivContourMapElevationProvider* elevationProvider,
                                                                const RivContourLineAppearance&       lineAppearance )
{
    if ( showContourLines )
    {
        std::vector<std::vector<cvf::BoundingBox>> labelBBoxes;
        std::vector<cvf::ref<cvf::Drawable>>       labelDrawables;

        if ( showContourLabels )
        {
            labelDrawables = createContourLabels( camera,
                                                  displayCoordTransform,
                                                  &labelBBoxes,
                                                  contourLinePolygons,
                                                  contourMapGrid,
                                                  mapper,
                                                  numberFormat,
                                                  precision,
                                                  elevationProvider,
                                                  lineAppearance );
        }

        std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels =
            createContourPolygons( displayCoordTransform, labelBBoxes, contourLinePolygons, mapper, contourMapGrid, elevationProvider );

        std::vector<double> tickValues;
        mapper->majorTickValues( &tickValues );

        for ( size_t i = 0; i < contourDrawablesForAllLevels.size(); ++i )
        {
            std::vector<cvf::ref<cvf::Drawable>> contourDrawables = contourDrawablesForAllLevels[i];

            cvf::Color3f backgroundColor( mapper->mapToColor( tickValues[i] ) );
            cvf::Color3f lineColor = lineAppearance.colorForLevel( backgroundColor );

            for ( cvf::ref<cvf::Drawable> contourDrawable : contourDrawables )
            {
                if ( contourDrawable.notNull() && contourDrawable->boundingBox().isValid() )
                {
                    caf::MeshEffectGenerator meshEffectGen( lineColor );
                    meshEffectGen.setLineWidth( lineAppearance.lineWidth );

                    // NB: polygon offset cannot be used to keep these lines clear of the map surface.
                    // It only applies to polygon primitives, and these are line primitives. Callers that
                    // draw the lines on top of other geometry have to lift them through the elevation
                    // provider instead.
                    cvf::ref<cvf::Effect> effect = meshEffectGen.generateCachedEffect();

                    cvf::ref<cvf::Part> part = new cvf::Part;
                    part->setName( "RivContourMapProjectionPartMgr::contourDrawable_mesh" );
                    part->setDrawable( contourDrawable.p() );
                    part->setEffect( effect.p() );
                    part->setPriority( RivPartPriority::MeshLines );
                    part->setSourceInfo( new RivMeshLinesSourceInfo( m_pdmObject.p() ) );

                    model->addPart( part.p() );
                }
            }
        }

        if ( showContourLabels )
        {
            for ( auto labelDrawableRef : labelDrawables )
            {
                cvf::ref<cvf::Part> part = new cvf::Part;
                part->setName( "RivContourMapProjectionPartMgr::labelDrawableRef" );
                part->setDrawable( labelDrawableRef.p() );
                part->setEffect( m_labelEffect.p() );
                part->setPriority( RivPartPriority::Text );
                part->setSourceInfo( new RivMeshLinesSourceInfo( m_pdmObject.p() ) );
                model->addPart( part.p() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableText> RivContourMapProjectionPartMgr::createTextLabel( const cvf::Color3f&      textColor,
                                                                             const cvf::Color3f&      backgroundColor,
                                                                             caf::FontTools::FontSize fontSize )
{
    auto font = RiaFontCache::getFont( fontSize );

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
cvf::ref<cvf::Part> RivContourMapProjectionPartMgr::createProjectionMapPart( const caf::DisplayCoordTransform*     displayCoordTransform,
                                                                             const std::vector<cvf::Vec4d>&        vertices,
                                                                             const RigContourMapGrid&              contourMapGrid,
                                                                             const cvf::Color3f&                   backgroundColor,
                                                                             cvf::ScalarMapper*                    scalarMapper,
                                                                             const RivContourMapElevationProvider* elevationProvider ) const
{
    if ( vertices.size() < 3u )
    {
        return cvf::ref<cvf::Part>();
    }

    // The vertices form a triangle soup, three consecutive entries per triangle. A triangle is dropped
    // when the elevation provider has no elevation for one of its vertices.
    std::vector<cvf::Vec3f> displayVertices;
    std::vector<double>     values;
    displayVertices.reserve( vertices.size() );
    values.reserve( vertices.size() );

    for ( size_t i = 0; i + 2 < vertices.size(); i += 3 )
    {
        std::array<cvf::Vec3d, 3> triangle;
        bool                      allVerticesDefined = true;

        for ( size_t n = 0; n < 3; ++n )
        {
            const cvf::Vec4d& vertex = vertices[i + n];
            auto              displayVertex =
                toDisplayCoord( cvf::Vec3d( vertex.x(), vertex.y(), vertex.z() ), contourMapGrid, displayCoordTransform, elevationProvider );
            if ( !displayVertex )
            {
                allVerticesDefined = false;
                break;
            }
            triangle[n] = *displayVertex;
        }

        if ( !allVerticesDefined ) continue;

        for ( size_t n = 0; n < 3; ++n )
        {
            displayVertices.push_back( cvf::Vec3f( triangle[n] ) );
            values.push_back( vertices[i + n].w() );
        }
    }

    if ( displayVertices.size() < 3u )
    {
        return cvf::ref<cvf::Part>();
    }

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( displayVertices );
    cvf::ref<cvf::UIntArray>  faceList    = new cvf::UIntArray( displayVertices.size() );
    for ( uint i = 0; i < displayVertices.size(); ++i )
    {
        ( *faceList )[i] = i;
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexUInt = new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_TRIANGLES, faceList.p() );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->addPrimitiveSet( indexUInt.p() );
    geo->setVertexArray( vertexArray.p() );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName( "RivContourMapProjectionPartMgr::createProjectionMapPart" );
    part->setDrawable( geo.p() );

    cvf::ref<cvf::Vec2fArray> textureCoords = createTextureCoords( values, scalarMapper );
    RivScalarMapperUtils::applyTextureResultsToPart( part.p(), textureCoords.p(), scalarMapper, 1.0f, caf::FC_NONE, true, backgroundColor );

    part->setSourceInfo( new RivObjectSourceInfo( m_pdmObject.p() ) );
    part->setPriority( RivPartPriority::BaseLevel );
    return part;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::ref<cvf::Drawable>>>
    RivContourMapProjectionPartMgr::createContourPolygons( const caf::DisplayCoordTransform*                 displayCoordTransform,
                                                           const std::vector<std::vector<cvf::BoundingBox>>& labelBBoxes,
                                                           const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                                           cvf::ScalarMapper*                                           scalarMapper,
                                                           const RigContourMapGrid&                                     contourMapGrid,
                                                           const RivContourMapElevationProvider* elevationProvider ) const
{
    std::vector<double> tickValues;
    scalarMapper->majorTickValues( &tickValues );

    std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels;
    contourDrawablesForAllLevels.resize( tickValues.size() );

    for ( size_t i = 1; i < contourLinePolygons.size(); ++i )
    {
        std::vector<cvf::ref<cvf::Drawable>> contourDrawables;

        for ( size_t j = 0; j < contourLinePolygons[i].size(); ++j )
        {
            if ( contourLinePolygons[i][j].vertices.empty() ) continue;

            // cvf::String::number does not allow precision on 'g' formats, so use Qt.
            QString     qLabelText = QString::number( contourLinePolygons[i][j].value, 'g', 2 );
            cvf::String labelText  = cvfqt::Utils::toString( qLabelText );

            const auto displaySegments =
                resampledDisplaySegments( contourLinePolygons[i][j].vertices, contourMapGrid, displayCoordTransform, elevationProvider );

            std::vector<cvf::Vec3f> displayLines;
            displayLines.reserve( displaySegments.size() * 2 );
            for ( const auto& [displayVertex1, displayVertex2] : displaySegments )
            {
                cvf::BoundingBox lineBBox;
                lineBBox.add( displayVertex1 );
                lineBBox.add( displayVertex2 );

                bool addOriginalSegment = true;
                if ( !labelBBoxes.empty() )
                {
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

            cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
            cvf::ref<cvf::UIntArray>               indexArray  = new cvf::UIntArray( indices );
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
    RivContourMapProjectionPartMgr::createContourLabels( const cvf::Camera*                                           camera,
                                                         const caf::DisplayCoordTransform*                            displayCoordTransform,
                                                         std::vector<std::vector<cvf::BoundingBox>>*                  labelBBoxes,
                                                         const std::vector<RigContourPolygonsTools::ContourPolygons>& contourLinePolygons,
                                                         const RigContourMapGrid&                                     contourMapGrid,
                                                         const cvf::ScalarMapper*                                     scalarMapper,
                                                         caf::NumberFormatType                                        numberFormat,
                                                         int                                                          precision,
                                                         const RivContourMapElevationProvider*                        elevationProvider,
                                                         const RivContourLineAppearance&                              lineAppearance ) const
{
    CAF_ASSERT( camera && displayCoordTransform && labelBBoxes );

    std::vector<cvf::ref<cvf::Drawable>> labelDrawables;
    labelBBoxes->clear();
    labelBBoxes->resize( contourLinePolygons.size() );

    if ( !camera->viewport() || camera->viewport()->width() == 0 ) return labelDrawables;

    if ( scalarMapper == nullptr ) return labelDrawables;

    if ( dynamic_cast<const caf::CategoryMapper*>( scalarMapper ) != nullptr ) return labelDrawables;

    std::vector<double> tickValues;
    scalarMapper->majorTickValues( &tickValues );

    const RigContourPolygonsTools::ContourPolygons* previousLevel = nullptr;
    for ( int64_t i = (int64_t)contourLinePolygons.size() - 1; i > 0; --i )
    {
        cvf::Color3f backgroundColor( scalarMapper->mapToColor( tickValues[i] ) );

        // A label belongs to its contour line, so it takes the same color
        cvf::Color3f                textColor = lineAppearance.colorForLevel( backgroundColor );
        cvf::ref<cvf::DrawableText> label     = createTextLabel( textColor, backgroundColor, lineAppearance.labelFontSize );

        for ( size_t j = 0; j < contourLinePolygons[i].size(); ++j )
        {
            if ( contourLinePolygons[i][j].vertices.empty() ) continue;

            QString     qLabelText = caf::PdmUiNumberFormat::valueToText( contourLinePolygons[i][j].value, numberFormat, precision );
            cvf::String labelText  = cvfqt::Utils::toString( qLabelText );

            size_t nVertices              = contourLinePolygons[i][j].vertices.size();
            size_t nLabels                = nVertices;
            double distanceSinceLastLabel = std::numeric_limits<double>::infinity();
            for ( size_t l = 0; l < nLabels; ++l )
            {
                size_t nVertex    = ( nVertices * l ) / nLabels;
                size_t nextVertex = ( nVertex + 1 ) % nVertices;

                const cvf::Vec3d& localVertex1 = contourLinePolygons[i][j].vertices[nVertex];
                const cvf::Vec3d& localVertex2 = contourLinePolygons[i][j].vertices[nextVertex];

                cvf::Vec3d lineCenter = ( localVertex1 + localVertex2 ) * 0.5;
                double     tolerance  = 1.0e-2 * contourMapGrid.sampleSpacing();

                if ( previousLevel && lineOverlapsWithPreviousContourLevel( lineCenter, *previousLevel, tolerance ) )
                {
                    continue;
                }

                auto globalVertex1Candidate = toDomainCoord( localVertex1, contourMapGrid, elevationProvider );
                auto globalVertex2Candidate = toDomainCoord( localVertex2, contourMapGrid, elevationProvider );
                if ( !globalVertex1Candidate || !globalVertex2Candidate ) continue;

                cvf::Vec3d globalVertex1 = *globalVertex1Candidate;
                cvf::Vec3d globalVertex2 = *globalVertex2Candidate;

                cvf::Vec3d globalVertex = 0.5 * ( globalVertex1 + globalVertex2 );

                cvf::Vec3d segment       = globalVertex2 - globalVertex1;
                cvf::Vec3d displayVertex = displayCoordTransform->transformToDisplayCoord( globalVertex );
                cvf::Vec3d windowVertex;
                camera->project( displayVertex, &windowVertex );
                CAF_ASSERT( !windowVertex.isUndefined() );
                displayVertex.z() += 10.0f;

                // The text direction is interpreted in screen space by the drawer
                const cvf::Vec3f labelDirection = lineAppearance.alignLabelsWithCamera ? cvf::Vec3f::X_AXIS
                                                                                       : cvf::Vec3f( segment.getNormalized() );

                cvf::BoundingBox windowBBox = label->textBoundingBox( labelText, cvf::Vec3f::ZERO, labelDirection );
                cvf::Vec3d       displayBBoxMin, displayBBoxMax;
                camera->unproject( windowBBox.min() + windowVertex, &displayBBoxMin );
                camera->unproject( windowBBox.max() + windowVertex, &displayBBoxMax );

                CAF_ASSERT( !displayBBoxMin.isUndefined() );
                CAF_ASSERT( !displayBBoxMax.isUndefined() );

                cvf::BoundingBox displayBBox( displayBBoxMin - cvf::Vec3d::Z_AXIS * 20.0, displayBBoxMax + cvf::Vec3d::Z_AXIS * 20.0 );

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
                            // Assert on invalid bounding box seen on Linux
                            if ( !displayBBox.isValid() || !existingBBox.isValid() ) continue;

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
                    CAF_ASSERT( !displayVertex.isUndefined() );
                    label->addText( labelText, displayVertexV, labelDirection );
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

        previousLevel = &contourLinePolygons[i];
    }
    return labelDrawables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivContourMapProjectionPartMgr::createPickPointVisDrawable( const caf::DisplayCoordTransform* displayCoordTransform,
                                                                                       const cvf::Vec2d&        pickPoint,
                                                                                       const RigContourMapGrid& contourMapGrid ) const
{
    std::vector<cvf::Vec3d> pickPointPolygon = RigContourPolygonsTools::generatePickPointPolygon( pickPoint, contourMapGrid );

    if ( pickPointPolygon.empty() )
    {
        return nullptr;
    }
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( pickPointPolygon.size() );

    for ( size_t i = 0; i < pickPointPolygon.size(); ++i )
    {
        // The pick point marker is only used by the 2d contour map views, and is always placed on the
        // contour map plane. Without an elevation provider a display coordinate is always produced.
        auto displayPoint = toDisplayCoord( pickPointPolygon[i], contourMapGrid, displayCoordTransform, nullptr );
        CAF_ASSERT( displayPoint );

        ( *vertexArray )[i] = cvf::Vec3f( displayPoint.value_or( cvf::Vec3d::ZERO ) );
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

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt( cvf::PrimitiveType::PT_LINES );
        cvf::ref<cvf::UIntArray>               indexArray  = new cvf::UIntArray( indices );
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
bool RivContourMapProjectionPartMgr::lineOverlapsWithPreviousContourLevel( const cvf::Vec3d&                               lineCenter,
                                                                           const RigContourPolygonsTools::ContourPolygons& previousLevel,
                                                                           double                                          tolerance )
{
    return RigContourPolygonsTools::lineOverlapsWithContourPolygons( lineCenter, previousLevel, tolerance );
}
