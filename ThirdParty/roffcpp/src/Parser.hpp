#pragma once

#include "Token.hpp"

#include <istream>
#include <variant>
#include <vector>

typedef std::variant<int, float, double, unsigned char, bool, std::string> RoffScalar;

class Parser
{
public:
    Parser( std::istream& stream, const std::vector<Token>& tokens );

    std::vector<std::pair<std::string, RoffScalar>> scalarNamedValues();

    std::pair<std::string, RoffScalar> parseSimpleType( std::vector<Token>::const_iterator& it,
                                                        const std::string&                  tagGroupName );

    static bool isSimpleType( Token::Kind kind );

    static std::string parseString( const Token& token, std::istream& stream );

    static int parseInt( const Token& token, std::istream& stream );

    static double parseDouble( const Token& token, std::istream& stream );

private:
    const std::vector<Token>* m_tokens;
    std::istream*             m_stream;
};
