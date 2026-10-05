#include "gtest/gtest.h"

#include "cvfBoundingBox.h"
#include "cvfCamera.h"

using namespace cvf;

//--------------------------------------------------------------------------------------------------
/// computeFitViewEyePosition() used to assert/abort (CVF_ASSERT in Plane::distanceToOrigin) when
/// the view direction and up vector are parallel: dir^up then becomes a zero vector, producing an
/// invalid (zero-normal) plane used internally. Reproduces the crash reported from
/// caf::Viewer::zoomAll() with a degenerate camera orientation.
//--------------------------------------------------------------------------------------------------
TEST( CvfCameraCrashTriageTest, FitViewWithParallelDirAndUpDoesNotCrash )
{
    Camera c;

    BoundingBox bb;
    bb.add( Vec3d( 0, 0, 0 ) );
    bb.add( Vec3d( 1, 1, 1 ) );

    Vec3d  inViewDir( 0, 0, 1 );
    Vec3d  inUp( 0, 0, 1 );
    double coverageFactor = 0.9;

    Vec3d eye = Camera::computeFitViewEyePosition( bb, inViewDir, inUp, coverageFactor, c.fieldOfViewYDeg(), 1.0 );

    EXPECT_FALSE( eye.isUndefined() );
}

//--------------------------------------------------------------------------------------------------
/// fitView() must also produce a valid, non-singular camera orientation when dir and up are
/// parallel: it uses the (uncorrected) up vector both for setFromLookAt() and internally in
/// computeFitViewEyePosition()/createLookAtMatrix(), so the whole up vector derivation must be
/// consistent, not only the right vector used to compute the eye position.
//--------------------------------------------------------------------------------------------------
TEST( CvfCameraCrashTriageTest, FitViewWithParallelDirAndUpProducesValidViewMatrix )
{
    Camera c;
    c.setViewport( 0, 0, 100, 100 );

    BoundingBox bb;
    bb.add( Vec3d( 0, 0, 0 ) );
    bb.add( Vec3d( 1, 1, 1 ) );

    Vec3d  inViewDir( 0, 0, 1 );
    Vec3d  inUp( 0, 0, 1 );
    double coverageFactor = 0.9;

    c.fitView( bb, inViewDir, inUp, coverageFactor );

    Vec3d eye;
    Vec3d vrp;
    Vec3d up;
    c.toLookAt( &eye, &vrp, &up );

    EXPECT_FALSE( eye.isUndefined() );
    EXPECT_FALSE( c.direction().isUndefined() );
    EXPECT_FALSE( c.up().isUndefined() );
    EXPECT_FALSE( c.right().isUndefined() );

    // direction, up and right should form a valid, mutually orthogonal basis (within floating point
    // tolerance). If the up vector correction is not applied consistently, the derived up/right
    // vectors from the (singular) view matrix will be degenerate (zero length or not orthogonal).
    EXPECT_NEAR( c.direction().length(), 1.0, 1e-6 );
    EXPECT_NEAR( c.up().length(), 1.0, 1e-6 );
    EXPECT_NEAR( c.right().length(), 1.0, 1e-6 );
    EXPECT_NEAR( c.direction() * c.up(), 0.0, 1e-6 );
    EXPECT_NEAR( c.direction() * c.right(), 0.0, 1e-6 );
    EXPECT_NEAR( c.up() * c.right(), 0.0, 1e-6 );
}
