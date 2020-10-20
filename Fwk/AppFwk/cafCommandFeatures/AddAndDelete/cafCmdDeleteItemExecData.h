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

#include "cafPdmField.h"
#include "cafPdmObject.h"

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class CmdDeleteItemExecData : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    CmdDeleteItemExecData()
    {
        CAF_PDM_InitObject( "CmdDeleteItemExecData uiName",
                            "",
                            "CmdDeleteItemExecData tooltip",
                            "CmdDeleteItemExecData whatsthis" );

        CAF_PDM_InitField( &m_pathToField, "PathToField", QString(), "PathToField", "", "PathToField tooltip", "PathToField whatsthis" );
        CAF_PDM_InitField( &m_indexToObject,
                           "indexToObject",
                           -1,
                           "indexToObject",
                           "",
                           "indexToObject tooltip",
                           "indexToObject whatsthis" );
        CAF_PDM_InitField( &m_deletedObjectAsXml,
                           "deletedObjectAsXml",
                           QString(),
                           "deletedObjectAsXml",
                           "",
                           "deletedObjectAsXml tooltip",
                           "deletedObjectAsXml whatsthis" );
    }

    caf::PdmPointer<PdmObjectHandle> m_rootObject;

    caf::PdmField<QString> m_pathToField;
    caf::PdmField<int>     m_indexToObject;
    caf::PdmField<QString> m_deletedObjectAsXml;
};

} // end namespace caf
