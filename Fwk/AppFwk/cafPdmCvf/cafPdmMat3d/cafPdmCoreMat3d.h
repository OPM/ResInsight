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

#pragma once

#include "cvfBase.h"
#include "cvfMatrix3.h"

#include "cafInternalPdmValueFieldSpecializations.h"

#include "cafPdmXmlMat3d.h"

namespace caf
{
template <>
class PdmValueFieldSpecialization<cvf::Mat3d>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const cvf::Mat3d& value )
    {
        QString str;

        QTextStream textStream( &str );
        textStream << value;

        return QVariant( str );
    }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, cvf::Mat3d& value )
    {
        QString str = variantValue.toString();

        QTextStream textStream( &str );

        textStream >> value;
    }

    static bool isEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return variantValue == variantValue2;
    }
};

} // end namespace caf
