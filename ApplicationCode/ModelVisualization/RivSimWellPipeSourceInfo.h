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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmPointer.h"

class RimSimWellInView;

class RivSimWellPipeSourceInfo : public cvf::Object
{
public:
    RivSimWellPipeSourceInfo(RimSimWellInView* eclipseWell, size_t branchIndex);

    RimSimWellInView* well() const;

    size_t branchIndex() const;

private:    
    caf::PdmPointer<RimSimWellInView> m_simWell;
    size_t m_branchIndex;
};
