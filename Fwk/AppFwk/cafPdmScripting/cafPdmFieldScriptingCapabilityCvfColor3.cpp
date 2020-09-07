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
#include "cafPdmFieldScriptingCapabilityCvfColor3.h"

#include <QColor>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<cvf::Color3f>::writeToField( cvf::Color3f&             fieldValue,
                                                                       QTextStream&              inputStream,
                                                                       caf::PdmScriptIOMessages* errorMessageContainer,
                                                                       bool                      stringsAreQuoted )
{
    QString fieldStringValue;
    PdmFieldScriptingCapabilityIOHandler<QString>::writeToField( fieldStringValue,
                                                                 inputStream,
                                                                 errorMessageContainer,
                                                                 stringsAreQuoted );

    QColor qColor( fieldStringValue );
    if ( qColor.isValid() )
    {
        fieldValue = cvf::Color3f( qColor.redF(), qColor.greenF(), qColor.blueF() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptingCapabilityIOHandler<cvf::Color3f>::readFromField( const cvf::Color3f& fieldValue,
                                                                        QTextStream&        outputStream,
                                                                        bool                quoteStrings,
                                                                        bool                quoteNonBuiltin )
{
    QColor  qColor( fieldValue.rByte(), fieldValue.gByte(), fieldValue.bByte() );
    QString fieldStringValue = qColor.name();
    PdmFieldScriptingCapabilityIOHandler<QString>::readFromField( fieldStringValue, outputStream, quoteStrings );
}
