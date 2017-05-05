/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "cafPdmPointer.h"

#include "cvfBase.h"
#include "cvfObject.h"

namespace caf {
    class PdmObject;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RivObjectSourceInfo : public cvf::Object
{
public:
    RivObjectSourceInfo(caf::PdmObject* object);
    
    caf::PdmObject* object() const;

private:    
    caf::PdmPointer<caf::PdmObject> m_object;
};
