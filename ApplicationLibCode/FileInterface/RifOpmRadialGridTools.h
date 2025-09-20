/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "cvfVector3.h"

#include <map>
#include <string>
#include <vector>

namespace Opm
{
namespace EclIO
{
    class EGrid;
}
} // namespace Opm

class RigMainGrid;
class RigGridBase;
class RigEclipseCaseData;

//==================================================================================================
///
//==================================================================================================
class RifOpmRadialGridTools
{
public:
    // If the grid is radial, the coordinates are imported and adjusted to fit the host cells
    static bool importCoordinatesForRadialGrid( const std::string& gridFilePath, RigMainGrid* mainGrid );

    static bool importCylindricalCoordinates( const std::string& gridFilePath, RigEclipseCaseData* caseData );

    static std::map<int, std::pair<double, double>>
        computeXyCenterForTopOfCells( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, RigGridBase* riGrid );

    static void lockToHostPillars( cvf::Vec3d&         riNode,
                                   Opm::EclIO::EGrid&  opmMainGrid,
                                   Opm::EclIO::EGrid&  opmGrid,
                                   std::array<int, 3>& ijkCell,
                                   int                 hostCellIndex,
                                   int                 opmCellIndex,
                                   size_t              opmNodeIndex,
                                   double              xCenterCoordOpm,
                                   double              yCenterCoordOpm );

    static bool createAngularGridRefinement( RigEclipseCaseData* caseData, size_t angularRefinement );
    static void convertNodesToCartesian( std::vector<cvf::Vec3d>& nodes );

private:
    static void
        transferCylindricalCoords( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, RigMainGrid* riMainGrid, RigGridBase* riGrid );

    static void
        transferCoordinatesRadial( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, RigMainGrid* riMainGrid, RigGridBase* riGrid );

    static std::vector<std::vector<cvf::Vec3d>>
        computeSnapToCoordinates( Opm::EclIO::EGrid& opmMainGrid, Opm::EclIO::EGrid& opmGrid, int mainGridCellIndex, int lgrCellIndex );
};
