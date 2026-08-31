#include "gtest/gtest.h"

#include "cafFilePath.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmScriptIOMessages.h"

#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"

#include <QString>
#include <QTextStream>

#include <utility>
#include <vector>

namespace
{
std::vector<QString> parseStringList( const QString& text, bool stringsAreQuoted )
{
    QString                  source = text;
    QTextStream              stream( &source );
    std::vector<QString>     destination;
    caf::PdmScriptIOMessages messages;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::writeToField( destination, stream, &messages, stringsAreQuoted );

    return destination;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// https://github.com/OPM/ResInsight/issues/14648
///
/// A string list sent from Python (via gRPC) is serialized as "[a, b, c]" and parsed with
/// stringsAreQuoted = false. Strings containing a comma are quoted by the Python client, and must
/// not be split into multiple items. If they are, set_discrete_property_category_names() fails with
/// "CategoryValues and CategoryNames must have matching sizes"
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListUnquotedWithCommaInString )
{
    // This is the text produced by rips when sending ["Coal,Calcite", "Channel"]. Only strings
    // containing separator characters are quoted.
    auto values = parseStringList( R"(["Coal,Calcite", Channel])", false );

    ASSERT_EQ( size_t( 2 ), values.size() );
    EXPECT_STREQ( "Coal,Calcite", values[0].toStdString().c_str() );
    EXPECT_STREQ( "Channel", values[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// https://github.com/OPM/ResInsight/issues/14648
///
/// Quoted strings containing a comma must be kept as one item
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListQuotedWithCommaInString )
{
    auto values = parseStringList( R"(["Coal,Calcite", "Channel"])", true );

    ASSERT_EQ( size_t( 2 ), values.size() );
    EXPECT_STREQ( "Coal,Calcite", values[0].toStdString().c_str() );
    EXPECT_STREQ( "Channel", values[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// https://github.com/OPM/ResInsight/issues/14648
///
/// Serialize and parse a string list containing commas
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListWithCommaRoundTrip )
{
    const std::vector<QString> source = { "Coal,Calcite", "Channel" };

    QString     serialized;
    QTextStream outputStream( &serialized );
    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::readFromField( source, outputStream, true, false );

    QTextStream              inputStream( &serialized );
    std::vector<QString>     destination;
    caf::PdmScriptIOMessages messages;
    const bool               stringsAreQuoted = true;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::writeToField( destination, inputStream, &messages, stringsAreQuoted );

    ASSERT_EQ( source.size(), destination.size() );
    EXPECT_STREQ( "Coal,Calcite", destination[0].toStdString().c_str() );
    EXPECT_STREQ( "Channel", destination[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// Text values are allowed to contain parentheses, also unbalanced ones. These characters must not
/// be interpreted as nested containers for text values.
///
/// NOTE: An unquoted text value can not contain the array end character ']', as this has always been
/// used to terminate the array. Text values containing brackets are quoted by the Python client.
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListWithBracketsAndParenthesesInString )
{
    {
        auto values = parseStringList( "[WELL-A (main), WELL-B (side), WELL-C]", false );

        ASSERT_EQ( size_t( 3 ), values.size() );
        EXPECT_STREQ( "WELL-A (main)", values[0].toStdString().c_str() );
        EXPECT_STREQ( "WELL-B (side)", values[1].toStdString().c_str() );
        EXPECT_STREQ( "WELL-C", values[2].toStdString().c_str() );
    }

    {
        // Unbalanced parentheses, both inside and in front of the value
        auto values = parseStringList( "[(WELL-A, WELL-B(1, WELL-C]", false );

        ASSERT_EQ( size_t( 3 ), values.size() );
        EXPECT_STREQ( "(WELL-A", values[0].toStdString().c_str() );
        EXPECT_STREQ( "WELL-B(1", values[1].toStdString().c_str() );
        EXPECT_STREQ( "WELL-C", values[2].toStdString().c_str() );
    }

    {
        // Values containing brackets are quoted by the Python client
        auto values = parseStringList( R"(["WELL-A [side]", WELL-B])", false );

        ASSERT_EQ( size_t( 2 ), values.size() );
        EXPECT_STREQ( "WELL-A [side]", values[0].toStdString().c_str() );
        EXPECT_STREQ( "WELL-B", values[1].toStdString().c_str() );
    }
}

//--------------------------------------------------------------------------------------------------
/// A quote is only given special meaning when it is the first character of a value. Text values
/// containing a quote are quoted and escaped by the Python client.
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListWithQuoteInsideString )
{
    auto values = parseStringList( R"(["12\" pipe", "8\" pipe"])", false );

    ASSERT_EQ( size_t( 2 ), values.size() );
    EXPECT_STREQ( "12\" pipe", values[0].toStdString().c_str() );
    EXPECT_STREQ( "8\" pipe", values[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// White space inside a text value must be preserved
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListWithWhiteSpaceInString )
{
    auto values = parseStringList( "[Well A, Well B ]", false );

    ASSERT_EQ( size_t( 2 ), values.size() );
    EXPECT_STREQ( "Well A", values[0].toStdString().c_str() );
    EXPECT_STREQ( "Well B", values[1].toStdString().c_str() );

    // Leading and trailing white space is preserved for quoted values
    auto quotedValues = parseStringList( R"([" Well A ", Well B])", false );

    ASSERT_EQ( size_t( 2 ), quotedValues.size() );
    EXPECT_STREQ( " Well A ", quotedValues[0].toStdString().c_str() );
    EXPECT_STREQ( "Well B", quotedValues[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// Empty lists and lists with empty strings
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListEmpty )
{
    EXPECT_EQ( size_t( 0 ), parseStringList( "[]", false ).size() );
    EXPECT_EQ( size_t( 0 ), parseStringList( "[ ]", false ).size() );
    EXPECT_EQ( size_t( 1 ), parseStringList( R"([""])", false ).size() );
}

//--------------------------------------------------------------------------------------------------
/// File paths are text values, and must be transferred without modification
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, FilePathList )
{
    QString                    source = R"([C:\Users\file (1).txt, /tmp/my data/case.EGRID])";
    QTextStream                stream( &source );
    std::vector<caf::FilePath> destination;
    caf::PdmScriptIOMessages   messages;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<caf::FilePath>>::writeToField( destination, stream, &messages, false );

    ASSERT_EQ( size_t( 2 ), destination.size() );
    EXPECT_STREQ( "C:\\Users\\file (1).txt", destination[0].path().toStdString().c_str() );
    EXPECT_STREQ( "/tmp/my data/case.EGRID", destination[1].path().toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// Numeric lists are unaffected
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, NumberLists )
{
    {
        QString                  source = "[1, 2, 3]";
        QTextStream              stream( &source );
        std::vector<int>         destination;
        caf::PdmScriptIOMessages messages;

        caf::PdmFieldScriptingCapabilityIOHandler<std::vector<int>>::writeToField( destination, stream, &messages, false );

        ASSERT_EQ( size_t( 3 ), destination.size() );
        EXPECT_EQ( 3, destination[2] );
    }

    {
        QString                  source = "[1.5,2.5 , -3.5]";
        QTextStream              stream( &source );
        std::vector<double>      destination;
        caf::PdmScriptIOMessages messages;

        caf::PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::writeToField( destination, stream, &messages, false );

        ASSERT_EQ( size_t( 3 ), destination.size() );
        EXPECT_DOUBLE_EQ( 1.5, destination[0] );
        EXPECT_DOUBLE_EQ( 2.5, destination[1] );
        EXPECT_DOUBLE_EQ( -3.5, destination[2] );
    }
}

//--------------------------------------------------------------------------------------------------
/// Nested containers must be parsed as one item each
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, NestedContainerLists )
{
    {
        QString                          source = "[[1, 2], [3, 4]]";
        QTextStream                      stream( &source );
        std::vector<std::vector<double>> destination;
        caf::PdmScriptIOMessages         messages;

        caf::PdmFieldScriptingCapabilityIOHandler<std::vector<std::vector<double>>>::writeToField( destination, stream, &messages, false );

        ASSERT_EQ( size_t( 2 ), destination.size() );
        ASSERT_EQ( size_t( 2 ), destination[0].size() );
        EXPECT_DOUBLE_EQ( 2.0, destination[0][1] );
        EXPECT_DOUBLE_EQ( 3.0, destination[1][0] );
    }

    {
        QString                              source = "[(true, 1.0), (false, 2.0)]";
        QTextStream                          stream( &source );
        std::vector<std::pair<bool, double>> destination;
        caf::PdmScriptIOMessages             messages;

        caf::PdmFieldScriptingCapabilityIOHandler<std::vector<std::pair<bool, double>>>::writeToField( destination, stream, &messages, false );

        ASSERT_EQ( size_t( 2 ), destination.size() );
        EXPECT_TRUE( destination[0].first );
        EXPECT_DOUBLE_EQ( 1.0, destination[0].second );
        EXPECT_FALSE( destination[1].first );
        EXPECT_DOUBLE_EQ( 2.0, destination[1].second );
    }
}

//--------------------------------------------------------------------------------------------------
/// A list of 3D vectors is parsed by a dedicated handler that reads the inner arrays from the same
/// stream. Verify that the stream position handling is intact.
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, Vec3dList )
{
    QString                           source = "[[1, 2, 3], [4, 5, 6]]";
    QTextStream                       stream( &source );
    std::vector<cvf::Vector3<double>> destination;
    caf::PdmScriptIOMessages          messages;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<cvf::Vector3<double>>>::writeToField( destination, stream, &messages, false );

    ASSERT_EQ( size_t( 2 ), destination.size() );
    EXPECT_DOUBLE_EQ( 1.0, destination[0].x() );
    EXPECT_DOUBLE_EQ( 3.0, destination[0].z() );
    EXPECT_DOUBLE_EQ( 4.0, destination[1].x() );
    EXPECT_DOUBLE_EQ( 6.0, destination[1].z() );
    EXPECT_TRUE( messages.m_messages.empty() );
}
