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
#include "cvfStructGrid.h"
#include "cvfVector3.h"

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

    void generateGeometry();

private:
    cvf::Vec3d                         m_startPosition;
    cvf::Vec3d                         m_normal;
    cvf::StructGridInterface::FaceType m_startFace;
    size_t                             m_cellIndex;

    cvf::cref<RigFault>    m_fault;
    cvf::cref<RigMainGrid> m_grid;
};
