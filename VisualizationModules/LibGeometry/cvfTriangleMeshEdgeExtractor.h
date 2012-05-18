//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cvfArray.h"
#include <map>

namespace cvf {


//==================================================================================================
//
// Extract mesh from triangles with a key per triangle. Mesh edges will not be produced between
// triangles with the same key.
//
//==================================================================================================
class TriangleMeshEdgeExtractor
{
public:
    TriangleMeshEdgeExtractor();

    void            addTriangles(const UIntArray& indices, const UIntArray& triangleKeys);

    ref<UIntArray>  lineIndices() const;

private:
    std::map<int64, uint> m_edgeMap;

    inline int64 edgeKey(uint v1, uint v2)
    {
        if (v1 < v2)
        {
            int64 key = v2;
            key <<= 32;
            return key + v1;
        }

        int64 key = v1;
        key <<= 32;
        return key + v2;
    }

    void addEdge(uint v1, uint v2, uint key);
};

}
