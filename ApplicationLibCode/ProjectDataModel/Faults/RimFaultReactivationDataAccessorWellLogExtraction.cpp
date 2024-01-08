/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023  Equinor ASA
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

#include "RimFaultReactivationDataAccessorWellLogExtraction.h"

#include "RiaEclipseUnitTools.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigFaultReactivationModel.h"
#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigGriddedPart3d.h"
#include "RigResultAccessorFactory.h"
#include "RigWellPath.h"

#include "RimFaultReactivationEnums.h"
#include "RimFracture.h"
#include "RimGeoMechCase.h"
#include "RimWellIADataAccess.h"

#include "cvfVector3.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorWellLogExtraction::RimFaultReactivationDataAccessorWellLogExtraction()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorWellLogExtraction::~RimFaultReactivationDataAccessorWellLogExtraction()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( const std::vector<cvf::Vec3d>& intersections,
                                                                                                  std::vector<double>&           values,
                                                                                                  const cvf::Vec3d&              position,
                                                                                                  double                         gradient )
{
    // Fill in missing values
    fillInMissingValues( intersections, values, gradient );

    // Linear interpolation between two points
    auto lerp = []( const cvf::Vec3d& start, const cvf::Vec3d& end, double t ) { return start + t * ( end - start ); };

    auto [topIdx, bottomIdx] = findIntersectionsForTvd( intersections, position.z() );
    if ( topIdx != -1 && bottomIdx != -1 )
    {
        double topValue    = values[topIdx];
        double bottomValue = values[bottomIdx];
        if ( !std::isinf( topValue ) && !std::isinf( bottomValue ) )
        {
            // Interpolate value from the two closest points.
            std::vector<double> xs     = { intersections[bottomIdx].z(), intersections[topIdx].z() };
            std::vector<double> ys     = { values[bottomIdx], values[topIdx] };
            double              porBar = RiaInterpolationTools::linear( xs, ys, position.z() );

            // Interpolate position from depth
            double     fraction           = RiaInterpolationTools::linear( xs, { 0.0, 1.0 }, position.z() );
            cvf::Vec3d extractionPosition = lerp( intersections[bottomIdx], intersections[topIdx], fraction );
            return { porBar, extractionPosition };
        }
    }

    return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimFaultReactivationDataAccessorWellLogExtraction::findIntersectionsForTvd( const std::vector<cvf::Vec3d>& intersections,
                                                                                                double                         tvd )
{
    int topIdx    = -1;
    int bottomIdx = -1;
    if ( intersections.size() >= 2 )
    {
        for ( size_t i = 1; i < intersections.size(); i++ )
        {
            auto top    = intersections[i - 1];
            auto bottom = intersections[i];
            if ( top.z() > tvd && bottom.z() < tvd )
            {
                topIdx    = static_cast<int>( i ) - 1;
                bottomIdx = static_cast<int>( i );
                break;
            }
        }
    }
    return std::make_pair( topIdx, bottomIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimFaultReactivationDataAccessorWellLogExtraction::findOverburdenAndUnderburdenIndex( const std::vector<double>& values )
{
    auto findLastOverburdenIndex = []( const std::vector<double>& values )
    {
        for ( size_t i = 0; i < values.size(); i++ )
        {
            if ( !std::isinf( values[i] ) ) return static_cast<int>( i );
        }

        return -1;
    };

    auto findFirstUnderburdenIndex = []( const std::vector<double>& values )
    {
        for ( size_t i = values.size() - 1; i > 0; i-- )
        {
            if ( !std::isinf( values[i] ) ) return static_cast<int>( i );
        }

        return -1;
    };

    int lastOverburdenIndex   = findLastOverburdenIndex( values );
    int firstUnderburdenIndex = findFirstUnderburdenIndex( values );
    return { lastOverburdenIndex, firstUnderburdenIndex };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorWellLogExtraction::computePorBarWithGradient( const std::vector<cvf::Vec3d>& intersections,
                                                                                     const std::vector<double>&     values,
                                                                                     int                            i1,
                                                                                     int                            i2,
                                                                                     double                         gradient )
{
    double tvdDiff = intersections[i2].z() - intersections[i1].z();
    return tvdDiff * gradient + values[i2];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorWellLogExtraction::fillInMissingValues( const std::vector<cvf::Vec3d>& intersections,
                                                                             std::vector<double>&           values,
                                                                             double                         gradient )
{
    CAF_ASSERT( intersections.size() == values.size() );

    auto calculatePorePressure = []( double depth, double gradient )
    { return RiaEclipseUnitTools::pascalToBar( gradient * 9.81 * depth * 1000.0 ); };

    auto computeGradient = []( double depth1, double value1, double depth2, double value2 )
    { return ( value2 - value1 ) / ( depth2 - depth1 ); };

    auto [lastOverburdenIndex, firstUnderburdenIndex] = findOverburdenAndUnderburdenIndex( values );

    // Fill in overburden values using gradient
    double topPorePressure = calculatePorePressure( std::abs( intersections[0].z() ), gradient );
    double overburdenGradient =
        computeGradient( intersections[0].z(), topPorePressure, intersections[lastOverburdenIndex].z(), values[lastOverburdenIndex] );

    for ( int i = 0; i < lastOverburdenIndex; i++ )
    {
        values[i] = computePorBarWithGradient( intersections, values, i, lastOverburdenIndex, -overburdenGradient );
    }

    // Fill in underburden values using gradient
    int    lastElementIndex    = static_cast<int>( values.size() ) - 1;
    double bottomPorePressure  = calculatePorePressure( std::abs( intersections[lastElementIndex].z() ), gradient );
    double underburdenGradient = computeGradient( intersections[firstUnderburdenIndex].z(),
                                                  values[firstUnderburdenIndex],
                                                  intersections[lastElementIndex].z(),
                                                  bottomPorePressure );

    for ( int i = lastElementIndex; i >= firstUnderburdenIndex; i-- )
    {
        values[i] = computePorBarWithGradient( intersections, values, i, firstUnderburdenIndex, -underburdenGradient );
    }

    // Interpolate the missing values (should only be intra-reservoir by now)
    std::vector<double> intersectionsZ;
    for ( auto i : intersections )
    {
        intersectionsZ.push_back( i.z() );
    }

    RiaInterpolationTools::interpolateMissingValues( intersectionsZ, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( const cvf::Vec3d& faultTopPosition,
                                                                                               const cvf::Vec3d& faultBottomPosition,
                                                                                               const cvf::Vec3d& offset )
{
    cvf::Vec3d faultTop = faultTopPosition + offset;
    cvf::Vec3d seabed( faultTop.x(), faultTop.y(), 0.0 );
    cvf::Vec3d faultBottom = faultBottomPosition + offset;
    cvf::Vec3d underburdenBottom( faultBottom.x(), faultBottom.y(), -10000.0 );
    return { seabed, faultTop, faultBottom, underburdenBottom };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFaultReactivationDataAccessorWellLogExtraction::generateMds( const std::vector<cvf::Vec3d>& points )
{
    CAF_ASSERT( points.size() >= 2 );

    // Assume first at zero, all other points relative to that.
    std::vector<double> mds = { 0.0 };
    double              sum = 0.0;
    for ( size_t i = 1; i < points.size(); i++ )
    {
        sum += points[i - 1].pointDistance( points[i] );
        mds.push_back( sum );
    }
    return mds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>, std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>>>
    RimFaultReactivationDataAccessorWellLogExtraction::createEclipseWellPathExtractors( const RigFaultReactivationModel& model,
                                                                                        RigEclipseCaseData&              eclipseCaseData )
{
    auto [faultTopPosition, faultBottomPosition] = model.faultTopBottom();
    auto   faultNormal                           = model.faultNormal();
    double distanceFromFault                     = 1.0;

    std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>                wellPaths;
    std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>> extractors;

    for ( auto gridPart : model.allGridParts() )
    {
        double                  sign = model.normalPointsAt() == gridPart ? 1.0 : -1.0;
        std::vector<cvf::Vec3d> wellPoints =
            RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( faultTopPosition,
                                                                                   faultBottomPosition,
                                                                                   sign * faultNormal * distanceFromFault );
        cvf::ref<RigWellPath> wellPath =
            new RigWellPath( wellPoints, RimFaultReactivationDataAccessorWellLogExtraction::generateMds( wellPoints ) );
        wellPaths[gridPart] = wellPath;

        std::string                          errorName = "fault reactivation data access";
        cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor( &eclipseCaseData, wellPath.p(), errorName );
        extractors[gridPart]                           = extractor;
    }

    return { wellPaths, extractors };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::vector<cvf::Vec3d>>
    RimFaultReactivationDataAccessorWellLogExtraction::extractValuesAndIntersections( const RigResultAccessor&    resultAccessor,
                                                                                      RigEclipseWellLogExtractor& extractor,
                                                                                      const RigWellPath&          wellPath )
{
    // Extract values along well path
    std::vector<double> values;
    extractor.curveData( &resultAccessor, &values );

    auto intersections = extractor.intersections();

    // Insert top of overburden point
    intersections.insert( intersections.begin(), wellPath.wellPathPoints().front() );
    values.insert( values.begin(), std::numeric_limits<double>::infinity() );

    // Insert bottom of underburden point
    intersections.push_back( wellPath.wellPathPoints().back() );
    values.push_back( std::numeric_limits<double>::infinity() );

    return { values, intersections };
}
