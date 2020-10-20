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
#include "cvfColor3.h"

#include "cafInternalPdmValueFieldSpecializations.h"
#include "cafPdmXmlColor3f.h"

#include <QColor>

namespace caf
{
//==================================================================================================
/// Partial specialization for PdmValueFieldSpecialization< cvf::Color3f >
//==================================================================================================

template <>
class PdmValueFieldSpecialization<cvf::Color3f>
{
public:
    static QVariant convert( const cvf::Color3f& value )
    {
        QColor col;
        col.setRgbF( value.r(), value.g(), value.b() );

        return col;
    }

    static void setFromVariant( const QVariant& variantValue, cvf::Color3f& value )
    {
        QColor col = variantValue.value<QColor>();

        value.set( col.redF(), col.greenF(), col.blueF() );
    }

    static bool isEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return variantValue == variantValue2;
    }
};

} // end namespace caf
