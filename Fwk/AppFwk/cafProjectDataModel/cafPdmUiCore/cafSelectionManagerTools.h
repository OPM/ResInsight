//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafSelectionManager.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Get the single selected object from selection manager. Then, starting at this object, find the
/// first object of given type searching upwards to root using firstAncestorOrThisOfType()
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] T* firstAncestorOfTypeFromSelectedObject()
{
    static_assert( !std::is_pointer<T>::value );

    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        return objHandle->firstAncestorOrThisOfType<T>();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Return all objects of given type from the selection manager
///
/// Consider replace SelectionManager::objectsByType with this function when all consumers
/// have been updated to use this function instead of SelectionManager::objectsByType
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T> selectedObjectsByType()
{
    using UnderlyingType = typename std::remove_pointer<T>::type;
    return caf::SelectionManager::instance()->objectsByType<UnderlyingType>();
}

//--------------------------------------------------------------------------------------------------
/// Return all objects of given type from the selection manager.
/// If objects of different type are selected, an empty list is returned.
//--------------------------------------------------------------------------------------------------
template <typename T>
[[nodiscard]] std::vector<T> selectedObjectsByTypeStrict()
{
    using UnderlyingType = typename std::remove_pointer<T>::type;
    return caf::SelectionManager::instance()->objectsByTypeStrict<UnderlyingType>();
}

} // end namespace caf
