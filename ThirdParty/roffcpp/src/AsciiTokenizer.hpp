#pragma once

#include "Token.hpp"
#include "Tokenizer.hpp"

#include <istream>
#include <optional>
#include <vector>

class AsciiTokenizer : public Tokenizer
{
public:
    AsciiTokenizer()
        : Tokenizer(){};
    virtual ~AsciiTokenizer(){};

    std::vector<Token>   tokenizeStream( std::istream& stream ) override;
    bool                 tokenizeComment( std::istream& stream ) override;
    bool                 tokenizeSpace( std::istream& stream ) override;
    void                 tokenizeDelimiter( std::istream& stream ) override;
    std::optional<Token> tokenizeString( std::istream& stream ) override;
    std::optional<Token> tokenizeAsciiNumber( std::istream& stream ) override;
    Token                tokenizeName( std::istream& stream ) override;
    Token                tokenizeKeyword( std::istream& stream ) override;
    Token                tokenizeSimpleType( std::istream& stream ) override;
    std::optional<Token> tokenizeValue( std::istream& stream ) override;
    std::vector<Token>   tokenizeArrayData( std::istream& stream ) override;
    std::vector<Token>   tokenizeAsciiTagKey( std::istream& stream ) override;
    std::vector<Token>   tokenizeArrayAsciiTagKey( std::istream& stream ) override;
    std::vector<Token>   tokenizeTagGroup( std::istream& stream ) override;
    std::vector<Token>   tokenizeTagKey( std::istream& stream ) override;

    Token tokenizeKeyword( std::istream& stream, const std::vector<std::pair<Token::Kind, std::string>>& keywords ) override;
    std::optional<Token> tokenizeWord( std::istream& stream, const std::string& keywork, Token::Kind kind ) override;
};
