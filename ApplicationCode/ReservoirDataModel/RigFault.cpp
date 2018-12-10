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

#include "RigFault.h"

#include "RigMainGrid.h"

cvf::ref<RigFaultsPrCellAccumulator> RigFault::m_faultsPrCellAcc;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFault::RigFault() {}

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
std::vector<size_t>& RigFault::connectionIndices()
{
    return m_connectionIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFault::connectionIndices() const
{
    return m_connectionIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFault::computeFaultFacesFromCellRanges(const RigMainGrid* mainGrid)
{
    if (!mainGrid) return;

    m_faultFaces.clear();

    for (size_t faceType = 0; faceType < 6; faceType++)
    {
        cvf::StructGridInterface::FaceType faceEnum = cvf::StructGridInterface::FaceType(faceType);

        const std::vector<cvf::CellRange>& cellRanges = m_cellRangesForFaces[faceType];

        for (const cvf::CellRange& cellRange : cellRanges)
        {
            cvf::Vec3st min, max;
            cellRange.range(min, max);

            for (size_t i = min.x(); i <= max.x(); i++)
            {
                if (i >= mainGrid->cellCountI())
                {
                    continue;
                }

                for (size_t j = min.y(); j <= max.y(); j++)
                {
                    if (j >= mainGrid->cellCountJ())
                    {
                        continue;
                    }

                    for (size_t k = min.z(); k <= max.z(); k++)
                    {
                        if (k >= mainGrid->cellCountK())
                        {
                            continue;
                        }

                        // Do not need to compute global grid cell index as for a maingrid localIndex == globalIndex
                        // size_t reservoirCellIndex = grid->reservoirCellIndex(gridLocalCellIndex);

                        size_t ni, nj, nk;
                        mainGrid->neighborIJKAtCellFace(i, j, k, faceEnum, &ni, &nj, &nk);
                        if (ni < mainGrid->cellCountI() && nj < mainGrid->cellCountJ() && nk < mainGrid->cellCountK())
                        {
                            size_t gridLocalCellIndex = mainGrid->cellIndexFromIJK(i, j, k);
                            size_t oppositeCellIndex  = mainGrid->cellIndexFromIJK(ni, nj, nk);

                            m_faultFaces.push_back(FaultFace(gridLocalCellIndex, faceEnum, oppositeCellIndex));
                        }
                        else
                        {
                            // cvf::Trace::show("Warning: Undefined Fault neighbor detected.");
                        }
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFault::accumulateFaultsPrCell(RigFaultsPrCellAccumulator* faultsPrCellAcc, int faultIdx)
{
    for (const FaultFace& ff : m_faultFaces)
    {
        // Could detect overlapping faults here .... if (faultsPrCellAcc->faultIdx(ff.m_nativeReservoirCellIndex, ff.m_nativeFace)
        // >= 0)

        faultsPrCellAcc->setFaultIdx(ff.m_nativeReservoirCellIndex, ff.m_nativeFace, faultIdx);
        faultsPrCellAcc->setFaultIdx(
            ff.m_oppositeReservoirCellIndex, cvf::StructGridInterface::oppositeFace(ff.m_nativeFace), faultIdx);
    }
}
