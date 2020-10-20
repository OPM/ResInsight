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
#include "cafPdmUiFieldSpecialization.h"
#include "cafPdmUiItem.h"

#include "cafPdmCoreColor3f.h"

namespace caf
{
template <>
class PdmUiFieldSpecialization<cvf::Color3f>
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert( const cvf::Color3f& value )
    {
        return PdmValueFieldSpecialization<cvf::Color3f>::convert( value );
    }

    /// Set the field value from a QVariant
    static void setFromVariant( const QVariant& variantValue, cvf::Color3f& value )
    {
        PdmValueFieldSpecialization<cvf::Color3f>::setFromVariant( variantValue, value );
    }

    static bool isDataElementEqual( const QVariant& variantValue, const QVariant& variantValue2 )
    {
        return PdmValueFieldSpecialization<cvf::Color3f>::isEqual( variantValue, variantValue2 );
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const cvf::Color3f& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects( const PdmDataValueField<cvf::Color3f>&, std::vector<PdmObjectHandle*>* ) {}
};

} // end namespace caf

//--------------------------------------------------------------------------------------------------
// If the macro for registering the editor is put as the single statement
// in a cpp file, a dummy static class must be used to make sure the compile unit
// is included
//--------------------------------------------------------------------------------------------------
class PdmColor3fInitializer
{
public:
    PdmColor3fInitializer();
};
static PdmColor3fInitializer pdmColor3fInitializer;
