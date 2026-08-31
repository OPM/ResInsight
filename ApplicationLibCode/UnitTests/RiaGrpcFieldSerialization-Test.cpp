#include "gtest/gtest.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmScriptIOMessages.h"

#include <QString>
#include <QTextStream>

#include <vector>

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
    QString source = R"(["Coal,Calcite", Channel])";

    QTextStream          stream( &source );
    std::vector<QString> destination;

    caf::PdmScriptIOMessages messages;
    const bool               stringsAreQuoted = false;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::writeToField( destination, stream, &messages, stringsAreQuoted );

    ASSERT_EQ( size_t( 2 ), destination.size() );
    EXPECT_STREQ( "Coal,Calcite", destination[0].toStdString().c_str() );
    EXPECT_STREQ( "Channel", destination[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
/// https://github.com/OPM/ResInsight/issues/14648
///
/// Quoted strings containing a comma must be kept as one item
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcFieldSerialization, StringListQuotedWithCommaInString )
{
    QString source = R"(["Coal,Calcite", "Channel"])";

    QTextStream          stream( &source );
    std::vector<QString> destination;

    caf::PdmScriptIOMessages messages;
    const bool               stringsAreQuoted = true;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::writeToField( destination, stream, &messages, stringsAreQuoted );

    ASSERT_EQ( size_t( 2 ), destination.size() );
    EXPECT_STREQ( "Coal,Calcite", destination[0].toStdString().c_str() );
    EXPECT_STREQ( "Channel", destination[1].toStdString().c_str() );
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
