#include "gtest/gtest.h"

#include "RifRftSegment.h"

#include "RiaRftDefines.h"

#include <map>
#include <set>
#include <vector>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Build a topology with two tubing branches and their device segments.
///
///   Tubing branch 1 (deep)    : segment numbers 1-2,  branch id 1
///   Tubing branch 2 (shallow)  : segment numbers 3-4,  branch id 2
///   Device segments for branch 2 : segment numbers 5-6,  branch ids 11-12, outflow to segments 3-4
///   Device segments for branch 1 : segment numbers 7-8,  branch ids 13-14, outflow to segments 1-2
///
/// The device segments for branch 2 are stored immediately before the device segments for branch 1.
//--------------------------------------------------------------------------------------------------
RifRftSegment buildTwoBranchSegment()
{
    // RifRftSegmentData( segnxt, brno, brnst, brnen, segNo )
    std::vector<RifRftSegmentData> topology;

    topology.emplace_back( 0, 1, 1, 2, 1 );
    topology.emplace_back( 1, 1, 1, 2, 2 );

    topology.emplace_back( 0, 2, 3, 4, 3 );
    topology.emplace_back( 3, 2, 3, 4, 4 );

    topology.emplace_back( 3, 11, 5, 5, 5 );
    topology.emplace_back( 4, 12, 6, 6, 6 );

    topology.emplace_back( 1, 13, 7, 7, 7 );
    topology.emplace_back( 2, 14, 8, 8, 8 );

    RifRftSegment segment;
    segment.setSegmentData( topology );

    return segment;
}

//--------------------------------------------------------------------------------------------------
/// Classify the two tubing branches, as done before device branches are identified.
//--------------------------------------------------------------------------------------------------
void classifyTubingBranches( RifRftSegment& segment )
{
    segment.setBranchType( 1, RiaDefines::RftBranchType::RFT_TUBING );
    segment.setOneBasedBranchIndex( 1, 1 );
    segment.setBranchType( 2, RiaDefines::RftBranchType::RFT_TUBING );
    segment.setOneBasedBranchIndex( 2, 2 );
}

//--------------------------------------------------------------------------------------------------
/// Measured depths (SEGLENST) indexed by topology position. Branch 1 (deep) is at greater measured
/// depth than branch 2 (shallow), and the depths increase monotonically across the boundary between
/// the branch-2 and branch-1 device segments.
//--------------------------------------------------------------------------------------------------
std::vector<double> seglenstValues()
{
    return { 3700.0, 3800.0, 2700.0, 2800.0, 2700.0, 2800.0, 3700.0, 3800.0 };
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Verify that a device branch only claims the device segments that actually connect to its tubing
/// branch, and does not absorb device segments belonging to another tubing branch.
///
/// Reproduces a topology where a lower-numbered tubing branch (index 1) is located at greater
/// measured depth than a higher-numbered tubing branch (index 2). A measured-depth-only terminator
/// would incorrectly absorb the branch-1 device segments into branch index 2.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, DeviceBranchDoesNotAbsorbSegmentsFromOtherBranch )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    auto seglenst = seglenstValues();

    // Build the device branch for tubing branch index 2, starting at the first device segment that
    // feeds branch 2 (segment number 5).
    segment.createDeviceBranch( 5, 2, seglenst );

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
    segment.createDeviceBranch( 7, 1, seglenst );
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_DEVICE, segment.branchType( 13 ) );
    EXPECT_EQ( RiaDefines::RftBranchType::RFT_DEVICE, segment.branchType( 14 ) );
    EXPECT_EQ( 1, segment.oneBasedBranchIndexForBranchId( 13 ) );
    EXPECT_EQ( 1, segment.oneBasedBranchIndexForBranchId( 14 ) );
}

