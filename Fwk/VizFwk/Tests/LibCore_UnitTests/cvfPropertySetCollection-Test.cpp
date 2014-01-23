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
#include "cvfPropertySetCollection.h"

#include "gtest/gtest.h"
#include <iostream>

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetCollectionTest, Constructor)
{
    ref<PropertySetCollection> psc = new PropertySetCollection;
    ASSERT_EQ(0, psc->count());
    ASSERT_EQ(0, psc->countOfType("dummy"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetCollectionTest, AddAndRetrieve)
{
    ref<PropertySetCollection> psc = new PropertySetCollection;

    psc->addPropertySet(new PropertySet("ctA"));
    psc->addPropertySet(new PropertySet("ctB"));
    ASSERT_EQ(2, psc->count());

    PropertySet* ps1 = psc->propertySet(0);
    PropertySet* ps2 = psc->propertySet(1);
    EXPECT_TRUE(ps1->classType() == "ctA");
    EXPECT_TRUE(ps2->classType() == "ctB");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetCollectionTest, GetPropertySetOfSpecificClass)
{
    ref<PropertySetCollection> psc = new PropertySetCollection;

    {
        PropertySet* ps1 = new PropertySet("ctA");
        PropertySet* ps2 = new PropertySet("ctB");
        PropertySet* ps3 = new PropertySet("ctA");
        ps1->setValue("name", "ps1");
        ps2->setValue("name", "ps2");
        ps3->setValue("name", "ps3");
        psc->addPropertySet(ps1);
        psc->addPropertySet(ps2);
        psc->addPropertySet(ps3);
    }

    ASSERT_EQ(3, psc->count());
    ASSERT_EQ(2, psc->countOfType("ctA"));
    ASSERT_EQ(1, psc->countOfType("ctB"));

    const PropertySet* ps1 = psc->propertySetOfType("ctA", 0);
    const PropertySet* ps2 = psc->propertySetOfType("ctB", 0);
    const PropertySet* ps3 = psc->propertySetOfType("ctA", 1);
    EXPECT_TRUE(ps1->value("name").getString() == "ps1");
    EXPECT_TRUE(ps2->value("name").getString() == "ps2");
    EXPECT_TRUE(ps3->value("name").getString() == "ps3");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PropertySetCollectionTest, FirstPropertySetOfType)
{
    ref<PropertySetCollection> psc = new PropertySetCollection;

    cvf::ref<PropertySet> ps1 = new PropertySet("ctA");
    cvf::ref<PropertySet> ps2 = new PropertySet("ctB");
    cvf::ref<PropertySet> ps3 = new PropertySet("ctA");
    psc->addPropertySet(ps1.p());
    psc->addPropertySet(ps2.p());
    psc->addPropertySet(ps3.p());
    ASSERT_EQ(3, psc->count());

    const PropertySet* psA = psc->firstPropertySetOfType("ctA");
    const PropertySet* psB = psc->firstPropertySetOfType("ctB");
    const PropertySet* psZ = psc->firstPropertySetOfType("ctZ");
    EXPECT_EQ(psA, ps1.p());
    EXPECT_EQ(psB, ps2.p());
    EXPECT_TRUE(psZ == NULL);
}


