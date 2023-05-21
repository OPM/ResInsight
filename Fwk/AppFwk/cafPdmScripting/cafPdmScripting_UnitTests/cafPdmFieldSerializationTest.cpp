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
