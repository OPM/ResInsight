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


#pragma once

#include "cvfCharArray.h"

#include <string>

namespace cvf {


//==================================================================================================
//
// A general unicode based string class 
//
//==================================================================================================
class String
{
public:
    String();
    String(const std::string& str);
    String(const std::wstring& str);
    String(const String& str);
    String(const char* str);
    String(const wchar_t* str);

    explicit String(char c);
    
    explicit String(int number);
    explicit String(int64 number);
    explicit String(uint number);
    explicit String(float number);
    explicit String(double number);

    String&         operator=(String rhs);
    bool            operator==(const String& rhs) const;
    bool            operator==(const wchar_t* rhs) const;
    bool            operator<(const String& rhs) const;
    bool            operator!=(const String& rhs) const;
    const String    operator+(const String& rhs) const;
    String&         operator+=(const String& rhs);

    const wchar_t&  operator[](size_t pos) const;
    wchar_t&        operator[](size_t pos);

    String              toLower() const;
    String              toUpper() const;
    String              trimmedRight() const;
    String              trimmedLeft() const;
    String              trimmed() const;
    String              simplified() const;

    std::vector<String> split(const String& delimiters = " ") const;
    size_t              find(const String& str, size_t start = 0) const;
    bool                startsWith(const String& str) const;
    String              subString(size_t start, size_t length = npos) const;
	void                replace(const String& before, const String& after);

    const wchar_t*      c_str() const;
    CharArray           toAscii() const;                // Useful when you need a const char* pointer.
    std::string         toStdString() const;
    std::wstring        toStdWString() const;
    CharArray           toUtf8() const;

    static String       fromAscii(const char* str, size_t strSize = npos);
    static String       fromUtf8(const char* str);

    bool                isEmpty() const;
    size_t              size() const;
    void                resize(size_t size);

    static String       number(float n, char format = 'g', int precision = -1);
    static String       number(double n, char format = 'g', int precision = -1);

    double              toDouble(bool* ok = NULL) const;
    double              toDouble(double defaultValue) const;
    float               toFloat(bool* ok = NULL) const;
    float               toFloat(float defaultValue) const;
    int                 toInt(bool* ok = NULL) const;
    int                 toInt(int defaultValue) const;
    uint                toUInt(bool* ok = NULL) const;
    uint                toUInt(uint defaultValue) const;
    int64               toInt64(bool* ok = NULL) const;
    int64               toInt64(int64 defaultValue) const;

    String              arg(const String& a, int fieldWidth = 0, const wchar_t& fillChar = ' ') const;
    String              arg(char a, int fieldWidth = 0, const wchar_t& fillChar = ' ') const;

    String              arg(int a, int fieldWidth = 0, const wchar_t& fillChar = ' ') const;
    String              arg(int64 a, int fieldWidth = 0, const wchar_t& fillChar = ' ') const;
    String              arg(uint a, int fieldWidth = 0, const wchar_t& fillChar = ' ') const;
    String              arg(float a, int fieldWidth = 0, char format = 'g', int precision = -1, const wchar_t& fillChar = ' ') const;
    String              arg(double a, int fieldWidth = 0, char format = 'g', int precision = -1, const wchar_t& fillChar = ' ') const;

    void                swap(String& other);

    static const size_t npos;       // Same as std::string::npos. This value, when used as the value for a count parameter n in string's member functions, roughly indicates "as many as possible".
                                    // As a return value it is usually used to indicate failure.
private:
    std::wstring m_string;
};

String   operator+(const char* str1, const String& str2);

}
