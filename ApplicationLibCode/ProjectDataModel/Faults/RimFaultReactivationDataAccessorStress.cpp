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

#include "RimFaultReactivationDataAccessorStress.h"

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
RimFaultReactivationDataAccessorStress::RimFaultReactivationDataAccessorStress( RimGeoMechCase*                geoMechCase,
                                                                                RimFaultReactivation::Property property,
                                                                                double                         gradient )
    : m_geoMechCase( geoMechCase )
    , m_property( property )
    , m_gradient( gradient )
{
    m_geoMechCaseData = geoMechCase->geoMechData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultReactivationDataAccessorStress::~RimFaultReactivationDataAccessorStress()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationDataAccessorStress::updateResultAccessor()
{
    const int partIndex = 0;

    auto loadFrameLambda = [&]( auto femParts, RigFemResultAddress addr, int timeStepIndex ) -> RigFemScalarResultFrames*
    {
        auto result     = femParts->findOrLoadScalarResult( partIndex, addr );
        int  frameIndex = result->frameCount( timeStepIndex ) - 1;
        if ( result->frameData( timeStepIndex, frameIndex ).empty() )
        {
            return nullptr;
        }
        return result;
    };

    auto femParts     = m_geoMechCaseData->femPartResults();
    m_femPart         = femParts->parts()->part( partIndex );
    int timeStepIndex = 0;
    m_s33Frames       = loadFrameLambda( femParts, getResultAddress( "ST", "S33" ), timeStepIndex );
    m_s11Frames       = loadFrameLambda( femParts, getResultAddress( "ST", "S11" ), timeStepIndex );
    m_s22Frames       = loadFrameLambda( femParts, getResultAddress( "ST", "S22" ), timeStepIndex );

    auto [faultTopPosition, faultBottomPosition] = m_model->faultTopBottom();
    auto   faultNormal                           = m_model->faultNormal();
    double distanceFromFault                     = 1.0;

    RigFemPartCollection* geoMechPartCollection = m_geoMechCaseData->femParts();
    std::string           errorName             = "fault reactivation data access";

    {
        std::vector<cvf::Vec3d> wellPoints = generateWellPoints( faultTopPosition, faultBottomPosition, faultNormal * distanceFromFault );
        m_faceAWellPath                    = new RigWellPath( wellPoints, generateMds( wellPoints ) );
        m_partIndexA                       = getPartIndexFromPoint( *geoMechPartCollection, wellPoints[1] );
        m_extractorA                       = new RigGeoMechWellLogExtractor( m_geoMechCaseData, partIndex, m_faceAWellPath.p(), errorName );
    }

    {
        std::vector<cvf::Vec3d> wellPoints = generateWellPoints( faultTopPosition, faultBottomPosition, -faultNormal * distanceFromFault );
        m_faceBWellPath                    = new RigWellPath( wellPoints, generateMds( wellPoints ) );
        m_partIndexB                       = getPartIndexFromPoint( *geoMechPartCollection, wellPoints[1] );
        m_extractorB                       = new RigGeoMechWellLogExtractor( m_geoMechCaseData, partIndex, m_faceBWellPath.p(), errorName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultReactivationDataAccessorStress::getPartIndexFromPoint( const RigFemPartCollection& partCollection, const cvf::Vec3d& point )
{
    const int idx = 0;

    // Find candidates for intersected global elements
    const cvf::BoundingBox intersectingBb( point, point );
    std::vector<size_t>    intersectedGlobalElementIndexCandidates;
    partCollection.findIntersectingGlobalElementIndices( intersectingBb, &intersectedGlobalElementIndexCandidates );

    if ( intersectedGlobalElementIndexCandidates.empty() ) return idx;

    // Iterate through global element candidates and check if point is in hexCorners
    for ( const auto& globalElementIndex : intersectedGlobalElementIndexCandidates )
    {
        const auto [part, elementIndex] = partCollection.partAndElementIndex( globalElementIndex );

        // Find nodes from element
        std::array<cvf::Vec3d, 8> coordinates;
        const bool                isSuccess = part->fillElementCoordinates( elementIndex, coordinates );
        if ( !isSuccess ) continue;

        const bool isPointInCell = RigHexIntersectionTools::isPointInCell( point, coordinates.data() );
        if ( isPointInCell ) return part->elementPartId();
    }

    // Utilize first part to have an id
    return idx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RimFaultReactivationDataAccessorStress::getResultAddress( const std::string& fieldName, const std::string& componentName )
{
    return RigFemResultAddress( RIG_ELEMENT_NODAL, fieldName, componentName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultReactivationDataAccessorStress::isMatching( RimFaultReactivation::Property property ) const
{
    return property == m_property;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStress::valueAtPosition( const cvf::Vec3d&                position,
                                                                const RigFaultReactivationModel& model,
                                                                RimFaultReactivation::GridPart   gridPart,
                                                                double                           topDepth,
                                                                double                           bottomDepth,
                                                                size_t                           elementIndex ) const
{
    if ( !m_s11Frames || !m_s22Frames || !m_s33Frames || !m_femPart ) return std::numeric_limits<double>::infinity();

    RimWellIADataAccess iaDataAccess( m_geoMechCase );
    int                 centerElementIdx = iaDataAccess.elementIndex( position );

    cvf::Vec3d topPosition( position.x(), position.y(), topDepth );
    int        topElementIdx = iaDataAccess.elementIndex( topPosition );

    cvf::Vec3d bottomPosition( position.x(), position.y(), bottomDepth );
    int        bottomElementIdx = iaDataAccess.elementIndex( bottomPosition );
    if ( centerElementIdx != -1 && topElementIdx != -1 && bottomElementIdx != -1 )
    {
        int timeStepIndex = 0;
        int frameIndex    = m_s33Frames->frameCount( timeStepIndex ) - 1;

        const std::vector<float>& s33Data = m_s33Frames->frameData( timeStepIndex, frameIndex );

        if ( m_property == RimFaultReactivation::Property::StressTop )
        {
            auto [porBar, extractionPos] = getPorBar( iaDataAccess, m_femPart, topPosition, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;
            double s33 = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::StressBottom )
        {
            auto [porBar, extractionPos] = getPorBar( iaDataAccess, m_femPart, bottomPosition, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;
            double s33 = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
            return RiaEclipseUnitTools::barToPascal( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::DepthTop )
        {
            return topDepth;
        }
        else if ( m_property == RimFaultReactivation::Property::DepthBottom )
        {
            return bottomDepth;
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentX )
        {
            auto [porBar, extractionPos] = getPorBar( iaDataAccess, m_femPart, position, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;

            const std::vector<float>& s11Data = m_s11Frames->frameData( timeStepIndex, frameIndex );
            double                    s11     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s11Data );
            double                    s33     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
            return ( s11 - porBar ) / ( s33 - porBar );
        }
        else if ( m_property == RimFaultReactivation::Property::LateralStressComponentY )
        {
            auto [porBar, extractionPos] = getPorBar( iaDataAccess, m_femPart, position, m_gradient, timeStepIndex, frameIndex );
            if ( std::isinf( porBar ) ) return porBar;

            const std::vector<float>& s22Data = m_s22Frames->frameData( timeStepIndex, frameIndex );
            double                    s22     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s22Data );
            double                    s33     = interpolatedResultValue( iaDataAccess, m_femPart, extractionPos, s33Data );
            return ( s22 - porBar ) / ( s33 - porBar );
        }
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultReactivationDataAccessorStress::interpolatedResultValue( RimWellIADataAccess&      iaDataAccess,
                                                                        const RigFemPart*         femPart,
                                                                        const cvf::Vec3d&         position,
                                                                        const std::vector<float>& scalarResults ) const
{
    return iaDataAccess.interpolatedResultValue( femPart, scalarResults, RIG_ELEMENT_NODAL, position );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec3d> RimFaultReactivationDataAccessorStress::getPorBar( RimWellIADataAccess& iaDataAccess,
                                                                                 const RigFemPart*    femPart,
                                                                                 const cvf::Vec3d&    position,
                                                                                 double               gradient,
                                                                                 int                  timeStepIndex,
                                                                                 int                  frameIndex ) const
{
    RigFemPartCollection*                partCollection = m_geoMechCaseData->femParts();
    cvf::ref<RigGeoMechWellLogExtractor> extractor      = m_partIndexA == getPartIndexFromPoint( *partCollection, position ) ? m_extractorA
                                                                                                                             : m_extractorB;
    if ( !extractor->valid() )
    {
        RiaLogging::error( "Invalid extractor when extracting PorBar" );
        return { std::numeric_limits<double>::infinity(), cvf::Vec3d::UNDEFINED };
    }

    RigFemResultAddress resAddr = RigFemAddressDefines::nodalPorBarAddress();
    std::vector<double> values;
    extractor->curveData( resAddr, timeStepIndex, frameIndex, &values );

    // Fill in missing values
    auto intersections = extractor->intersections();
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
std::pair<int, int> RimFaultReactivationDataAccessorStress::findIntersectionsForTvd( const std::vector<cvf::Vec3d>& intersections, double tvd )
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
std::pair<int, int> RimFaultReactivationDataAccessorStress::findOverburdenAndUnderburdenIndex( const std::vector<double>& values )
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
double RimFaultReactivationDataAccessorStress::computePorBarWithGradient( const std::vector<cvf::Vec3d>& intersections,
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
void RimFaultReactivationDataAccessorStress::fillInMissingValues( const std::vector<cvf::Vec3d>& intersections,
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
std::vector<double> RimFaultReactivationDataAccessorStress::generateMds( const std::vector<cvf::Vec3d>& points )
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
std::vector<cvf::Vec3d> RimFaultReactivationDataAccessorStress::generateWellPoints( const cvf::Vec3d& faultTopPosition,
                                                                                    const cvf::Vec3d& faultBottomPosition,
                                                                                    const cvf::Vec3d& offset )
{
    cvf::Vec3d faultTop = faultTopPosition + offset;
    cvf::Vec3d seabed( faultTop.x(), faultTop.y(), 0.0 );
    cvf::Vec3d faultBottom = faultBottomPosition + offset;
    cvf::Vec3d underburdenBottom( faultBottom.x(), faultBottom.y(), -10000.0 );
    return { seabed, faultTop, faultBottom, underburdenBottom };
}
