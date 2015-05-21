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

#include <stdlib.h>

#include "RigFemTypes.h"
#include "cvfObject.h"
#include "cvfAssert.h"

#include "cvfVector3.h"
#include <vector>

class RigFemPartGrid;

class RigFemPartNodes
{
public:
     std::vector<int>           nodeIds;
     std::vector<cvf::Vec3f>    coordinates;
};

class RigFemPart : public cvf::Object
{
public:
    RigFemPart();
    virtual ~RigFemPart();

    int                         elementPartId() const                      { return m_elementPartId; }
    void                        setElementPartId(int partId)               { m_elementPartId = partId; }

    void                        preAllocateElementStorage(int elementCount);
    void                        appendElement(RigElementType elmType, int elementId, const int* connectivities);

    size_t                      elementCount() const                       { return m_elementId.size(); }
    
    int                         elmId(size_t elementIdx) const             { return m_elementId[elementIdx]; }
    RigElementType              elementType(size_t elementIdx) const       { return m_elementTypes[elementIdx]; }
    const int*                  connectivities(size_t elementIdx) const    { return &m_allAlementConnectivities[m_elementConnectivityStartIndices[elementIdx]];}
    size_t                      elementNodeResultIdx(int elementIdx, int elmLocalNodeIdx) const { return m_elementConnectivityStartIndices[elementIdx] + elmLocalNodeIdx;}

    RigFemPartNodes&            nodes()                                    {return m_nodes;}
    const RigFemPartNodes&      nodes() const                              {return m_nodes;}

    const RigFemPartGrid*       structGrid();      
    
private:
    int                         m_elementPartId;
    std::vector<int>            m_elementId;
    std::vector<RigElementType> m_elementTypes;
    std::vector<size_t>         m_elementConnectivityStartIndices;
    std::vector<int>            m_allAlementConnectivities;

    RigFemPartNodes             m_nodes;

    cvf::ref<RigFemPartGrid>    m_structGrid;
};

#include "cvfStructGrid.h"

class RigFemPartGrid : public cvf::StructGridInterface
{
public:
    RigFemPartGrid(RigFemPart* femPart);
    virtual ~RigFemPartGrid();

    virtual size_t      gridPointCountI() const;
    virtual size_t      gridPointCountJ() const;
    virtual size_t      gridPointCountK() const;

    virtual bool        isCellValid(size_t i, size_t j, size_t k) const;
    virtual cvf::Vec3d  minCoordinate() const;
    virtual cvf::Vec3d  maxCoordinate() const;
    virtual bool        cellIJKNeighbor(size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex) const;
    virtual size_t      cellIndexFromIJK(size_t i, size_t j, size_t k) const;
    virtual bool        ijkFromCellIndex(size_t cellIndex, size_t* i, size_t* j, size_t* k) const;
    virtual bool        cellIJKFromCoordinate(const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k) const;
    virtual void        cellCornerVertices(size_t cellIndex, cvf::Vec3d vertices[8]) const;
    virtual cvf::Vec3d  cellCentroid(size_t cellIndex) const;
    virtual void        cellMinMaxCordinates(size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate) const;
    virtual size_t      gridPointIndexFromIJK(size_t i, size_t j, size_t k) const;
    virtual cvf::Vec3d  gridPointCoordinate(size_t i, size_t j, size_t k) const;

 
 private:
    void generateStructGridData();


    RigFemPart* m_femPart;
};