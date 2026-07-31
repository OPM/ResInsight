/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>

#include <optional>

namespace caf
{
class PdmObjectHandle;
}

//==================================================================================================
///
/// Interactive resolution of name conflicts between siblings in the project tree.
///
//==================================================================================================
namespace RiuNameConflictTools
{
// Returns the name to apply, or nothing if the user cancelled the rename.
//
// If desiredName is already unique among the siblings of the object, it is returned unchanged.
// Otherwise the user is asked to accept an auto-generated unique name. When no GUI is available
// (console mode, regression tests, scripting) the unique name is returned without prompting.
std::optional<QString> resolveRenameConflict( const caf::PdmObjectHandle* object, const QString& desiredName );

}; // namespace RiuNameConflictTools
