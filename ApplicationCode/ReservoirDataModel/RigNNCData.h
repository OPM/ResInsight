/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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
#include "cvfVector3.h"

#include <vector>

#include "cvfStructGrid.h"
#include "cafFixedArray.h"

class RigMainGrid;


class RigConnection
{
public:
    RigConnection( ) 
        : m_c1GlobIdx(cvf::UNDEFINED_SIZE_T),
          m_c1Face(cvf::StructGridInterface::NO_FACE),
          m_c2GlobIdx(cvf::UNDEFINED_SIZE_T),
          m_transmissibility(0.0)
    {}

    size_t                              m_c1GlobIdx;
    cvf::StructGridInterface::FaceType  m_c1Face;
    size_t                              m_c2GlobIdx;

    double                              m_transmissibility;

    std::vector<cvf::Vec3d>             m_polygon;

 /*   enum NNCType
    {
        
    };*/
};


class RigNNCData : public cvf::Object
{
   
public:
    RigNNCData();

    const std::vector<size_t>& findConnectionIndices(size_t globalCellIndex, cvf::StructGridInterface::FaceType face) const;
    void processConnections(const RigMainGrid& mainGrid);
  
    std::vector<RigConnection>&         connections()        { return m_connections; }
    const std::vector<RigConnection>&   connections() const  { return m_connections; };

private:
    typedef std::map<size_t, caf::FixedArray<std::vector<size_t>, 7 > > ConnectionSearchMap;
    ConnectionSearchMap m_cellIdxToFaceToConnectionIdxMap;
    std::vector<RigConnection> m_connections; 
};
