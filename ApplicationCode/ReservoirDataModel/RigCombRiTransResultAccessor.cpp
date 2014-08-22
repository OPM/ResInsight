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

#include "RigCombRiTransResultAccessor.h"

#include "RigMainGrid.h"
#include "RigGridBase.h"
#include "RigCell.h"

#include <cmath>
#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCombRiTransResultAccessor::RigCombRiTransResultAccessor(const RigGridBase* grid)
    : m_grid(grid)
{
    m_cdarchy = 0.008527; // (ECLIPSE 100) (METRIC)
}

//--------------------------------------------------------------------------------------------------
/// Only sensible to provide the positive values, as the negative ones will never be used.
/// The negative faces gets their value from the neighbor cell in that direction
//--------------------------------------------------------------------------------------------------
void RigCombRiTransResultAccessor::setPermResultAccessors( RigResultAccessor* xPermAccessor,
                                                           RigResultAccessor* yPermAccessor,
                                                           RigResultAccessor* zPermAccessor)

{
    m_xPermAccessor = xPermAccessor;
    m_yPermAccessor = yPermAccessor;
    m_zPermAccessor = zPermAccessor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigCombRiTransResultAccessor::setNTGResultAccessor(RigResultAccessor* ntgAccessor)
{
    m_ntgAccessor = ntgAccessor;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{
    CVF_TIGHT_ASSERT(false);

    return HUGE_VAL;
}

double RigCombRiTransResultAccessor::getPermValue(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    switch (faceId)
    {
    case cvf::StructGridInterface::POS_I:
    case cvf::StructGridInterface::NEG_I:
        {
            return m_xPermAccessor->cellScalar(gridLocalCellIndex);
        }
        break;
    case cvf::StructGridInterface::POS_J:
    case cvf::StructGridInterface::NEG_J:
        {
            return m_yPermAccessor->cellScalar(gridLocalCellIndex);
        }
        break;
    case cvf::StructGridInterface::POS_K:
    case cvf::StructGridInterface::NEG_K:
        {
            return m_zPermAccessor->cellScalar(gridLocalCellIndex);
        }
        break;
    }
    return HUGE_VAL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::getNtgValue(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId ) const
{
    double ntg = 1.0;

    if (faceId != cvf::StructGridInterface::POS_K && faceId != cvf::StructGridInterface::NEG_K)
    {
        m_ntgAccessor->cellScalar(gridLocalCellIndex);
    }

    return ntg;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::halfCellTransmissibility(double perm, double ntg, const cvf::Vec3d& centerToFace, const cvf::Vec3d& faceAreaVec)
{
    return perm*ntg*(faceAreaVec*centerToFace) / (centerToFace*centerToFace);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::newtran(double cdarchy, double mult, double halfCellTrans, double neighborHalfCellTrans)
{
    return cdarchy * mult / ( ( 1 / halfCellTrans) + (1 / neighborHalfCellTrans) );
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::calculateHalfCellTrans(size_t gridLocalCellIndex, size_t neighborGridCellIdx, cvf::StructGridInterface::FaceType faceId, bool isFaultFace) const
{
    const RigCell& cell = m_grid->cell(gridLocalCellIndex);

  
    cvf::Vec3d faceAreaVec;
    cvf::Vec3d faceCenter;

    if (isFaultFace)
    {
        
        calculateConnectionGeometry( m_grid, gridLocalCellIndex, neighborGridCellIdx, faceId, &faceCenter, &faceAreaVec);
    }
    else
    {
        const RigCell& cell = m_grid->cell(gridLocalCellIndex);
        faceCenter = cell.faceCenter(faceId);
        
        faceAreaVec = cell.faceNormalWithAreaLenght(faceId);
    }

    cvf::Vec3d centerToFace = faceCenter - cell.center();

    double perm = getPermValue(gridLocalCellIndex, faceId);
    double ntg = getNtgValue(gridLocalCellIndex, faceId );

    return halfCellTransmissibility(perm, ntg, centerToFace, faceAreaVec);
}

void RigCombRiTransResultAccessor::calculateConnectionGeometry(  const RigGridBase* grid, size_t gridLocalCellIndex, size_t neighborGridCellIdx, 
                                                                cvf::StructGridInterface::FaceType faceId,
                                                                cvf::Vec3d* faceCenter, cvf::Vec3d* faceAreaVec)
{
    CVF_TIGHT_ASSERT(faceCenter && faceAreaVec);

    *faceCenter = cvf::Vec3d::ZERO;
    *faceAreaVec = cvf::Vec3d::ZERO; 
    const RigMainGrid* mainGrid = grid->mainGrid();

    const RigCell& c1 = grid->cell(gridLocalCellIndex);
    const RigCell& c2 = grid->cell(neighborGridCellIdx);

    std::vector<size_t> polygon;
    std::vector<cvf::Vec3d> intersections;
    caf::SizeTArray4 face1;
    caf::SizeTArray4 face2;
    c1.faceIndices(faceId, &face1);
    c2.faceIndices(cvf::StructGridInterface::oppositeFace(faceId), &face2);

    bool foundOverlap = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads(
        &polygon, 
        &intersections, 
        (cvf::EdgeIntersectStorage<size_t>*)NULL, 
        cvf::wrapArrayConst(&(mainGrid->nodes())), 
        face1.data(), 
        face2.data(), 
        1e-6);


    if (foundOverlap)
    {
        std::vector<cvf::Vec3d> realPolygon;

        for (size_t pIdx = 0; pIdx < polygon.size(); ++pIdx)
        {
            if (polygon[pIdx] < grid->mainGrid()->nodes().size())
                realPolygon.push_back(grid->mainGrid()->nodes()[polygon[pIdx]]);
            else
                realPolygon.push_back(intersections[polygon[pIdx] - grid->mainGrid()->nodes().size()]);
        }

        // Polygon center
        for (size_t pIdx = 0; pIdx < realPolygon.size(); ++pIdx)
        {
            *faceCenter += realPolygon[pIdx];
        }

        *faceCenter *= 1.0/realPolygon.size();

        // Polygon area vector

        *faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D(realPolygon);

    }

}
//--------------------------------------------------------------------------------------------------
/// 
/// Neighbor cell transmisibilities only is calculated here
/// Not NNC transmisibilities. That has to be done separately elsewhere 
///
/// Todo: What about Grid to Grid connections ?
/// Todo: needs optimization. Things are done several times. Caching of the results should be considered. etc.
///
//--------------------------------------------------------------------------------------------------
double RigCombRiTransResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    size_t i, j, k, neighborGridCellIdx;
    m_grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

    if (m_grid->cellIJKNeighbor(i, j, k, faceId, &neighborGridCellIdx))
    {
        size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
        const RigFault* fault = m_grid->mainGrid()->findFaultFromCellIndexAndCellFace(reservoirCellIndex, faceId);
        bool isOnFault = fault;

        double halfCellTrans = 0;
        double neighborHalfCellTrans = 0;

        halfCellTrans = calculateHalfCellTrans(gridLocalCellIndex, neighborGridCellIdx, faceId, isOnFault);
        neighborHalfCellTrans = calculateHalfCellTrans(neighborGridCellIdx, gridLocalCellIndex, cvf::StructGridInterface::oppositeFace(faceId), isOnFault);

        return newtran(m_cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans);
    }

    return HUGE_VAL;
}
