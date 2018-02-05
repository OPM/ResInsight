#include "gtest/gtest.h"

#include "RifReaderEclipseRft.h"
#include "RifEclipseRftAddress.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//TEST(RifReaderEclipseRftTest, TestRifEclipseRftAddress)
TEST(DISABLED_RifReaderEclipseRftTest, TestRifEclipseRftAddress)
{
    QString filename = "C:\\Users\\Rebecca Cox\\Dropbox\\norne\\norne\\NORNE_ATW2013.RFT";
    
    RifReaderEclipseRft reader(filename);

    std::vector<RifEclipseRftAddress> addresses = reader.eclipseRftAddresses();

    /*for (RifEclipseRftAddress address : addresses)
    {
        std::cout << address.wellName() << std::endl;
    }*/

    for (RifEclipseRftAddress address : addresses)
    {
        std::vector<double> values;
        reader.values(address, &values);
        EXPECT_TRUE(values.size() > 0);
    }

    ASSERT_TRUE(addresses.size() > 0);

    std::vector<double> values;
    reader.values(addresses[0], &values);
    ASSERT_TRUE(values.size() > 0);

    std::cout << "First value: " << values.front() << ", last value: " << values.back() << std::endl;
}
