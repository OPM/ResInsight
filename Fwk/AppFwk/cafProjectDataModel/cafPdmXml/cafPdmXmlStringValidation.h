#pragma once

constexpr bool isLowerCase(char c) {
    return (c >= 'a' && c <= 'z');
}

constexpr bool isUpperCase(char c) {
    return (c >= 'A' && c <= 'Z');
}

constexpr bool isLetter(char c) {
    return isLowerCase(c) || isUpperCase(c);
}

constexpr bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

constexpr bool isValidFirstCharacterInXmlKeyword(char c) {
    return (c != '.') && !isDigit(c);
}

constexpr bool isValidCharacterInXmlKeyword(char c) {
    return  isLetter(c) || isDigit(c) || (c == '_');
}

template< unsigned N > constexpr
bool isValidXmlKeyword(const char(&arr)[N], unsigned i = 0)
{
    return (!isValidCharacterInXmlKeyword(arr[i])) ? false :
        i == N-2 ? true :
        isValidXmlKeyword(arr, i + 1);
}

template< unsigned N > constexpr
bool isFirstCharacterValidInXmlKeyword(const char(&arr)[N])
{
    static_assert(N>0, "String literal is too small");

    return isValidFirstCharacterInXmlKeyword(arr[0]);
}

template< unsigned N > constexpr
bool isFirstThreeCharactersXml(const char(&arr)[N])
{
    return (N < 3) ? false : arr[0] == 'x' && arr[1] == 'm' && arr[2] == 'l';
}

