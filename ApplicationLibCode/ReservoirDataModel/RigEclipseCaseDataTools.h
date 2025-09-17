/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "QString"

#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <utility>

class RigEclipseCaseData;
class RigSimWellData;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigEclipseCaseDataTools
{
public:
    static QString firstProducer( RigEclipseCaseData* eclipseCaseData );

    static cvf::BoundingBox wellBoundingBoxInDomainCoords( RigEclipseCaseData*   eclipseCaseData,
                                                           const RigSimWellData* simWellData,
                                                           int                   timeStepIndex,
                                                           bool                  isAutoDetectingBranches,
                                                           bool                  isUsingCellCenterForPipe );

    static std::pair<cvf::Vec3st, cvf::Vec3st> wellBoundingBoxIjk( RigEclipseCaseData*   eclipseCaseData,
                                                                   const RigSimWellData* simWellData,
                                                                   int                   timeStepIndex,
                                                                   bool                  isAutoDetectingBranches,
                                                                   bool                  isUsingCellCenterForPipe );

    static std::pair<cvf::Vec3st, cvf::Vec3st> wellsBoundingBoxIjk( RigEclipseCaseData*                       eclipseCaseData,
                                                                    const std::vector<const RigSimWellData*>& simWells,
                                                                    int                                       timeStepIndex,
                                                                    bool                                      isAutoDetectingBranches,
                                                                    bool                                      isUsingCellCenterForPipe );

    static std::pair<cvf::Vec3st, cvf::Vec3st>
        expandBoundingBoxIjk( RigEclipseCaseData* eclipseCaseData, const cvf::Vec3st& minIjk, const cvf::Vec3st& maxIjk, size_t numPadding );

    static cvf::ref<cvf::UByteArray>
        createVisibilityFromIjkBounds( RigEclipseCaseData* eclipseCaseData, const cvf::Vec3st& minIjk, const cvf::Vec3st& maxIjk );
};
