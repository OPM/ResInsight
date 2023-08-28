#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderEclipseRft.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigWellLogExtractor.h"
#include "RimEclipseResultCase.h"
#include "RimFileWellPath.h"
#include <vector>

static const QString CASE_REAL_TEST_DATA_DIRECTORY_01 = QString( "%1/drogon/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifReaderEclipseRftTest, TestRifEclipseRftAddress )
{
    QString gridFilename     = CASE_REAL_TEST_DATA_DIRECTORY_01 + "DROGON-01.EGRID";
    QString rftFilename      = CASE_REAL_TEST_DATA_DIRECTORY_01 + "DROGON-01.RFT";
    QString wellPathFilename = CASE_REAL_TEST_DATA_DIRECTORY_01 + "DROGON-01.RFT";

    RifReaderEclipseRft reader( rftFilename );
    // reader.getMd( "well-01", )

    auto* rimResultReservoir = new RimEclipseResultCase();
    rimResultReservoir->setGridFileName( gridFilename );
    // rimResultReservoir->setCaseInfo( caseName, fileName );
    rimResultReservoir->openEclipseGridFile();
    // rimResultReservoir->setReaderSettings( readerSettings );

    RimFileWellPath wellPath;
    wellPath.setFilepath( wellPathFilename );
    auto        wellPathGeo = wellPath.wellPathGeometry();
    std::string errorMsg;

    RigEclipseWellLogExtractor extractor( rimResultReservoir->eclipseCaseData(), wellPathGeo, errorMsg );
    extractor.averageMdForCell( 1 );
}

/*
std::set<RifEclipseRftAddress> addresses = reader.eclipseRftAddresses();

/ *for (RifEclipseRftAddress address : addresses)
{
    std::cout << address.wellName() << std::endl;
}* /

for ( RifEclipseRftAddress address : addresses )
{
    std::vector<double> values;
    reader.values( address, &values );
    EXPECT_TRUE( values.size() > 0 );
}

ASSERT_TRUE( addresses.size() > 0 );

std::vector<double> values;
reader.values( *addresses.begin(), &values );
ASSERT_TRUE( values.size() > 0 );

std::cout << "First value: " << values.front() << ", last value: " << values.back() << std::endl;
*/
