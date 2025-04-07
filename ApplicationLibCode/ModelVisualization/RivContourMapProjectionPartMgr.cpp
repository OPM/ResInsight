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
#include "cvfScalarMapper.h"
#include "cvfViewport.h"
#include "cvfqtUtils.h"

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
void RivContourMapProjectionPartMgr::appendProjectionToModel( cvf::ModelBasicList*              model,
                                                              const caf::DisplayCoordTransform* displayCoordTransform,
                                                              const std::vector<cvf::Vec4d>&    vertices,
                                                              const RigContourMapGrid&          contourMapGrid,
                                                              const cvf::Color3f&               backgroundColor,
                                                              cvf::ScalarMapper*                scalarMapper ) const
{
    cvf::ref<cvf::Part> mapPart = createProjectionMapPart( displayCoordTransform, vertices, contourMapGrid, backgroundColor, scalarMapper );
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
                                                                const RigContourMapGrid&          contourMapGrid,
                                                                cvf::ScalarMapper*                mapper,
                                                                bool                              showContourLines,
                                                                bool                              showContourLabels,
                                                                RiaNumberFormat::NumberFormatType numberFormat,
                                                                int                               precision )
{
    if ( showContourLines )
    {
        std::vector<std::vector<cvf::BoundingBox>> labelBBoxes;
        std::vector<cvf::ref<cvf::Drawable>>       labelDrawables;

        if ( showContourLabels )
        {
            labelDrawables =
                createContourLabels( camera, displayCoordTransform, &labelBBoxes, contourLinePolygons, contourMapGrid, mapper, numberFormat, precision );
        }

        std::vector<std::vector<cvf::ref<cvf::Drawable>>> contourDrawablesForAllLevels =
            createContourPolygons( displayCoordTransform, labelBBoxes, contourLinePolygons, mapper, contourMapGrid );

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
                    caf::MeshEffectGenerator::createAndConfigurePolygonOffsetRenderState( caf::PO_1 );

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
cvf::ref<cvf::DrawableText> RivContourMapProjectionPartMgr::createTextLabel( const cvf::Color3f& textColor, const cvf::Color3f& backgroundColor )
{
    auto font = RiaFontCache::getFont( RiaFontCache::FontSize::FONT_SIZE_10 );

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
cvf::ref<cvf::Part> RivContourMapProjectionPartMgr::createProjectionMapPart( const caf::DisplayCoordTransform* displayCoordTransform,
                                                                             const std::vector<cvf::Vec4d>&    vertices,
                                                                             const RigContourMapGrid&          contourMapGrid,
                                                                             const cvf::Color3f&               backgroundColor,
                                                                             cvf::ScalarMapper*                scalarMapper ) const
{
    if ( vertices.size() < 3u )
    {
        return cvf::ref<cvf::Part>();
    }
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray( vertices.size() );
    cvf::ref<cvf::UIntArray>  faceList    = new cvf::UIntArray( vertices.size() );
    std::vector<double>       values( vertices.size() );
    for ( uint i = 0; i < vertices.size(); ++i )
    {
        cvf::Vec3d globalVertex = cvf::Vec3d( vertices[i].x(), vertices[i].y(), vertices[i].z() ) + contourMapGrid.origin3d();
        cvf::Vec3f displayVertexPos( displayCoordTransform->transformToDisplayCoord( globalVertex ) );
        ( *vertexArray )[i] = displayVertexPos;
        ( *faceList )[i]    = i;
        values[i]           = vertices[i].w();
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
                                                           const RigContourMapGrid& contourMapGrid ) const
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

            size_t nVertices = contourLinePolygons[i][j].vertices.size();

            std::vector<cvf::Vec3f> displayLines;
            displayLines.reserve( nVertices * 2 );
            for ( size_t v = 0; v < nVertices; ++v )
            {
                cvf::Vec3d globalVertex1  = contourLinePolygons[i][j].vertices[v] + contourMapGrid.origin3d();
                cvf::Vec3d displayVertex1 = displayCoordTransform->transformToDisplayCoord( globalVertex1 );

                cvf::Vec3d globalVertex2;
                if ( v < nVertices - 1 )
                    globalVertex2 = contourLinePolygons[i][j].vertices[v + 1] + contourMapGrid.origin3d();
                else
                    globalVertex2 = contourLinePolygons[i][j].vertices[0] + contourMapGrid.origin3d();

                cvf::Vec3d displayVertex2 = displayCoordTransform->transformToDisplayCoord( globalVertex2 );

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
                                                         RiaNumberFormat::NumberFormatType                            numberFormat,
                                                         int                                                          precision ) const
{
    CVF_ASSERT( camera && displayCoordTransform && labelBBoxes );

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
        cvf::Color3f                backgroundColor( scalarMapper->mapToColor( tickValues[i] ) );
        cvf::Color3f                textColor = RiaColorTools::contrastColor( backgroundColor );
        cvf::ref<cvf::DrawableText> label     = createTextLabel( textColor, backgroundColor );

        for ( size_t j = 0; j < contourLinePolygons[i].size(); ++j )
        {
            if ( contourLinePolygons[i][j].vertices.empty() ) continue;

            QString     qLabelText = RiaNumberFormat::valueToText( contourLinePolygons[i][j].value, numberFormat, precision );
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

                cvf::Vec3d globalVertex1 = localVertex1 + contourMapGrid.origin3d();
                cvf::Vec3d globalVertex2 = localVertex2 + contourMapGrid.origin3d();

                cvf::Vec3d globalVertex = 0.5 * ( globalVertex1 + globalVertex2 );

                cvf::Vec3d segment       = globalVertex2 - globalVertex1;
                cvf::Vec3d displayVertex = displayCoordTransform->transformToDisplayCoord( globalVertex );
                cvf::Vec3d windowVertex;
                camera->project( displayVertex, &windowVertex );
                CVF_ASSERT( !windowVertex.isUndefined() );
                displayVertex.z() += 10.0f;
                cvf::BoundingBox windowBBox = label->textBoundingBox( labelText, cvf::Vec3f::ZERO, cvf::Vec3f( segment.getNormalized() ) );
                cvf::Vec3d       displayBBoxMin, displayBBoxMax;
                camera->unproject( windowBBox.min() + windowVertex, &displayBBoxMin );
                camera->unproject( windowBBox.max() + windowVertex, &displayBBoxMax );

                CVF_ASSERT( !displayBBoxMin.isUndefined() );
                CVF_ASSERT( !displayBBoxMax.isUndefined() );

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
        cvf::Vec3d globalPoint = pickPointPolygon[i] + contourMapGrid.origin3d();
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
