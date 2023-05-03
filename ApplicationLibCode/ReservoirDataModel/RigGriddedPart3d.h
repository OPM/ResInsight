/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "cvfObject.h"
#include "cvfVector3.h"

#include <map>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigGriddedPart3d : public cvf::Object
{
public:
    enum class BorderSurface
    {
        UpperSurface = 0,
        FaultSurface,
        LowerSurface
    };

public:
    RigGriddedPart3d();
    ~RigGriddedPart3d() override;

    void reset();

    void generateGeometry( std::vector<cvf::Vec3d> inputPoints, int nHorzCells, int nVertCellsLower, int nVertCellsMiddle, int nVertCellsUpper );

    const std::vector<cvf::Vec3d>&                            vertices() const;
    const std::vector<std::vector<unsigned int>>&             elementIndices() const;
    const std::map<BorderSurface, std::vector<unsigned int>>& borderSurfaceElements() const;
    const std::vector<std::vector<cvf::Vec3d>>&               meshLines() const;

protected:
    cvf::Vec3d stepVector( cvf::Vec3d start, cvf::Vec3d stop, int nSteps );
    void       generateMeshlines( std::vector<cvf::Vec3d> cornerPoints, int numHorzCells, int numVertCells );

private:
    std::vector<cvf::Vec3d>                            m_vertices;
    std::vector<std::vector<unsigned int>>             m_elementIndices;
    std::map<BorderSurface, std::vector<unsigned int>> m_borderSurfaceElements;
    std::vector<std::vector<cvf::Vec3d>>               m_meshLines;

    double m_thickness;
};
