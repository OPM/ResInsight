/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class RimNoCommonAreaNNC;

//==================================================================================================
///  Placeholder class used to create a folder in the tree view. 
///  TODO: Remove this class when new tree view is integrated
//==================================================================================================
class RimNoCommonAreaNncCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimNoCommonAreaNncCollection();
    ~RimNoCommonAreaNncCollection() override;

    void                        updateName();

    caf::PdmFieldHandle* userDescriptionField() override;
    
    caf::PdmField<QString>                      name;
    caf::PdmChildArrayField<RimNoCommonAreaNNC*>  noCommonAreaNncs;
};
