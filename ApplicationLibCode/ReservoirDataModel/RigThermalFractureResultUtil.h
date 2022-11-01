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

#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <QString>

#include <memory>
#include <vector>

class RigThermalFractureDefinition;
class RigFractureGrid;

class MinMaxAccumulator;
class PosNegAccumulator;

namespace cvf
{
class BoundingBox;
}; // namespace cvf

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
                                                int                                                 activeTimeStepIndex,
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

    static void appendDataToResultStatistics( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                              const QString&                                      resultName,
                                              const QString&                                      unit,
                                              MinMaxAccumulator&                                  minMaxAccumulator,
                                              PosNegAccumulator&                                  posNegAccumulator );

    static std::pair<double, double> minMaxDepth( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                                  int activeTimeStepIndex );

    static std::pair<cvf::Vec3d, cvf::Vec3d>
        computePositionAndRotation( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                    size_t                                              timeStepIndex );

private:
    static std::pair<std::vector<double>, std::vector<double>>
        generateUniformMesh( const cvf::BoundingBox& bb, int numSamplesX, int numSamplesY );

    static double linearSampling( double minValue, double maxValue, int numSamples, std::vector<double>& samples );

    static cvf::Mat4d rotationMatrixBetweenVectors( const cvf::Vec3d& v1, const cvf::Vec3d& v2 );

    static std::vector<cvf::Vec3d>
        getRelativeCoordinates( std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                size_t                                              timeStepIndex );

    static std::vector<double> scaleVector( const std::vector<double>& xs, double scaleFactor );

    static std::vector<double> adjustedYCoordsAroundWellPathPosition( const std::vector<double>& ys, double offset );

    static double interpolateProperty( const cvf::Vec3d&                                   position,
                                       const std::vector<cvf::Vec3d>&                      points,
                                       std::shared_ptr<const RigThermalFractureDefinition> fractureDefinition,
                                       size_t                                              propertyIndex,
                                       size_t                                              timeStepIndex );

    static const int NUM_SAMPLES_X = 50;
    static const int NUM_SAMPLES_Y = 40;
};
