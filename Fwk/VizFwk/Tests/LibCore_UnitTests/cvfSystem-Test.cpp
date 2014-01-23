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
#include "cvfSystem.h"
#include "cvfString.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, memcpy)
{
    const char src[] = "Test123";

    {
        char dst[10];
        memset(dst, 'X', 10);
        ASSERT_EQ('X', dst[0]);
        ASSERT_EQ('X', dst[9]);

        // SHould be OK to copy 0 bytes
        EXPECT_TRUE(System::memcpy(dst, 10, src, 0));

        EXPECT_TRUE(System::memcpy(dst, 10, src, 8));
        EXPECT_STREQ("Test123", dst);
        EXPECT_EQ(0,   dst[7]);
        EXPECT_EQ('X', dst[8]);
        EXPECT_EQ('X', dst[9]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, strcpy)
{
    const char src[] = "Test123";
    
    {
        char dst[10];
        memset(dst, 'X', 10);

        EXPECT_TRUE(System::strcpy(dst, 10, src));
        EXPECT_STREQ("Test123", dst);
        EXPECT_EQ(0,  dst[7]);
    }

    {
        char dst[8];
        memset(dst, 'X', 8);

        EXPECT_TRUE(System::strcpy(dst, 8, src));
        EXPECT_STREQ("Test123", dst);
        EXPECT_EQ(0,  dst[7]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, strcat)
{
    const char str1[] = "Test1";
    const char str2[] = "Str2";

    {
        char str[6];
        str[0] = '\0';
        ASSERT_TRUE(System::strcat(str, sizeof(str), str1));
        ASSERT_STREQ("Test1", str);
    }

    {
        char str[20];
        str[0] = '\0';
        ASSERT_TRUE(System::strcat(str, sizeof(str), str1));
        ASSERT_TRUE(System::strcat(str, sizeof(str), str2));
        ASSERT_STREQ("Test1Str2", str);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, sprintf)
{
    {
        char buf[100];
        int numWritten = System::sprintf(buf, 100, "%d %s %.2f", 999, "ABC", 1.23f);
        EXPECT_EQ(12, numWritten);
        EXPECT_STREQ("999 ABC 1.23", buf);
    }

    {
        char buf[5];
        int numWritten = System::sprintf(buf, 5, "%s", "ABCD");
        EXPECT_EQ(4, numWritten);
        EXPECT_STREQ("ABCD", buf);
    }

    {
        char buf[5];
        memset(buf, 'X', 5);

        int numWritten = System::sprintf(buf, 4, "%s", "ABC");
        EXPECT_EQ(3, numWritten);
        EXPECT_EQ('C',  buf[2]);
        EXPECT_EQ('\0', buf[3]);
        EXPECT_EQ('X',  buf[4]);
    }

    {
        char buf[4];
        int numWritten = System::sprintf(buf, 4, "%s", "ABCD");
        EXPECT_EQ(-1, numWritten);
        EXPECT_STREQ("ABC", buf);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, swprintf)
{
    {
        wchar_t buf[100];
        int numWritten = System::swprintf(buf, 100, L"%d %ls %.2f", 999, L"ABC", 1.23f);
        EXPECT_EQ(12, numWritten);

        /// Expected string "999 ABC 1.23"
        EXPECT_EQ(L'9',   buf[0]);
        EXPECT_EQ(L'A',   buf[4]);
        EXPECT_EQ(L'.',   buf[9]);
        EXPECT_EQ(L'3',   buf[11]);
        EXPECT_EQ(L'\0',  buf[12]);
    }

    {
        wchar_t buf[5];
        int numWritten = System::swprintf(buf, 5, L"%ls", L"ABCD");
        EXPECT_EQ(4, numWritten);
        EXPECT_EQ(L'A',   buf[0]);
        EXPECT_EQ(L'B',   buf[1]);
        EXPECT_EQ(L'C',   buf[2]);
        EXPECT_EQ(L'D',   buf[3]);
        EXPECT_EQ(L'\0',  buf[4]);
    }

    {
        wchar_t buf[5];
        buf[0] = L'X';
        buf[1] = L'X';
        buf[2] = L'X';
        buf[3] = L'X';
        buf[4] = L'X';

        int numWritten = System::swprintf(buf, 4, L"%ls", L"ABC");
        EXPECT_EQ(3, numWritten);
        EXPECT_EQ(L'C',  buf[2]);
        EXPECT_EQ(L'\0', buf[3]);
        EXPECT_EQ(L'X',  buf[4]);
    }

    {
        wchar_t buf[4];
        int numWritten = System::swprintf(buf, 4, L"%ls", L"ABCD");
        EXPECT_EQ(-1, numWritten);
        EXPECT_EQ(L'A',   buf[0]);
        EXPECT_EQ(L'B',   buf[1]);
        EXPECT_EQ(L'C',   buf[2]);
        EXPECT_EQ(L'\0',  buf[3]);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, strlen)
{
    char str0[] = "";
    char str1[] = "Test123";

    EXPECT_EQ(0, System::strlen(str0));
    EXPECT_EQ(7, System::strlen(str1));

    EXPECT_EQ(0, System::strlen(NULL));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SystemTest, strcmp)
{
    char str0[] = "";
    char str1[] = "Test1";
    char str2[] = "Test2";

    EXPECT_EQ(0, System::strcmp(str0, ""));
    EXPECT_EQ(0, System::strcmp(str0, str0));
    EXPECT_EQ(0, System::strcmp(str1, "Test1"));
    EXPECT_EQ(0, System::strcmp(str1, str1));
    EXPECT_EQ(0, System::strcmp(NULL, NULL));

    EXPECT_GT(System::strcmp(str1, str0), 0);
    EXPECT_LT(System::strcmp(str1, str2), 0);
    EXPECT_GT(System::strcmp(str0, NULL), 0);
    EXPECT_GT(System::strcmp(str1, NULL), 0);
    EXPECT_GT(System::strcmp(str2, NULL), 0);
}


