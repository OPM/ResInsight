/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "gtest/gtest.h"

#include "RivAnnotationSourceInfo.h"
#include "RivAnnotationTools.h"
#include "RivPipeSurfaceLabelSourceInfo.h"

#include "cvfCamera.h"
#include "cvfDrawableText.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfViewport.h"

#include <optional>

namespace
{
const double viewportWidth  = 1000.0;
const double viewportHeight = 800.0;

//--------------------------------------------------------------------------------------------------
/// Create a perspective camera using the near and far plane distances computed by caf::Viewer, see
/// caf::Viewer::calculateNearFarPlanes()
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Camera> createPerspectiveCamera( const cvf::Vec3d& eye, const cvf::Vec3d& viewRefPoint, double nearPlane, double farPlane )
{
    cvf::ref<cvf::Camera> camera = new cvf::Camera;
    camera->setViewport( 0, 0, static_cast<int>( viewportWidth ), static_cast<int>( viewportHeight ) );
    camera->setFromLookAt( eye, viewRefPoint, cvf::Vec3d::Z_AXIS );
    camera->setProjectionAsPerspective( 40.0, nearPlane, farPlane );

    return camera;
}

//--------------------------------------------------------------------------------------------------
/// Create a part representing a surface intersection curve, with the annotation source info used to
/// create labels
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> createAnnotationPart( const std::vector<cvf::Vec3d>& displayCoords )
{
    auto annotationObject = new RivAnnotationSourceInfo( "Surface Name", displayCoords );
    annotationObject->setLabelPositionStrategyHint( RivAnnotationTools::LabelPositionStrategy::RIGHT );

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setSourceInfo( annotationObject );

    return part;
}

//--------------------------------------------------------------------------------------------------
/// Find the position of the single label text created by RivAnnotationTools::addAnnotationLabels()
//--------------------------------------------------------------------------------------------------
std::optional<cvf::Vec3d> findLabelPosition( cvf::ModelBasicList* model )
{
    for ( cvf::uint i = 0; i < model->partCount(); i++ )
    {
        const auto* part = model->part( i );
        if ( dynamic_cast<const cvf::DrawableText*>( part->drawable() ) )
        {
            return cvf::Vec3d( part->drawable()->boundingBox().center() );
        }
    }

    return std::nullopt;
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// The label is moved towards the camera to be drawn in front of other geometry. The offset is derived from the zoom level, and can be much
/// larger than the distance between the camera and the curve. Labels outside the view frustum are silently discarded by the text renderer.
//--------------------------------------------------------------------------------------------------
TEST( RivAnnotationToolsTest, LabelIsInsideViewFrustum )
{
    const cvf::Vec3d eye( 0.0, -1500.0, 0.0 );

    // The near plane is placed just in front of the closest geometry, and the far plane just behind
    auto camera = createPerspectiveCamera( eye, cvf::Vec3d::ZERO, 0.8 * 1500.0, 1.2 * 1500.0 );

    std::vector<cvf::Vec3d> curveCoords;
    for ( int i = -10; i <= 10; i++ )
    {
        curveCoords.emplace_back( i * 50.0, 0.0, 0.0 );
    }

    cvf::Collection<cvf::Part> partCollection;
    partCollection.push_back( createAnnotationPart( curveCoords ).p() );

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    RivAnnotationTools annoTool;
    annoTool.addAnnotationLabels( partCollection, camera.p(), model.p(), true );

    auto labelPosition = findLabelPosition( model.p() );
    ASSERT_TRUE( labelPosition.has_value() );

    cvf::Vec3d screenCoord;
    ASSERT_TRUE( camera->project( labelPosition.value(), &screenCoord ) );

    // A label outside the near and far planes is not drawn
    EXPECT_GT( screenCoord.z(), 0.0 );
    EXPECT_LT( screenCoord.z(), 1.0 );

    EXPECT_GT( screenCoord.x(), 0.0 );
    EXPECT_LT( screenCoord.x(), viewportWidth );
    EXPECT_GT( screenCoord.y(), 0.0 );
    EXPECT_LT( screenCoord.y(), viewportHeight );
}

//--------------------------------------------------------------------------------------------------
/// cvf::Camera::project() divides by the homogeneous w-coordinate, and coordinates behind the camera are mirrored into the viewport. Such
/// coordinates must not be used as anchor points for a label.
//--------------------------------------------------------------------------------------------------
TEST( RivAnnotationToolsTest, AnchorPointsBehindCameraAreIgnored )
{
    const cvf::Vec3d eye( 0.0, -100.0, 0.0 );

    auto camera = createPerspectiveCamera( eye, cvf::Vec3d::ZERO, 10.0, 2000.0 );

    // A curve along the view direction, where the first coordinates are behind the camera. The coordinates behind the camera are mirrored
    // into the right part of the viewport, and are closer to the label position at 90% of the viewport width than any of the coordinates in
    // front of the camera.
    std::vector<cvf::Vec3d> curveCoords;
    for ( int i = -10; i <= 10; i++ )
    {
        curveCoords.emplace_back( -300.0, i * 50.0, 0.0 );
    }

    cvf::Collection<cvf::Part> partCollection;
    partCollection.push_back( createAnnotationPart( curveCoords ).p() );

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    RivAnnotationTools annoTool;
    annoTool.addAnnotationLabels( partCollection, camera.p(), model.p(), true );

    auto labelPosition = findLabelPosition( model.p() );
    ASSERT_TRUE( labelPosition.has_value() );

    // The label must be located in front of the camera
    const double distanceAlongViewDir = ( labelPosition.value() - camera->position() ) * camera->direction();
    EXPECT_GT( distanceAlongViewDir, camera->nearPlane() );

    cvf::Vec3d screenCoord;
    ASSERT_TRUE( camera->project( labelPosition.value(), &screenCoord ) );

    EXPECT_GT( screenCoord.z(), 0.0 );
    EXPECT_LT( screenCoord.z(), 1.0 );
}

//--------------------------------------------------------------------------------------------------
/// Labels along a pipe are placed at the radial position of the pipe closest to the camera
//--------------------------------------------------------------------------------------------------
TEST( RivAnnotationToolsTest, PipeSurfaceLabelIsClosestToCamera )
{
    const cvf::Vec3d center( 10.0, 20.0, 30.0 );
    const cvf::Vec3d tangent( 2.0, 0.0, 0.0 );
    const double     radius = 2.0;

    // The component of the direction to the camera along the pipe is ignored
    auto position = RivPipeSurfaceLabelSourceInfo::closestSurfacePointToCamera( center, tangent, radius, { 5.0, 0.0, 1.0 } );

    auto offset = position - center;
    EXPECT_NEAR( 0.0, offset * tangent, 1e-9 );
    EXPECT_GT( offset.z(), 0.0 );
    EXPECT_NEAR( 0.0, offset.y(), 1e-9 );
    EXPECT_NEAR( radius, offset.length(), 1e-9 );

    // Looking along the pipe, there is no radial position closest to the camera
    position = RivPipeSurfaceLabelSourceInfo::closestSurfacePointToCamera( center, tangent, radius, { -3.0, 0.0, 0.0 } );
    EXPECT_NEAR( 0.0, ( position - center ).length(), 1e-9 );
}

//--------------------------------------------------------------------------------------------------
/// Labels are offset from the anchor point perpendicular to the pipe on the screen, and upwards
//--------------------------------------------------------------------------------------------------
TEST( RivAnnotationToolsTest, PipeSurfaceLabelOffsetDirection )
{
    auto direction = RivPipeSurfaceLabelSourceInfo::screenOffsetDirection( { 10.0, 0.0 } );
    EXPECT_NEAR( 0.0, direction.x(), 1e-9 );
    EXPECT_NEAR( 1.0, direction.y(), 1e-9 );

    direction = RivPipeSurfaceLabelSourceInfo::screenOffsetDirection( { -3.0, 3.0 } );
    EXPECT_NEAR( 0.0, direction * cvf::Vec2d( -3.0, 3.0 ), 1e-9 );
    EXPECT_GT( direction.y(), 0.0 );
    EXPECT_NEAR( 1.0, direction.length(), 1e-9 );

    // A pipe seen end-on has no direction on the screen
    direction = RivPipeSurfaceLabelSourceInfo::screenOffsetDirection( { 0.0, 0.0 } );
    EXPECT_NEAR( 0.0, direction.x(), 1e-9 );
    EXPECT_NEAR( 1.0, direction.y(), 1e-9 );
}

//--------------------------------------------------------------------------------------------------
/// A pipe surface label creates an anchor line, an anchor point and a text drawn above the anchor point
//--------------------------------------------------------------------------------------------------
TEST( RivAnnotationToolsTest, PipeSurfaceLabelIsAboveAnchorPoint )
{
    const double radius = 2.0;

    std::vector<RivPipeSurfaceLabelSourceInfo::Label> labels = { { "1000", cvf::Vec3d::ZERO, cvf::Vec3d::X_AXIS } };

    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setSourceInfo( new RivPipeSurfaceLabelSourceInfo( labels, radius, cvf::Color3f::BLACK, cvf::Color3f::RED, nullptr ) );

    cvf::Collection<cvf::Part> parts;
    parts.push_back( part.p() );

    auto camera = createPerspectiveCamera( { 0.0, -100.0, 0.0 }, cvf::Vec3d::ZERO, 1.0, 1000.0 );

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    RivAnnotationTools annotationTools;
    annotationTools.addAnnotationLabels( parts, camera.p(), model.p(), true );

    ASSERT_EQ( 3u, model->partCount() );

    const auto labelPosition = findLabelPosition( model.p() );
    ASSERT_TRUE( labelPosition.has_value() );

    const cvf::Vec3d anchor( 0.0, -radius, 0.0 );

    cvf::Vec3d screenAnchor;
    cvf::Vec3d screenLabel;
    ASSERT_TRUE( camera->project( anchor, &screenAnchor ) );
    ASSERT_TRUE( camera->project( labelPosition.value(), &screenLabel ) );

    EXPECT_GT( screenLabel.y(), screenAnchor.y() );
    EXPECT_NEAR( screenAnchor.x(), screenLabel.x(), 1.0 );
}
