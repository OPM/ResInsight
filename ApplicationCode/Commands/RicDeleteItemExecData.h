/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"

//==================================================================================================
/// 
//==================================================================================================
class RicDeleteItemExecData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicDeleteItemExecData()
    {
        CAF_PDM_InitObject("CmdDeleteItemExecData uiName", "", "CmdDeleteItemExecData tooltip", "CmdDeleteItemExecData whatsthis");

        CAF_PDM_InitField(&m_pathToField, "PathToField", QString(), "PathToField", "", "PathToField tooltip", "PathToField whatsthis");
        CAF_PDM_InitField(&m_indexToObject, "indexToObject", -1, "indexToObject", "", "indexToObject tooltip", "indexToObject whatsthis");
        CAF_PDM_InitField(&m_deletedObjectAsXml, "deletedObjectAsXml", QString(), "deletedObjectAsXml", "", "deletedObjectAsXml tooltip", "deletedObjectAsXml whatsthis");
    }

    caf::PdmPointer<caf::PdmObjectHandle> m_rootObject;

    caf::PdmField<QString>  m_pathToField;
    caf::PdmField<int>      m_indexToObject;
    caf::PdmField<QString>  m_deletedObjectAsXml;
};
