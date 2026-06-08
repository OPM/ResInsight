#include "gtest/gtest.h"

#include "RifRftSegment.h"

#include "RiaRftDefines.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// Verify that a device branch only claims the device segments that actually connect to its tubing
/// branch, and does not absorb device segments belonging to another tubing branch.
///
/// Reproduces a topology where a lower-numbered tubing branch (index 1) is located at greater
/// measured depth than a higher-numbered tubing branch (index 2). The device segments are stored so
/// that the branch-2 device segments are immediately followed by the branch-1 device segments with
/// monotonically increasing measured depth. A measured-depth-only terminator would incorrectly
/// absorb the branch-1 device segments into branch index 2.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, DeviceBranchDoesNotAbsorbSegmentsFromOtherBranch )
{
    // RifRftSegmentData( segnxt, brno, brnst, brnen, segNo )
    std::vector<RifRftSegmentData> topology;

    // Tubing branch 1 (deep), segment numbers 1-2
    topology.emplace_back( 0, 1, 1, 2, 1 );
    topology.emplace_back( 1, 1, 1, 2, 2 );

    // Tubing branch 2 (shallow), segment numbers 3-4
    topology.emplace_back( 0, 2, 3, 4, 3 );
    topology.emplace_back( 3, 2, 3, 4, 4 );

    // Device segments feeding tubing branch 2 (shallow), segment numbers 5-6.
    // Their outflow (segNext) points to the branch-2 tubing segments 3 and 4.
    topology.emplace_back( 3, 11, 5, 5, 5 );
    topology.emplace_back( 4, 12, 6, 6, 6 );

    // Device segments feeding tubing branch 1 (deep), segment numbers 7-8.
    // Their outflow (segNext) points to the branch-1 tubing segments 1 and 2.
    topology.emplace_back( 1, 13, 7, 7, 7 );
    topology.emplace_back( 2, 14, 8, 8, 8 );

    RifRftSegment segment;
    segment.setSegmentData( topology );

    // Measured depths (SEGLENST), indexed by topology position. Branch 1 (deep) is at greater
    // measured depth than branch 2 (shallow), and the depths increase monotonically across the
    // boundary between the branch-2 and branch-1 device segments.
    std::vector<double> seglenstValues = { 3700.0, 3800.0, 2700.0, 2800.0, 2700.0, 2800.0, 3700.0, 3800.0 };

    // Classify the tubing branches, as done before device branches are identified.
    segment.setBranchType( 1, RiaDefines::RftBranchType::RFT_TUBING );
    segment.setOneBasedBranchIndex( 1, 1 );
    segment.setBranchType( 2, RiaDefines::RftBranchType::RFT_TUBING );
    segment.setOneBasedBranchIndex( 2, 2 );

    // Build the device branch for tubing branch index 2, starting at the first device segment that
    // feeds branch 2 (segment number 5).
    segment.createDeviceBranch( 5, 2, seglenstValues );

    // The device segments feeding branch 2 must be assigned to branch index 2.
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_DEVICE, segment.branchType( 11 ) );
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_DEVICE, segment.branchType( 12 ) );
    EXPECT_EQ( 2, segment.oneBasedBranchIndexForBranchId( 11 ) );
    EXPECT_EQ( 2, segment.oneBasedBranchIndexForBranchId( 12 ) );

    // The device segments feeding branch 1 must NOT be absorbed into branch index 2.
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_UNKNOWN, segment.branchType( 13 ) );
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_UNKNOWN, segment.branchType( 14 ) );
    EXPECT_EQ( -1, segment.oneBasedBranchIndexForBranchId( 13 ) );
    EXPECT_EQ( -1, segment.oneBasedBranchIndexForBranchId( 14 ) );

    // Building the device branch for tubing branch index 1 assigns the remaining device segments.
    segment.createDeviceBranch( 7, 1, seglenstValues );
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_DEVICE, segment.branchType( 13 ) );
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_DEVICE, segment.branchType( 14 ) );
    EXPECT_EQ( 1, segment.oneBasedBranchIndexForBranchId( 13 ) );
    EXPECT_EQ( 1, segment.oneBasedBranchIndexForBranchId( 14 ) );
}
