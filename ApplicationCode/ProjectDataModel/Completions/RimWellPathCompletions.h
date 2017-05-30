/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cafPdmChildField.h"

class RimFishbonesCollection;
class RimPerforationCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathCompletions : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathCompletions();

    RimFishbonesCollection*     fishbonesCollection() const;
    RimPerforationCollection*   perforationCollection() const;

private:
    caf::PdmChildField<RimFishbonesCollection*>     m_fishbonesCollection;
    caf::PdmChildField<RimPerforationCollection*>   m_perforationCollection;
};
