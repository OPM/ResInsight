/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include <gsl/gsl>

#include <vector>

class RimWellPath;
class RigWellPath;

//==================================================================================================
///
//==================================================================================================
class RigWellPathGeometryExporter
{
public:
    static void computeWellPathDataForExport( gsl::not_null<const RimWellPath*> wellPath,
                                              double                            mdStepSize,
                                              std::vector<double>&              xValues,
                                              std::vector<double>&              yValues,
                                              std::vector<double>&              tvdValues,
                                              std::vector<double>&              mdValues,
                                              bool&                             showTextMdRkb );

    static void computeWellPathDataForExport( const RigWellPath&   wellPath,
                                              double               mdStepSize,
                                              double               rkbOffset,
                                              std::vector<double>& xValues,
                                              std::vector<double>& yValues,
                                              std::vector<double>& tvdValues,
                                              std::vector<double>& mdValues );
};
