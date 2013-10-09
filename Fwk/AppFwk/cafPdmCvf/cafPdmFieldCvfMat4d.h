//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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
#include <QVariant>
#include <QList>
//#include <QStringList>
//#include <QXmlStreamReader>
//#include <QXmlStreamWriter>
#include <QTextStream>

//#include "cafPdmUiItem.h"
//#include "cafPdmObjectFactory.h"

#include "cafPdmFieldImpl.h"
#include "cvfAssert.h"
#include "cvfMatrix4.h"

namespace caf 
{

    // Forward declarations
    template <typename T> class PdmField;


//==================================================================================================
/// Partial specialization for PdmField<  cvf::Mat4d >
//==================================================================================================

template <>
class PdmFieldTypeSpecialization < cvf::Mat4d >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const cvf::Mat4d& value)
    {
        return QVariant();
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, cvf::Mat4d& value)
    {
        
    }

    /// Methods to get a list of options for a field
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const cvf::Mat4d& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField< cvf::Mat4d >& field, std::vector<PdmObject*>* objects)
    { }

};

}

//==================================================================================================
/// QTextStream Stream operator for cvf::Color3f
//==================================================================================================

void operator >> (QTextStream& str, cvf::Mat4d& value);
void operator << (QTextStream& str, const cvf::Mat4d& value);
