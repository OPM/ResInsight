#pragma once

#include <string>

class Token
{
public:
    enum class Kind
    {
        ROFF_ASC,
        ROFF_BIN,
        TAG,
        ENDTAG,
        STRING_LITERAL,
        NUMERIC_VALUE,
        BINARY_NUMERIC_VALUE,
        NAME,
        CHAR,
        BOOL,
        BYTE,
        INT,
        FLOAT,
        DOUBLE,
        ARRAY,
        ARRAYBLOB,
    };

    Token( Kind, size_t start, size_t end );

    Kind   kind() const;
    size_t start() const;
    size_t end() const;

    static std::string kindToString( Kind );

private:
    Kind   m_kind;
    size_t m_start;
    size_t m_end;
};
