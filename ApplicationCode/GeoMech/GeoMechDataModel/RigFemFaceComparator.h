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

#include <vector>
#include <algorithm>
#include <climits>

class RigFemFaceComparator
{
public:
    RigFemFaceComparator() : m_minMainFaceNodeIdx(std::numeric_limits<int>::max()),  m_faceIdxToMinMainFaceNodeIdx(0) {}

    void setMainFace(const int* elmNodes, const int * localFaceIndices, int faceNodeCount)
    {
        m_canonizedMainFaceIdxes.resize(faceNodeCount);
        m_minMainFaceNodeIdx = std::numeric_limits<int>::max();
        m_faceIdxToMinMainFaceNodeIdx = 0;

        for(int fnIdx = 0; fnIdx < faceNodeCount; ++fnIdx)
        {
            int nodeIdx = elmNodes[localFaceIndices[fnIdx]];
            m_canonizedMainFaceIdxes[fnIdx] = nodeIdx;
            if (nodeIdx < m_minMainFaceNodeIdx) 
            {
                m_minMainFaceNodeIdx = nodeIdx;
                m_faceIdxToMinMainFaceNodeIdx = fnIdx;
            }
        }
    }

    bool isSameButOposite(const int* elmNodes, const int * localFaceIndices, int faceNodeCount)
    {
        if (faceNodeCount != static_cast<int>(m_canonizedMainFaceIdxes.size())) return false;

        // Find min node index in face
        int minNodeIdx = std::numeric_limits<int>::max();
        int faceIdxToMinNodeIdx = 0;

        for (int fnIdx = 0; fnIdx < faceNodeCount; ++fnIdx)
        {
            int nodeIdx = elmNodes[localFaceIndices[fnIdx]];
            if (nodeIdx < minNodeIdx)
            {
                minNodeIdx = nodeIdx;
                faceIdxToMinNodeIdx = fnIdx;
            }
        }

        // Compare faces
        {
            if (minNodeIdx != m_minMainFaceNodeIdx )  return false;

            int canFaceIdx = m_faceIdxToMinMainFaceNodeIdx;
            int fnIdx = faceIdxToMinNodeIdx;
            int count = 0;

            for (; count < faceNodeCount;
                 --fnIdx, ++canFaceIdx, ++count)
            {
                if (fnIdx < 0) fnIdx = faceNodeCount - 1;
                if (canFaceIdx == faceNodeCount) canFaceIdx = 0;

                if (elmNodes[localFaceIndices[fnIdx]] != m_canonizedMainFaceIdxes[canFaceIdx]) return false;
            }

            return true;
        }
    }

private:
    std::vector<int> m_canonizedMainFaceIdxes;
    int m_minMainFaceNodeIdx;
    int m_faceIdxToMinMainFaceNodeIdx;
};


