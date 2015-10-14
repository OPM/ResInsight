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
#include "RigCaseData.h"
#include "RigWellPath.h"
#include "RigResultAccessor.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"

#include "RigWellLogExtractionTools.h"
#include "RigMainGrid.h"

//==================================================================================================
/// 
//==================================================================================================

RigEclipseWellLogExtractor::RigEclipseWellLogExtractor(const RigCaseData* aCase, const RigWellPath* wellpath)
    : m_caseData(aCase), RigWellLogExtractor(wellpath)
{
    calculateIntersection();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigEclipseWellLogExtractor::calculateIntersection()
{
    std::map<RigMDCellIdxEnterLeaveIntersectionSorterKey, HexIntersectionInfo > uniqueIntersections;

    {
        const std::vector<cvf::Vec3d>& nodeCoords =  m_caseData->mainGrid()->nodes();

        bool isCellFaceNormalsOut = m_caseData->mainGrid()->isFaceNormalsOutwards();

        if (!m_wellPath->m_wellPathPoints.size()) return ;


        for (size_t wpp = 0; wpp < m_wellPath->m_wellPathPoints.size() - 1; ++wpp)
        {
            cvf::BoundingBox bb;
            cvf::Vec3d p1 = m_wellPath->m_wellPathPoints[wpp];
            cvf::Vec3d p2 = m_wellPath->m_wellPathPoints[wpp+1];

            bb.add(p1);
            bb.add(p2);

            std::vector<size_t> closeCells = findCloseCells(bb);
            std::vector<HexIntersectionInfo> intersections;

            cvf::Vec3d hexCorners[8];
            for (size_t cIdx = 0; cIdx < closeCells.size(); ++cIdx)
            {
                const RigCell& cell = m_caseData->mainGrid()->cells()[closeCells[cIdx]];
                const caf::SizeTArray8& cornerIndices = cell.cornerIndices();

                hexCorners[0] = nodeCoords[cornerIndices[0]];
                hexCorners[1] = nodeCoords[cornerIndices[1]];
                hexCorners[2] = nodeCoords[cornerIndices[2]];
                hexCorners[3] = nodeCoords[cornerIndices[3]];
                hexCorners[4] = nodeCoords[cornerIndices[4]];
                hexCorners[5] = nodeCoords[cornerIndices[5]];
                hexCorners[6] = nodeCoords[cornerIndices[6]];
                hexCorners[7] = nodeCoords[cornerIndices[7]];

                int intersectionCount = RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners, closeCells[cIdx], &intersections);
            }

            // Now, with all the intersections of this piece of line, we need to 
            // sort them in order, and set the measured depth and corresponding cell index

            // Inserting the intersections in this map will remove identical intersections
            // and sort them according to MD, CellIdx, Leave/enter

            double md1 = m_wellPath->m_measuredDepths[wpp];
            double md2 = m_wellPath->m_measuredDepths[wpp+1];

            for (size_t intIdx = 0; intIdx < intersections.size(); ++intIdx)
            {
                if (!isCellFaceNormalsOut) intersections[intIdx].m_isIntersectionEntering = !intersections[intIdx].m_isIntersectionEntering ;

                double lenghtAlongLineSegment1 = (intersections[intIdx].m_intersectionPoint - p1).length();
                double lenghtAlongLineSegment2 = (p2 - intersections[intIdx].m_intersectionPoint).length();
                double measuredDepthDiff       = md2 - md1;
                double lineLength              = lenghtAlongLineSegment1 + lenghtAlongLineSegment2;
                double measuredDepthOfPoint    = 0.0;

                if (lineLength > 0.00001)
                {
                    measuredDepthOfPoint = md1 + measuredDepthDiff*lenghtAlongLineSegment1/(lineLength);
                }
                else
                {
                    measuredDepthOfPoint = md1;
                }

                uniqueIntersections.insert(std::make_pair(RigMDCellIdxEnterLeaveIntersectionSorterKey(measuredDepthOfPoint,
                                                                                                      intersections[intIdx].m_hexIndex,
                                                                                                      intersections[intIdx].m_isIntersectionEntering),
                                                          intersections[intIdx]));
            }
        }
    }

    {
        // For same MD and same cell, remove enter/leave pairs, as they only touches the wellpath, and should not contribute.

        std::map<RigMDCellIdxEnterLeaveIntersectionSorterKey, HexIntersectionInfo >::iterator it1 = uniqueIntersections.begin();
        std::map<RigMDCellIdxEnterLeaveIntersectionSorterKey, HexIntersectionInfo >::iterator it2 = uniqueIntersections.begin();

        std::vector<std::map<RigMDCellIdxEnterLeaveIntersectionSorterKey, HexIntersectionInfo >::iterator> iteratorsToIntersectonsToErase;

        while (it2 != uniqueIntersections.end())
        {
            ++it2;
            if (it2 != uniqueIntersections.end())
            {
                if (RigHexIntersector::isEqualDepth(it1->first.measuredDepth, it2->first.measuredDepth))
                {
                    if (it1->first.hexIndex == it2->first.hexIndex)
                    {
                        // Remove the two from the map, as they just are a touch of the cell surface
                        CVF_TIGHT_ASSERT(!it1->first.isEnteringCell && it2->first.isEnteringCell);

                        iteratorsToIntersectonsToErase.push_back(it1);
                        iteratorsToIntersectonsToErase.push_back(it2);
                    }
                }
            }
            ++it1;
        }

        // Erase all the intersections that is not needed
        for (size_t erItIdx = 0; erItIdx < iteratorsToIntersectonsToErase.size(); ++erItIdx)
        {
            uniqueIntersections.erase(iteratorsToIntersectonsToErase[erItIdx]);
        }
    }

    // Copy the map into a different sorting regime, with enter leave more significant than cell index
    std::map<RigMDEnterLeaveCellIdxIntersectionSorterKey, HexIntersectionInfo > sortedUniqueIntersections;
    {
        std::map<RigMDCellIdxEnterLeaveIntersectionSorterKey, HexIntersectionInfo >::iterator it = uniqueIntersections.begin();
        while (it != uniqueIntersections.end())
        {
            sortedUniqueIntersections.insert(std::make_pair(RigMDEnterLeaveCellIdxIntersectionSorterKey(it->first.measuredDepth, it->first.isEnteringCell, it->first.hexIndex),
                                                       it->second));
            ++it;
        }
    }


    // Make sure we have sensible pairs of intersections. One pair for each in/out of a cell
    {
        // Add an intersection for the well startpoint that is inside the first cell
        std::map<RigMDEnterLeaveCellIdxIntersectionSorterKey, HexIntersectionInfo >::iterator it = sortedUniqueIntersections.begin();
        if (it != sortedUniqueIntersections.end() && !it->first.isEnteringCell) // Leaving a cell as first intersection. Well starts inside a cell.
        {
            // Needs wellpath start point in front
            HexIntersectionInfo firstLeavingPoint = it->second;
            firstLeavingPoint.m_intersectionPoint =  m_wellPath->m_wellPathPoints[0];

            sortedUniqueIntersections.insert(std::make_pair(RigMDEnterLeaveCellIdxIntersectionSorterKey(m_wellPath->m_measuredDepths[0], firstLeavingPoint.m_hexIndex, true),
                firstLeavingPoint));
        }

        // Add an intersection for the well endpoint possibly inside the last cell.
        std::map<RigMDEnterLeaveCellIdxIntersectionSorterKey, HexIntersectionInfo >::reverse_iterator rit = sortedUniqueIntersections.rbegin();
        if (rit != sortedUniqueIntersections.rend() && rit->first.isEnteringCell) // Entering a cell as last intersection. Well ends inside a cell.
        {
            // Needs wellpath end point at end
            HexIntersectionInfo lastEnterPoint = rit->second;
            lastEnterPoint.m_intersectionPoint =  m_wellPath->m_wellPathPoints.back();

            sortedUniqueIntersections.insert(std::make_pair(RigMDEnterLeaveCellIdxIntersectionSorterKey(m_wellPath->m_measuredDepths.back(), lastEnterPoint.m_hexIndex, false),
                lastEnterPoint));
        }
    }

    std::map<RigMDEnterLeaveIntersectionSorterKey, HexIntersectionInfo > filteredSortedIntersections;

    {
        std::map<RigMDEnterLeaveCellIdxIntersectionSorterKey, HexIntersectionInfo >::iterator it1 = sortedUniqueIntersections.begin();
        std::map<RigMDEnterLeaveCellIdxIntersectionSorterKey, HexIntersectionInfo >::iterator it2;

        while (it1 != sortedUniqueIntersections.end())
        {
            it2 = it1;
            ++it2;

            if (it2 == sortedUniqueIntersections.end()) break;

            // If we have a proper pair, insert in the filtered list and continue
            if ( !RigMDEnterLeaveCellIdxIntersectionSorterKey::isProperPair(  it1->first, it2->first))
            {
                //CVF_ASSERT(false);
                cvf::Trace::show(cvf::String("Well log curve is inaccurate around MD:  ") + cvf::String::number((double)(it1->first.measuredDepth)) + ", " + cvf::String::number((double)(it2->first.measuredDepth)));
            }
            {
                filteredSortedIntersections.insert(std::make_pair(RigMDEnterLeaveIntersectionSorterKey(it1->first.measuredDepth, it1->first.isEnteringCell),
                                                                  it1->second));
                ++it1;
                filteredSortedIntersections.insert(std::make_pair(RigMDEnterLeaveIntersectionSorterKey(it1->first.measuredDepth, it1->first.isEnteringCell),
                                                                  it1->second));
                ++it1;
            }
        }
    }
    
    {
        // Now populate the return arrays
        std::map<RigMDEnterLeaveIntersectionSorterKey, HexIntersectionInfo >::iterator it;

        it = filteredSortedIntersections.begin();
        while (it != filteredSortedIntersections.end())
        {
            m_measuredDepth.push_back(it->first.measuredDepth);
            m_trueVerticalDepth.push_back(abs(it->second.m_intersectionPoint[2]));
            m_intersections.push_back(it->second.m_intersectionPoint);
            m_intersectedCells.push_back(it->second.m_hexIndex);
            m_intersectedCellFaces.push_back(it->second.m_face);
            ++it;
        }
    }
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


