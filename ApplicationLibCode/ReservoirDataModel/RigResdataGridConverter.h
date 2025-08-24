/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "cvfArray.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

#include <QString>
#include <vector>

class RigEclipseCaseData;
class RigCell;
class RigActiveCellInfo;
class RigGridExportAdapter;

//==================================================================================================
//
// Utility class for converting Eclipse grids to various formats
//
//==================================================================================================
class RigResdataGridConverter
{
public:
    static bool exportGrid( const QString&         gridFileName,
                            RigEclipseCaseData*    eclipseCase,
                            bool                   exportInLocalCoordinates,
                            const cvf::UByteArray* cellVisibilityOverrideForActnum = nullptr,
                            const cvf::Vec3st&     min                             = cvf::Vec3st::ZERO,
                            const cvf::Vec3st&     max                             = cvf::Vec3st::UNDEFINED,
                            const cvf::Vec3st&     refinement                      = cvf::Vec3st( 1, 1, 1 ) );

    static void convertGridToCornerPointArrays( const RigGridExportAdapter& gridAdapter,
                                                std::vector<float>&         coordArray,
                                                std::vector<float>&         zcornArray,
                                                std::vector<int>&           actnumArray );

private:
    RigResdataGridConverter() = delete;
};
