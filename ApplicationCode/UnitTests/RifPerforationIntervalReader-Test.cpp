#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifPerforationIntervalReader.h"

#include <cmath> // Needed for HUGE_VAL on Linux
#include <numeric>

static const QString PERFORATION_TEST_DATA_DIRECTORY = QString("%1/RifPerforationIntervalReader/").arg(TEST_DATA_DIR);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RifPerforationIntervalReaderTest, SpacesInWellNameHandledSuccessfully)
{
    std::map<QString, std::vector<RifPerforationInterval> >
        perforationIntervals = RifPerforationIntervalReader::readPerforationIntervals(PERFORATION_TEST_DATA_DIRECTORY + "perforations_with_space_after_well_name.ev");

    EXPECT_EQ(size_t(10), perforationIntervals["A1_RI_HZX"].size());
}
