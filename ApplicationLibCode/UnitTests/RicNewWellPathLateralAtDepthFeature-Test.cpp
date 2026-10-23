#include "gtest/gtest.h"

#include "WellPathCommands/RicNewWellPathLateralAtDepthFeature.h"

#include "RiaApplication.h"

#include "Well/RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimWellPath.h"

#include "cvfVector3.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// A tie-in depth above the start of the parent well path, like the default depth of the Python
/// interface, gives an empty subset of the parent well path. The lateral must still be created.
//--------------------------------------------------------------------------------------------------
TEST( RicNewWellPathLateralAtDepthFeature, TieInDepthAboveStartOfParent )
{
    RimWellPath parentWellPath;

    const std::vector<cvf::Vec3d> points         = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, -100.0 }, { 0.0, 0.0, -200.0 } };
    const std::vector<double>     measuredDepths = { 100.0, 200.0, 300.0 };

    auto* geometry = new RigWellPath;
    geometry->setWellPathPoints( points, measuredDepths );
    parentWellPath.setWellPathGeometry( geometry );

    RimModeledWellPath* lateral = RicNewWellPathLateralAtDepthFeature::createLateralAtMeasuredDepth( &parentWellPath, 0.0 );
    EXPECT_TRUE( lateral != nullptr );

    RiaApplication::instance()->closeProject();
}
