//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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


#include "cvfBase.h"
#include "cvfPropertySet.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetTest, Constructor)
{
    PropertySet ps("mySet");
    ASSERT_TRUE(ps.classType() == "mySet");
    ASSERT_TRUE(ps.isEmpty());
    ASSERT_FALSE(ps.contains("dummy"));

    Variant v = ps.value("dummy2");
    ASSERT_EQ(Variant::INVALID, v.type());
    ASSERT_FALSE(v.isValid());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetTest, SimpleSet)
{
    PropertySet ps("mySet");
    ASSERT_TRUE(ps.isEmpty());

    ps.setValue("key_int", Variant(static_cast<int>(123)));
    ps.setValue("key_double", Variant(1.234));
    ps.setValue("key_string", Variant("myString"));

    EXPECT_FALSE(ps.isEmpty());
    EXPECT_TRUE(ps.contains("key_int"));
    EXPECT_TRUE(ps.contains("key_double"));
    EXPECT_TRUE(ps.contains("key_string"));

    {
        Variant v = ps.value("key_int");
        ASSERT_EQ(Variant::INT, v.type());
        ASSERT_EQ(123, v.getInt());
    }
    {
        Variant v = ps.value("key_double");
        ASSERT_EQ(Variant::DOUBLE, v.type());
        ASSERT_EQ(1.234, v.getDouble());
    }
    {
        Variant v = ps.value("key_string");
        ASSERT_EQ(Variant::STRING, v.type());
        ASSERT_TRUE(v.getString() == "myString");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetTest, EqualsOperator)
{
    ref<PropertySet> ps0 = new PropertySet("propSet");
    ps0->setValue("myInt", 123);
    ps0->setValue("myDouble", 1.234);
    ASSERT_TRUE(*ps0 == *ps0);

    {
        ref<PropertySet> ps = new PropertySet("propSet");
        ps->setValue("myDouble", 1.234);
        ps->setValue("myInt", 123);
        ASSERT_TRUE(*ps == *ps0);
    }
    {
        ref<PropertySet> ps = new PropertySet("propSet");
        ps->setValue("myInt", 999);
        ps->setValue("myDouble", 1.234);
        ASSERT_FALSE(*ps == *ps0);
    }
    {
        ref<PropertySet> ps = new PropertySet("propSet");
        ps->setValue("myIntXX", 123);
        ps->setValue("myDouble", 1.234);
        ASSERT_FALSE(*ps == *ps0);
    }
    {
        ref<PropertySet> ps = new PropertySet("propSet");
        ps->setValue("myInt", 123);
        ps->setValue("myInt_2", 456);
        ps->setValue("myDouble", 1.234);
        ASSERT_FALSE(*ps == *ps0);
    }
    {
        ref<PropertySet> ps = new PropertySet("ABC");
        ps->setValue("myInt", 123);
        ps->setValue("myDouble", 1.234);
        ASSERT_FALSE(*ps == *ps0);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetTest, ReplaceKey)
{
    PropertySet ps("mySet");
    ASSERT_TRUE(ps.isEmpty());

    ps.setValue("key1", Variant(1234));
    ps.setValue("key2", Variant(1.234));

    {
        Variant v1 = ps.value("key1");
        Variant v2 = ps.value("key2");
        Variant v3 = ps.value("key3");
        ASSERT_EQ(Variant::INT, v1.type());
        ASSERT_EQ(Variant::DOUBLE, v2.type());
        ASSERT_EQ(Variant::INVALID, v3.type());
        ASSERT_EQ(1234, v1.getInt());
        ASSERT_EQ(1.234, v2.getDouble());
    }

    ps.setValue("key3", Variant("myString"));
    ps.setValue("key2", Variant(true));

    {
        Variant v1 = ps.value("key1");
        Variant v2 = ps.value("key2");
        Variant v3 = ps.value("key3");
        ASSERT_EQ(Variant::INT, v1.type());
        ASSERT_EQ(Variant::BOOL, v2.type());
        ASSERT_EQ(Variant::STRING, v3.type());
        ASSERT_EQ(1234, v1.getInt());
        ASSERT_EQ(true, v2.getBool());
        ASSERT_TRUE(v3.getString() == "myString");
    }

    ps.setValue("key1", Variant());

    {
        Variant v1 = ps.value("key1");
        Variant v2 = ps.value("key2");
        Variant v3 = ps.value("key3");
        ASSERT_EQ(Variant::INVALID, v1.type());
        ASSERT_EQ(Variant::BOOL, v2.type());
        ASSERT_EQ(Variant::STRING, v3.type());
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetTest, AllKeysAndAllValues)
{
    PropertySet ps("mySet");
    ps.setValue("key1", Variant(1234));
    ps.setValue("key2", Variant(1.234));
    ps.setValue("key3", Variant(true));

    std::vector<String> allKeys = ps.allKeys();
    std::vector<Variant> allValues = ps.allValues();

    ASSERT_EQ(3, allKeys.size());
    ASSERT_EQ(3, allValues.size());

    bool saw1 = false;
    bool saw2 = false;
    bool saw3 = false;
    for (size_t i = 0; i < 3; i++)
    {
        const String& s = allKeys[i];
        const Variant& v = allValues[i];

        if (s == "key1")
        {
            EXPECT_EQ(1234, v.getInt());
            saw1 = true;
        }
        else if (s == "key2")
        {
            EXPECT_EQ(1.234, v.getDouble());
            saw2 = true;
        }
        else if (s == "key3")
        {
            EXPECT_EQ(true, v.getBool());
            saw3 = true;
        }
    }

    EXPECT_TRUE(saw1);
    EXPECT_TRUE(saw2);
    EXPECT_TRUE(saw3);
}







