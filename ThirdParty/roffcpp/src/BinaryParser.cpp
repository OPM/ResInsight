#include "BinaryParser.hpp"

#include "RoffScalar.hpp"

#include <cassert>
#include <cctype>
#include <stdexcept>
#include <variant>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
BinaryParser::BinaryParser()
    : Parser()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string BinaryParser::parseString( const Token& token, std::istream& stream ) const
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
int BinaryParser::parseInt( const Token& token, std::istream& stream ) const
{
    stream.clear();
    int  length = Token::binaryTokenSizeInBytes( Token::Kind::INT );
    auto start  = token.start();
    int  myint;
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( &myint ), length );
    return myint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double BinaryParser::parseDouble( const Token& token, std::istream& stream ) const
{
    stream.clear();
    int    length = Token::binaryTokenSizeInBytes( Token::Kind::DOUBLE );
    auto   start  = token.start();
    double value;
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( &value ), length );
    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float BinaryParser::parseFloat( const Token& token, std::istream& stream ) const
{
    stream.clear();
    int   length = Token::binaryTokenSizeInBytes( Token::Kind::FLOAT );
    auto  start  = token.start();
    float value;
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( &value ), length );
    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool BinaryParser::parseBool( const Token& token, std::istream& stream ) const
{
    return static_cast<bool>( parseInt( token, stream ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned char BinaryParser::parseByte( const Token& token, std::istream& stream ) const
{
    return static_cast<unsigned char>( parseInt( token, stream ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> BinaryParser::parseStringArray( const std::vector<Token>& tokens,
                                                         std::istream&             stream,
                                                         long                      startIndex,
                                                         long                      arrayLength ) const
{
    stream.clear();
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
std::vector<int> BinaryParser::parseIntArray( const std::vector<Token>& tokens,
                                              std::istream&             stream,
                                              long                      startIndex,
                                              long                      arrayLength ) const
{
    stream.clear();
    std::vector<int> values( arrayLength );
    int              length = Token::binaryTokenSizeInBytes( Token::Kind::INT );
    auto             start  = tokens[startIndex].start();
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( values.data() ), static_cast<std::streamsize>(arrayLength) * length );
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<char> BinaryParser::parseByteArray( const std::vector<Token>& tokens,
                                                std::istream&             stream,
                                                long                      startIndex,
                                                long                      arrayLength ) const
{
    stream.clear();
    std::vector<char> values( arrayLength );
    int               length = Token::binaryTokenSizeInBytes( Token::Kind::BYTE );
    auto              start  = tokens[startIndex].start();
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( values.data() ), static_cast<std::streamsize>(arrayLength) * length );
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> BinaryParser::parseFloatArray( const std::vector<Token>& tokens,
                                                  std::istream&             stream,
                                                  long                      startIndex,
                                                  long                      arrayLength ) const
{
    stream.clear();
    std::vector<float> values( arrayLength );
    int                length = Token::binaryTokenSizeInBytes( Token::Kind::FLOAT );
    auto               start  = tokens[startIndex].start();
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( values.data() ), static_cast<std::streamsize>(arrayLength) * length );
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> BinaryParser::parseDoubleArray( const std::vector<Token>& tokens,
                                                    std::istream&             stream,
                                                    long                      startIndex,
                                                    long                      arrayLength ) const
{
    stream.clear();
    std::vector<double> values( arrayLength );
    int                 length = Token::binaryTokenSizeInBytes( Token::Kind::DOUBLE );
    auto                start  = tokens[startIndex].start();
    stream.seekg( start );
    stream.read( reinterpret_cast<char*>( values.data() ), static_cast<std::streamsize>(arrayLength) * length );

    return values;
}
