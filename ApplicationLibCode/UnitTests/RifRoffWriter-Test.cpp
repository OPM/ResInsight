#include "gtest/gtest.h"

#include "RifRoffWriter.h"

#include "Reader.hpp"

#include <QTemporaryDir>

#include <fstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static roff::RoffScalar findScalar( const std::vector<std::pair<std::string, roff::RoffScalar>>& values, const std::string& keyword )
{
    for ( const auto& [key, value] : values )
    {
        if ( key == keyword ) return value;
    }

    throw std::runtime_error( "Keyword not found: " + keyword );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRoffWriter, RoundTripScalarValues )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    std::string filePath = tempDir.filePath( "scalars.roffbin" ).toStdString();

    {
        std::ofstream stream( filePath, std::ios::binary );
        ASSERT_TRUE( stream.good() );

        RifRoffWriter writer( stream );
        writer.writeFileType();
        writer.writeFileDataTag( "parameter" );
        writer.writeVersionTag();

        writer.startTag( "values" );
        writer.writeInt( "myInt", 42 );
        writer.writeBool( "myBool", true );
        writer.writeByte( "myByte", 7 );
        writer.writeFloat( "myFloat", 1.5f );
        writer.writeDouble( "myDouble", 2.25 );
        writer.writeString( "myString", "hello roff" );
        writer.endTag();

        writer.writeEofTag();
    }

    std::ifstream stream( filePath, std::ios::binary );
    ASSERT_TRUE( stream.good() );

    roff::Reader reader( stream );
    ASSERT_NO_THROW( reader.parse() );

    auto scalarValues = reader.scalarNamedValues();

    EXPECT_EQ( 1, std::get<int>( findScalar( scalarValues, "filedata.byteswaptest" ) ) );
    EXPECT_EQ( std::string( "parameter" ), std::get<std::string>( findScalar( scalarValues, "filedata.filetype" ) ) );
    EXPECT_EQ( 2, std::get<int>( findScalar( scalarValues, "version.major" ) ) );
    EXPECT_EQ( 0, std::get<int>( findScalar( scalarValues, "version.minor" ) ) );

    EXPECT_EQ( 42, std::get<int>( findScalar( scalarValues, "values.myInt" ) ) );
    EXPECT_TRUE( std::get<bool>( findScalar( scalarValues, "values.myBool" ) ) );
    EXPECT_EQ( 7, std::get<unsigned char>( findScalar( scalarValues, "values.myByte" ) ) );
    EXPECT_FLOAT_EQ( 1.5f, std::get<float>( findScalar( scalarValues, "values.myFloat" ) ) );
    EXPECT_DOUBLE_EQ( 2.25, std::get<double>( findScalar( scalarValues, "values.myDouble" ) ) );
    EXPECT_EQ( std::string( "hello roff" ), std::get<std::string>( findScalar( scalarValues, "values.myString" ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRoffWriter, RoundTripArrays )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    std::string filePath = tempDir.filePath( "arrays.roffbin" ).toStdString();

    const std::vector<int>         intValues    = { 1, 2, 3, -4, 1000000 };
    const std::vector<float>       floatValues  = { 0.5f, -1.25f, 3.75f };
    const std::vector<double>      doubleValues = { 0.1, -2.5, 1e10, -1e-10 };
    const std::vector<char>        byteValues   = { 0, 1, 2, 3 };
    const std::vector<std::string> stringValues = { "first", "second", "third" };

    {
        std::ofstream stream( filePath, std::ios::binary );
        ASSERT_TRUE( stream.good() );

        RifRoffWriter writer( stream );
        writer.writeFileType();
        writer.writeFileDataTag( "parameter" );
        writer.writeVersionTag();

        writer.startTag( "arrays" );
        writer.writeIntArray( "myInts", intValues );
        writer.writeFloatArray( "myFloats", floatValues );
        writer.writeDoubleArray( "myDoubles", doubleValues );
        writer.writeByteArray( "myBytes", byteValues );
        writer.writeStringArray( "myStrings", stringValues );
        writer.endTag();

        writer.writeEofTag();
    }

    std::ifstream stream( filePath, std::ios::binary );
    ASSERT_TRUE( stream.good() );

    roff::Reader reader( stream );
    ASSERT_NO_THROW( reader.parse() );

    EXPECT_EQ( intValues.size(), reader.getArrayLength( "arrays.myInts" ) );
    EXPECT_EQ( intValues, reader.getIntArray( "arrays.myInts" ) );

    EXPECT_EQ( floatValues.size(), reader.getArrayLength( "arrays.myFloats" ) );
    EXPECT_EQ( floatValues, reader.getFloatArray( "arrays.myFloats" ) );

    EXPECT_EQ( doubleValues.size(), reader.getArrayLength( "arrays.myDoubles" ) );
    EXPECT_EQ( doubleValues, reader.getDoubleArray( "arrays.myDoubles" ) );

    EXPECT_EQ( byteValues.size(), reader.getArrayLength( "arrays.myBytes" ) );
    EXPECT_EQ( byteValues, reader.getByteArray( "arrays.myBytes" ) );

    EXPECT_EQ( stringValues.size(), reader.getArrayLength( "arrays.myStrings" ) );
    EXPECT_EQ( stringValues, reader.getStringArray( "arrays.myStrings" ) );
}

//--------------------------------------------------------------------------------------------------
/// The parameter tag layout used by RMS/xtgeo grid properties: the parameter name key gives the
/// name used to look up the data array
//--------------------------------------------------------------------------------------------------
TEST( RifRoffWriter, RoundTripParameterTag )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    std::string filePath = tempDir.filePath( "parameter.roffbin" ).toStdString();

    const std::vector<float> parameterValues = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };

    {
        std::ofstream stream( filePath, std::ios::binary );
        ASSERT_TRUE( stream.good() );

        RifRoffWriter writer( stream );
        writer.writeFileType();
        writer.writeFileDataTag( "parameter" );
        writer.writeVersionTag();

        writer.startTag( "dimensions" );
        writer.writeInt( "nX", 3 );
        writer.writeInt( "nY", 2 );
        writer.writeInt( "nZ", 1 );
        writer.endTag();

        writer.startTag( "parameter" );
        writer.writeString( "name", "MY_PROPERTY" );
        writer.writeFloatArray( "data", parameterValues );
        writer.endTag();

        writer.writeEofTag();
    }

    std::ifstream stream( filePath, std::ios::binary );
    ASSERT_TRUE( stream.good() );

    roff::Reader reader( stream );
    ASSERT_NO_THROW( reader.parse() );

    auto scalarValues = reader.scalarNamedValues();
    EXPECT_EQ( 3, std::get<int>( findScalar( scalarValues, "dimensions.nX" ) ) );
    EXPECT_EQ( 2, std::get<int>( findScalar( scalarValues, "dimensions.nY" ) ) );
    EXPECT_EQ( 1, std::get<int>( findScalar( scalarValues, "dimensions.nZ" ) ) );

    // The reader exposes the parameter data array under the parameter name
    EXPECT_EQ( parameterValues.size(), reader.getArrayLength( "MY_PROPERTY" ) );
    EXPECT_EQ( parameterValues, reader.getFloatArray( "MY_PROPERTY" ) );
}