//--------------------------------------------------------------------------------------------------
/// branchIds() returns the unique branch numbers in sorted order.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, BranchIdsAreUniqueAndSorted )
{
    RifRftSegment segment = buildTwoBranchSegment();

    std::vector<int> expected = { 1, 2, 11, 12, 13, 14 };
    EXPECT_EQ( expected, segment.branchIds() );
}

//--------------------------------------------------------------------------------------------------
/// Segment lookup by segment number and by index, including lookups that do not match.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, SegmentLookup )
{
    RifRftSegment segment = buildTwoBranchSegment();

    // Lookup by segment number
    const auto* seg3 = segment.segmentData( 3 );
    ASSERT_NE( nullptr, seg3 );
    EXPECT_EQ( 3, seg3->segNo() );
    EXPECT_EQ( 2, seg3->segBrno() );

    EXPECT_EQ( nullptr, segment.segmentData( 99 ) );

    // Segment number to index
    EXPECT_EQ( 2, segment.segmentIndexFromSegmentNumber( 3 ) );
    EXPECT_EQ( -1, segment.segmentIndexFromSegmentNumber( 99 ) );

    // Lookup by index
    const auto* firstSeg = segment.segmentDataByIndex( 0 );
    ASSERT_NE( nullptr, firstSeg );
    EXPECT_EQ( 1, firstSeg->segNo() );
    EXPECT_EQ( 1, firstSeg->segBrno() );
}

//--------------------------------------------------------------------------------------------------
/// Unknown branches default to RFT_UNKNOWN type and branch index -1.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, UnknownBranchDefaults )
{
    RifRftSegment segment = buildTwoBranchSegment();

    EXPECT_EQ( RiaDefines::RftBranchType::RFT_UNKNOWN, segment.branchType( 11 ) );
    EXPECT_EQ( -1, segment.oneBasedBranchIndexForBranchId( 11 ) );
    EXPECT_EQ( -1, segment.oneBasedBranchIndexForBranchId( 999 ) );
}

//--------------------------------------------------------------------------------------------------
/// tubingBranchIds() returns only the branches classified as tubing.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, TubingBranchIdsFiltersByType )
{
    RifRftSegment segment = buildTwoBranchSegment();

    // No branch is classified yet
    EXPECT_TRUE( segment.tubingBranchIds().empty() );

    classifyTubingBranches( segment );

    std::vector<int> expected = { 1, 2 };
    EXPECT_EQ( expected, segment.tubingBranchIds() );
}

//--------------------------------------------------------------------------------------------------
/// segmentIndicesForBranchNumber() returns the indices for one branch, or all when branchNumber<=0.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, SegmentIndicesForBranchNumber )
{
    RifRftSegment segment = buildTwoBranchSegment();

    std::vector<size_t> expectedBranch2 = { 2, 3 };
    EXPECT_EQ( expectedBranch2, segment.segmentIndicesForBranchNumber( 2 ) );

    // branchNumber <= 0 returns all segment indices
    EXPECT_EQ( 8u, segment.segmentIndicesForBranchNumber( 0 ).size() );
    EXPECT_EQ( 8u, segment.segmentIndicesForBranchNumber( -1 ).size() );
}

//--------------------------------------------------------------------------------------------------
/// segmentIndicesForBranchIndex() filters on both the one-based branch index and the branch type.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, SegmentIndicesForBranchIndexFiltersByIndexAndType )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    std::vector<size_t> expectedBranch2 = { 2, 3 };
    EXPECT_EQ( expectedBranch2, segment.segmentIndicesForBranchIndex( 2, RiaDefines::RftBranchType::RFT_TUBING ) );

    std::vector<size_t> expectedBranch1 = { 0, 1 };
    EXPECT_EQ( expectedBranch1, segment.segmentIndicesForBranchIndex( 1, RiaDefines::RftBranchType::RFT_TUBING ) );

    // Matching index but wrong branch type yields nothing
    EXPECT_TRUE( segment.segmentIndicesForBranchIndex( 2, RiaDefines::RftBranchType::RFT_DEVICE ).empty() );

    // branchIndex <= 0 returns all segment indices
    EXPECT_EQ( 8u, segment.segmentIndicesForBranchIndex( 0, RiaDefines::RftBranchType::RFT_TUBING ).size() );
}

