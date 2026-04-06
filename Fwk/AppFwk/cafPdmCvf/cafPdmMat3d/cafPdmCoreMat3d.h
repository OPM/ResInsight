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

#include "cafPdmFieldTraits.h"

#include "cafPdmXmlMat3d.h"

#include <QTextStream>

namespace caf
{

inline QVariant pdmToVariant( const cvf::Mat3d& value )
{
    QString     str;
    QTextStream textStream( &str );
    textStream << value;
    return QVariant( str );
}

inline void pdmFromVariant( const QVariant& v, cvf::Mat3d& out )
{
    QString     str = v.toString();
    QTextStream textStream( &str );
    textStream >> out;
}

template <>
struct PdmVariantEqualImpl<cvf::Mat3d>
{
    static bool equal( const QVariant& a, const QVariant& b ) { return a.toString() == b.toString(); }
};

} // end namespace caf
