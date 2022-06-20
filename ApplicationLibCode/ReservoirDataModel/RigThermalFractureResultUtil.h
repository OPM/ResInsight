/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "RiaDefines.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <QString>

#include <memory>
#include <vector>

class RigThermalFractureDefinition;
class RigFractureGrid;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigThermalFractureResultUtil
{
public:
    RigThermalFractureResultUtil();
    ~RigThermalFractureResultUtil();

    static std::vector<std::vector<double>>
        getDataAtTimeIndex( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                            const QString&                                      resultName,
                            const QString&                                      unitName,
                            size_t                                              timeStepIndex );

    static void createFractureTriangleGeometry( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                double                                              xScaleFactor,
                                                double                                              yScaleFactor,
                                                double                   wellPathIntersectionAtFractureDepth,
                                                std::vector<cvf::Vec3f>* vertices,
                                                std::vector<cvf::uint>*  triangleIndices );

    static std::vector<double> fractureGridResults( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                    const QString&                                      resultName,
                                                    const QString&                                      unitName,
                                                    size_t                                              timeStepIndex );

    static cvf::cref<RigFractureGrid>
        createFractureGrid( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                            const QString&                                      resultName,
                            int                                                 activeTimeStepIndex,
                            double                                              xScaleFactor,
                            double                                              yScaleFactor,
                            double                                              wellPathIntersectionAtFractureDepth,
                            RiaDefines::EclipseUnitSystem                       requiredUnitSet );
};
