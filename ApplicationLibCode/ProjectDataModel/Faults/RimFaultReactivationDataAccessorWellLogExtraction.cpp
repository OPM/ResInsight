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

#include "RigFaultReactivationModel.h"
#include "RigFemAddressDefines.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechCaseData.h"
#include "RigGriddedPart3d.h"
#include "RigResultAccessorFactory.h"
#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigGeoMechWellLogExtractor.h"
#include "Well/RigWellPath.h"

#include "RimFaultReactivationEnums.h"
#include "RimFracture.h"
#include "RimGeoMechCase.h"
#include "RimWellIADataAccess.h"

#include "cvfGeometryTools.h"
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
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorWellLogExtraction::calculatePorBar( const RigFaultReactivationModel& model,
                                                                                                  RimFaultReactivation::GridPart   gridPart,
                                                                                                  const std::vector<cvf::Vec3d>& intersections,
                                                                                                  std::vector<double>& values,
                                                                                                  const cvf::Vec3d&    position,
                                                                                                  RimFaultReactivation::ElementSets elementSet,
                                                                                                  double gradient )
{
    if ( elementSet == RimFaultReactivation::ElementSets::OverBurden || elementSet == RimFaultReactivation::ElementSets::UnderBurden )
    {
        return { calculatePorePressure( std::abs( position.z() ), gradient ), position };
    }

    // Fill in missing values
    fillInMissingValuesWithGradient( intersections, values, gradient );
    auto [value, extractionPosition] = findValueAndPosition( intersections, values, position );

    double minDistance = computeMinimumDistance( position, intersections );
    if ( minDistance < 1.0 )
    {
        return { value, extractionPosition };
    }
    else
    {
        return { value, cvf::Vec3d::UNDEFINED };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d>
    RimFaultReactivationDataAccessorWellLogExtraction::calculateTemperature( const std::vector<cvf::Vec3d>& intersections,
                                                                             const cvf::Vec3d&              position,
                                                                             double                         seabedTemperature,
                                                                             double                         gradient )
{
    return { calculateTemperature( seabedTemperature, intersections[0].z(), std::abs( position.z() ), gradient ), position };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d>
    RimFaultReactivationDataAccessorWellLogExtraction::findValueAndPosition( const std::vector<cvf::Vec3d>& intersections,
                                                                             const std::vector<double>&     values,
                                                                             const cvf::Vec3d&              position )
{
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
    else if ( position.z() <= intersections.back().z() )
    {
        return { values.back(), intersections.back() };
    }

    return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimFaultReactivationDataAccessorWellLogExtraction::findIntersectionsForTvd( const std::vector<cvf::Vec3d>& intersections,
                                                                                                double tvd )
{
    int topIdx    = -1;
    int bottomIdx = -1;
    if ( intersections.size() >= 2 )
    {
        for ( size_t i = 1; i < intersections.size(); i++ )
        {
            auto top    = intersections[i - 1];
            auto bottom = intersections[i];
            if ( top.z() >= tvd && bottom.z() < tvd )
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
int RimFaultReactivationDataAccessorWellLogExtraction::findLastOverburdenIndex( const std::vector<double>& values )
{
    for ( size_t i = 0; i < values.size(); i++ )
    {
        if ( !std::isinf( values[i] ) ) return static_cast<int>( i );
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorWellLogExtraction::computeValueWithGradient( const std::vector<cvf::Vec3d>& intersections,
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
void RimFaultReactivationDataAccessorWellLogExtraction::fillInMissingValuesWithGradient( const std::vector<cvf::Vec3d>& intersections,
                                                                                         std::vector<double>&           values,
                                                                                         double                         gradient )
{
    CAF_ASSERT( intersections.size() == values.size() );

    auto [lastOverburdenIndex, firstUnderburdenIndex] = findOverburdenAndUnderburdenIndex( values );

    // Fill in overburden values using gradient
    double topPorePressure = calculatePorePressure( std::abs( intersections[0].z() ), gradient );
    insertOverburdenValues( intersections, values, lastOverburdenIndex, topPorePressure );

    // Fill in underburden values using gradient
    int    lastElementIndex   = static_cast<int>( values.size() ) - 1;
    double bottomPorePressure = calculatePorePressure( std::abs( intersections[lastElementIndex].z() ), gradient );
    insertUnderburdenValues( intersections, values, firstUnderburdenIndex, bottomPorePressure );

    // Interpolate the missing values (should only be intra-reservoir by now)
    std::vector<double> intersectionsZ = extractDepthValues( intersections );
    RiaInterpolationTools::interpolateMissingValues( intersectionsZ, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorWellLogExtraction::fillInMissingValuesWithTopValue( const std::vector<cvf::Vec3d>& intersections,
                                                                                         std::vector<double>&           values,
                                                                                         double                         topValue )
{
    CAF_ASSERT( intersections.size() == values.size() );

    auto [lastOverburdenIndex, firstUnderburdenIndex] = findOverburdenAndUnderburdenIndex( values );

    // Fill in overburden values using gradient
    insertOverburdenValues( intersections, values, lastOverburdenIndex, topValue );

    // Fill in underburden values
    double bottomValue = values[firstUnderburdenIndex - 1];
    insertUnderburdenValues( intersections, values, firstUnderburdenIndex, bottomValue );

    // Interpolate the missing values (should only be intra-reservoir by now)
    std::vector<double> intersectionsZ = extractDepthValues( intersections );
    RiaInterpolationTools::interpolateMissingValues( intersectionsZ, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( const cvf::Vec3d& faultTopPosition,
                                                                                               const cvf::Vec3d& faultBottomPosition,
                                                                                               double            seabedDepth,
                                                                                               double            bottomDepth,
                                                                                               const cvf::Vec3d& offset )
{
    cvf::Vec3d faultTop = faultTopPosition + offset;
    cvf::Vec3d seabed( faultTop.x(), faultTop.y(), seabedDepth );
    cvf::Vec3d faultBottom = faultBottomPosition + offset;
    cvf::Vec3d underburdenBottom( faultBottom.x(), faultBottom.y(), bottomDepth );
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
                                                                                        RigEclipseCaseData&              eclipseCaseData,
                                                                                        double                           seabedDepth )
{
    auto [faultTopPosition, faultBottomPosition] = model.faultTopBottom();
    auto faultNormal                             = model.modelNormal() ^ cvf::Vec3d::Z_AXIS;
    faultNormal.normalize();

    double distanceFromFault     = 1.0;
    auto [topDepth, bottomDepth] = model.depthTopBottom();

    std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>                wellPaths;
    std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>> extractors;

    for ( auto gridPart : model.allGridParts() )
    {
        double                  sign = model.normalPointsAt() == gridPart ? -1.0 : 1.0;
        std::vector<cvf::Vec3d> wellPoints =
            RimFaultReactivationDataAccessorWellLogExtraction::generateWellPoints( faultTopPosition,
                                                                                   faultBottomPosition,
                                                                                   seabedDepth,
                                                                                   bottomDepth,
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorWellLogExtraction::computeGradient( double depth1, double value1, double depth2, double value2 )
{
    return ( value2 - value1 ) / ( depth2 - depth1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFaultReactivationDataAccessorWellLogExtraction::extractDepthValues( const std::vector<cvf::Vec3d>& intersections )
{
    std::vector<double> intersectionsZ;
    for ( auto i : intersections )
    {
        intersectionsZ.push_back( -i.z() );
    }
    return intersectionsZ;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorWellLogExtraction::insertUnderburdenValues( const std::vector<cvf::Vec3d>& intersections,
                                                                                 std::vector<double>&           values,
                                                                                 int                            firstUnderburdenIndex,
                                                                                 double                         bottomValue )
{
    int    lastElementIndex    = static_cast<int>( values.size() ) - 1;
    double underburdenGradient = computeGradient( intersections[firstUnderburdenIndex].z(),
                                                  values[firstUnderburdenIndex],
                                                  intersections[lastElementIndex].z(),
                                                  bottomValue );

    for ( int i = lastElementIndex; i >= firstUnderburdenIndex; i-- )
    {
        values[i] = computeValueWithGradient( intersections, values, i, firstUnderburdenIndex, -underburdenGradient );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorWellLogExtraction::insertOverburdenValues( const std::vector<cvf::Vec3d>& intersections,
                                                                                std::vector<double>&           values,
                                                                                int                            lastOverburdenIndex,
                                                                                double                         topValue )
{
    double overburdenGradient =
        computeGradient( intersections[0].z(), topValue, intersections[lastOverburdenIndex].z(), values[lastOverburdenIndex] );

    for ( int i = 0; i < lastOverburdenIndex; i++ )
    {
        values[i] = computeValueWithGradient( intersections, values, i, lastOverburdenIndex, -overburdenGradient );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorWellLogExtraction::computeMinimumDistance( const cvf::Vec3d&              position,
                                                                                  const std::vector<cvf::Vec3d>& positions )
{
    double minDistance = std::numeric_limits<double>::max();
    for ( size_t i = 1; i < positions.size(); i++ )
    {
        cvf::Vec3d point1 = positions[i - 1];
        cvf::Vec3d point2 = positions[i - 0];

        double candidateDistance = cvf::GeometryTools::linePointSquareDist( point1, point2, position );
        minDistance              = std::min( candidateDistance, minDistance );
    }

    return std::sqrt( minDistance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, RimFaultReactivation::ElementSets> RimFaultReactivationDataAccessorWellLogExtraction::findElementSetForPoint(
    const RigGriddedPart3d&                                                       part,
    const cvf::Vec3d&                                                             point,
    const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets )
{
    for ( auto [elementSet, elements] : elementSets )
    {
        for ( unsigned int elementIndex : elements )
        {
            // Find nodes from element
            auto positions = part.elementCorners( elementIndex );

            std::array<cvf::Vec3d, 8> coordinates;
            for ( size_t i = 0; i < positions.size(); ++i )
            {
                coordinates[i] = positions[i];
            }

            if ( RigHexIntersectionTools::isPointInCell( point, coordinates ) )
            {
                return { true, elementSet };
            }
        }
    }

    return { false, RimFaultReactivation::ElementSets::OverBurden };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorWellLogExtraction::calculatePorePressure( double depth, double gradient )
{
    return RiaEclipseUnitTools::pascalToBar( gradient * 9.81 * depth * 1000.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorWellLogExtraction::calculateTemperature( double topValue, double topDepth, double depth, double gradient )
{
    double tvdDiff = topDepth - depth;
    return tvdDiff * gradient + topValue;
}
