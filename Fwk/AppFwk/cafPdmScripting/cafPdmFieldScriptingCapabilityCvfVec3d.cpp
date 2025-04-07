//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<cvf::Vector3<double>>::writeToField( cvf::Vector3<double>& fieldValue,
                                                                               QTextStream&          inputStream,
                                                                               caf::PdmScriptIOMessages* errorMessageContainer,
                                                                               bool stringsAreQuoted )
{
    std::vector<double> fieldVectorValue;
    PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::writeToField( fieldVectorValue,
                                                                             inputStream,
                                                                             errorMessageContainer,
                                                                             stringsAreQuoted );
    if ( fieldVectorValue.size() == 3u )
    {
        for ( int i = 0; i < 3; ++i )
        {
            fieldValue[i] = fieldVectorValue[i];
        }
    }
    else
    {
        QString errMsg = QString( "Expected three dimensions in the vector, got %1" ).arg( fieldVectorValue.size() );
        errorMessageContainer->addError( errMsg );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<cvf::Vector3<double>>::readFromField( const cvf::Vector3<double>& fieldValue,
                                                                                QTextStream& outputStream,
                                                                                bool         quoteStrings,
                                                                                bool         quoteNonBuiltin )
{
    std::vector<double> fieldVectorValue( 3u );
    for ( int i = 0; i < 3; ++i )
    {
        fieldVectorValue[i] = fieldValue[i];
    }

    PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::readFromField( fieldVectorValue, outputStream, quoteStrings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<std::vector<cvf::Vector3<double>>>::writeToField(
    std::vector<cvf::Vector3<double>>& fieldValue,
    QTextStream&                       inputStream,
    caf::PdmScriptIOMessages*          errorMessageContainer,
    bool                               stringsAreQuoted )
{
    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
    QChar chr = errorMessageContainer->readCharWithLineNumberCount( inputStream );
    if ( chr == QChar( '[' ) )
    {
        while ( !inputStream.atEnd() )
        {
            std::vector<double> fieldVectorValue;

            PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::writeToField( fieldVectorValue,
                                                                                     inputStream,
                                                                                     errorMessageContainer,
                                                                                     stringsAreQuoted );
            if ( fieldVectorValue.size() == 3u )
            {
                fieldValue.push_back(
                    cvf::Vector3<double>( fieldVectorValue[0], fieldVectorValue[1], fieldVectorValue[2] ) );
            }
            else
            {
                QString errMsg =
                    QString( "Expected three dimensions in the vector, got %1" ).arg( fieldVectorValue.size() );
                errorMessageContainer->addError( errMsg );
                return;
            }

            errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
            QChar nextChar = errorMessageContainer->peekNextChar( inputStream );
            if ( nextChar == QChar( ']' ) )
            {
                nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                break;
            }
            else if ( nextChar == QChar( ',' ) )
            {
                nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<std::vector<cvf::Vector3<double>>>::readFromField(
    const std::vector<cvf::Vector3<double>>& fieldValue,
    QTextStream&                             outputStream,
    bool                                     quoteStrings,
    bool                                     quoteNonBuiltin )
{
    outputStream << "[";

    for ( size_t i = 0; i < fieldValue.size(); ++i )
    {
        const auto&         vec3             = fieldValue[i];
        std::vector<double> fieldVectorValue = { vec3.x(), vec3.y(), vec3.z() };
        PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::readFromField( fieldVectorValue,
                                                                                  outputStream,
                                                                                  quoteStrings );
        if ( i < fieldValue.size() - 1 )
        {
            outputStream << ", ";
        }
    }

    outputStream << "]";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<cvf::Matrix4<double>>::writeToField( cvf::Matrix4<double>& fieldValue,
                                                                               QTextStream&          inputStream,
                                                                               caf::PdmScriptIOMessages* errorMessageContainer,
                                                                               bool stringsAreQuoted )
{
    std::vector<double> fieldVectorValue;
    PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::writeToField( fieldVectorValue,
                                                                             inputStream,
                                                                             errorMessageContainer,
                                                                             stringsAreQuoted );
    if ( fieldVectorValue.size() == 16u )
    {
        for ( int row = 0; row < 4; ++row )
        {
            for ( int col = 0; col < 4; ++col )
            {
                fieldValue( row, col ) = fieldVectorValue[row * 4 + col];
            }
        }
    }
    else
    {
        QString errMsg = QString( "Expected 16 values, got %1" ).arg( fieldVectorValue.size() );
        errorMessageContainer->addError( errMsg );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<cvf::Matrix4<double>>::readFromField( const cvf::Matrix4<double>& fieldValue,
                                                                                QTextStream& outputStream,
                                                                                bool         quoteStrings,
                                                                                bool         quoteNonBuiltin )
{
    std::vector<double> fieldVectorValue( 16u );
    for ( int row = 0; row < 4; ++row )
    {
        for ( int col = 0; col < 4; ++col )
        {
            fieldVectorValue[row * 4 + col] = fieldValue( row, col );
        }
    }

    PdmFieldScriptingCapabilityIOHandler<std::vector<double>>::readFromField( fieldVectorValue, outputStream, quoteStrings );
}
