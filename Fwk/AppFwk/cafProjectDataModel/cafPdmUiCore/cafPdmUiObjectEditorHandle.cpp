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

#include "cafPdmUiObjectEditorHandle.h"

#include "cafPdmUiObjectHandle.h"

namespace caf
{

std::set<QPointer<PdmUiObjectEditorHandle>> PdmUiObjectEditorHandle::m_sRegisteredObjectEditors;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiObjectEditorHandle::PdmUiObjectEditorHandle()
{
    m_sRegisteredObjectEditors.insert(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiObjectEditorHandle::~PdmUiObjectEditorHandle()
{
    m_sRegisteredObjectEditors.erase(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiObjectEditorHandle::setPdmObject(PdmObjectHandle* object)
{
    cleanupBeforeSettingPdmObject();

    caf::PdmUiObjectHandle* uiObject = uiObj(object);
    this->bindToPdmItem(uiObject);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmUiObjectEditorHandle::pdmObject()
{
    PdmUiObjectHandle* uiObject = dynamic_cast<PdmUiObjectHandle*>(pdmItem());
    if (uiObject)
    {
        return uiObject->objectHandle();
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const caf::PdmObjectHandle* PdmUiObjectEditorHandle::pdmObject() const
{
    const PdmUiItem* pdmItem = this->pdmItem();

    const PdmUiObjectHandle* uiObject = dynamic_cast<const PdmUiObjectHandle*>(pdmItem);
    if (uiObject)
    {
        return uiObject->objectHandle();
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// This function is intended to be called after an object has been created or deleted,
/// to ensure all object editors are updated (including valueOptions and UI representation of objects)
//--------------------------------------------------------------------------------------------------
void PdmUiObjectEditorHandle::updateUiAllObjectEditors()
{
    for (PdmUiObjectEditorHandle* objEditorHandle : m_sRegisteredObjectEditors)
    {
        if (objEditorHandle != nullptr)
        {
            objEditorHandle->updateUi();
        }
    }
}

} //End of namespace caf

