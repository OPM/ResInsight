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
#include "cvfFlags.h"

#include "gtest/gtest.h"

using namespace cvf;


enum MyColor
{
    MyNoColor = 0x0000,
    MyRed = 0x0001,
    MyBlue = 0x0002,
    MyGreen = 0x004
};

typedef Flags<MyColor> MyColors;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, Constructors)
{
    // Default ctor
    MyColors clrs0;
    EXPECT_TRUE(clrs0 == MyNoColor);

    // Constructor with initialization
    MyColors clrs1(MyRed);
    EXPECT_TRUE(clrs1 == MyRed);

    // Copy constructor
    MyColors clrs2(clrs1);
    EXPECT_TRUE(clrs2 == MyRed);

    MyColors clrs3 = clrs1;
    EXPECT_TRUE(clrs3 == MyRed);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, AssignmentOperators)
{
    // Assignment from enum
    MyColors clrs1;
    clrs1 = MyRed;
    EXPECT_TRUE(clrs1 == MyRed);

    // Assignment from other object
    MyColors clrs2;
    clrs2 = clrs1;
    EXPECT_TRUE(clrs1 == MyRed);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, Comparison)
{
    MyColors clrs1(MyRed);
    MyColors clrs2(MyRed);
    MyColors clrs3(MyBlue);
    MyColors clrs4(MyBlue);
    
    EXPECT_TRUE(clrs1 == MyRed);
    EXPECT_TRUE(clrs1 != MyBlue);
    EXPECT_TRUE(clrs1 == clrs2);

    EXPECT_TRUE(clrs3 == MyBlue);
    EXPECT_TRUE(clrs3 != MyRed);
    EXPECT_TRUE(clrs3 == clrs4);

    EXPECT_TRUE(clrs1 != clrs3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, TestForFlag)
{
    MyColors clrs1(MyRed);
    EXPECT_TRUE(clrs1.testFlag(MyRed));
    EXPECT_FALSE(clrs1.testFlag(MyBlue));

    MyColors clrs2(MyBlue);
    EXPECT_TRUE(clrs2.testFlag(MyBlue));
    EXPECT_FALSE(clrs2.testFlag(MyGreen));

    MyColors clrs3;
    clrs3 |= MyRed;
    clrs3 |= MyGreen;
    EXPECT_TRUE(clrs3.testFlag(MyRed));
    EXPECT_TRUE(clrs3.testFlag(MyGreen));
    EXPECT_FALSE(clrs3.testFlag(MyBlue));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, BitwiseOrAssigment)
{
    MyColors clrs1;
    EXPECT_TRUE(clrs1 == MyNoColor);

    clrs1 |= MyRed;
    EXPECT_TRUE(clrs1 == MyRed);

    clrs1 |= MyBlue;
    EXPECT_TRUE(clrs1 == (MyRed | MyBlue));

    clrs1 |= MyNoColor;
    EXPECT_TRUE(clrs1 == (MyRed | MyBlue));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, BitwiseOr)
{
    MyColors clrs1(MyRed);
    MyColors clrs2(MyBlue);

    MyColors clrs3 = clrs1 | clrs2;
    EXPECT_TRUE(clrs3 == (MyRed | MyBlue));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, BitwiseAndAssigment)
{
    MyColors clrs1(MyRed);
    clrs1 |= MyBlue;
    EXPECT_TRUE(clrs1 == (MyRed | MyBlue));

    clrs1 &= MyRed;
    EXPECT_TRUE(clrs1 == MyRed);

    clrs1 &= MyNoColor;
    EXPECT_TRUE(clrs1 == MyNoColor);
    EXPECT_TRUE(clrs1 == 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(FlagsTest, BitwiseAnd)
{
    MyColors clrs1(MyRed);
    MyColors clrs2(MyBlue);

    MyColors clrs3 = clrs1 | clrs2;
    EXPECT_TRUE(clrs3 == (MyRed | MyBlue));

    clrs3 = clrs3 & MyRed;
    EXPECT_TRUE(clrs3 == MyRed);

    clrs3 = clrs3 & MyBlue;
    EXPECT_TRUE(clrs3 == 0);
}


