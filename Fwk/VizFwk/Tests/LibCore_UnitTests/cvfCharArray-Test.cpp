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
#include "cvfCharArray.h"
#include "cvfSystem.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, Construction)
{
    CharArray ca0;
    ASSERT_EQ(0, ca0.size());

    CharArray ca1(5, 'a');
    ASSERT_EQ(5, ca1.size());
    EXPECT_EQ('a', ca1[0]);
    EXPECT_EQ('a', ca1[4]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, CopyConstruction)
{
    CharArray ca0(5, 'a');
    ASSERT_EQ(5, ca0.size());
    EXPECT_EQ('a', ca0[0]);
    EXPECT_EQ('a', ca0[4]);

    {
        CharArray ca(ca0);
        ASSERT_EQ(5, ca.size());
        EXPECT_EQ('a', ca[0]);
        EXPECT_EQ('a', ca[4]);

        EXPECT_NE(ca0.ptr(), ca.ptr());

        ca[0] = 'b';
        EXPECT_EQ('a', ca0[0]);
        EXPECT_EQ('b', ca[0]);
    }

    {
        CharArray ca;
        ca = ca0;
        ASSERT_EQ(5, ca.size());
        EXPECT_EQ('a', ca[0]);
        EXPECT_EQ('a', ca[4]);

        EXPECT_NE(ca0.ptr(), ca.ptr());

        ca[0] = 'b';
        EXPECT_EQ('a', ca0[0]);
        EXPECT_EQ('b', ca[0]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, ConstructFromCharPtr)
{
    char myStr[] = "TestString";

    CharArray ca(myStr);
    ASSERT_EQ(10, ca.size());

    EXPECT_EQ('T', ca[0]);
    EXPECT_EQ('g', ca[9]);

    myStr[0] = 'X';
    EXPECT_EQ('T', ca[0]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, Resize)
{
    CharArray ca;
    ASSERT_EQ(0, ca.size());

    ca.resize(4);
    ASSERT_EQ(4, ca.size());

    ca.resize(0);
    ASSERT_EQ(0, ca.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, Accessors)
{
    CharArray ca(4, 'a');

    EXPECT_EQ('a', ca[0]);
    EXPECT_EQ('a', ca[3]);

    ca[0] = '0';
    ca[1] = '1';
    ca[2] = '2';
    ca[3] = '3';

    EXPECT_EQ('0', ca[0]);
    EXPECT_EQ('1', ca[1]);
    EXPECT_EQ('2', ca[2]);
    EXPECT_EQ('3', ca[3]);
}


#if CVF_ENABLE_TIGHT_ASSERTS == 1
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayDeathTest, IllegalAccessors)
{
    CharArray ca;
    EXPECT_DEATH(ca[0], "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, GetCharPtr)
{
    CharArray ca(4, 'a');
    EXPECT_EQ(4, ca.size());

    const char* cptr = ca.ptr();
    EXPECT_EQ(4, System::strlen(cptr));

    ca[2] = '\0';
    EXPECT_EQ(4, ca.size());
    EXPECT_EQ(2, System::strlen(cptr));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CharArrayTest, UnsignedVsSignedChar)
{
    {
        char ch  = '\xc6';
        unsigned char uch = (unsigned char)ch;

        EXPECT_EQ(0, memcmp(&ch, &uch, 1));
    }

    {
        CharArray a;
        unsigned char vUC = 0xc6; // 'Æ'
        signed char   vSC = (-256  + 0xc6);
        char          vC  = '\xc6';

        a.push_back(vUC);
        a.push_back(vSC);
        a.push_back(vC);

        EXPECT_EQ(a[0], a[1]);
        EXPECT_EQ(a[0], a[2]);
    }

    {
        CharArray a;

        unsigned int val = 198;  // 'Æ'

        a.push_back(static_cast<unsigned char>(val));
        a.push_back(static_cast<char>(val));
        a.push_back(static_cast<signed char>(val));

        EXPECT_EQ(a[0], a[1]);
        EXPECT_EQ(a[0], a[2]);
   }
    
    {
        CharArray a;

        unsigned int val = 198; // 'Æ'

        a.push_back((unsigned char)val);
        a.push_back((char)val);
        a.push_back((signed char)val);

        EXPECT_EQ(a[0], a[1]);
        EXPECT_EQ(a[0], a[2]);
    }
}
