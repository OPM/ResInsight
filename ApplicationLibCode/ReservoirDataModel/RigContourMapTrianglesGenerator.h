/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RigContourPolygonsTools.h"

class RigContourMapGrid;
class RigContourMapProjection;

//==================================================================================================
///
///
//==================================================================================================
class RigContourMapTrianglesGenerator
{
public:
    using ContourPolygons = std::vector<RigContourPolygonsTools::ContourPolygon>;

    static std::vector<cvf::Vec4d> generateTrianglesWithVertexValues( const RigContourMapGrid&            contourMapGrid,
                                                                      const RigContourMapProjection&      contourMapProjection,
                                                                      const std::vector<ContourPolygons>& contourPolygons,
                                                                      const std::vector<double>&          contourLevels,
                                                                      const std::vector<double>&          contourLevelCumulativeAreas,
                                                                      bool                                discrete,
                                                                      double                              sampleSpacing );

    static std::pair<std::vector<ContourPolygons>, std::vector<double>>
        generateContourPolygons( const RigContourMapGrid&       contourMapGrid,
                                 const RigContourMapProjection& contourMapProjection,
                                 const std::vector<double>&     initialContourLevels,
                                 double                         sampleSpacing,
                                 double                         sampleSpacingFactor,
                                 bool                           smoothContourLines );
};
