#pragma once

#include "Token.hpp"

#include "RoffScalar.hpp"

#include <istream>
#include <map>
#include <variant>
#include <vector>

class Parser
{
public:
    Parser();

    void parse( std::istream&                                     stream,
                const std::vector<Token>&                         tokens,
                std::vector<std::pair<std::string, RoffScalar>>&  scalarValues,
                std::vector<std::pair<std::string, Token::Kind>>& arrayTypes,
                std::map<std::string, std::pair<long, long>>&     arrayInfo ) const;

    std::pair<std::string, RoffScalar> parseSimpleType( std::vector<Token>::const_iterator& it,
                                                        const std::string&                  tagGroupName,
                                                        std::istream&                       stream ) const;

    virtual std::vector<std::string> parseStringArray( const std::vector<Token>& tokens,
                                                       std::istream&             stream,
                                                       long                      startIndex,
                                                       long                      arrayLength ) const = 0;

    virtual std::vector<int>
        parseIntArray( const std::vector<Token>& tokens, std::istream& stream, long startIndex, long arrayLength ) const = 0;

    virtual std::vector<double> parseDoubleArray( const std::vector<Token>& tokens,
                                                  std::istream&             stream,
                                                  long                      startIndex,
                                                  long                      arrayLength ) const = 0;

    virtual std::vector<float> parseFloatArray( const std::vector<Token>& tokens,
                                                std::istream&             stream,
                                                long                      startIndex,
                                                long                      arrayLength ) const = 0;

    virtual std::vector<char> parseByteArray( const std::vector<Token>& tokens,
                                              std::istream&             stream,
                                              long                      startIndex,
                                              long                      arrayLength ) const = 0;

    virtual std::string parseString( const Token& token, std::istream& stream ) const = 0;

    virtual int parseInt( const Token& token, std::istream& stream ) const = 0;

    virtual double parseDouble( const Token& token, std::istream& stream ) const = 0;

    static bool isSimpleType( Token::Kind kind );
};
