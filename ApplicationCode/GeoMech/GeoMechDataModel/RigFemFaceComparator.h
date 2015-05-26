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


class RigFemFaceComparator
{
public:
    void setMainFace(const int* elmNodes, const int * localFaceIndices, int faceNodeCount)
    {
        canonizeFace(elmNodes, localFaceIndices, faceNodeCount, &m_canonizedMainFaceIdxes);
    }

    bool isSameButOposite(const int* elmNodes, const int * localFaceIndices, int faceNodeCount)
    {
        canonizeOpositeFace(elmNodes, localFaceIndices, faceNodeCount, &m_canonizedOtherFaceIdxes);

        return m_canonizedMainFaceIdxes == m_canonizedOtherFaceIdxes;
    }

private:
    void canonizeFace( const int* elmNodes, 
                       const int * localFaceIndices, 
                       int faceNodeCount, 
                       std::vector<int>* canonizedFace)
    {
        canonizedFace->resize(faceNodeCount);
        int minNodeIdx = INT_MAX;
        int faceIdxToMinNodeIdx = 0;

        for(int fnIdx = 0; fnIdx < faceNodeCount; ++fnIdx)
        {
            int nodeIdx = elmNodes[localFaceIndices[fnIdx]];
            (*canonizedFace)[fnIdx] = nodeIdx;
            if (nodeIdx < minNodeIdx) 
            {
                minNodeIdx = nodeIdx;
                faceIdxToMinNodeIdx = fnIdx;
            }
        }

        std::rotate(canonizedFace->begin(),
                    canonizedFace->begin() + faceIdxToMinNodeIdx,
                    canonizedFace->end());
    }

    void canonizeOpositeFace(const int* elmNodes,
                       const int * localFaceIndices, 
                       int faceNodeCount, 
                       std::vector<int>* canonizedFace)
    {
        canonizedFace->resize(faceNodeCount);
        int minNodeIdx = INT_MAX;
        int faceIdxToMinNodeIdx = 0;

        int canFaceIdx = 0;
        for(int fnIdx = faceNodeCount -1; fnIdx >= 0; --fnIdx, ++canFaceIdx)
        {
            int nodeIdx = elmNodes[localFaceIndices[fnIdx]];
            (*canonizedFace)[canFaceIdx] = nodeIdx;
            if (nodeIdx < minNodeIdx) 
            {
                minNodeIdx = nodeIdx;
                faceIdxToMinNodeIdx = canFaceIdx;
            }
        }

        std::rotate(canonizedFace->begin(),
                    canonizedFace->begin() + faceIdxToMinNodeIdx,
                    canonizedFace->end());
    }

    std::vector<int> m_canonizedMainFaceIdxes;
    std::vector<int> m_canonizedOtherFaceIdxes;
};