//--------------------------------------------------------------------------------------------------
/// segmentNumbersForBranchIndex() returns segment numbers for a given branch index and type.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, SegmentNumbersForBranchIndex )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    std::vector<int> expected = { 3, 4 };
    EXPECT_EQ( expected, segment.segmentNumbersForBranchIndex( 2, RiaDefines::RftBranchType::RFT_TUBING ) );
}

//--------------------------------------------------------------------------------------------------
/// branchIdsAndOneBasedBranchIndices() filters by branch type, and returns all for RFT_UNKNOWN.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, BranchIdsAndOneBasedBranchIndicesFiltersByType )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    auto               tubingMap      = segment.branchIdsAndOneBasedBranchIndices( RiaDefines::RftBranchType::RFT_TUBING );
    std::map<int, int> expectedTubing = { { 1, 1 }, { 2, 2 } };
    EXPECT_EQ( expectedTubing, tubingMap );

    // No device branches are assigned yet
    EXPECT_TRUE( segment.branchIdsAndOneBasedBranchIndices( RiaDefines::RftBranchType::RFT_DEVICE ).empty() );

    // RFT_UNKNOWN returns all branches that have a one-based index assigned
    auto allMap = segment.branchIdsAndOneBasedBranchIndices( RiaDefines::RftBranchType::RFT_UNKNOWN );
    EXPECT_EQ( expectedTubing, allMap );
}

//--------------------------------------------------------------------------------------------------
/// After both device branches are built, they are reported with their tubing branch index.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, DeviceBranchesReportedAfterBuild )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    auto seglenst = seglenstValues();
    segment.createDeviceBranch( 5, 2, seglenst );
    segment.createDeviceBranch( 7, 1, seglenst );

    auto               deviceMap      = segment.branchIdsAndOneBasedBranchIndices( RiaDefines::RftBranchType::RFT_DEVICE );
    std::map<int, int> expectedDevice = { { 11, 2 }, { 12, 2 }, { 13, 1 }, { 14, 1 } };
    EXPECT_EQ( expectedDevice, deviceMap );

    std::set<int> expectedIndices = { 1, 2 };
    EXPECT_EQ( expectedIndices, segment.uniqueOneBasedBranchIndices( RiaDefines::RftBranchType::RFT_TUBING ) );
}

//--------------------------------------------------------------------------------------------------
/// createDeviceBranch() stops claiming segments when it reaches an already typed branch.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, DeviceBranchStopsAtAlreadyTypedSegment )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    // Pre-classify the first branch-2 device segment as a device segment of a different branch
    segment.setBranchType( 11, RiaDefines::RftBranchType::RFT_DEVICE );
    segment.setOneBasedBranchIndex( 11, 1 );

    auto seglenst = seglenstValues();
    segment.createDeviceBranch( 5, 2, seglenst );

    // The already typed branch keeps its index and nothing further is claimed
    EXPECT_EQ( 1, segment.oneBasedBranchIndexForBranchId( 11 ) );
    EXPECT_EQ( -1, segment.oneBasedBranchIndexForBranchId( 12 ) );
}

//--------------------------------------------------------------------------------------------------
/// nonContinuousDeviceSegmentIndices() returns nothing when there are fewer than two device segments.
//--------------------------------------------------------------------------------------------------
TEST( RifRftSegmentTest, NonContinuousDeviceSegmentIndicesEmptyForFewSegments )
{
    RifRftSegment segment = buildTwoBranchSegment();
    classifyTubingBranches( segment );

    // Branch index 2 has no device segments before they are built
    EXPECT_TRUE( segment.nonContinuousDeviceSegmentIndices( 2 ).empty() );
}
