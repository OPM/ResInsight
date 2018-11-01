/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaCellDividingTools.h"

#include "cvfAssert.h"

#include <cmath>


//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/// Splits a line in a number of equal parts
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> splitLine(cvf::Vec3d ptStart, cvf::Vec3d ptEnd, size_t partCount);

//--------------------------------------------------------------------------------------------------
/// Calculates all points on a face described by edge points from all four edges.
/// The result is a grid of points including the given edge points
///
///                    edgeXPtsHigh
///                  |-------------|
///                  |             |
///   edgeYPtsLow    |             |   edgeYPtsHigh
///                  |             |
///                  |-------------|
///                    edgeXPtsLow
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> calcFacePoints(const std::vector<cvf::Vec3d> edgeXPtsLow,
                                                    const std::vector<cvf::Vec3d> edgeXPtsHigh,
                                                    const std::vector<cvf::Vec3d> edgeYPtsLow,
                                                    const std::vector<cvf::Vec3d> edgeYPtsHigh);

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>
RiaCellDividingTools::createHexCornerCoords(std::array<cvf::Vec3d, 8> mainCellCorners, size_t nx, size_t ny, size_t nz)
{
    std::array<std::pair<size_t, size_t>, 12> edgeCorners = {
        std::make_pair(0, 1),
        std::make_pair(3, 2),
        std::make_pair(4, 5),
        std::make_pair(7, 6), // X
        std::make_pair(0, 3),
        std::make_pair(4, 7),
        std::make_pair(1, 2),
        std::make_pair(5, 6), // Y
        std::make_pair(0, 4),
        std::make_pair(1, 5),
        std::make_pair(3, 7),
        std::make_pair(2, 6), // Z
    };

    std::array<size_t, 3>                   nxyz = {nx, ny, nz};
    std::array<std::vector<cvf::Vec3d>, 12> edgePoints;

    for (int i = 0; i < 12; i++)
    {
        int partCountsIndex = i / 4;
        edgePoints[i] =
            splitLine(mainCellCorners[edgeCorners[i].first], mainCellCorners[edgeCorners[i].second], nxyz[partCountsIndex]);
    }

    // lowIJ, highIJ, lowJK, highKJ,
    std::vector<std::vector<std::vector<cvf::Vec3d>>> nodes;
    nodes.reserve((nx + 1) * (ny + 1) * (nz + 1));

    auto xyFacePtsLow  = calcFacePoints(edgePoints[0], edgePoints[1], edgePoints[4], edgePoints[6]);
    auto xyFacePtsHigh = calcFacePoints(edgePoints[2], edgePoints[3], edgePoints[5], edgePoints[7]);
    auto yzFacePtsLow  = calcFacePoints(edgePoints[4], edgePoints[5], edgePoints[8], edgePoints[10]);
    auto yzFacePtsHigh = calcFacePoints(edgePoints[6], edgePoints[7], edgePoints[9], edgePoints[11]);
    auto xzFacePtsLow  = calcFacePoints(edgePoints[0], edgePoints[2], edgePoints[8], edgePoints[9]);
    auto xzFacePtsHigh = calcFacePoints(edgePoints[1], edgePoints[3], edgePoints[10], edgePoints[11]);

    nodes.push_back(xyFacePtsLow);

    for (size_t z = 1; z < nz; z++)
    {
        auto xyFacePoints = calcFacePoints(xzFacePtsLow[z], xzFacePtsHigh[z], yzFacePtsLow[z], yzFacePtsHigh[z]);
        nodes.push_back(xyFacePoints);
    }

    nodes.push_back(xyFacePtsHigh);

    std::vector<cvf::Vec3d> coords;
    coords.reserve(nx * ny * nz * 8);

    for (size_t z = 1; z < nz + 1; z++)
    {
        for (size_t y = 1; y < ny + 1; y++)
        {
            for (size_t x = 1; x < nx + 1; x++)
            {
                std::array<cvf::Vec3d, 8> cs;

                cs[0] = nodes[z - 1][y - 1][x - 1];
                cs[1] = nodes[z - 1][y - 1][x];
                cs[2] = nodes[z - 1][y][x];
                cs[3] = nodes[z - 1][y][x - 1];

                cs[4] = nodes[z][y - 1][x - 1];
                cs[5] = nodes[z][y - 1][x];
                cs[6] = nodes[z][y][x];
                cs[7] = nodes[z][y][x - 1];

                coords.insert(coords.end(), cs.begin(), cs.end());
            }
        }
    }
    return coords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> splitLine(cvf::Vec3d ptStart, cvf::Vec3d ptEnd, size_t partCount)
{
    std::vector<cvf::Vec3d> pts = {ptStart};

    for (size_t i = 1; i < partCount; i++)
    {
        pts.push_back(cvf::Vec3d(ptStart.x() + (ptEnd.x() - ptStart.x()) * i / partCount,
                                 ptStart.y() + (ptEnd.y() - ptStart.y()) * i / partCount,
                                 ptStart.z() + (ptEnd.z() - ptStart.z()) * i / partCount));
    }
    pts.push_back(ptEnd);
    return pts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> calcFacePoints(const std::vector<cvf::Vec3d> edgeXPtsLow,
                                                    const std::vector<cvf::Vec3d> edgeXPtsHigh,
                                                    const std::vector<cvf::Vec3d> edgeYPtsLow,
                                                    const std::vector<cvf::Vec3d> edgeYPtsHigh)
{
    CVF_ASSERT(edgeXPtsLow.size() == edgeXPtsHigh.size() && edgeYPtsLow.size() == edgeYPtsHigh.size());

    size_t xSize = edgeXPtsLow.size();
    size_t ySize = edgeYPtsLow.size();

    std::vector<std::vector<cvf::Vec3d>> pts;

    // Add low edge points
    pts.push_back(edgeXPtsLow);

    // Interior points
    for (size_t y = 1; y < ySize - 1; y++)
    {
        auto interiorPts = splitLine(edgeYPtsLow[y], edgeYPtsHigh[y], xSize - 1);
        pts.push_back(interiorPts);
    }

    // Add low edge points
    pts.push_back(edgeXPtsHigh);
    return pts;
}
