/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigEclipseWellLogExtractor.h"
#include <map>
#include "RigEclipseCaseData.h"
#include "RigWellPath.h"
#include "RigResultAccessor.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"

#include "RigWellLogExtractionTools.h"
#include "RigMainGrid.h"

//==================================================================================================
/// 
//==================================================================================================

RigEclipseWellLogExtractor::RigEclipseWellLogExtractor(const RigEclipseCaseData* aCase, const RigWellPath* wellpath, const std::string& wellCaseErrorMsgName)
    : m_caseData(aCase), RigWellLogExtractor(wellpath, wellCaseErrorMsgName)
{
    calculateIntersection();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::calculateIntersection()
{
    std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo > uniqueIntersections;

    const std::vector<cvf::Vec3d>& nodeCoords =  m_caseData->mainGrid()->nodes();
    bool isCellFaceNormalsOut = m_caseData->mainGrid()->isFaceNormalsOutwards();

    if (!m_wellPath->m_wellPathPoints.size()) return ;

    for (size_t wpp = 0; wpp < m_wellPath->m_wellPathPoints.size() - 1; ++wpp)
    {
        std::vector<HexIntersectionInfo> intersections;
        cvf::Vec3d p1 = m_wellPath->m_wellPathPoints[wpp];
        cvf::Vec3d p2 = m_wellPath->m_wellPathPoints[wpp+1];


        cvf::BoundingBox bb;

        bb.add(p1);
        bb.add(p2);

        std::vector<size_t> closeCells = findCloseCells(bb);


        cvf::Vec3d hexCorners[8];
        for (size_t cIdx = 0; cIdx < closeCells.size(); ++cIdx)
        {
            const RigCell& cell = m_caseData->mainGrid()->globalCellArray()[closeCells[cIdx]];

            if (cell.isInvalid()) continue;

            const caf::SizeTArray8& cornerIndices = cell.cornerIndices();

            hexCorners[0] = nodeCoords[cornerIndices[0]];
            hexCorners[1] = nodeCoords[cornerIndices[1]];
            hexCorners[2] = nodeCoords[cornerIndices[2]];
            hexCorners[3] = nodeCoords[cornerIndices[3]];
            hexCorners[4] = nodeCoords[cornerIndices[4]];
            hexCorners[5] = nodeCoords[cornerIndices[5]];
            hexCorners[6] = nodeCoords[cornerIndices[6]];
            hexCorners[7] = nodeCoords[cornerIndices[7]];

            //int intersectionCount = RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners, closeCells[cIdx], &intersections);
            RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners, closeCells[cIdx], &intersections);
        }

        if (!isCellFaceNormalsOut)
        {
            for (size_t intIdx = 0; intIdx < intersections.size(); ++intIdx)
            {
                intersections[intIdx].m_isIntersectionEntering = !intersections[intIdx].m_isIntersectionEntering ;
            }
        }

        // Now, with all the intersections of this piece of line, we need to 
        // sort them in order, and set the measured depth and corresponding cell index

        // Inserting the intersections in this map will remove identical intersections
        // and sort them according to MD, CellIdx, Leave/enter

        double md1 = m_wellPath->m_measuredDepths[wpp];
        double md2 = m_wellPath->m_measuredDepths[wpp+1];

        insertIntersectionsInMap(intersections,
                                 p1, md1, p2, md2,
                                 &uniqueIntersections);

    }

    this->populateReturnArrays(uniqueIntersections);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::curveData(const RigResultAccessor* resultAccessor, std::vector<double>* values)
{
    CVF_TIGHT_ASSERT(values);
    values->resize(m_intersections.size());

    for (size_t cpIdx = 0; cpIdx < m_intersections.size(); ++cpIdx)
    {
        size_t cellIdx = m_intersectedCells[cpIdx];
        cvf::StructGridInterface::FaceType cellFace = m_intersectedCellFaces[cpIdx];
        (*values)[cpIdx] = resultAccessor->cellFaceScalarGlobIdx(cellIdx, cellFace);
    }
   
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigEclipseWellLogExtractor::findCloseCells(const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;
    m_caseData->mainGrid()->findIntersectingCells(bb, &closeCells);
    return closeCells;
}


