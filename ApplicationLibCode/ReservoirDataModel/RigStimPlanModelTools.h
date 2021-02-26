/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include <QString>

#include <vector>

class RigEclipseCaseData;
class RigFault;

struct WellPathCellIntersectionInfo;

//==================================================================================================
///
///
//==================================================================================================
class RigStimPlanModelTools
{
public:
    static cvf::Vec3d calculateTSTDirection( RigEclipseCaseData* eclipseCaseData,
                                             const cvf::Vec3d&   anchorPosition,
                                             double              boundingBoxHorizontal,
                                             double              boundingBoxVertical );

    static bool findThicknessTargetPoints( RigEclipseCaseData* eclipseCaseData,
                                           const cvf::Vec3d&   position,
                                           const cvf::Vec3d&   direction,
                                           double              extractionDepthTop,
                                           double              extractionDepthBottom,
                                           cvf::Vec3d&         topPosition,

                                           cvf::Vec3d& bottomPosition );

    static double calculateFormationDip( const cvf::Vec3d& direction );

    static QString vecToString( const cvf::Vec3d& vec );

    static std::tuple<const RigFault*, double, cvf::Vec3d, double>
        findClosestFaultBarrier( RigEclipseCaseData* eclipseCaseData,
                                 const cvf::Vec3d&   position,
                                 const cvf::Vec3d&   directionToBarrier );

    static std::vector<WellPathCellIntersectionInfo> generateBarrierIntersections( RigEclipseCaseData* eclipseCaseData,
                                                                                   const cvf::Vec3d&   position,
                                                                                   const cvf::Vec3d& directionToBarrier );

    static std::vector<WellPathCellIntersectionInfo>
        generateBarrierIntersectionsBetweenPoints( RigEclipseCaseData* eclipseCaseData,
                                                   const cvf::Vec3d&   startPosition,
                                                   const cvf::Vec3d&   endPosition );
};
