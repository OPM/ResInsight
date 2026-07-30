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

#pragma once

#include <ostream>
#include <string>
#include <vector>

//==================================================================================================
/// Writer for binary ROFF (RMS open file format) files.
///
/// Produces the format read by ThirdParty/roffcpp: the file type keyword "roff-bin" followed by
/// tag groups, where every keyword, name and string value is a null-terminated token and numeric
/// values are stored as little-endian binary directly after the value name.
///
/// Typical use:
///     std::ofstream stream( fileName, std::ios::binary );
///     RifRoffWriter writer( stream );
///     writer.writeFileType();
///     writer.writeFileDataTag( "parameter" );
///     writer.writeVersionTag();
///     writer.startTag( "dimensions" );
///     writer.writeInt( "nX", 10 );
///     ...
///     writer.endTag();
///     writer.writeEofTag();
//==================================================================================================
class RifRoffWriter
{
public:
    explicit RifRoffWriter( std::ostream& stream );

    void writeFileType();

    void startTag( const std::string& tagName );
    void endTag();

    void writeString( const std::string& keyName, const std::string& value );
    void writeInt( const std::string& keyName, int value );
    void writeBool( const std::string& keyName, bool value );
    void writeByte( const std::string& keyName, unsigned char value );
    void writeFloat( const std::string& keyName, float value );
    void writeDouble( const std::string& keyName, double value );

    void writeIntArray( const std::string& keyName, const std::vector<int>& values );
    void writeFloatArray( const std::string& keyName, const std::vector<float>& values );
    void writeDoubleArray( const std::string& keyName, const std::vector<double>& values );
    void writeByteArray( const std::string& keyName, const std::vector<char>& values );
    void writeStringArray( const std::string& keyName, const std::vector<std::string>& values );

    // Standard ROFF header and footer tags
    void writeFileDataTag( const std::string& fileType );
    void writeVersionTag();
    void writeEofTag();

private:
    void writeToken( const std::string& token );
    void writeArrayHeader( const std::string& elementType, const std::string& keyName, int elementCount );
    void writeBinary( const void* data, size_t sizeInBytes );

    std::ostream& m_stream;
};
