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

#pragma once
#include <cmath>

//==================================================================================================
///  
/// 
//==================================================================================================

struct RigWellLogExtractionTools
{
    static bool isEqualDepth(double d1, double d2)
    {
        double depthDiff = d1 - d2;

        const double tolerance = 0.1;// Meters To handle inaccuracies across faults

        return (fabs(depthDiff) < tolerance); // Equal depth
    }
};

//==================================================================================================
///  Class used to sort the intersections along the wellpath,
///  Use as key in a map
/// Sorting according to MD first, then Cell Idx, then Leaving before entering cell
//==================================================================================================

struct RigMDCellIdxEnterLeaveKey
{
    RigMDCellIdxEnterLeaveKey(double md, size_t cellIdx, bool entering): measuredDepth(md), hexIndex(cellIdx), isEnteringCell(entering){}

    double measuredDepth;
    size_t hexIndex;
    bool   isEnteringCell; // As opposed to leaving.
    bool   isLeavingCell() const { return !isEnteringCell;}

    bool operator < (const RigMDCellIdxEnterLeaveKey& other) const 
    {
        if (RigWellLogExtractionTools::isEqualDepth(measuredDepth, other.measuredDepth))
        {
            if (hexIndex == other.hexIndex)
            {
                if (isEnteringCell == other.isEnteringCell)
                {
                    // Completely equal: intersections at cell edges or corners or edges of the face triangles
                    return false;
                }

                return !isEnteringCell; // Sort Leaving cell before (less than) entering cell
            }

            return (hexIndex < other.hexIndex);
        }

        // The depths are not equal

        return (measuredDepth < other.measuredDepth);
    }
};


//==================================================================================================
///  Class used to sort the intersections along the wellpath,
///  Use as key in a map
/// Sorting according to MD first,then Leaving before entering cell, then Cell Idx, 
//==================================================================================================

struct RigMDEnterLeaveCellIdxKey
{
    RigMDEnterLeaveCellIdxKey(double md, bool entering, size_t cellIdx ): measuredDepth(md), hexIndex(cellIdx), isEnteringCell(entering){}

    double measuredDepth;
    bool   isEnteringCell; // As opposed to leaving.
    bool   isLeavingCell() const { return !isEnteringCell;}
    size_t hexIndex;

    bool operator < (const RigMDEnterLeaveCellIdxKey& other) const
    {
        if (RigWellLogExtractionTools::isEqualDepth(measuredDepth, other.measuredDepth))
        {
            if (isEnteringCell == other.isEnteringCell)
            {
                if (hexIndex == other.hexIndex)
                {
                    // Completely equal: intersections at cell edges or corners or edges of the face triangles
                    return false;
                }

                return (hexIndex < other.hexIndex);
            }

            return isLeavingCell(); // Sort Leaving cell before (less than) entering cell
        }

        // The depths are not equal

        return (measuredDepth < other.measuredDepth);
    }

    static bool isProperCellEnterLeavePair(const RigMDEnterLeaveCellIdxKey& key1, const RigMDEnterLeaveCellIdxKey& key2 )
    {
        return ( key1.hexIndex == key2.hexIndex 
                && key1.isEnteringCell && key2.isLeavingCell()
                && !RigWellLogExtractionTools::isEqualDepth(key1.measuredDepth, key2.measuredDepth));
    }
};


