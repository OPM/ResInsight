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

#include "RigGriddedPart3d.h"
#include "RimFaultReactivationDataAccessor.h"
#include "RimFaultReactivationEnums.h"

#include "cvfObject.h"

#include <vector>

class RigWellPath;
class RigEclipseWellLogExtractor;
class RigEclipseCaseData;
class RigResultAccessor;
class RigGriddedPart3d;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorWellLogExtraction
{
public:
    RimFaultReactivationDataAccessorWellLogExtraction();
    ~RimFaultReactivationDataAccessorWellLogExtraction();

    static std::pair<double, cvf::Vec3d> calculatePorBar( const RigFaultReactivationModel&  model,
                                                          RimFaultReactivation::GridPart    gridPart,
                                                          const std::vector<cvf::Vec3d>&    intersections,
                                                          std::vector<double>&              values,
                                                          const cvf::Vec3d&                 position,
                                                          RimFaultReactivation::ElementSets elementSet,
                                                          double                            gradient );

    static std::pair<double, cvf::Vec3d> calculateTemperature( const std::vector<cvf::Vec3d>& intersections,
                                                               const cvf::Vec3d&              position,
                                                               double                         seabedTemperature,
                                                               double                         gradient );
    static std::pair<std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>,
                     std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>>>
        createEclipseWellPathExtractors( const RigFaultReactivationModel& model, RigEclipseCaseData& eclipseCaseData, double seabedDepth );

    static std::vector<cvf::Vec3d> generateWellPoints( const cvf::Vec3d& faultTopPosition,
                                                       const cvf::Vec3d& faultBottomPosition,
                                                       double            seabedDepth,
                                                       double            bottomDepth,
                                                       const cvf::Vec3d& offset );

    static std::vector<double> generateMds( const std::vector<cvf::Vec3d>& points );

    static std::pair<std::vector<double>, std::vector<cvf::Vec3d>> extractValuesAndIntersections( const RigResultAccessor& resultAccessor,
                                                                                                  RigEclipseWellLogExtractor& extractor,
                                                                                                  const RigWellPath&          wellPath );
    static std::pair<int, int> findIntersectionsForTvd( const std::vector<cvf::Vec3d>& intersections, double tvd );

    static std::pair<bool, RimFaultReactivation::ElementSets>
        findElementSetForPoint( const RigGriddedPart3d&                                                       part,
                                const cvf::Vec3d&                                                             point,
                                const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets );

    static int findLastOverburdenIndex( const std::vector<double>& values );

    static double computeGradient( double depth1, double value1, double depth2, double value2 );

protected:
    static std::pair<int, int> findOverburdenAndUnderburdenIndex( const std::vector<double>& values );
    static double              computeValueWithGradient( const std::vector<cvf::Vec3d>& intersections,
                                                         const std::vector<double>&     values,
                                                         int                            i1,
                                                         int                            i2,
                                                         double                         gradient );
    static void fillInMissingValuesWithGradient( const std::vector<cvf::Vec3d>& intersections, std::vector<double>& values, double gradient );
    static void fillInMissingValuesWithTopValue( const std::vector<cvf::Vec3d>& intersections, std::vector<double>& values, double topValue );

    static std::pair<double, cvf::Vec3d>
        findValueAndPosition( const std::vector<cvf::Vec3d>& intersections, const std::vector<double>& values, const cvf::Vec3d& position );

    static std::vector<double> extractDepthValues( const std::vector<cvf::Vec3d>& intersections );

    static void   insertUnderburdenValues( const std::vector<cvf::Vec3d>& intersections,
                                           std::vector<double>&           values,
                                           int                            firstUnderburdenIndex,
                                           double                         bottomValue );
    static void   insertOverburdenValues( const std::vector<cvf::Vec3d>& intersections,
                                          std::vector<double>&           values,
                                          int                            lastOverburdenIndex,
                                          double                         topValue );
    static double computeMinimumDistance( const cvf::Vec3d& position, const std::vector<cvf::Vec3d>& positions );

    static double calculatePorePressure( double depth, double gradient );
    static double calculateTemperature( double topValue, double topDepth, double depth, double gradient );
};
