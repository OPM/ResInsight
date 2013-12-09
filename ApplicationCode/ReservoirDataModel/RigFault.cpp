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

#include "RigFault.h"
#include "RigMainGrid.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFault::RigFault()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFault::addCellRangeForFace(cvf::StructGridInterface::FaceType face, const cvf::CellRange& cellRange)
{
    size_t faceIndex = static_cast<size_t>(face);
    CVF_ASSERT(faceIndex < 6);

    m_cellRangesForFaces[faceIndex].push_back(cellRange);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFault::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigFault::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigFault::FaultFace>& RigFault::faultFaces()
{
    return m_faultFaces;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RigFault::FaultFace>& RigFault::faultFaces() const
{
    return m_faultFaces;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFault::computeFaultFacesFromCellRanges(const RigMainGrid* grid)
{
    if (!grid) return;

    m_faultFaces.clear();

    for (size_t faceType = 0; faceType < 6; faceType++)
    {
        cvf::StructGridInterface::FaceType faceEnum = cvf::StructGridInterface::FaceType(faceType);

        const std::vector<cvf::CellRange>& cellRanges = m_cellRangesForFaces[faceType];

        for (size_t i = 0; i < cellRanges.size(); i++)
        {
            const cvf::CellRange& cellRange = cellRanges[i];

            cvf::Vec3st min, max;
            cellRange.range(min, max);

            for (size_t i = min.x(); i <= max.x(); i++)
            {
                if (i >= grid->cellCountI())
                {
                    continue;
                }

                for (size_t j = min.y(); j <= max.y(); j++)
                {
                    if (j >= grid->cellCountJ())
                    {
                        continue;
                    }

                    for (size_t k = min.z(); k <= max.z(); k++)
                    {
                        if (k >= grid->cellCountK())
                        {
                            continue;
                        }

                        size_t localCellIndex = grid->cellIndexFromIJK(i, j, k);

                        // Do not need to compute global grid cell index as for a maingrid localIndex == globalIndex
                        //size_t globalCellIndex = grid->globalGridCellIndex(localCellIndex);

                        cvf::StructGridInterface::FaceType oppositeFace = grid->oppositeFace(faceEnum);

                        size_t ni, nj, nk;
                        grid->ijkFromCellIndex(localCellIndex, &i, &j, &k);
                        grid->neighborIJKAtCellFace(i, j, k, faceEnum, &ni, &nj, &nk);

                        size_t oppositeCellIndex = grid->cellIndexFromIJK(ni, nj, nk);

                        m_faultFaces.push_back(FaultFace(localCellIndex, faceEnum, oppositeCellIndex, oppositeFace));
                    }
                }
            }
        }
    }
}


