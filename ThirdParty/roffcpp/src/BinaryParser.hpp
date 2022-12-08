#pragma once

#include "Parser.hpp"
#include "Token.hpp"

#include "RoffScalar.hpp"

#include <istream>
#include <map>
#include <variant>
#include <vector>

class BinaryParser : public Parser
{
public:
    BinaryParser();

    void parse( std::istream&                                     stream,
                const std::vector<Token>&                         tokens,
                std::vector<std::pair<std::string, RoffScalar>>&  scalarValues,
                std::vector<std::pair<std::string, Token::Kind>>& arrayTypes,
                std::map<std::string, std::pair<long, long>>&     arrayInfo ) const;

    std::pair<std::string, RoffScalar> parseSimpleType( std::vector<Token>::const_iterator& it,
                                                        const std::string&                  tagGroupName,
                                                        std::istream&                       stream ) const;

    std::vector<std::string> parseStringArray( const std::vector<Token>& tokens,
                                               std::istream&             stream,
                                               long                      startIndex,
                                               long                      arrayLength ) const override;

    std::vector<int> parseIntArray( const std::vector<Token>& tokens,
                                    std::istream&             stream,
                                    long                      startIndex,
                                    long                      arrayLength ) const override;

    std::vector<double> parseDoubleArray( const std::vector<Token>& tokens,
                                          std::istream&             stream,
                                          long                      startIndex,
                                          long                      arrayLength ) const override;

    std::vector<float> parseFloatArray( const std::vector<Token>& tokens,
                                        std::istream&             stream,
                                        long                      startIndex,
                                        long                      arrayLength ) const override;

    std::vector<char> parseByteArray( const std::vector<Token>& tokens,
                                      std::istream&             stream,
                                      long                      startIndex,
                                      long                      arrayLength ) const override;

    static bool isSimpleType( Token::Kind kind );

    std::string   parseString( const Token& token, std::istream& stream ) const override;
    int           parseInt( const Token& token, std::istream& stream ) const override;
    double        parseDouble( const Token& token, std::istream& stream ) const override;
    float         parseFloat( const Token& token, std::istream& stream ) const override;
    bool          parseBool( const Token& token, std::istream& stream ) const override;
    unsigned char parseByte( const Token& token, std::istream& stream ) const override;
};
