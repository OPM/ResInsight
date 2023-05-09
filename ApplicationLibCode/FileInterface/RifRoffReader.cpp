/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RifRoffReader.h"

#include <QFile>
#include <QTextStream>

#include "roffcpp/src/Parser.hpp"
#include "roffcpp/src/Reader.hpp"

#include <fstream>

void RifRoffReader::readCodeNames( const QString& filename, const QString& parameterTagName, std::map<int, QString>& codeNames )
{
    codeNames.clear();

    std::ifstream stream( filename.toStdString(), std::ios::binary );
    if ( !stream.good() )
    {
        throw RifRoffReaderException( "Unable to open roff file." );
    }

    roff::Reader reader( stream );
    reader.parse();

    std::vector<std::pair<std::string, roff::Token::Kind>> arrayTypes = reader.getNamedArrayTypes();

    // Find array types by keywords
    const std::string codeNamesKeyword  = parameterTagName.toStdString() + roff::Parser::postFixCodeNames();
    const std::string codeValuesKeyword = parameterTagName.toStdString() + roff::Parser::postFixCodeValues();
    auto              codeNamesItr      = std::find_if( arrayTypes.begin(),
                                      arrayTypes.end(),
                                      [&codeNamesKeyword]( const auto& arrayType ) { return arrayType.first == codeNamesKeyword; } );
    auto              codeValuesItr     = std::find_if( arrayTypes.begin(),
                                       arrayTypes.end(),
                                       [&codeValuesKeyword]( const auto& arrayType ) { return arrayType.first == codeValuesKeyword; } );

    if ( codeNamesItr == arrayTypes.end() ) throw RifRoffReaderException( "Code names not found." );
    if ( codeValuesItr == arrayTypes.end() ) throw RifRoffReaderException( "Code values not found." );

    const auto readCodeNames  = reader.getStringArray( codeNamesKeyword );
    const auto readCodeValues = reader.getIntArray( codeValuesKeyword );

    if ( readCodeNames.size() != readCodeValues.size() )
    {
        throw RifRoffReaderException( "Inconsistent code names and values: must be equal length." );
    }

    for ( size_t i = 0; i < readCodeNames.size(); ++i )
    {
        codeNames[readCodeValues[i]] = QString::fromStdString( readCodeNames[i] );
    }
}
