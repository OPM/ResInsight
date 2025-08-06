//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2013 Ceetron Solutions AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <utility>

class AggregatedTypes : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    AggregatedTypes()
    {
        CAF_PDM_InitObject( "SimpleObj" );

        auto pair = std::make_pair( false, 12.0 );
        CAF_PDM_InitField( &m_checkableDouble, "CheckableDouble", pair, "label text" );

        auto strPair = std::make_pair( false, QString( "msj" ) );
        CAF_PDM_InitField( &m_checkableString, "CheckableString", strPair, "label text" );

        CAF_PDM_InitFieldNoDefault( &m_optionalDouble, "OptionalDouble", "Optional Double" );
    }

    ~AggregatedTypes() {}

    caf::PdmField<std::pair<bool, double>>  m_checkableDouble;
    caf::PdmField<std::pair<bool, QString>> m_checkableString;

    caf::PdmField<std::optional<double>> m_optionalDouble;
};
CAF_PDM_SOURCE_INIT( AggregatedTypes, "AggregatedTypes" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AggregatedTypes, AggregatedTypes )
{
    AggregatedTypes obj;
    EXPECT_EQ( obj.m_checkableDouble().first, false );
    EXPECT_EQ( obj.m_checkableDouble().second, 12.0 );
    EXPECT_EQ( obj.m_optionalDouble(), std::nullopt );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( AggregatedTypeTest, MyTest )
{
    QString          xml;
    QXmlStreamWriter xmlStream( &xml );
    xmlStream.setAutoFormatting( true );

    const double  testValue         = 0.0012;
    const QString testString        = "string with spaces";
    const double  testOptionalValue = 123.0012;

    {
        AggregatedTypes myObj;

        auto testPair           = std::make_pair( true, testValue );
        myObj.m_checkableDouble = testPair;

        auto testStrPair        = std::make_pair( true, testString );
        myObj.m_checkableString = testStrPair;

        myObj.m_optionalDouble = testOptionalValue;

        xml = myObj.writeObjectToXmlString();
    }

    {
        AggregatedTypes myObj;

        myObj.readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );

        auto fieldValue = myObj.m_checkableDouble();
        ASSERT_TRUE( fieldValue.first );
        ASSERT_DOUBLE_EQ( testValue, fieldValue.second );

        auto fieldStrValue = myObj.m_checkableString();
        ASSERT_STREQ( testString.toStdString().data(), fieldStrValue.second.toStdString().data() );

        auto fieldOptionalValue = myObj.m_optionalDouble();
        ASSERT_TRUE( fieldOptionalValue.has_value() );
        ASSERT_DOUBLE_EQ( testOptionalValue, fieldOptionalValue.value() );
    }
}
