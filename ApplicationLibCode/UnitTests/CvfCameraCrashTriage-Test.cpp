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
