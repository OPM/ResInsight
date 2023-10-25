/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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
#include "cvfPlane.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <array>
#include <vector>

class RigFault;
class RigMainGrid;

class RigFaultReactivationModelGenerator : cvf::Object
{
public:
    RigFaultReactivationModelGenerator( cvf::Vec3d position, cvf::Vec3d normal, size_t cellIndex, cvf::StructGridInterface::FaceType face );
    ~RigFaultReactivationModelGenerator();

    void setFault( const RigFault* fault );
    void setGrid( const RigMainGrid* grid );
    void setFaultBufferDepth( double aboveFault, double belowFault );

    void generateGeometry();

protected:
    static const std::array<int, 4> faceIJCornerIndexes( cvf::StructGridInterface::FaceType face );
    cvf::Vec3d                      lineIntersect( const cvf::Plane& plane, cvf::Vec3d lineA, cvf::Vec3d lineB );
    std::map<double, cvf::Vec3d>    elementLayers( cvf::StructGridInterface::FaceType face, const std::vector<size_t>& cellIndexColumn );
    cvf::Vec3d                      extrapolatePoint( cvf::Vec3d startPoint, cvf::Vec3d endPoint, double stopDepth );

private:
    cvf::Vec3d                         m_startPosition;
    cvf::Vec3d                         m_normal;
    cvf::StructGridInterface::FaceType m_startFace;
    size_t                             m_cellIndex;

    cvf::cref<RigFault>    m_fault;
    cvf::cref<RigMainGrid> m_grid;

    double m_bufferAboveFault;
    double m_bufferBelowFault;
};
