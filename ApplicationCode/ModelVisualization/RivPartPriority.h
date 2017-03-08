/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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



//--------------------------------------------------------------------------------------------------
/// Class used to manage part render priorities
///
/// The render priority determines the order in which parts get rendered. Parts with lower priorities
/// get rendered first. The default priority is 0.
///
/// See also cvf::Part::setPriority()
///
//--------------------------------------------------------------------------------------------------
class RivPartPriority
{
public:
    enum PartType
    {
        BaseLevel,
        Fault,
        Nnc,
        Intersection,
        CrossSectionNnc,
        MeshLines, 
        FaultMeshLines,
        Transparent,
        TransparentFault,
        TransparentNnc,
        TransparentMeshLines,
        Highlight,
        Text
    };
};
