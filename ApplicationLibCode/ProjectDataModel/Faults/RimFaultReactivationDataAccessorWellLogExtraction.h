/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -    Equinor ASA
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

#include "RimFaultReactivationDataAccessor.h"
#include "RimFaultReactivationEnums.h"

#include "RigFemResultAddress.h"

#include <vector>

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RigFemPart;
class RigFemPartResultsCollection;
class RigFemScalarResultFrames;
class RigGeoMechCaseData;
class RigGriddedPart3d;
class RimGeoMechCase;
class RimWellIADataAccess;
class RimModeledWellPath;
class RigWellPath;
class RigFemPartCollection;
class RigGeoMechWellLogExtractor;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorWellLogExtraction
{
public:
    RimFaultReactivationDataAccessorWellLogExtraction();
    ~RimFaultReactivationDataAccessorWellLogExtraction();

    static std::pair<double, cvf::Vec3d>
        getPorBar( const std::vector<cvf::Vec3d>& intersections, std::vector<double>& values, const cvf::Vec3d& position, double gradient );

protected:
    static std::pair<int, int> findIntersectionsForTvd( const std::vector<cvf::Vec3d>& intersections, double tvd );
    static std::pair<int, int> findOverburdenAndUnderburdenIndex( const std::vector<double>& values );
    static double              computePorBarWithGradient( const std::vector<cvf::Vec3d>& intersections,
                                                          const std::vector<double>&     values,
                                                          int                            i1,
                                                          int                            i2,
                                                          double                         gradient );
    static void fillInMissingValues( const std::vector<cvf::Vec3d>& intersections, std::vector<double>& values, double gradient );

    static std::vector<cvf::Vec3d>
        generateWellPoints( const cvf::Vec3d& faultTopPosition, const cvf::Vec3d& faultBottomPosition, const cvf::Vec3d& offset );

    static std::vector<double> generateMds( const std::vector<cvf::Vec3d>& points );
};