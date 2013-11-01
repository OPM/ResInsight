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
#include "cvfString.h"
#include "cvfSystem.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Construction)
{
    {
        const char* nullStr = NULL;
        String str(nullStr);
        ASSERT_EQ(0, str.size());

        const wchar_t* nullStrW = NULL;
        String strW(nullStrW);
        ASSERT_EQ(0, strW.size());
    }

    {
        const char* emptyStr = "";
        String str(emptyStr);
        ASSERT_EQ(0, str.size());

        const wchar_t* emptyStrW = L"";
        String strW(emptyStrW);
        ASSERT_EQ(0, strW.size());
    }

    {
        String str1("Test");
        ASSERT_EQ(4, str1.size());

        EXPECT_EQ('T', str1[0]);
        EXPECT_EQ('e', str1[1]);
        EXPECT_EQ('s', str1[2]);
        EXPECT_EQ('t', str1[3]);
    }

    {
        String str2 = "fIsKs";
        ASSERT_EQ(5, str2.size());
        EXPECT_EQ('f', str2[0]);
        EXPECT_EQ('I', str2[1]);
        EXPECT_EQ('s', str2[2]);
        EXPECT_EQ('K', str2[3]);
        EXPECT_EQ('s', str2[4]);
    }

    {
        String str3(L"Ab3");
        ASSERT_EQ(3, str3.size());
        EXPECT_EQ('A', str3[0]);
        EXPECT_EQ('b', str3[1]);
        EXPECT_EQ('3', str3[2]);
    }

    {
        std::string stds = "1 !# 3";
        String str4(stds);
        ASSERT_EQ('1', str4[0]);
        EXPECT_EQ(' ', str4[1]);
        EXPECT_EQ('!', str4[2]);
        EXPECT_EQ('#', str4[3]);
        EXPECT_EQ(' ', str4[4]);
        EXPECT_EQ('3', str4[5]);
    }

    {
        // Constructor from single char
        String c1('a');
        EXPECT_STREQ("a", c1.toAscii().ptr());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ConstructionFromNumbers)
{
    int       n1 = 123;
    int64     n2 = 12345789012345;
    float     n3 = 123.456f;
    double    n4 = 123.456;
    cvf::uint n5 = 4294967295;

    // Constructors from numbers
    String s1(n1);
    String s2(n2);
    String s3(n3);
    String s4(n4);
    String s5(n5);

    EXPECT_STREQ("123",             s1.toAscii().ptr());
    EXPECT_STREQ("12345789012345",  s2.toAscii().ptr());
    EXPECT_STREQ("123.456",         s3.toAscii().ptr());
    EXPECT_STREQ("123.456",         s4.toAscii().ptr());
    EXPECT_STREQ("4294967295",      s5.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, c_str)
{
    {
        const char* nullStr = NULL;
        String str(nullStr);
        ASSERT_STREQ(L"", str.c_str());

        const wchar_t* nullStrW = NULL;
        String strW(nullStrW);
        ASSERT_STREQ(L"", strW.c_str());
    }

    {
        const char* emptyStr = "";
        String str(emptyStr);
        ASSERT_STREQ(L"", str.c_str());

        const wchar_t* emptyStrW = L"";
        String strW(emptyStrW);
        ASSERT_STREQ(L"", strW.c_str());
    }

    {
        String str("A");
        ASSERT_STREQ(L"A", str.c_str());

        String strW(L"B");
        ASSERT_STREQ(L"B", strW.c_str());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Assignment)
{
    const String str0("Test");
    String str1;
    str1 = str0;
    ASSERT_EQ(4, str1.size());
    EXPECT_EQ('T', str1[0]);
    EXPECT_EQ('e', str1[1]);
    EXPECT_EQ('s', str1[2]);
    EXPECT_EQ('t', str1[3]);

    String str2;
    str2 = "fIsKs";
    ASSERT_EQ(5, str2.size());
    EXPECT_EQ('f', str2[0]);
    EXPECT_EQ('I', str2[1]);
    EXPECT_EQ('s', str2[2]);
    EXPECT_EQ('K', str2[3]);
    EXPECT_EQ('s', str2[4]);

    String str3;
    str3 = L"Ab3";
    ASSERT_EQ(3, str3.size());
    EXPECT_EQ('A', str3[0]);
    EXPECT_EQ('b', str3[1]);
    EXPECT_EQ('3', str3[2]);

    String str4;
    std::string stds = "1 !# 3";
    str4 = stds;
    ASSERT_EQ('1', str4[0]);
    EXPECT_EQ(' ', str4[1]);
    EXPECT_EQ('!', str4[2]);
    EXPECT_EQ('#', str4[3]);
    EXPECT_EQ(' ', str4[4]);
    EXPECT_EQ('3', str4[5]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Resize)
{
    String s1;
    s1.resize(3);
    EXPECT_EQ(3, s1.size());
    s1[0] = 'T';
    s1[1] = 'x';
    s1[2] = 't';

    EXPECT_TRUE(s1 == "Txt");

    s1[0] = 'F';
    s1[1] = 'r';
    s1[2] = 'v';

    EXPECT_TRUE(s1 == "Frv");

    // Inserting 0 will not modify the string length!
    s1[1] = '\0';
    EXPECT_EQ(3, s1.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(StringDeathTest, IllegalAccess)
{
    String s1;
    s1.resize(2);

    s1[0] = 'T';
    s1[1] = 'x';
    EXPECT_DEATH(s1[2] = 't', "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Compare)
{
    String s1("Testing");
    String s2 = L"Testing";
    String s3 = s1;

    EXPECT_TRUE(s1 == s2);
    EXPECT_TRUE(s1 == s3);
    EXPECT_TRUE(s2 == s3);
    EXPECT_FALSE(s1 != s3);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, LessThan)
{
    String s1("Testing");
    String s2("Texting");
 
    EXPECT_TRUE(s1 < s2);
    EXPECT_FALSE(s1 < s1);
    EXPECT_FALSE(s1 == s2);

    s1 = "B";
    s2 = "A";

    EXPECT_FALSE(s1 < s2);
    EXPECT_FALSE(s1 < s1);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, CaseConversion)
{
    const String s0;
    EXPECT_STREQ("", s0.toUpper().toAscii().ptr());
    EXPECT_STREQ("", s0.toLower().toAscii().ptr());

    const String s("MyTestString");

    const String expectUpper("MYTESTSTRING");
    const String expectLower("myteststring");
    
    String l = s.toLower();
    EXPECT_STREQ(expectLower.toAscii().ptr(), l.toAscii().ptr());

    String u = s.toUpper();
    EXPECT_STREQ(expectUpper.toAscii().ptr(), u.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, trimmedRight)
{
    {
        String s("");
        const String e("");
        s = s.trimmedRight();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t");
        const String e("");
        s = s.trimmedRight();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s("AA \t\n\v\f\r");
        const String e("AA");
        s = s.trimmedRight();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" ab c");
        const String e(" ab c");
        s = s.trimmedRight();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" ab c  ");
        const String e(" ab c");
        s = s.trimmedRight();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, trimmedLeft)
{
    {
        String s("");
        const String e("");
        s = s.trimmedLeft();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t");
        const String e("");
        s = s.trimmedLeft();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t\n\v\f\rAA");
        const String e("AA");
        s = s.trimmedLeft();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s("ab c ");
        const String e("ab c ");
        s = s.trimmedLeft();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" ab c  ");
        const String e("ab c  ");
        s = s.trimmedLeft();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, trimmed)
{
    {
        String s("");
        const String e("");
        s = s.trimmed();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t");
        const String e("");
        s = s.trimmed();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t\nAA\v\f\r");
        const String e("AA");
        s = s.trimmed();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" ab c  ");
        const String e("ab c");
        s = s.trimmed();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, simplified)
{
    {
        String s("");
        const String e("");
        s = s.simplified();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t");
        const String e("");
        s = s.simplified();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }

    {
        String s(" \t\n\v\f\r AA\v\f\rBB\n");
        const String e("AA BB");
        s = s.simplified();
        EXPECT_STREQ(e.toAscii().ptr(), s.toAscii().ptr());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, toAscii)
{

    String s1("Testing");

    char pszString[20];
    System::strcpy(pszString, 20, s1.toAscii().ptr());

    EXPECT_EQ('T', pszString[0]);
    EXPECT_EQ('e', pszString[1]);
    EXPECT_EQ('s', pszString[2]);
    EXPECT_EQ('t', pszString[3]);
    EXPECT_EQ('i', pszString[4]);
    EXPECT_EQ('n', pszString[5]);
    EXPECT_EQ('g', pszString[6]);
    EXPECT_EQ( 0 , pszString[7]);

    //Will fail!
    //const char* p2 = s1.toAscii().ptr();
    //ASSERT_EQ('T', p2[0]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Concatenation)
{
    String s1("sum");
    String s2(L"ofTwo");

    String s3 = s1 + s2;    
    ASSERT_EQ(8, s3.size());
    EXPECT_EQ('s', s3[0]);
    EXPECT_EQ('u', s3[1]);
    EXPECT_EQ('m', s3[2]);
    EXPECT_EQ('o', s3[3]);
    EXPECT_EQ('f', s3[4]);
    EXPECT_EQ('T', s3[5]);
    EXPECT_EQ('w', s3[6]);
    EXPECT_EQ('o', s3[7]);

    String s5 = s2 + s1;    
    ASSERT_EQ(8, s5.size());
    EXPECT_EQ('o', s5[0]);
    EXPECT_EQ('f', s5[1]);
    EXPECT_EQ('T', s5[2]);
    EXPECT_EQ('w', s5[3]);
    EXPECT_EQ('o', s5[4]);
    EXPECT_EQ('s', s5[5]);
    EXPECT_EQ('u', s5[6]);
    EXPECT_EQ('m', s5[7]);


    String s4 = s1;
    s4 += s2;
    ASSERT_EQ(8, s3.size());
    EXPECT_EQ('s', s3[0]);
    EXPECT_EQ('u', s3[1]);
    EXPECT_EQ('m', s3[2]);
    EXPECT_EQ('o', s3[3]);
    EXPECT_EQ('f', s3[4]);
    EXPECT_EQ('T', s3[5]);
    EXPECT_EQ('w', s3[6]);
    EXPECT_EQ('o', s3[7]);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Conversion)
{
    std::string s1 = "Hei";
    String s2(s1);
    EXPECT_TRUE(s2 == L"Hei");
    EXPECT_FALSE(s2 == L"Heia");

    std::wstring s3 = L"Hei";
    String s4;
    s4 = s3;
    EXPECT_TRUE(s4 == L"Hei");
    EXPECT_FALSE(s4 == L"Heia");

    String s5("Test1");
    std::string s6 = s5.toStdString();
    EXPECT_TRUE(s6 == "Test1");

    String s7("Test2");
    std::wstring s8 = s7.toStdWString();
    EXPECT_TRUE(s8 == L"Test2");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, isEmpty)
{
    const String s0;
    EXPECT_TRUE(s0.isEmpty());

    const String s1 = "";
    EXPECT_TRUE(s1.isEmpty());

    const char* psz = NULL;
    const String s2 = psz;
    EXPECT_TRUE(s2.isEmpty());

    String s3 = "AA";
    EXPECT_FALSE(s3.isEmpty());

    s3 = s0;
    EXPECT_TRUE(s3.isEmpty());

    s3 = s1;
    EXPECT_TRUE(s3.isEmpty());

    s3 = s2;
    EXPECT_TRUE(s3.isEmpty());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Number)
{
    // float
    String s1 = String::number(1.234f);
    String s2 = String::number(-1.23e5f);
    String s3 = String::number(5.4e13f);

    EXPECT_STREQ("1.234", s1.toAscii().ptr());
    EXPECT_STREQ("-123000", s2.toAscii().ptr());

#ifdef WIN32
    EXPECT_STREQ("5.4e+013", s3.toAscii().ptr());
#else
    EXPECT_STREQ("5.4e+13", s3.toAscii().ptr());
#endif

    String s4 = String::number(-1.2375e5f, 'e', 2);
#ifdef WIN32
    EXPECT_STREQ("-1.24e+005", s4.toAscii().ptr());
#else
    EXPECT_STREQ("-1.24e+05", s4.toAscii().ptr());
#endif

    String s5 = String::number(-1.2375e5f, 'f', 2);
    EXPECT_STREQ("-123750.00", s5.toAscii().ptr());

    String s6 = String::number(-1.2375e3f, 'g');
    EXPECT_STREQ("-1237.5", s6.toAscii().ptr());

    // double
    String s11 = String::number(1.234);
    String s12 = String::number(-1.23e5);
    String s13 = String::number(5.4e13);

    EXPECT_STREQ("1.234", s11.toAscii().ptr());
    EXPECT_STREQ("-123000", s12.toAscii().ptr());

#ifdef WIN32
    EXPECT_STREQ("5.4e+013", s13.toAscii().ptr());
#else
    EXPECT_STREQ("5.4e+13", s13.toAscii().ptr());
#endif


    String s14 = String::number(-1.2375e5, 'e', 2);
#ifdef WIN32
    EXPECT_STREQ("-1.24e+005", s14.toAscii().ptr());
#else
    EXPECT_STREQ("-1.24e+05", s14.toAscii().ptr());
#endif

    String s15 = String::number(-1.2375e5, 'f', 2);
    EXPECT_STREQ("-123750.00", s15.toAscii().ptr());

    String s16 = String::number(-1.2375e4, 'g');
    EXPECT_STREQ("-12375", s16.toAscii().ptr());

    String s17 = String::number(1234567891012345.0, 'f', 4);
    EXPECT_STREQ("1234567891012345.0000", s17.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, toDouble)
{
    {
        String s = "123.456";
        bool ok = false;
        EXPECT_EQ(123.456, s.toDouble());
        EXPECT_EQ(123.456, s.toDouble(&ok));
        EXPECT_EQ(123.456, s.toDouble(999.0));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-123.456";
        bool ok = false;
        EXPECT_EQ(-123.456, s.toDouble());
        EXPECT_EQ(-123.456, s.toDouble(&ok));
        EXPECT_EQ(-123.456, s.toDouble(999.0));
        EXPECT_TRUE(ok);
    }

    {
        String s;
        bool ok = false;
        EXPECT_EQ(0.0, s.toDouble());
        EXPECT_EQ(0.0, s.toDouble(&ok));
        EXPECT_EQ(999.0, s.toDouble(999.0));
        EXPECT_FALSE(ok);
    }

    {
        String s = "noVal";
        bool ok = false;
        EXPECT_EQ(0.0, s.toDouble());
        EXPECT_EQ(0.0, s.toDouble(&ok));
        EXPECT_EQ(999.0, s.toDouble(999.0));
        EXPECT_FALSE(ok);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, toFloat)
{
    {
        String s = "123.456";
        bool ok = false;
        EXPECT_EQ(123.456f, s.toFloat());
        EXPECT_EQ(123.456f, s.toFloat(&ok));
        EXPECT_EQ(123.456f, s.toFloat(999.0f));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-123.456";
        bool ok = false;
        EXPECT_EQ(-123.456f, s.toFloat());
        EXPECT_EQ(-123.456f, s.toFloat(&ok));
        EXPECT_EQ(-123.456f, s.toFloat(999.0f));
        EXPECT_TRUE(ok);
    }

    {
        String s;
        bool ok = false;
        EXPECT_EQ(0.0, s.toFloat());
        EXPECT_EQ(0.0, s.toFloat(&ok));
        EXPECT_EQ(999.0f, s.toFloat(999.0f));
        EXPECT_FALSE(ok);
    }

    {
        String s = "noVal";
        bool ok = false;
        EXPECT_EQ(0.0, s.toFloat());
        EXPECT_EQ(0.0, s.toFloat(&ok));
        EXPECT_EQ(999.0f, s.toFloat(999.0f));
        EXPECT_FALSE(ok);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, toInt)
{
    {
        String s = "123";
        bool ok = false;
        EXPECT_EQ(123, s.toInt());
        EXPECT_EQ(123, s.toInt(&ok));
        EXPECT_EQ(123, s.toInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "0";
        bool ok = false;
        EXPECT_EQ(0, s.toInt());
        EXPECT_EQ(0, s.toInt(&ok));
        EXPECT_EQ(0, s.toInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-2147483648";
        bool ok = false;
        EXPECT_EQ((-2147483647 - 1), s.toInt());
        EXPECT_EQ((-2147483647 - 1), s.toInt(&ok));
        EXPECT_EQ((-2147483647 - 1), s.toInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "2147483647";
        bool ok = false;
        EXPECT_EQ(2147483647, s.toInt());
        EXPECT_EQ(2147483647, s.toInt(&ok));
        EXPECT_EQ(2147483647, s.toInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-2147483649";
        bool ok = false;
        EXPECT_EQ(0, s.toInt());
        EXPECT_EQ(0, s.toInt(&ok));
        EXPECT_EQ(999, s.toInt(999));
        EXPECT_FALSE(ok);
    }

    {
        String s = "2147483648";
        bool ok = false;
        EXPECT_EQ(0, s.toInt());
        EXPECT_EQ(0, s.toInt(&ok));
        EXPECT_EQ(999, s.toInt(999));
        EXPECT_FALSE(ok);
    }

    {
        String s = "noVal";
        bool ok = false;
        EXPECT_EQ(0, s.toInt());
        EXPECT_EQ(0, s.toInt(&ok));
        EXPECT_EQ(999, s.toInt(999));
        EXPECT_FALSE(ok);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, toUInt)
{
    {
        String s = "123";
        bool ok = false;
        EXPECT_EQ(123, s.toUInt());
        EXPECT_EQ(123, s.toUInt(&ok));
        EXPECT_EQ(123, s.toUInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "0";
        bool ok = false;
        EXPECT_EQ(0, s.toUInt());
        EXPECT_EQ(0, s.toUInt(&ok));
        EXPECT_EQ(0, s.toUInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-123";
        bool ok = false;
        EXPECT_EQ(0, s.toUInt());
        EXPECT_EQ(0, s.toUInt(&ok));
        EXPECT_EQ(999, s.toUInt(999));
        EXPECT_FALSE(ok);
    }

    {
        String s = "4294967295";
        bool ok = false;
        EXPECT_EQ(4294967295u, s.toUInt());
        EXPECT_EQ(4294967295u, s.toUInt(&ok));
        EXPECT_EQ(4294967295u, s.toUInt(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "4294967296";
        bool ok = false;
        EXPECT_EQ(0, s.toUInt());
        EXPECT_EQ(0, s.toUInt(&ok));
        EXPECT_EQ(999, s.toUInt(999));
        EXPECT_FALSE(ok);
    }

    {
        String s = "noVal";
        bool ok = false;
        EXPECT_EQ(0, s.toUInt());
        EXPECT_EQ(0, s.toUInt(&ok));
        EXPECT_EQ(999, s.toUInt(999));
        EXPECT_FALSE(ok);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, toInt64)
{
    {
        String s = "123";
        bool ok = false;
        EXPECT_EQ(123, s.toInt64());
        EXPECT_EQ(123, s.toInt64(&ok));
        EXPECT_EQ(123, s.toInt64(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "0";
        bool ok = false;
        EXPECT_EQ(0, s.toInt64());
        EXPECT_EQ(0, s.toInt64(&ok));
        EXPECT_EQ(0, s.toInt64(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-9223372036854775808";
        bool ok = false;
        EXPECT_EQ((-9223372036854775807 - 1), s.toInt64());
        EXPECT_EQ((-9223372036854775807 - 1), s.toInt64(&ok));
        EXPECT_EQ((-9223372036854775807 - 1), s.toInt64(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = " 9223372036854775807";
        bool ok = false;
        EXPECT_EQ(9223372036854775807, s.toInt64());
        EXPECT_EQ(9223372036854775807, s.toInt64(&ok));
        EXPECT_EQ(9223372036854775807, s.toInt64(999));
        EXPECT_TRUE(ok);
    }

    {
        String s = "-9223372036854775809";
        bool ok = false;
        EXPECT_EQ(0, s.toInt64());
        EXPECT_EQ(0, s.toInt64(&ok));
        EXPECT_EQ(999, s.toInt64(999));
        EXPECT_FALSE(ok);
    }

    {
        String s = "9223372036854775808";
        bool ok = false;
        EXPECT_EQ(0, s.toInt64());
        EXPECT_EQ(0, s.toInt64(&ok));
        EXPECT_EQ(999, s.toInt64(999));
        EXPECT_FALSE(ok);
    }

    {
        String s = "noVal";
        bool ok = false;
        EXPECT_EQ(0, s.toInt64());
        EXPECT_EQ(0, s.toInt64(&ok));
        EXPECT_EQ(999, s.toInt64(999));
        EXPECT_FALSE(ok);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, StringBuilding)
{
    // float
    float fps = 123.456f;
    String s1 = "Framerate: ";
    s1 += String(fps);
    s1 += " (fps)";

    EXPECT_STREQ("Framerate: 123.456 (fps)", s1.toAscii().ptr());

    String s4 = "Framerate: " + String(fps) + " (fps)";
    EXPECT_STREQ("Framerate: 123.456 (fps)", s4.toAscii().ptr());

    String s2 = "Framerate: ";
    s2 += String::number(fps, 'f', 1);
    s2 += " (fps)";
    EXPECT_STREQ("Framerate: 123.5 (fps)", s2.toAscii().ptr());

    String s3 = "Framerate: " + String::number(fps, 'f', 1) + " (fps)";
    EXPECT_STREQ("Framerate: 123.5 (fps)", s3.toAscii().ptr());

    // Double
    double dfps = 123456789012345.6;
    String s11 = "Framerate: ";
    s11 += String(dfps);
    s11 += " (fps)";

#ifdef WIN32
    EXPECT_STREQ("Framerate: 1.23457e+014 (fps)", s11.toAscii().ptr());
#else
    EXPECT_STREQ("Framerate: 1.23457e+14 (fps)", s11.toAscii().ptr());
#endif

    String s13 = "Framerate: " + String::number(dfps, 'f', 1) + " (fps)";
    EXPECT_STREQ("Framerate: 123456789012345.6 (fps)", s13.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ComponentManipulation)
{
    String s1 = "Testing";
    s1[2] = 'z';
    s1[6] = '!';

    EXPECT_STREQ("Teztin!", s1.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, StringArray)
{
    typedef std::vector<String> StringArray;

    StringArray a;
    a.push_back("Apple");
    a.push_back("Banana");
    a.push_back("Orange");

    EXPECT_STREQ("Apple", a[0].toAscii().ptr());
    EXPECT_STREQ("Banana", a[1].toAscii().ptr());
    EXPECT_STREQ("Orange", a[2].toAscii().ptr());

    StringArray b;
    b.resize(3);
    b[0] = "Apple";
    b[1] = "Banana";
    b[2] = "Orange";

    EXPECT_STREQ("Apple", b[0].toAscii().ptr());
    EXPECT_STREQ("Banana", b[1].toAscii().ptr());
    EXPECT_STREQ("Orange", b[2].toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Split)
{
    std::vector<String> tokens;

    // Default delimeter is " "
    String source = "a b c";
    tokens = source.split();
    EXPECT_EQ(3, tokens.size());

    tokens.clear();
    source = "a:b c";
    tokens = source.split();
    EXPECT_EQ(2, tokens.size());

    tokens.clear();
    source = "a:b  ::  c :";
    String delimeters = " :";
    tokens = source.split(delimeters);
    EXPECT_EQ(3, tokens.size());

    EXPECT_EQ(1, tokens[0].size());
    EXPECT_EQ(1, tokens[1].size());
    EXPECT_EQ(1, tokens[2].size());

    tokens.clear();
    source = "";
    tokens = source.split();
    EXPECT_EQ(0, tokens.size());

    tokens.clear();
    source = "1.23\t4.56   5.67  \t\t   6.83   ";
    tokens = source.split(" \t");
    EXPECT_EQ(4, tokens.size());
    EXPECT_STREQ("1.23", tokens[0].toAscii().ptr());
    EXPECT_STREQ("4.56", tokens[1].toAscii().ptr());
    EXPECT_STREQ("5.67", tokens[2].toAscii().ptr());
    EXPECT_STREQ("6.83", tokens[3].toAscii().ptr());

    tokens.clear();
    source = "1.23, 4.56,5.67, 6.8 ";
    tokens = source.split(" ,");
    EXPECT_EQ(4, tokens.size());
    EXPECT_STREQ("1.23", tokens[0].toAscii().ptr());
    EXPECT_STREQ("4.56", tokens[1].toAscii().ptr());
    EXPECT_STREQ("5.67", tokens[2].toAscii().ptr());
    EXPECT_STREQ("6.8", tokens[3].toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, find)
{
    String s = "abc def\nghij";
    EXPECT_EQ(7, s.find("\n"));
    EXPECT_EQ(String::npos, s.find("#"));

    String s1 = "da#dsjk#dah";
    EXPECT_EQ(2, s1.find("#"));
    EXPECT_EQ(7, s1.find("#", 3));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, startsWith)
{
    const String s0 = "abc def";
    EXPECT_TRUE(s0.startsWith("a"));
    EXPECT_TRUE(s0.startsWith("abc d"));
    EXPECT_TRUE(s0.startsWith("abc def"));

    EXPECT_FALSE(s0.startsWith("b"));
    EXPECT_FALSE(s0.startsWith("abc def "));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, subString)
{
    String s = "testing 123";

    EXPECT_STREQ("sting", s.subString(2, 5).toAscii().ptr());
    EXPECT_STREQ("g 123", s.subString(6).toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, replace)
{
    const String src = "Testing aa aaab";
    
    {
        String s(src);
        s.replace("a", "");
        EXPECT_STREQ("Testing  b", s.toAscii().ptr());
    }

    {
        String s(src);
        s.replace("", "X");
        EXPECT_STREQ("Testing aa aaab", s.toAscii().ptr());
    }

    {
        String s(src);
        s.replace("t", "X");
        EXPECT_STREQ("TesXing aa aaab", s.toAscii().ptr());
    }

    {
        String s(src);
        s.replace("aa", "XX");
        EXPECT_STREQ("Testing XX XXab", s.toAscii().ptr());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, fromAscii)
{
    {
        const char* nullStr = NULL;
        String s = String::fromAscii(nullStr);
        EXPECT_EQ(0, s.size());
        EXPECT_STREQ(L"", s.c_str());
        EXPECT_TRUE(s == String());
    }

    {
        const char* nullStr = NULL;
        String s = String::fromAscii(nullStr, 0);
        EXPECT_EQ(0, s.size());
        EXPECT_STREQ(L"", s.c_str());
        EXPECT_TRUE(s == String());
    }

    {
        const char* emptyStr = "";
        String s = String::fromAscii(emptyStr);
        EXPECT_EQ(0, s.size());
        EXPECT_STREQ(L"", s.c_str());
        EXPECT_TRUE(s == String());
    }

    {
        const char* emptyStr = "";
        String s = String::fromAscii(emptyStr, 0);
        EXPECT_EQ(0, s.size());
        EXPECT_STREQ(L"", s.c_str());
        EXPECT_TRUE(s == String());
    }

    {
        const char* str = "ABC";
        String s = String::fromAscii(str);
        EXPECT_EQ(3, s.size());
        EXPECT_STREQ(L"ABC", s.c_str());
        EXPECT_TRUE(s == String("ABC"));
    }

    {
        const char* str = "ABC";
        String s = String::fromAscii(str, 3);
        EXPECT_EQ(3, s.size());
        EXPECT_STREQ(L"ABC", s.c_str());
        EXPECT_TRUE(s == String("ABC"));
    }

    {
        const char* str = "ABC";
        String s = String::fromAscii(str, 1);
        EXPECT_EQ(1, s.size());
        EXPECT_STREQ(L"A", s.c_str());
        EXPECT_TRUE(s == String("A"));
    }

    {
        const char* str = "ABC";
        String s = String::fromAscii(str, 0);
        EXPECT_EQ(0, s.size());
        EXPECT_STREQ(L"", s.c_str());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ConvertToFromUtf8)
{
    // For generating test data see:
    // http://rishida.net/tools/conversion/
    // http://slayeroffice.com/tools/unicode_lookup/
    // http://www.unicodetools.com/unicode/utf8-to-latin-converter.php

    {
        const wchar_t* uniStr = L"abc";
        const char* utfStr = "abc";
        
        String str(uniStr);
        CharArray ca = str.toUtf8();
        ASSERT_EQ(3, ca.size());
        ASSERT_STREQ(utfStr, ca.ptr());

        str = String::fromUtf8(utfStr);
        ASSERT_EQ(3, str.size());
        ASSERT_TRUE(str == uniStr);
    }

    {
		// ���
        const wchar_t* uniStr = L"\x00e6\x00f8\x00e5";
        const char* utfStr = "æøå";

        String str(uniStr);
        CharArray ca = str.toUtf8();
        ASSERT_EQ(6, ca.size());
        ASSERT_STREQ(utfStr, ca.ptr());

        str = String::fromUtf8(utfStr);
        ASSERT_EQ(3, str.size());
        ASSERT_TRUE(str == uniStr);
    }

    {
		// a�c
        const wchar_t* uniStr = L"a \x00f8 c";
        const char* utfStr = "a ø c";

        String str(uniStr);
        CharArray ca = str.toUtf8();
        ASSERT_EQ(6, ca.size());
        ASSERT_STREQ(utfStr, ca.ptr());

        str = String::fromUtf8(utfStr);
        ASSERT_EQ(5, str.size());
        ASSERT_TRUE(str == uniStr);
    }

    {
        // Greek small, alfa, beta, gamma, delta and epsilon
        const wchar_t* uniStr = L"\x03B1\x03B2\x03B3\x03B4\x03B5";
        const char* utfStr = "αβγδε";

        String str(uniStr);
        CharArray ca = str.toUtf8();
        ASSERT_EQ(10, ca.size());
        ASSERT_STREQ(utfStr, ca.ptr());

        str = String::fromUtf8(utfStr);
        ASSERT_EQ(5, str.size());
        ASSERT_TRUE(str == uniStr);
    }

    {
        // Greek word 'kosme'
        // from http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
        const wchar_t* uniStr = L"\x03BA\x1F79\x03C3\x03BC\x03B5";
        const char* utfStr = "\xCE\xBA\xE1\xBD\xB9\xCF\x83\xCE\xBC\xCE\xB5";

        String str(uniStr);
        CharArray ca = str.toUtf8();
        ASSERT_EQ(11, ca.size());
        ASSERT_STREQ(utfStr, ca.ptr());

        str = String::fromUtf8(utfStr);
        ASSERT_EQ(5, str.size());
        ASSERT_TRUE(str == uniStr);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ArgInt)
{
    String s = String("%1").arg(123);
    EXPECT_TRUE(s == "123");

    String s1 = String("%1 + %1 = %2").arg(2).arg(4);
    EXPECT_TRUE(s1 == "2 + 2 = 4");

    String s2 = String("%1 + %1 = %2").arg(2, 3).arg(4,2);
    EXPECT_TRUE(s2 == "  2 +   2 =  4");

    // Right justified padding
    String s3 = String("%1 %2 %3").arg(123, 5).arg(-12,5).arg(44432,5);
    EXPECT_TRUE(s3 == "  123   -12 44432");

    // Left justified padding
    String s4 = String("%1 %2 %3").arg(123, -5).arg(-12,-5).arg(44432,-5);
    EXPECT_TRUE(s4 == "123   -12   44432");

    // Right justified padding with custom padding char
    String("%1").arg(123, 5, L'x');

    String s5 = String("%1 %2 %3").arg(123, 5, L'x').arg(-12,5, L'y').arg((int)44432,5, L'z');
    EXPECT_TRUE(s5 == "xx123 yy-12 44432");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ArgUint)
{
    {
        cvf::uint test = 0;
        String s = String("%1").arg(test);
        EXPECT_TRUE(s == "0");
    }

    {
        cvf::uint test = 10;
        String s = String("%1").arg(test);
        EXPECT_TRUE(s == "10");
    }

    {
        cvf::uint test = 4294967295;
        String s = String("%1").arg(test);
        EXPECT_TRUE(s == "4294967295");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ArgInt64)
{
    String s = String("%1").arg((int64)1234567890123456);
    EXPECT_TRUE(s == "1234567890123456");
    
    String s1((int64)1234567890123456);
    EXPECT_TRUE(s1 == s);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringText, ArgString)
{
    String s0 = String("NoArg").arg(123);
    EXPECT_STREQ("NoArg", s0.toAscii().ptr());

    String s = String("Test: %1").arg("frv");
    EXPECT_TRUE(s == "Test: frv");

    // Unicode test
    String arg = L"\x03BA\x1F79\x03C3\x03BC\x03B5";
    String s2 = String("Test: %1 xx").arg(arg);
    EXPECT_TRUE(s2 == L"Test: \x03BA\x1F79\x03C3\x03BC\x03B5 xx");

    // Padding
    String s3 = String("%1%2%3").arg("test1", 8).arg("ja", 4).arg("nei", -4);
    EXPECT_TRUE(s3 == "   test1  janei ");

    // Padding with custom pad character
    String s4 = String("%1%2%3").arg("test1", 8, L'_').arg("ja", 4, L'_').arg("nei", -6, L'_');
    EXPECT_TRUE(s4 == "___test1__janei___");

    // String with % character
    String s5 = String("Reading file %1 (%2% complete)").arg("Test.txt").arg(92.3); 
    EXPECT_TRUE(s5 == "Reading file Test.txt (92.3% complete)");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringText, ArgChar)
{
    String s = String("Test: %1").arg('a');
    EXPECT_TRUE(s == "Test: a");

    s = String("Test: %1").arg('a', 2);
    EXPECT_TRUE(s == "Test:  a");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ArgDouble)
{
    String s = String("Test: %1").arg(123.456);
    EXPECT_TRUE(s == "Test: 123.456");

    String s2 = String("Test: %1").arg(123.456, 9, 'f', 2);
    EXPECT_TRUE(s2 == "Test:    123.46");

    String s3 = String("%1 of %2").arg(123.456).arg(456.789);
    EXPECT_TRUE(s3 == "123.456 of 456.789");

    String s4 = String("%1%2%3").arg(12.34, 8).arg(23.45, 8).arg(34.56, 8);
    EXPECT_TRUE(s4 == "   12.34   23.45   34.56");

    String s5 = String("%1%2%3").arg(12.34, -8).arg(23.45, -8).arg(34.56, -8);
    EXPECT_TRUE(s5 == "12.34   23.45   34.56   ");

    String s6 = String("%1%2%3").arg(12.34, -8, 'g', -1, L'#').arg(123.45, -8, 'g', -1, L'#').arg(34.56, -8);
    EXPECT_TRUE(s6 == "12.34###123.45##34.56   ");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, ArgFloat)
{
    String s = String("Test: %1").arg(123.456f);
    EXPECT_TRUE(s == "Test: 123.456");

    String s2 = String("Test: %1").arg(123.456f, 9, 'f', 2);
    EXPECT_TRUE(s2 == "Test:    123.46");

    String s3 = String("%1 of %2").arg(123.456f).arg(456.789f);
    EXPECT_TRUE(s3 == "123.456 of 456.789");

    String s4 = String("%1%2%3").arg(12.34f, 8).arg(23.45f, 8).arg(34.56f, 8);
    EXPECT_TRUE(s4 == "   12.34   23.45   34.56");

    String s5 = String("%1%2%3").arg(12.34f, -8).arg(23.45f, -8).arg(34.56f, -8);
    EXPECT_TRUE(s5 == "12.34   23.45   34.56   ");

    String s6 = String("%1%2%3").arg(12.34f, -8, 'g', -1, L'#').arg(123.45f, -8, 'g', -1, L'#').arg(34.56f, -8);
    EXPECT_TRUE(s6 == "12.34###123.45##34.56   ");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(StringTest, Swap)
{
    String s1 = "en";
    String s2 = "to";
    ASSERT_TRUE(s1 == "en");
    ASSERT_TRUE(s2 == "to");

    s1.swap(s2);
    EXPECT_TRUE(s1 == "to");
    EXPECT_TRUE(s2 == "en");
}
