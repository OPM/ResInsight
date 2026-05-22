//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2026- Equinor ASA
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

#include "cafAppEnum.h"

#include <QString>

#include <map>
#include <typeinfo>

namespace caf
{
//==================================================================================================
/// Optional override for the Python StrEnum class name the code generator emits for a given
/// caf::AppEnum<T>. Without an override, the generator derives the class name from the script
/// keyword of the first field it visits — which is sensitive to class/iteration order. Use this
/// registry to pin a canonical Python name for an AppEnum, independent of any field's script
/// keyword.
//==================================================================================================
class PdmScriptEnumNameRegistry
{
public:
    template <typename EnumType>
    static void registerName( const QString& scriptClassName )
    {
        registry()[QString::fromLatin1( typeid( caf::AppEnum<EnumType> ).name() )] = scriptClassName;
    }

    static QString lookup( const QString& dataTypeName )
    {
        const auto& m  = registry();
        auto        it = m.find( dataTypeName );
        if ( it == m.end() ) return QString();
        return it->second;
    }

private:
    static std::map<QString, QString>& registry()
    {
        static std::map<QString, QString> s_map;
        return s_map;
    }
};

} // namespace caf
