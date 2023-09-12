/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RivAnnotationTools.h"

#include "RiaColorTools.h"
#include "RiaFontCache.h"

#include "RivAnnotationSourceInfo.h"
#include "RivObjectSourceInfo.h"
#include "RivPartPriority.h"
#include "RivPolylineGenerator.h"

#include "cafAppEnum.h"
#include "cafEffectGenerator.h"

#include "cvfCamera.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfViewport.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace caf
{

template <>
void caf::AppEnum<RivAnnotationTools::LabelPositionStrategy>::setUp()
{
    addItem( RivAnnotationTools::LabelPositionStrategy::LEFT, "LEFT", "Left" );
    addItem( RivAnnotationTools::LabelPositionStrategy::RIGHT, "RIGHT", "Right" );
    addItem( RivAnnotationTools::LabelPositionStrategy::LEFT_AND_RIGHT, "LEFT_AND_RIGHT", "Left and Right" );
    addItem( RivAnnotationTools::LabelPositionStrategy::COUNT_HINT, "COUNT_HINT", "Count Hint" );
    addItem( RivAnnotationTools::LabelPositionStrategy::ALL, "All", "All" );
    addItem( RivAnnotationTools::LabelPositionStrategy::NONE, "None", "Disabled" );
    // RivAnnotationTools::LabelPositionStrategy::UNKNOWN is not included, as this is enum is not supposed to be displayed in GUI
    setDefault( RivAnnotationTools::LabelPositionStrategy::RIGHT );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivAnnotationTools::RivAnnotationTools()
    : m_overrideStrategy( RivAnnotationTools::LabelPositionStrategy::UNDEFINED )
    , m_labelCountHint( 5 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationTools::setOverrideLabelPositionStrategy( LabelPositionStrategy strategy )
{
    // By default, each annotation object has a label position strategy. Use this method to override the default.

    m_overrideStrategy = strategy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationTools::setCountHint( int countHint )
{
    m_labelCountHint = countHint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivAnnotationTools::createPartFromPolyline( const cvf::Color3f& color, const std::vector<cvf::Vec3d>& polyLine )
{
    cvf::ref<cvf::DrawableGeo> drawableGeo = RivPolylineGenerator::createLineAlongPolylineDrawable( polyLine );
    if ( drawableGeo.isNull() ) return nullptr;

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setDrawable( drawableGeo.p() );

    caf::MeshEffectGenerator colorEffgen( color );
    cvf::ref<cvf::Effect>    eff = colorEffgen.generateCachedEffect();

    part->setEffect( eff.p() );
    part->setPriority( RivPartPriority::PartType::MeshLines );

    return part;
}

struct LabelTextAndPosition
{
    std::string label;
    cvf::Vec3d  labelPosition;
    cvf::Vec3d  lineAnchorPosition;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto computeScalingFactorFromZoom = []( const cvf::Camera* camera ) -> double
{
    double scalingFactor = 1.0;

    if ( camera )
    {
        auto       viewPort = camera->viewport();
        cvf::Vec3d vpCorner1( 0, viewPort->height(), 0 );
        cvf::Vec3d vpCorner2( viewPort->width(), 0, 0 );
        bool       unprojOk = true;
        cvf::Vec3d corner1Display, corner2Display, e1;
        unprojOk &= camera->unproject( vpCorner1, &corner1Display );
        unprojOk &= camera->unproject( vpCorner2, &corner2Display );

        if ( unprojOk )
        {
            scalingFactor = 10 * ( corner1Display - corner2Display ).length();
        }
    }

    return scalingFactor;
};

//--------------------------------------------------------------------------------------------------
/// Project candidate coordinates to screen space, and compare with the normalized viewport position. Create a label item for the closest
/// coordinate.
//--------------------------------------------------------------------------------------------------
auto createLabelForClosestCoordinate = []( const cvf::Camera*             camera,
                                           const RivAnnotationSourceInfo* annotationObject,
                                           const double                   viewportWidth,
                                           const double                   normalizedXPosition,
                                           const double                   anchorLineScalingFactor ) -> std::optional<LabelTextAndPosition>
{
    if ( !camera || !annotationObject ) return std::nullopt;

    cvf::Vec3d anchorPosition;
    double     smallestDistance = std::numeric_limits<double>::max();
    for ( const auto& displayCoord : annotationObject->anchorPointsInDisplayCoords() )
    {
        cvf::Vec3d screenCoord;
        camera->project( displayCoord, &screenCoord );

        double horizontalDistance = std::fabs( normalizedXPosition * viewportWidth - screenCoord.x() );

        if ( horizontalDistance < smallestDistance )
        {
            smallestDistance = horizontalDistance;
            anchorPosition   = displayCoord;
        }
    }

    if ( smallestDistance == std::numeric_limits<double>::max() ) return std::nullopt;

    const cvf::Vec3d directionPointToCam = ( camera->position() - anchorPosition ).getNormalized();

    cvf::Vec3d labelPosition = anchorPosition + directionPointToCam * anchorLineScalingFactor;

    const double maxScreenSpaceAdjustment = viewportWidth * 0.05;
    if ( smallestDistance < maxScreenSpaceAdjustment )
    {
        // Establish a fixed horizontal anchor point for the label in screen coordinates. Achieved through conversion to screen coordinates,
        // adjusting the x-coordinate, and then reverting to display coordinates.
        cvf::Vec3d screenCoord;
        camera->project( labelPosition, &screenCoord );

        screenCoord.x() = normalizedXPosition * viewportWidth;
        camera->unproject( screenCoord, &labelPosition );
    }

    LabelTextAndPosition info = { annotationObject->text(), anchorPosition, labelPosition };
    return info;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto createMultipleLabels = []( const cvf::Camera*             camera,
                                const RivAnnotationSourceInfo* annotationObject,
                                const double                   viewportWidth,
                                const double                   viewportHeight,
                                const double                   anchorLineScalingFactor ) -> std::vector<LabelTextAndPosition>
{
    if ( !annotationObject || annotationObject->texts().empty() ) return {};

    std::vector<LabelTextAndPosition> labelInfo;

    std::vector<cvf::Vec3d>  labelCoords;
    std::vector<std::string> labelTexts;

    const auto candidateCoords = annotationObject->anchorPointsInDisplayCoords();
    const auto candidateLabels = annotationObject->texts();

    if ( candidateCoords.size() == candidateLabels.size() )
    {
        for ( size_t i = 0; i < annotationObject->anchorPointsInDisplayCoords().size(); i++ )
        {
            const auto& displayCoord = candidateCoords[i];

            cvf::Vec3d screenCoord;
            camera->project( displayCoord, &screenCoord );

            if ( screenCoord.x() > 0 && screenCoord.x() < viewportWidth && screenCoord.y() > 0 && screenCoord.y() < viewportHeight )
            {
                const auto& text = candidateLabels[i];
                labelCoords.push_back( displayCoord );
                labelTexts.push_back( text );
            }
        }

        for ( size_t i = 0; i < labelCoords.size(); i++ )
        {
            const cvf::Vec3d lineAnchorPosition  = labelCoords[i];
            const cvf::Vec3d directionPointToCam = ( camera->position() - lineAnchorPosition ).getNormalized();
            const cvf::Vec3d labelPosition       = lineAnchorPosition + directionPointToCam * anchorLineScalingFactor;

            labelInfo.push_back( { labelTexts[i], lineAnchorPosition, labelPosition } );
        }
    }

    return labelInfo;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivAnnotationTools::addAnnotationLabels( const cvf::Collection<cvf::Part>& partCollection,
                                              const cvf::Camera*                camera,
                                              cvf::ModelBasicList*              model )
{
    if ( !camera || !model ) return;

    const double anchorLineScalingFactor = computeScalingFactorFromZoom( camera );

    for ( auto p : partCollection )
    {
        auto annotationObject = dynamic_cast<RivAnnotationSourceInfo*>( p->sourceInfo() );
        if ( annotationObject )
        {
            std::vector<LabelTextAndPosition> labels;

            const auto viewportWidth  = camera->viewport()->width();
            const auto viewportHeight = camera->viewport()->height();

            if ( !annotationObject->texts().empty() )
            {
                labels = createMultipleLabels( camera, annotationObject, viewportWidth, viewportHeight, anchorLineScalingFactor );
            }
            else
            {
                auto strategy = annotationObject->labelPositionStrategyHint();
                if ( m_overrideStrategy != LabelPositionStrategy::UNDEFINED )
                {
                    // Can override annotation object strategy defined in Rim3dView::updateScreenSpaceModel()
                    strategy = m_overrideStrategy;
                }

                if ( strategy == LabelPositionStrategy::RIGHT || strategy == LabelPositionStrategy::LEFT_AND_RIGHT )
                {
                    // Close to the right edge of the visible screen area
                    const auto normalizedXPosition = 0.9;

                    auto labelCandidate =
                        createLabelForClosestCoordinate( camera, annotationObject, viewportWidth, normalizedXPosition, anchorLineScalingFactor );
                    if ( labelCandidate.has_value() ) labels.push_back( labelCandidate.value() );
                }

                if ( strategy == LabelPositionStrategy::LEFT || strategy == LabelPositionStrategy::LEFT_AND_RIGHT )
                {
                    // Close to the left edge of the visible screen area
                    const auto normalizedXPosition = 0.1;

                    auto labelCandidate =
                        createLabelForClosestCoordinate( camera, annotationObject, viewportWidth, normalizedXPosition, anchorLineScalingFactor );
                    if ( labelCandidate.has_value() ) labels.push_back( labelCandidate.value() );
                }

                if ( strategy == LabelPositionStrategy::COUNT_HINT || strategy == LabelPositionStrategy::ALL )
                {
                    std::vector<cvf::Vec3d> visibleCoords;
                    for ( const auto& v : annotationObject->anchorPointsInDisplayCoords() )
                    {
                        cvf::Vec3d screenCoord;
                        camera->project( v, &screenCoord );

                        if ( screenCoord.x() > 0 && screenCoord.x() < viewportWidth && screenCoord.y() > 0 && screenCoord.y() < viewportHeight )
                            visibleCoords.push_back( v );
                    }

                    size_t stride = 1;
                    if ( strategy == LabelPositionStrategy::COUNT_HINT )
                    {
                        stride = std::max( size_t( 1 ), visibleCoords.size() / std::max( 1, m_labelCountHint - 1 ) );
                    }

                    for ( size_t i = 0; i < visibleCoords.size(); i += stride )
                    {
                        size_t adjustedIndex = std::min( i, visibleCoords.size() - 1 );

                        const cvf::Vec3d lineAnchorPosition  = visibleCoords[adjustedIndex];
                        const cvf::Vec3d directionPointToCam = ( camera->position() - lineAnchorPosition ).getNormalized();
                        const cvf::Vec3d labelPosition       = lineAnchorPosition + directionPointToCam * anchorLineScalingFactor;

                        labels.push_back( { annotationObject->text(), lineAnchorPosition, labelPosition } );
                    }
                }
            }

            for ( const auto& [labelText, lineAnchorPosition, labelPosition] : labels )
            {
                {
                    // Line part

                    std::vector<cvf::Vec3d> points = { lineAnchorPosition, labelPosition };

                    auto anchorLineColor = cvf::Color3f::BLACK;
                    auto part            = RivAnnotationTools::createPartFromPolyline( anchorLineColor, points );

                    if ( part.notNull() )
                    {
                        part->setName( "AnnotationObjectAnchorPoints" );
                        model->addPart( part.p() );
                    }
                }

                {
                    // Text part

                    auto backgroundColor = annotationObject->showColor() ? annotationObject->color() : cvf::Color3f::LIGHT_GRAY;
                    auto textColor       = RiaColorTools::contrastColor( backgroundColor );
                    auto fontSize        = 10;
                    auto font            = RiaFontCache::getFont( fontSize );

                    auto drawableText = createDrawableText( font.p(), textColor, backgroundColor, labelText, cvf::Vec3f( labelPosition ) );

                    auto part = createPart( drawableText.p() );
                    part->setName( "RivAnnotationTools: " + labelText );

                    model->addPart( part.p() );
                }
            }
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableText> RivAnnotationTools::createDrawableText( cvf::Font*         font,
                                                                    cvf::Color3f       textColor,
                                                                    cvf::Color3f       backgroundColor,
                                                                    const std::string& text,
                                                                    const cvf::Vec3f&  position )
{
    auto drawableText = new cvf::DrawableText;

    drawableText->setFont( font );
    drawableText->setCheckPosVisible( false );
    drawableText->setUseDepthBuffer( true );
    drawableText->setDrawBorder( true );
    drawableText->setDrawBackground( true );
    drawableText->setVerticalAlignment( cvf::TextDrawer::BASELINE );
    drawableText->setBackgroundColor( backgroundColor );
    drawableText->setBorderColor( RiaColorTools::computeOffsetColor( backgroundColor, 0.5f ) );
    drawableText->setTextColor( textColor );
    drawableText->addText( cvf::String( text ), position );

    return drawableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivAnnotationTools::createPart( cvf::DrawableText* drawableText )
{
    auto part = new cvf::Part;
    part->setDrawable( drawableText );

    auto eff = new cvf::Effect();
    part->setEffect( eff );
    part->setPriority( RivPartPriority::PartType::Text );

    return part;
}
