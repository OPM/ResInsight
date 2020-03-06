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

#include "cafPdmObjectCapability.h"
#include "cafPdmObjectScriptabilityRegister.h"

#include <QString>

#include <map>

class QTextStream;

#define RICF_HEADER_INIT \
    CAF_CMD_HEADER_INIT; \
    CAF_PDM_HEADER_INIT

// RICF_SOURCE_INIT calls CAF_FACTORY_REGISTER2 to avoid name conflicts with CAF_PDM_SOURCE_INIT
#define RICF_SOURCE_INIT( ClassName, CommandIdName, CommandKeyword )                             \
    const std::string& ClassName::idNameStatic()                                                 \
    {                                                                                            \
        static std::string id = CommandIdName;                                                   \
        return id;                                                                               \
    }                                                                                            \
    CAF_FACTORY_REGISTER2( caf::CmdFeature, ClassName, std::string, ClassName::idNameStatic() ); \
    CAF_PDM_SOURCE_INIT( ClassName, CommandKeyword )

#define CAF_PDM_InitScriptableObject( uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_PDM_InitObject( uiName, iconResourceName, toolTip, whatsThis );              \
    caf::PdmObjectScriptabilityRegister::registerScriptClassNameAndComment( classKeyword(), classKeyword(), whatsThis );

#define CAF_PDM_InitScriptableObjectWithNameAndComment( uiName, iconResourceName, toolTip, whatsThis, scriptClassName, scriptComment ) \
    CAF_PDM_InitObject( uiName, iconResourceName, toolTip, whatsThis );                                                                \
    caf::PdmObjectScriptabilityRegister::registerScriptClassNameAndComment( classKeyword(), scriptClassName, scriptComment );

namespace caf
{
class PdmObject;
class PdmObjectHandle;
class PdmObjectFactory;
class PdmScriptIOMessages;

//==================================================================================================
//
//
//
//==================================================================================================
class PdmObjectScriptability : public PdmObjectCapability
{
public:
    PdmObjectScriptability( PdmObjectHandle* owner, bool giveOwnership );

    ~PdmObjectScriptability() override;

    void readFields( QTextStream& inputStream, PdmObjectFactory* objectFactory, PdmScriptIOMessages* errorMessageContainer );
    void writeFields( QTextStream& outputStream ) const;

private:
    caf::PdmObjectHandle* m_owner;
};
} // namespace caf
