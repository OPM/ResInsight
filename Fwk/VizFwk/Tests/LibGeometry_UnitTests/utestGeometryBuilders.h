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

#include "cvfBoundingBox.h"
#include "cvfGeometryBuilder.h"


//==================================================================================================
//
// GeometryBuilder implementation that only builds triangles, for use in unit tests
//
//==================================================================================================
class BuilderTris : public GeometryBuilder
{
public:
    virtual cvf::uint addVertices(const Vec3fArray& vertices)
    {
        cvf::uint numExistingVerts = static_cast<cvf::uint>(vertexArr.size());

        size_t i;
        for (i = 0; i < vertices.size(); i++)
        {
            vertexArr.push_back(vertices[i]);
        }

        return numExistingVerts;
    }

    virtual cvf::uint vertexCount() const
    {
        return static_cast<cvf::uint>(vertexArr.size());
    }

    virtual void transformVertexRange(cvf::uint startIdx, cvf::uint endIdx, const Mat4f& mat)
    {
        size_t i;
        for (i = static_cast<size_t>(startIdx); i <= static_cast<size_t>(endIdx); i++)
        {
            vertexArr[i].transformPoint(mat);
        }
    }

    BoundingBox vertexBoundingBox() const
    {
        BoundingBox bb;

        size_t i;
        for (i = 0; i < vertexArr.size(); i++)
        {
            bb.add(vertexArr[i]);
        }

        return bb;
    }

    virtual void addTriangle(cvf::uint i0, cvf::uint i1, cvf::uint i2)
    {
        triArr.push_back(i0);
        triArr.push_back(i1);
        triArr.push_back(i2);
    }

    cvf::uint triCount()
    {
        return static_cast<cvf::uint>(triArr.size()/3);
    }

public:
    std::vector<Vec3f>  vertexArr;
    std::vector<cvf::uint>   triArr;
};



//==================================================================================================
//
// GeometryBuilder implementation that builds triangles and quads, for use in unit tests
//
//==================================================================================================
class BuilderTrisQuads : public BuilderTris
{
public:
    virtual void addQuad(cvf::uint i0, cvf::uint i1, cvf::uint i2, cvf::uint i3)
    {
        quadArr.push_back(i0);
        quadArr.push_back(i1);
        quadArr.push_back(i2);
        quadArr.push_back(i3);
    }

    cvf::uint quadCount()
    {
        return static_cast<cvf::uint>(quadArr.size()/4);
    }

public:
    std::vector<cvf::uint>   quadArr;
};
