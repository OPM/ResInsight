//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextStream>

#include "cafPdmUiItem.h"
#include "cafPdmObjectFactory.h"

#include "cafPdmFieldImpl.h"

#include "cvfColor3.h"

namespace caf 
{

    // Forward declarations
    template <typename T> class PdmField;


//==================================================================================================
/// Partial specialization for PdmField<  cvf::Color3f >
//==================================================================================================

template <>
class PdmFieldTypeSpecialization < cvf::Color3f >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const cvf::Color3f& value)
    {
        QColor col;
        col.setRgbF(value.r(), value.g(), value.b());

        return col;
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, cvf::Color3f& value)
    {
        QColor col = variantValue.value<QColor>();

        value.set(col.redF(), col.greenF(), col.blueF());
    }

    /// Methods to get a list of options for a field
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const cvf::Color3f& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField< cvf::Color3f >& field, std::vector<PdmObject*>* objects)
    { }

};

}

//==================================================================================================
/// QTextStream Stream operator for cvf::Color3f
//==================================================================================================

void operator >> (QTextStream& str, cvf::Color3f& value);
void operator << (QTextStream& str, const cvf::Color3f& value);
