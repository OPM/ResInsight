#pragma once

#include "Token.hpp"

#include <istream>
#include <map>
#include <variant>
#include <vector>

typedef std::variant<int, float, double, unsigned char, bool, std::string> RoffScalar;

class Parser
{
public:
    Parser( std::istream& stream, const std::vector<Token>& tokens );

    void parse();

    std::vector<std::pair<std::string, RoffScalar>> scalarNamedValues() const;

    std::vector<std::pair<std::string, Token::Kind>> getNamedArrayTypes() const;

    std::pair<std::string, RoffScalar> parseSimpleType( std::vector<Token>::const_iterator& it,
                                                        const std::string&                  tagGroupName );

    std::vector<std::string> getStringArray( const std::string& keyword );
    std::vector<int>         getIntArray( const std::string& keyword );
    std::vector<double>      getDoubleArray( const std::string& keyword );
    std::vector<float>       getFloatArray( const std::string& keyword );

    static bool isSimpleType( Token::Kind kind );

    static std::string parseString( const Token& token, std::istream& stream );

    static int parseInt( const Token& token, std::istream& stream );

    static double parseDouble( const Token& token, std::istream& stream );

private:
    const std::vector<Token>*                        m_tokens;
    std::istream*                                    m_stream;
    std::vector<std::pair<std::string, RoffScalar>>  m_scalarValues;
    std::vector<std::pair<std::string, Token::Kind>> m_arrayTypes;
    std::map<std::string, std::pair<long, long>>     m_arrayInfo;
};
