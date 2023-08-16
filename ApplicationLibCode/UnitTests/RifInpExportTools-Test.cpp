#include "gtest/gtest.h"

#include "RifInpExportTools.h"

#include <sstream>
#include <string>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifInpExportTools, PrintLine )
{
    std::string line = "this is the line";

    std::stringstream stream;
    ASSERT_TRUE( RifInpExportTools::printLine( stream, line ) );

    std::string res = stream.str();
    ASSERT_TRUE( res.find( line ) != std::string::npos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifInpExportTools, PrintHeading )
{
    std::string line = "this is the heading";

    std::stringstream stream;
    ASSERT_TRUE( RifInpExportTools::printHeading( stream, line ) );

    std::string res = stream.str();
    ASSERT_TRUE( res.find( '*' + line ) != std::string::npos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifInpExportTools, PrintComment )
{
    std::string line = "this is the comment";

    std::stringstream stream;
    ASSERT_TRUE( RifInpExportTools::printComment( stream, line ) );

    std::string expectedString = std::string( "** " ).append( line );
    std::string res            = stream.str();
    ASSERT_TRUE( res.find( expectedString ) != std::string::npos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifInpExportTools, PrintNodes )
{
    std::vector<cvf::Vec3d> nodes = {
        cvf::Vec3d( 1.0, 1.1, 1.2 ),
        cvf::Vec3d( 2.0, 2.1, 2.2 ),
        cvf::Vec3d( 3.0, 3.1, 3.2 ),
        cvf::Vec3d( 4.0, 4.1, 4.2 ),
    };

    std::stringstream stream;
    ASSERT_TRUE( RifInpExportTools::printNodes( stream, nodes ) );

    auto splitLines = []( const std::string& input )
    {
        std::istringstream       stream( input );
        std::string              line;
        std::vector<std::string> lines;

        while ( std::getline( stream, line ) )
        {
            lines.push_back( line );
        }
        return lines;
    };

    auto lines = splitLines( stream.str() );
    ASSERT_EQ( 5u, lines.size() );

    std::string res = stream.str();
    ASSERT_TRUE( res.find( std::string( "*Node" ) ) != std::string::npos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifInpExportTools, Elements )
{
    std::vector<std::vector<unsigned int>> elements = {
        { 1, 1, 1, 1 },
        { 2, 2, 2, 2 },
        { 3, 3, 3, 3 },
        { 4, 4, 4, 4 },
        { 5, 5, 5, 5 },
    };

    std::stringstream stream;
    ASSERT_TRUE( RifInpExportTools::printElements( stream, elements ) );

    auto splitLines = []( const std::string& input )
    {
        std::istringstream       stream( input );
        std::string              line;
        std::vector<std::string> lines;

        while ( std::getline( stream, line ) )
        {
            lines.push_back( line );
        }
        return lines;
    };

    auto lines = splitLines( stream.str() );
    ASSERT_EQ( 6u, lines.size() );

    std::string res = stream.str();
    ASSERT_TRUE( res.find( std::string( "*Element" ) ) != std::string::npos );
}
