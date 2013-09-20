//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
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
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfArray.h"
#include "cvfVector3.h"

namespace cvf {

//=================================================================================================
// 
// Vertex Compactor class
// 
//=================================================================================================
class VertexCompactor
{
public:
    VertexCompactor(const UIntValueArray& origIndices, const Vec3fValueArray& origVertexArray);

    ref<UIntArray>     indices();
    ref<Vec3fArray>    vertexArray();
    ref<UIntArray>     perVertexOriginalIndices();              

private:
    void               computeCompactedIndices();

private:
    const UIntValueArray&   m_origIndices;                  // Indices into the original vertex array
    const Vec3fValueArray&  m_origVertexArray;              // Original vertex array

    ref<UIntArray>          m_newIndices;                   // Indices into new vertex array
    ref<UIntArray>          m_newToOldVertexIndexMapping;   // For each new vertex, this array will store the vertex' original (source) index into origVertexArray
};

}
