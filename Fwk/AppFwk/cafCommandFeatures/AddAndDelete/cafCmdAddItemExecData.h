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

#include "cafPdmObject.h"
#include "cafPdmField.h"

namespace caf 
{


//==================================================================================================
/// 
//==================================================================================================
class CmdAddItemExecData : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    CmdAddItemExecData()
    {
        CAF_PDM_InitObject("CmdAddItemExecData uiName", "", "CmdAddItemExecData tooltip", "CmdAddItemExecData whatsthis");

        CAF_PDM_InitField(&m_pathToField, "PathToField", QString(), "PathToField", "", "PathToField tooltip", "PathToField whatsthis");
        CAF_PDM_InitField(&m_indexAfter, "indexAfter", -1, "indexAfter", "", "indexAfter tooltip", "indexAfter whatsthis");
        CAF_PDM_InitField(&m_createdItemIndex, "createdItemIndex", -1, "createdItemIndex", "", "createdItemIndex tooltip", "createdItemIndex whatsthis");
    }

    caf::PdmPointer<PdmObjectHandle> m_rootObject;

    caf::PdmField<QString>  m_pathToField;
    caf::PdmField<int>      m_indexAfter;
    caf::PdmField<int>      m_createdItemIndex;
};



} // end namespace caf
