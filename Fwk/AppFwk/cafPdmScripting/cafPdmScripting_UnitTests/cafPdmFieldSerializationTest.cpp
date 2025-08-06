//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "gtest/gtest.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmScriptIOMessages.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmFieldSerialization, Strings )
{
    QString     source = "This is a string";
    QTextStream stream( &source );
    QString     destination;

    caf::PdmScriptIOMessages messages;
    bool                     stringsAreQuoted = false;

    caf::PdmFieldScriptingCapabilityIOHandler<QString>::writeToField( destination, stream, &messages, stringsAreQuoted );
    EXPECT_STREQ( source.toStdString().data(), destination.toStdString().data() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmFieldSerialization, StringList )
{
    // Items in a string list is not quoted when coming from Python
    QString              source = "[Here, are, four, strings]";
    QTextStream          stream( &source );
    std::vector<QString> destination;

    caf::PdmScriptIOMessages messages;
    bool                     stringsAreQuoted = false;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::writeToField( destination,
                                                                                   stream,
                                                                                   &messages,
                                                                                   stringsAreQuoted );

    EXPECT_EQ( size_t( 4 ), destination.size() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmFieldSerialization, StringListQuoted )
{
    // Items in a string list are quoted when coming from command file
    QString source = R"(["B-2H", "B-4H"])";

    QTextStream          stream( &source );
    std::vector<QString> destination;

    caf::PdmScriptIOMessages messages;
    bool                     stringsAreQuoted = true;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<QString>>::writeToField( destination,
                                                                                   stream,
                                                                                   &messages,
                                                                                   stringsAreQuoted );

    EXPECT_EQ( (size_t)2, destination.size() );
    EXPECT_STREQ( "B-2H", destination[0].toStdString().c_str() );
    EXPECT_STREQ( "B-4H", destination[1].toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmFieldSerialization, ValueList )
{
    std::vector<float> floatValues = { 1.5, 0.0001, 1.77e10 };

    QString     text;
    QTextStream stream( &text );

    caf::PdmScriptIOMessages messages;
    bool                     stringsAreQuoted = true;

    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<float>>::readFromField( floatValues,
                                                                                  stream,
                                                                                  &messages,
                                                                                  stringsAreQuoted );

    const QString expected = "[1.5, 0.0001, 1.77e+10]";
    EXPECT_STREQ( expected.toStdString().c_str(), text.toStdString().c_str() );

    std::vector<float> result;
    caf::PdmFieldScriptingCapabilityIOHandler<std::vector<float>>::writeToField( result, stream, &messages, stringsAreQuoted );

    EXPECT_EQ( floatValues.size(), result.size() );
    for ( size_t i = 0; i < floatValues.size(); ++i )
    {
        EXPECT_FLOAT_EQ( floatValues[i], result[i] );
    }
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmFieldSerialization, OptionalValues )
{
    {
        std::optional<QString> optionalString = std::nullopt;
        QString                text;
        QTextStream            stream( &text );

        caf::PdmScriptIOMessages messages;
        bool                     stringsAreQuoted = true;

        caf::PdmFieldScriptingCapabilityIOHandler<std::optional<QString>>::readFromField( optionalString,
                                                                                          stream,
                                                                                          &messages,
                                                                                          stringsAreQuoted );

        const QString expected = "\"\"";
        EXPECT_STREQ( expected.toStdString().c_str(), text.toStdString().c_str() );

        std::optional<QString> result;
        caf::PdmFieldScriptingCapabilityIOHandler<std::optional<QString>>::writeToField( result,
                                                                                         stream,
                                                                                         &messages,
                                                                                         stringsAreQuoted );

        EXPECT_FALSE( result.has_value() );
    }

    {
        QString                sourceText     = "Test string with spaces";
        std::optional<QString> optionalString = sourceText;

        QString     text;
        QTextStream stream( &text );

        caf::PdmScriptIOMessages messages;
        bool                     stringsAreQuoted = true;

        caf::PdmFieldScriptingCapabilityIOHandler<std::optional<QString>>::readFromField( optionalString,
                                                                                          stream,
                                                                                          &messages,
                                                                                          stringsAreQuoted );

        const QString expected = "\"Test string with spaces\"";
        EXPECT_STREQ( expected.toStdString().c_str(), text.toStdString().c_str() );

        std::optional<QString> result;
        caf::PdmFieldScriptingCapabilityIOHandler<std::optional<QString>>::writeToField( result,
                                                                                         stream,
                                                                                         &messages,
                                                                                         stringsAreQuoted );

        EXPECT_STREQ( sourceText.toStdString().c_str(), result.value().toStdString().c_str() );
    }
}
