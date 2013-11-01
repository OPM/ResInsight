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
#include "cvfLogEvent.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LogEventTest, DefaultConstruction)
{
    LogEvent le;

    EXPECT_STREQ(L"", le.source().c_str());
    EXPECT_STREQ(L"", le.message().c_str());
    EXPECT_EQ(Logger::LL_ERROR, le.level());

    EXPECT_STREQ("", le.location().fileName());
    EXPECT_STREQ("", le.location().functionName());
    EXPECT_EQ(-1, le.location().lineNumber());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LogEventTest, NormalConstruction)
{
    const wchar_t* src = L"mySourceLogger";
    const wchar_t* msg = L"myMessage";
    const Logger::Level level = Logger::LL_WARNING;
    const char* file = "theFile.cpp";
    const char* func = "theClass::theFunction()";
    const int line  = 123;

    LogEvent le(src, msg, level, CodeLocation(file, func, line));
    
    EXPECT_STREQ(src, le.source().c_str());
    EXPECT_STREQ(msg, le.message().c_str());
    EXPECT_EQ(level, le.level());
    EXPECT_STREQ(file, le.location().fileName());
    EXPECT_STREQ(func, le.location().functionName());
    EXPECT_EQ(line, le.location().lineNumber());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LogEventTest, CopyConstructionAndAssignment)
{
    const wchar_t* src = L"mySourceLogger";
    const wchar_t* msg = L"myMessage";
    const Logger::Level level = Logger::LL_WARNING;
    const char* file = "theFile.cpp";
    const char* func = "theClass::theFunction()";
    const int line  = 123;

    LogEvent le0(src, msg, level, CodeLocation(file, func, line));

    {
        LogEvent le(le0);

        EXPECT_STREQ(src, le.source().c_str());
        EXPECT_STREQ(msg, le.message().c_str());
        EXPECT_EQ(level, le.level());
        EXPECT_STREQ(file, le.location().fileName());
        EXPECT_STREQ(func, le.location().functionName());
        EXPECT_EQ(line, le.location().lineNumber());
    }

    {
        LogEvent le;
        le = le0;

        EXPECT_STREQ(src, le.source().c_str());
        EXPECT_STREQ(msg, le.message().c_str());
        EXPECT_EQ(level, le.level());
        EXPECT_STREQ(file, le.location().fileName());
        EXPECT_STREQ(func, le.location().functionName());
        EXPECT_EQ(line, le.location().lineNumber());
    }
}


