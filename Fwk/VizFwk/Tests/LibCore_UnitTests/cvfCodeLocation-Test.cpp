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
#include "cvfCodeLocation.h"

#include "gtest/gtest.h"
#include "cvfTrace.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CodeLocationTest, DefaultConstructor)
{
    CodeLocation cl;

    EXPECT_STREQ("", cl.fileName());
    EXPECT_STREQ("", cl.functionName());
    EXPECT_EQ(-1, cl.lineNumber());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CodeLocationTest, ConstructionAndQuery)
{
    const char fileName[] = "c:/myDir/myFile.txt";
    const char funcName[] = "MyClass::myFunc";
    const int lineNumber = 123;
    
    CodeLocation cl(fileName, funcName, lineNumber);
    EXPECT_STREQ(fileName, cl.fileName());
    EXPECT_STREQ(funcName, cl.functionName());
    EXPECT_EQ(lineNumber, cl.lineNumber());

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CodeLocationTest, ConstructionUsingMacro)
{
    {
        CodeLocation cl = CVF_CODE_LOCATION;
        EXPECT_STRCASEEQ("cvfCodeLocation-Test.cpp", cl.shortFileName());
//         cvf::Trace::show("file: %s", cl.fileName());
//         cvf::Trace::show("func: %s", cl.functionName());
//         cvf::Trace::show("line: %d", cl.lineNumber());
    }

    {
        CodeLocation cl(__FILE__, CVF_CODELOC_FUNCNAME, __LINE__);
        EXPECT_STRCASEEQ("cvfCodeLocation-Test.cpp", cl.shortFileName());
//         cvf::Trace::show("file: %s", cl.fileName());
//         cvf::Trace::show("func: %s", cl.functionName());
//         cvf::Trace::show("line: %d", cl.lineNumber());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(CodeLocationTest, shortFileName)
{
    {
        const char fileName[] = "c:/myDir/myFile.txt";
        CodeLocation cl(fileName, NULL, 0);
        EXPECT_STREQ("myFile.txt", cl.shortFileName());
    }

    {
        const char fileName[] = "/myDir/myFile.txt";
        CodeLocation cl(fileName, NULL, 0);
        EXPECT_STREQ("myFile.txt", cl.shortFileName());
    }

#ifdef WIN32
    {
        const char fileName[] = "c:\\myDir\\myFile.txt";
        CodeLocation cl(fileName, NULL, 0);
        EXPECT_STREQ("myFile.txt", cl.shortFileName());
    }

    {
        const char fileName[] = "c:\\myDir/myFile.txt";
        CodeLocation cl(fileName, NULL, 0);
        EXPECT_STREQ("myFile.txt", cl.shortFileName());
    }
#endif
}


