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

enum RigElementType
{
    HEX8
};

static const int elmentNodeCount(RigElementType elmType)
{
    static int elementTypeCounts[1] = {8};

    return elementTypeCounts[elmType];
}

static const int elmentFaceCount(RigElementType elmType)
{
    const static int elementFaceCounts[1] = {6};

    return elementFaceCounts[elmType];
}

    // HEX8
    //     7---------6               
    //    /|        /|     |k        
    //   / |       / |     | /j      
    //  4---------5  |     |/        
    //  |  3------|--2     *---i     
    //  | /       | /                
    //  |/        |/                 
    //  0---------1                    

static const int* elementLocalFaceIndices(RigElementType elmType, int faceIdx, int* faceNodeCount)
{
    static const int HEX8_Faces[6][4] = { {1, 2, 6, 5 }, {0,4,7,3}, {3,7,6,2}, {0,1,5,4}, {4,5,6,7} ,{0,3,2,1} };

    (*faceNodeCount) = 4;
    return HEX8_Faces[faceIdx];
}