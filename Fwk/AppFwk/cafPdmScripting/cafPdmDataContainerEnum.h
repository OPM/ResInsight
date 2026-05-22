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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

namespace caf
{
//==================================================================================================
/// Container returned by scriptable methods that produce a single enum value. The Python code
/// generator detects methods declaring an enum return type (see PdmEnumObjectMethod) and wraps
/// the container's `value` field into the corresponding Python StrEnum at the call site.
//==================================================================================================
class PdmDataContainerEnum : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    PdmDataContainerEnum();

    PdmField<QString> m_value;
};

} // namespace caf
