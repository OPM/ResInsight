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

class QString;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLog : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLog();
    
    void setName(const QString& name);

    virtual caf::PdmFieldHandle* userDescriptionField()  { return &m_name; }

private:
    caf::PdmField<QString> m_name;
};

