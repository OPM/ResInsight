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

#include <map>

namespace cvf {



//==================================================================================================
//
// 
//
//==================================================================================================
class OutlineEdgeExtractor
{
public:
    OutlineEdgeExtractor(double creaseAngle, const Vec3fValueArray& vertexArray);

public:
    void            addPrimitives(uint verticesPerPrimitive, const uint* indices, size_t indexCount);
    void            addPrimitives(uint verticesPerPrimitive, const UIntArray& indices);
    void            addFaceList(const UIntArray& faceList);

    ref<UIntArray>              lineIndices() const;
    const std::vector<Vec3f>&   faceNormals();          // Surface normals for the faces, normalized

private:
    Vec3f           computeFaceNormal(const uint* faceVertexIndices, uint numVerticesInFace) const;
    bool            isFaceAngleAboveThreshold(size_t faceIdx1, size_t faceIdx2) const;

private:
    double                  m_creaseAngle;          // Threshold crease angle in radians
    const Vec3fValueArray&  m_vertexArray;
    std::map<int64, size_t> m_edgeToFaceIndexMap;   
    std::vector<Vec3f>      m_faceNormals;          // Surface normals for the faces, normalized
};


}


