/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RifRoffWriter.h"

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRoffWriter::RifRoffWriter( std::ostream& stream )
    : m_stream( stream )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeFileType()
{
    writeToken( "roff-bin" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::startTag( const std::string& tagName )
{
    writeToken( "tag" );
    writeToken( tagName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::endTag()
{
    writeToken( "endtag" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeString( const std::string& keyName, const std::string& value )
{
    writeToken( "char" );
    writeToken( keyName );
    writeToken( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeInt( const std::string& keyName, int value )
{
    writeToken( "int" );
    writeToken( keyName );
    writeBinary( &value, sizeof( value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeBool( const std::string& keyName, bool value )
{
    const unsigned char byteValue = value ? 1 : 0;

    writeToken( "bool" );
    writeToken( keyName );
    writeBinary( &byteValue, sizeof( byteValue ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeByte( const std::string& keyName, unsigned char value )
{
    writeToken( "byte" );
    writeToken( keyName );
    writeBinary( &value, sizeof( value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeFloat( const std::string& keyName, float value )
{
    writeToken( "float" );
    writeToken( keyName );
    writeBinary( &value, sizeof( value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeDouble( const std::string& keyName, double value )
{
    writeToken( "double" );
    writeToken( keyName );
    writeBinary( &value, sizeof( value ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeIntArray( const std::string& keyName, const std::vector<int>& values )
{
    writeArrayHeader( "int", keyName, static_cast<int>( values.size() ) );
    writeBinary( values.data(), values.size() * sizeof( int ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeFloatArray( const std::string& keyName, const std::vector<float>& values )
{
    writeArrayHeader( "float", keyName, static_cast<int>( values.size() ) );
    writeBinary( values.data(), values.size() * sizeof( float ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeDoubleArray( const std::string& keyName, const std::vector<double>& values )
{
    writeArrayHeader( "double", keyName, static_cast<int>( values.size() ) );
    writeBinary( values.data(), values.size() * sizeof( double ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeByteArray( const std::string& keyName, const std::vector<char>& values )
{
    writeArrayHeader( "byte", keyName, static_cast<int>( values.size() ) );
    writeBinary( values.data(), values.size() * sizeof( char ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeStringArray( const std::string& keyName, const std::vector<std::string>& values )
{
    writeArrayHeader( "char", keyName, static_cast<int>( values.size() ) );
    for ( const std::string& value : values )
    {
        writeToken( value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeFileDataTag( const std::string& fileType )
{
    const std::string creationDate = QDateTime::currentDateTime().toString( "dd/MM/yyyy hh:mm:ss" ).toStdString();

    startTag( "filedata" );
    writeInt( "byteswaptest", 1 );
    writeString( "filetype", fileType );
    writeString( "creationDate", creationDate );
    endTag();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeVersionTag()
{
    startTag( "version" );
    writeInt( "major", 2 );
    writeInt( "minor", 0 );
    endTag();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeEofTag()
{
    startTag( "eof" );
    endTag();
}

//--------------------------------------------------------------------------------------------------
/// All ROFF tokens, including string values, are written with a terminating null character
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeToken( const std::string& token )
{
    m_stream.write( token.c_str(), token.size() + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeArrayHeader( const std::string& elementType, const std::string& keyName, int elementCount )
{
    writeToken( "array" );
    writeToken( elementType );
    writeToken( keyName );
    writeBinary( &elementCount, sizeof( elementCount ) );
}

//--------------------------------------------------------------------------------------------------
/// Numeric values are written in the native little-endian byte order, as signaled by the
/// "filedata.byteswaptest" value
//--------------------------------------------------------------------------------------------------
void RifRoffWriter::writeBinary( const void* data, size_t sizeInBytes )
{
    m_stream.write( reinterpret_cast<const char*>( data ), sizeInBytes );
}
