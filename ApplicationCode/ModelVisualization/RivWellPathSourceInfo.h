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

class RimWellPath;
//class RivPipeQuadToSegmentMapper;

//==================================================================================================
///  
///  TODO: Implement and add RivPipeQuadToSegmentMapper
//==================================================================================================
class RivWellPathSourceInfo : public cvf::Object
{
public:
    RivWellPathSourceInfo(RimWellPath* wellPath/*, RivPipeQuadToSegmentMapper* quadToSegmentMapper*/);

    RimWellPath* wellPath() const;

private:    
    caf::PdmPointer<RimWellPath>    m_wellPath;
    //RivPipeQuadToSegmentMapper*     m_quadToSegmentMapper;
};
