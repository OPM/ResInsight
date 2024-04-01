//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2023 Ceetron Solutions AS
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

#include "cafPdmFieldHandle.h"
#include "cafPdmUiFieldHandleInterface.h"

namespace caf
{

// This is a template function, and auto is used to allow the compiler to deduce the type of the arguments
void setValueWithFieldChanged( auto fieldHandle, auto fieldValue )
{
    PdmUiFieldHandleInterface* uiFieldHandleInterface = fieldHandle->template capability<PdmUiFieldHandleInterface>();

    if ( uiFieldHandleInterface )
    {
        QVariant oldValue = uiFieldHandleInterface->toUiBasedQVariant();

        fieldHandle->setValue( fieldValue );

        QVariant newUiBasedQVariant = uiFieldHandleInterface->toUiBasedQVariant();

        uiFieldHandleInterface->notifyFieldChanged( oldValue, newUiBasedQVariant );
    }
    else
    {
        fieldHandle->setValue( fieldValue );
    }
}
} // end namespace caf
