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
