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
#include "cvfMath.h"

#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <limits>
#include <iosfwd>

namespace cvf {



//==================================================================================================
///
/// \class cvf::String
/// \ingroup Core
///
/// A general unicode based string class. 
/// 
/// The string class supports conversion to and from 
///  - std::string
///  - std::wstring
///  - const char*
///  - const wchar_t*
/// 
/// A separate class cvf::CharArray is used to be able to support the toAscii() method (conversion
/// to a const char* string).
/// 
//==================================================================================================

const size_t String::npos = static_cast<size_t>(-1);

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
String::String()
{
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given std::string
//--------------------------------------------------------------------------------------------------
String::String(const std::string& str)
{
    m_string.resize(str.size(), L' ');
    std::copy(str.begin(), str.end(), m_string.begin());
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given std::wstring
//--------------------------------------------------------------------------------------------------
String::String(const std::wstring& str)
{
    m_string = str;
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given String
//--------------------------------------------------------------------------------------------------
String::String(const String& str)
{
    m_string = str.m_string;
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given char* string
//--------------------------------------------------------------------------------------------------
String::String(const char* str)
{
    if (str != NULL)
    {
        // Raw conversion, no UTF8
        m_string = std::wstring(str, str + strlen(str));
    }
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given wchar_t* string
//--------------------------------------------------------------------------------------------------
String::String(const wchar_t* str)
{
    if (str != NULL)
    {
        m_string = str;
    }
}

//--------------------------------------------------------------------------------------------------
/// Create a string from the given char
//--------------------------------------------------------------------------------------------------
String::String(char c)
{
    std::wstringstream sstr;
    sstr << c;
    m_string = sstr.str();
}

//--------------------------------------------------------------------------------------------------
/// Create a string from the given integer (using default formatting)
//--------------------------------------------------------------------------------------------------
String::String(int number)
{
    std::wstringstream sstr;
    sstr << number;
    m_string = sstr.str();
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given 64 bit integer (using default formatting)
//--------------------------------------------------------------------------------------------------
String::String(int64 number)
{
    std::wstringstream sstr;
    sstr << number;
    m_string = sstr.str();
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given unsigned integer (using default formatting)
//--------------------------------------------------------------------------------------------------
String::String(uint number)
{
    std::wstringstream sstr;
    sstr << number;
    m_string = sstr.str();
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given float (using default formatting)
//--------------------------------------------------------------------------------------------------
String::String(float number)
{
    std::wstringstream sstr;
    sstr << number;
    m_string = sstr.str();
}


//--------------------------------------------------------------------------------------------------
/// Create a string from the given double (using default formatting)
//--------------------------------------------------------------------------------------------------
String::String(double number)
{
    std::wstringstream sstr;
    sstr << number;
    m_string = sstr.str();
}


//--------------------------------------------------------------------------------------------------
/// Set the contents of this string to the passed string
//--------------------------------------------------------------------------------------------------
String& String::operator=(String rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Check if this string is equal to the passed string
//--------------------------------------------------------------------------------------------------
bool String::operator==(const String& rhs) const
{
    return m_string == rhs.m_string;
}


//--------------------------------------------------------------------------------------------------
/// Check if this string is equal to the passed wchar_t* string
//--------------------------------------------------------------------------------------------------
bool String::operator==(const wchar_t* rhs) const
{
    return m_string == rhs;
}


//--------------------------------------------------------------------------------------------------
/// Check if this string is less than the passed string
//--------------------------------------------------------------------------------------------------
bool String::operator<(const String& rhs) const
{
    return m_string < rhs.m_string;
}


//--------------------------------------------------------------------------------------------------
/// Check if this string is not equal to the passed string
//--------------------------------------------------------------------------------------------------
bool String::operator!=(const String& rhs) const
{
    return !operator==(rhs);
}


//--------------------------------------------------------------------------------------------------
/// Return the concatenation of this string and the passed string (rhs)
//--------------------------------------------------------------------------------------------------
const String String::operator+(const String& rhs) const
{
    std::wstring sum = m_string + rhs.m_string;

    return sum;
}


//--------------------------------------------------------------------------------------------------
/// Set this string to the concatenation of this string and the passed string (rhs)
//--------------------------------------------------------------------------------------------------
String& String::operator+=(const String& rhs)
{
    m_string += rhs.m_string;
    return *this;
}


//--------------------------------------------------------------------------------------------------
///  Get a const ref to the unicode character at the given position
//--------------------------------------------------------------------------------------------------
const wchar_t& String::operator[](size_t pos) const
{
    CVF_TIGHT_ASSERT(pos < size());

    return m_string[pos];
}


//--------------------------------------------------------------------------------------------------
/// Get a modifiable ref to the unicode character at the given position
//--------------------------------------------------------------------------------------------------
wchar_t& String::operator[](size_t pos)
{
    CVF_TIGHT_ASSERT(pos < size());

    return m_string[pos];
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the string has no characters, otherwise returns false.
//--------------------------------------------------------------------------------------------------
bool String::isEmpty() const
{
    return m_string.empty();
}


//--------------------------------------------------------------------------------------------------
///  Returns the length of the string
//--------------------------------------------------------------------------------------------------
size_t String::size() const
{
    return m_string.size();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void String::resize(size_t size)
{
    m_string.resize(size);
}


//--------------------------------------------------------------------------------------------------
/// Returns a lowercase copy of the string.
//--------------------------------------------------------------------------------------------------
String String::toLower() const
{
    std::wstring retStr;

    size_t strLen = m_string.size();
    if (strLen > 0)
    {
        retStr.resize(strLen);

        size_t i;
        for (i = 0; i < strLen; ++i)
        {
            retStr[i] = towlower(m_string[i]);
        } 
    }

    return retStr;
}


//--------------------------------------------------------------------------------------------------
/// Returns an uppercase copy of the string.
//--------------------------------------------------------------------------------------------------
String String::toUpper() const
{
    std::wstring retStr;

    size_t strLen = m_string.size();
    if (strLen > 0)
    {
        retStr.resize(strLen);

        size_t i;
        for (i = 0; i < strLen; ++i)
        {
            retStr[i] = towupper(m_string[i]);
        } 
    }

    return retStr;
}



//--------------------------------------------------------------------------------------------------
/// Returns string with trailing whitespace removed
//--------------------------------------------------------------------------------------------------
String String::trimmedRight() const
{
    // Same whitespace characters as isspace()
    const std::wstring whitespaces(L" \t\n\v\f\r");

    size_t pos = m_string.find_last_not_of(whitespaces);
    if (pos != std::wstring::npos)
    {
        std::wstring retStr(m_string);
        retStr.erase(pos + 1);
        return retStr;
    }
    else
    {
        // string is all whitespace
        return String();
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns string with leading whitespace removed
//--------------------------------------------------------------------------------------------------
String String::trimmedLeft() const
{
    // Same whitespace characters as isspace()
    const std::wstring whitespaces(L" \t\n\v\f\r");

    size_t pos = m_string.find_first_not_of(whitespaces); 
    if (pos != std::wstring::npos)
    {
        std::wstring retStr = m_string.substr(pos);
        return retStr;
    }
    else
    {
        return String();
    }
}


//--------------------------------------------------------------------------------------------------
/// Return string with leading and trailing whitespace removed
//--------------------------------------------------------------------------------------------------
String String::trimmed() const
{
    // Same whitespace characters as isspace()
    const std::wstring whitespaces(L" \t\n\v\f\r");

    size_t startpos = m_string.find_first_not_of(whitespaces);
    size_t endpos = m_string.find_last_not_of(whitespaces);

    // If all spaces or empty return an empty string
    if ((startpos == std::wstring::npos) || (endpos == std::wstring::npos))
    {
        return String();
    }
    else
    {
        std::wstring retStr = m_string.substr(startpos, endpos - startpos + 1);
        return retStr;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns simplified string.
/// 
/// Strips leading and trailing whitespace. Replaces sequences of internal whitespace with one space
//--------------------------------------------------------------------------------------------------
String String::simplified() const
{
    // Get rid of leading and trailing whitespace
    String str = this->trimmed();

    std::wstring newStr;
    size_t length = str.size();
    size_t i;
    for (i = 0; i < length; i++)
    {
        bool charIsWhitespace = iswspace(str[i]) ? true : false;
        if (charIsWhitespace && i > 0 && iswspace(str[i - 1])) 
        {
        }
        else 
        {
            if (charIsWhitespace)
            {
                // replace whitespace with space
                newStr += L" ";
            }
            else
            {
                newStr += str[i];
            }
        }
    }

    return String(newStr);
}


//--------------------------------------------------------------------------------------------------
/// Get the string as an ASCII 8 bit char text. 
/// 
/// A CharArray is used as a transport vehicle. This class has a ptr() method which allows for 
/// conversion to const char*.\n
/// To get a const char pointer, do the following:
/// \code
/// System::strcpy(szString, 20, myString.toAscii().ptr());
/// \endcode
///
/// \sa CharArray
//--------------------------------------------------------------------------------------------------
CharArray String::toAscii() const
{
    CharArray ascii;

    size_t numUnicodeChars = m_string.size();
    size_t i;
    for(i = 0; i < numUnicodeChars; i++)
    {
        unsigned int uc = m_string[i];
        if (uc < 0xff)
        {
            ascii.push_back(static_cast<char>(uc));
        }
        else
        {
            ascii.push_back('?');
        }
    }

    return ascii;
}


//--------------------------------------------------------------------------------------------------
/// Convert a string to a std::string.
///
/// Note that conversion is done via a call to toAscii()
//--------------------------------------------------------------------------------------------------
std::string String::toStdString() const
{
    CharArray array = toAscii();    
    std::string str = array.ptr();

    return str; 
}


//--------------------------------------------------------------------------------------------------
/// Convert a string to a std::wstring (std unicode string)
//--------------------------------------------------------------------------------------------------
std::wstring String::toStdWString() const
{
    return m_string;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CharArray String::toUtf8() const
{
    // From http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451
    static const unsigned int MASKBITS      = 0x3F;
    static const unsigned int MASKBYTE      = 0x80;
    static const unsigned int MASK2BYTES    = 0xC0;
    static const unsigned int MASK3BYTES    = 0xE0;
    static const unsigned int MASK4BYTES    = 0xF0;
    static const unsigned int MASK5BYTES    = 0xF8;
    static const unsigned int MASK6BYTES    = 0xFC;

    size_t numUnicodeChars = m_string.size();

    CharArray charArr;

    size_t i;
    for(i = 0; i < numUnicodeChars; i++)
    {
        unsigned int uc = m_string[i];

        // 0xxxxxxx
        if (uc < 0x80)
        {
            charArr.push_back(static_cast<char>(uc));
        }
        // 110xxxxx 10xxxxxx
        else if (uc < 0x800)
        {
            charArr.push_back(static_cast<char>(MASK2BYTES | (uc >> 6)));
            charArr.push_back(static_cast<char>(MASKBYTE   | (uc & MASKBITS)));
        }
        // 1110xxxx 10xxxxxx 10xxxxxx
        else if (uc < 0x10000)
        {
            charArr.push_back(static_cast<char>(MASK3BYTES | (uc >> 12)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 6) & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | (uc & MASKBITS)));
        }

        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (uc < 0x200000)
        {
            charArr.push_back(static_cast<char>(MASK4BYTES | (uc >> 18)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 12) & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 6)  & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | (uc & MASKBITS)));
        }
        // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (uc < 0x4000000)
        {
            charArr.push_back(static_cast<char>(MASK5BYTES | (uc >> 24)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 18) & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 12) & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 6)  & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | (uc & MASKBITS)));
        }
        // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (uc < 0x8000000)
        {
            charArr.push_back(static_cast<char>(MASK6BYTES | (uc >> 30)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 18) & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 12) & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | ((uc >> 6)  & MASKBITS)));
            charArr.push_back(static_cast<char>(MASKBYTE   | (uc & MASKBITS)));
        }
    }

    return charArr;
}



//--------------------------------------------------------------------------------------------------
/// Returns a String initialized with the first \a strSize characters from the string str
///
/// If \a strSize is npos, this function will compute the length of the string.
//--------------------------------------------------------------------------------------------------
cvf::String String::fromAscii(const char* str, size_t strSize)
{
    if (str != NULL)
    {
        if (strSize == npos)
        {
            strSize = strlen(str);
        }

        // Raw conversion, no UTF8
        return String(std::wstring(str, str + strSize));
    }
    else
    {
        return String();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String String::fromUtf8(const char* utfStr)
{
    // From http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451
    static const unsigned int MASKBITS      = 0x3F;
    //static const unsigned int MASKBYTE      = 0x80;
    static const unsigned int MASK2BYTES    = 0xC0;
    static const unsigned int MASK3BYTES    = 0xE0;
    static const unsigned int MASK4BYTES    = 0xF0;
    static const unsigned int MASK5BYTES    = 0xF8;
    static const unsigned int MASK6BYTES    = 0xFC;


    size_t utfStringLength = System::strlen(utfStr);
    if (utfStringLength == 0)
    {
        return String();
    }


    std::wstring uStr;

    size_t i = 0;
    while (i < utfStringLength)
    {
        // 4 byte unicode character
        unsigned int unicodeChar = 0;

        // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        if ((utfStr[i] & MASK6BYTES) == MASK6BYTES)
        {
            if (i + 5 < utfStringLength)
            {
                unicodeChar = ((utfStr[i] & 0x01) << 30) | ((utfStr[i + 1] & MASKBITS) << 24) | ((utfStr[i + 2] & MASKBITS) << 18) | ((utfStr[i + 3] & MASKBITS) << 12) | ((utfStr[i + 4] & MASKBITS) << 6) | (utfStr[i + 5] & MASKBITS);
            }

            i += 6;
        }

        // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if ((utfStr[i] & MASK5BYTES) == MASK5BYTES)
        {
            if (i + 4 < utfStringLength)
            {
                unicodeChar = ((utfStr[i] & 0x03) << 24) | ((utfStr[i + 1] & MASKBITS) << 18) | ((utfStr[i + 2] & MASKBITS) << 12) | ((utfStr[i + 3] & MASKBITS) << 6) | (utfStr[i + 4] & MASKBITS);
            }

            i += 5;
        }

        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if ((utfStr[i] & MASK4BYTES) == MASK4BYTES)
        {
            if (i + 3 < utfStringLength)
            {
                unicodeChar = ((utfStr[i] & 0x07) << 18) | ((utfStr[i + 1] & MASKBITS) << 12) | ((utfStr[i + 2] & MASKBITS) << 6) | (utfStr[i + 3] & MASKBITS);
            }

            i += 4;
        }


        // 1110xxxx 10xxxxxx 10xxxxxx
        else if ((utfStr[i] & MASK3BYTES) == MASK3BYTES)
        {
            if (i + 2 < utfStringLength)
            {
                unicodeChar = ((utfStr[i] & 0x0F) << 12) | ((utfStr[i + 1] & MASKBITS) << 6) | (utfStr[i + 2] & MASKBITS);
            }

            i += 3;
        }

        // 110xxxxx 10xxxxxx
        else if ((utfStr[i] & MASK2BYTES) == MASK2BYTES)
        {
            if (i + 1 < utfStringLength)
            {
                unicodeChar = ((utfStr[i] & 0x1F) << 6) | (utfStr[i + 1] & MASKBITS);
            }

            i += 2;
        }

        // 0xxxxxxx (utfStr[i] < MASKBYTE)
        else 
        {
            CVF_TIGHT_ASSERT(utfStr[i] >= 0);
            unicodeChar = static_cast<unsigned int>(utfStr[i]);
            i += 1;
        }

        wchar_t uc = static_cast<wchar_t>(unicodeChar);
        uStr.push_back(uc);
    }

    return String(uStr);
}


//--------------------------------------------------------------------------------------------------
/// Returns a null-terminated sequence of wchar_t characters
//--------------------------------------------------------------------------------------------------
const wchar_t* String::c_str() const
{
    return m_string.c_str();
}


//--------------------------------------------------------------------------------------------------
/// Convert the given number to a string with the specified format
///
/// \param     n            The number to convert
/// \param     format       'g': default, 'e' : scientific notation (1.234e4). 'f' : Fixed notation (1234.0)
/// \param     precision    The precision for floating-point values. Only used for 'f' and 'e'
/// 
/// \return A string with the given number
//--------------------------------------------------------------------------------------------------
String String::number(float n, char format, int precision)
{
    std::wstringstream sstr;
    
    switch(format)
    {
        case 'g' : sstr << n; break;
        case 'f' : sstr << std::fixed << std::setprecision(precision) << n; break;
        case 'e' : sstr << std::scientific << std::setprecision(precision) << n; break;
    }

    std::wstring str =  sstr.str();
    return str;
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to a double value (if possible)
/// 
/// \param ok  If not NULL, this will be set to true if conversion is ok, or to false if not
/// 
/// \return  Returns the double value found at the start of the string. 0.0 if an error occurred.
//--------------------------------------------------------------------------------------------------
double String::toDouble(bool* ok) const
{
    double val = 0;
    std::wstringstream stream(m_string);
    stream >> val;

    bool convertOk = !stream.fail();

    if (ok)
    {
        *ok = convertOk;
    }

    if (convertOk)
    {
        return val;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to a double value (if possible)
/// 
/// \param defaultValue  The value returned if the conversion failed.
/// 
/// \return  Returns the double value found at the start of the string or defaultValue if the 
///          conversion was not possible.
//--------------------------------------------------------------------------------------------------
double String::toDouble(double defaultValue) const
{
    bool ok = false;
    double val = toDouble(&ok);
    if (ok)
    {
        return val;
    }
    else
    {
        return defaultValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to a float value (if possible)
/// 
/// \param ok  If not NULL, this will be set to true if conversion is ok, or to false if not
/// 
/// \return  Returns the float value found at the start of the string. 0.0f if an error occurred.
//--------------------------------------------------------------------------------------------------
float String::toFloat(bool* ok) const
{
    float val = 0;
    std::wstringstream stream(m_string);
    stream >> val;

    bool convertOk = !stream.fail();

    if (ok)
    {
        *ok = convertOk;
    }

    if (convertOk)
    {
        return val;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to a float value (if possible)
/// 
/// \param defaultValue  The value returned if the conversion failed.
/// 
/// \return  Returns the float value found at the start of the string or defaultValue if the 
///          conversion was not possible.
//--------------------------------------------------------------------------------------------------
float String::toFloat(float defaultValue) const
{
    bool ok = false;
    float val = toFloat(&ok);
    if (ok)
    {
        return val;
    }
    else
    {
        return defaultValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to a integer value (if possible)
/// 
/// \param ok  If not NULL, this will be set to true if conversion is ok, or to false if not
/// 
/// \return  Returns the integer value found at the start of the string. 0 if an error occurred.
//--------------------------------------------------------------------------------------------------
int String::toInt(bool* ok) const
{
    int val = 0;
    std::wstringstream stream(m_string);
    stream >> val;

    bool convertOk = !stream.fail();

    if (ok)
    {
        *ok = convertOk;
    }

    if (convertOk)
    {
        return val;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to a integer value (if possible)
/// 
/// \param defaultValue  The value returned if the conversion failed.
/// 
/// \return  Returns the integer value found at the start of the string or defaultValue if the 
///          conversion was not possible.
//--------------------------------------------------------------------------------------------------
int String::toInt(int defaultValue) const
{
    bool ok = false;
    int val = toInt(&ok);
    if (ok)
    {
        return val;
    }
    else
    {
        return defaultValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to an unsigned integer value 
/// 
/// \param ok  If not NULL, this will be set to true if conversion is ok, or to false if not
/// 
/// \return  Returns the unsigned integer value found at the start of the string. 0 if an error occurred.
//--------------------------------------------------------------------------------------------------
uint String::toUInt(bool* ok) const
{
    // The functions in the standard library does not honor unsignedness but gladly converts a negative integer
    // to an unsigned integer, so use our int64 implementation instead
    bool convertOk = false;
    int64 val = toInt64(&convertOk);
    if (convertOk)
    {
        if (val > std::numeric_limits<uint>::max() || val < 0)
        {
            convertOk = false;
        }
    }

    if (ok)
    {
        *ok = convertOk;
    }

    if (convertOk)
    {
        return static_cast<uint>(val);
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the contents of the string to an unsigned integer value
/// 
/// \param defaultValue  The value returned if the conversion failed.
/// 
/// \return  Returns the value found at the start of the string or defaultValue if the 
///          conversion was not possible.
//--------------------------------------------------------------------------------------------------
uint String::toUInt(uint defaultValue) const
{
    bool ok = false;
    uint val = toUInt(&ok);
    if (ok)
    {
        return val;
    }
    else
    {
        return defaultValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int64 String::toInt64(bool* ok) const
{
    int64 val = 0;
    std::wstringstream stream(m_string);
    stream >> val;

    bool convertOk = !stream.fail();

    if (ok)
    {
        *ok = convertOk;
    }

    if (convertOk)
    {
        return val;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int64 String::toInt64(int64 defaultValue) const
{
    bool ok = false;
    int64 val = toInt64(&ok);
    if (ok)
    {
        return val;
    }
    else
    {
        return defaultValue;
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert the given number to a string with the specified format
///
/// \param     n            The number to convert
/// \param     format       'g': default, 'e' : scientific notation (1.234e4). 'f' : Fixed notation (1234.0)
/// \param     precision    The precision for floating-point values. Only used for 'f' and 'e'
/// 
/// \return A string with the given number
//--------------------------------------------------------------------------------------------------
String String::number(double n, char format, int precision)
{
    std::wstringstream sstr;

    switch(format)
    {
        case 'g' : sstr << n; break;
        case 'f' : sstr << std::fixed << std::setprecision(precision) << n; break;
        case 'e' : sstr << std::scientific << std::setprecision(precision) << n; break;
    }

    std::wstring str =  sstr.str();
    return str;
}


//--------------------------------------------------------------------------------------------------
///  To allow String s1 = "Test" + s2;
//--------------------------------------------------------------------------------------------------
String operator+(const char* str1, const String& str2)
{ 
    return String(str1) + str2; 
}


//--------------------------------------------------------------------------------------------------
/// Create a collection of token string based on specified delimiters.
///
/// \param      delimiters  String containing characters used to split into token strings
//--------------------------------------------------------------------------------------------------
std::vector<String> String::split(const String& delimiters) const
{
    std::vector<String> tokens;
    if (size() == 0) return tokens;
    
    std::wstring stdString = m_string;
    std::wstring stdDelimiters = delimiters.toStdWString();

    std::wstring::size_type lastPos = stdString.find_first_not_of(stdDelimiters, 0);
    std::wstring::size_type pos     = stdString.find_first_of(stdDelimiters, lastPos);

    while (std::wstring::npos != pos || std::wstring::npos != lastPos)
    {
        std::wstring stdToken = stdString.substr(lastPos, pos - lastPos);
        tokens.push_back(String(stdToken));
        lastPos = stdString.find_first_not_of(stdDelimiters, pos);
        pos = stdString.find_first_of(stdDelimiters, lastPos);
    }

    return tokens;
}


//--------------------------------------------------------------------------------------------------
/// Returns the position of the first occurrence of str starting from start
/// Returns String::npos if not found
//--------------------------------------------------------------------------------------------------
size_t String::find(const String& str, size_t start) const
{
    std::wstring stdFindStr= str.toStdWString();
    std::wstring::size_type pos = m_string.find(stdFindStr, start);

    if (pos == std::string::npos)
    {
        return npos;
    }

    return pos;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool String::startsWith(const String& str) const
{
    return (find(str) == 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String String::subString(size_t pos, size_t length) const
{
    CVF_ASSERT(pos < size());

    return m_string.substr(pos, length);
}


//--------------------------------------------------------------------------------------------------
/// Replaces occurrences of the string \a before with the string \a after
//--------------------------------------------------------------------------------------------------
void String::replace(const String& before, const String& after)
{
    // Guard against empty string to avoid endless loop
    if (before.isEmpty())
    {
        return;
    }

    // Try and find the first match, next is npos if nothing was found
    std::wstring::size_type next = m_string.find(before.m_string);
    while (next != std::wstring::npos)
    {
        // Inside the loop, so we found a match. Do the replacement.
        m_string.replace(next, before.m_string.length(), after.m_string);   
        
        // Move to just after the replace
        // This is the point were we start the next search from. 
        next += after.m_string.length();

        // Search for the next match starting after the last match that was found.
        next = m_string.find(before.m_string, next);
    }
}


//--------------------------------------------------------------------------------------------------
/// Convert a wchar_t to a single digit. Return -1 if not between 0-9
//--------------------------------------------------------------------------------------------------
int digitValue(const wchar_t& character)
{
    int val = character - '0';
    if (val < 0 || val > 9)
    {
        val = -1;
    }

    return val;
}


// Local helper struct for storing found arg info
struct ArgInfo
{
    int smallestArgIndex;           // lowest %x sequence number
    int smallestArgCount;           // number of occurrences of the lowest #x sequence number
    int totalArgLength;             // total length of %x sequences which will be replaced
};


//--------------------------------------------------------------------------------------------------
/// Find the smallest %x, and count the number of occurrences and the total length used by them
//--------------------------------------------------------------------------------------------------
static ArgInfo findSmallestArgs(const String &s)
{
    const wchar_t* strBegin = s.c_str();
    const wchar_t* strEnd = strBegin + s.size();

    ArgInfo argInfo;

    argInfo.smallestArgIndex = UNDEFINED_INT;
    argInfo.smallestArgCount = 0;
    argInfo.totalArgLength = 0;

    const wchar_t* c = strBegin;
    while (c != strEnd) 
    {
        while (c != strEnd && *c != '%')
        {
            ++c;
        }

        if (c == strEnd)
        {
            break;
        }

        const wchar_t* argStart = c;

        if (++c == strEnd)
        {
            break;
        }

        int argNumber = digitValue(*c);

        if (argNumber == -1)
        {
            continue;
        }

        ++c;

        int secondArgDigit = digitValue(*c);

        if (c != strEnd && secondArgDigit != -1) 
        {
            argNumber = (10*argNumber) + secondArgDigit;
            ++c;
        }

        if (argNumber > argInfo.smallestArgIndex)
        {
            continue;
        }

        if (argNumber < argInfo.smallestArgIndex) 
        {
            argInfo.smallestArgIndex = argNumber;
            argInfo.smallestArgCount = 0;
            argInfo.totalArgLength = 0;
        }

        ++argInfo.smallestArgCount;
        argInfo.totalArgLength += static_cast<int>(c - argStart);
    }

    return argInfo;
}


//--------------------------------------------------------------------------------------------------
/// Return a string where the %x (where x=info.smallestArgIndex) is replaced with the given value
//--------------------------------------------------------------------------------------------------
static String replaceArgs(const String &s, const ArgInfo& info, int fieldWidth, const String& arg, const wchar_t& fillChar)
{
    const wchar_t* strBegin = s.c_str();
    const wchar_t* strEnd = strBegin + s.size();

    unsigned int absFieldWidth = static_cast<unsigned int>(Math::abs(fieldWidth));
    size_t resultLength = s.size() - info.totalArgLength + info.smallestArgCount*CVF_MAX(absFieldWidth, arg.size());

    std::wstring resultString;
    resultString.resize(resultLength);
    wchar_t* resultBuffer = &resultString[0];
    wchar_t* rc = resultBuffer;
    const wchar_t*  c = strBegin;
    int repl_cnt = 0;

    while (c != strEnd) 
    {
        const wchar_t *textStart = c;

        while (*c != '%')
        {
            ++c;
        }

        const wchar_t* argStart = c++;

        int argIdx = digitValue(*c);

        if (argIdx != -1) 
        {
            if (c + 1 != strEnd && digitValue(*(c + 1)) != -1) 
            {
                argIdx = (10*argIdx) + digitValue(*(c + 1));
                ++c;
            }
        }

        if (argIdx != info.smallestArgIndex) 
        {
            memcpy(rc, textStart, (c - textStart)*sizeof(wchar_t));
            rc += c - textStart;
        }
        else 
        {
            ++c;

            memcpy(rc, textStart, (argStart - textStart)*sizeof(wchar_t));
            rc += argStart - textStart;

            size_t pad_chars = CVF_MAX(absFieldWidth, arg.size()) - arg.size();

            if (fieldWidth > 0) 
            { 
                // left padded
                unsigned int i;
                for (i = 0; i < pad_chars; ++i)
                {
                    *(rc++) = fillChar;;
                }
            }

            memcpy(rc, arg.c_str(), arg.size()*sizeof(wchar_t));
            rc += arg.size();

            if (fieldWidth < 0) 
            {
                // right padded
                unsigned int i;
                for (i = 0; i < pad_chars; ++i)
                {
                    *(rc++) = fillChar;
                }
            }

            if (++repl_cnt == info.smallestArgCount) 
            {
                memcpy(rc, c, (strEnd - c)*sizeof(wchar_t));
                rc += strEnd - c;
                CVF_ASSERT(((rc - resultBuffer) >= 0) && (static_cast<size_t>(rc - resultBuffer) == resultLength));
                c = strEnd;
            }
        }
    }

    CVF_ASSERT(rc == resultBuffer + resultLength);

    return String(resultString);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by string a.
/// 
/// \param a            The string to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param fillChar     The character that will be inserted if the passed string is shorter than
///                     the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(const String& a, int fieldWidth, const wchar_t& fillChar) const
{
    ArgInfo info = findSmallestArgs(*this);

    if (info.smallestArgCount == 0)
    {
        // Show warning??
        return *this;
    }

    return replaceArgs(*this, info, fieldWidth, a, fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by the integer a.
/// 
/// \param a            The character to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param fillChar     The character that will be inserted if the string representation of a is shorter 
///                     than the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(char a, int fieldWidth, const wchar_t& fillChar) const
{
    return arg(String(a), fieldWidth, fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by the integer a.
/// 
/// \param a            The unsigned int value to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param fillChar     The character that will be inserted if the string representation of a is shorter 
///                     than the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(uint a, int fieldWidth, const wchar_t& fillChar) const
{
    return arg(String(a), fieldWidth, fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by the integer a.
/// 
/// \param a            The int value to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param fillChar     The character that will be inserted if the string representation of a is shorter 
///                     than the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(int a, int fieldWidth, const wchar_t& fillChar) const
{
    return arg(static_cast<int64>(a), fieldWidth, fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by the 64 bit integer a.
/// 
/// \param a            The int64 value to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param fillChar     The character that will be inserted if the string representation of a is shorter 
///                     than the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(int64 a, int fieldWidth, const wchar_t& fillChar) const
{
    ArgInfo info = findSmallestArgs(*this);
    
    if (info.smallestArgCount == 0)
    {
        // Show warning??
        return *this;
    }

    return replaceArgs(*this, info, fieldWidth, String(a), fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by the float a.
/// 
/// \param a            The float value to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param format       'g': default, 'e' : scientific notation (1.234e4). 'f' : Fixed notation (1234.0)
/// \param precision    The precision for floating-point values. Only used for 'f' and 'e'
/// \param fillChar     The character that will be inserted if the string representation of a is shorter 
///                     than the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(float a, int fieldWidth, char format, int precision, const wchar_t& fillChar) const
{
    ArgInfo info = findSmallestArgs(*this);

    if (info.smallestArgCount == 0)
    {
        // Show warning??
        return *this;
    }

    return replaceArgs(*this, info, fieldWidth, String::number(a, format, precision), fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Returns a copy of this string with the lowest numbered place marker (e.g. %1, %2,..%99) replaced 
/// by the double a.
/// 
/// \param a            The double value to insert at the lowest %x
/// \param fieldWidth   The minimal number of characters the argument will occupy. Positive for right
///                     aligned text, negative for left aligned text.
/// \param format       'g': default, 'e' : scientific notation (1.234e4). 'f' : Fixed notation (1234.0)
/// \param precision    The precision for floating-point values. Only used for 'f' and 'e'
/// \param fillChar     The character that will be inserted if the string representation of a is shorter 
///                     than the specified fieldWidth. If the length of a is shorter than fieldWidth, 
///                     the string will be padded with this character.
///
/// This method searches the current string for the lowest %x value, and then replaces all of the 
/// lowest occurrence with the passed data.
/// Example of use:
/// \code
/// String test = String("Reading file %1 (%2 of %3)").arg(filename).arg(fileIndex + 1).arg(fileCount); 
/// \endcode
//--------------------------------------------------------------------------------------------------
String String::arg(double a, int fieldWidth, char format, int precision, const wchar_t& fillChar) const
{
    ArgInfo info = findSmallestArgs(*this);

    if (info.smallestArgCount == 0)
    {
        // Show warning??
        return *this;
    }

    return replaceArgs(*this, info, fieldWidth, String::number(a, format, precision), fillChar);
}


//--------------------------------------------------------------------------------------------------
/// Exchanges the contents of the two strings.
/// 
/// \param other  Modifiable reference to the string that should have its contents swapped.
/// 
/// \warning Note that signature differs from normal practice. This is done to be 
///          consistent with the signature of std::swap()
//--------------------------------------------------------------------------------------------------
void String::swap(String& other)
{
    m_string.swap(other.m_string);
}


} // namespace cvf

