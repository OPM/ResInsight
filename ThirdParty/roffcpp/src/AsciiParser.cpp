#include "AsciiParser.hpp"

#include "RoffScalar.hpp"

#include <cassert>
#include <cctype>
#include <stdexcept>
#include <variant>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AsciiParser::AsciiParser()
    : Parser()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string AsciiParser::parseString( const Token& token, std::istream& stream ) const
{
    stream.clear();
    stream.seekg( token.start() );
    size_t      length = token.end() - token.start();
    std::string res;
    res.resize( length );
    stream.read( const_cast<char*>( res.data() ), length );
    return res;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int AsciiParser::parseInt( const Token& token, std::istream& stream ) const
{
    std::string res = parseString( token, stream );
    return std::stoi( res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double AsciiParser::parseDouble( const Token& token, std::istream& stream ) const
{
    std::string res = parseString( token, stream );
    return std::stod( res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float AsciiParser::parseFloat( const Token& token, std::istream& stream ) const
{
    std::string res = parseString( token, stream );
    return std::stof( res );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AsciiParser::parseBool( const Token& token, std::istream& stream ) const
{
    return static_cast<bool>( parseInt( token, stream ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned char AsciiParser::parseByte( const Token& token, std::istream& stream ) const
{
    return static_cast<unsigned char>( parseInt( token, stream ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> AsciiParser::parseStringArray( const std::vector<Token>& tokens,
                                                        std::istream&             stream,
                                                        long                      startIndex,
                                                        long                      arrayLength ) const
{
    std::vector<std::string> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( parseString( tokens[i], stream ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int>
    AsciiParser::parseIntArray( const std::vector<Token>& tokens, std::istream& stream, long startIndex, long arrayLength ) const
{
    std::vector<int> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( parseInt( tokens[i], stream ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<char> AsciiParser::parseByteArray( const std::vector<Token>& tokens,
                                               std::istream&             stream,
                                               long                      startIndex,
                                               long                      arrayLength ) const
{
    std::vector<char> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( static_cast<char>( parseInt( tokens[i], stream ) ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> AsciiParser::parseFloatArray( const std::vector<Token>& tokens,
                                                 std::istream&             stream,
                                                 long                      startIndex,
                                                 long                      arrayLength ) const
{
    std::vector<float> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( static_cast<float>( parseDouble( tokens[i], stream ) ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> AsciiParser::parseDoubleArray( const std::vector<Token>& tokens,
                                                   std::istream&             stream,
                                                   long                      startIndex,
                                                   long                      arrayLength ) const
{
    std::vector<double> values;
    for ( long i = startIndex; i < startIndex + arrayLength; i++ )
    {
        values.push_back( parseDouble( tokens[i], stream ) );
    }

    return values;
}
