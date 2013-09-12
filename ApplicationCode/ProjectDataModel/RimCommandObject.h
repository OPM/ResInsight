/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmDocument.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCommandObject();
    virtual ~RimCommandObject();

    virtual void redo() {};
    virtual void undo() {};
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandExecuteScript : public RimCommandObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCommandExecuteScript();
    virtual ~RimCommandExecuteScript();

    caf::PdmField<QString> scriptText;

    virtual void redo();
    virtual void undo();
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandFactory
{
public:
    static void createCommandObjects(const caf::PdmObjectGroup& selectedObjects, std::vector<RimCommandObject*>* commandObjects);
};

