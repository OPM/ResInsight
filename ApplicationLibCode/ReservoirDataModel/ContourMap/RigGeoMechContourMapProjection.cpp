/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "RigGeoMechContourMapProjection.h"

#include "RiaImageTools.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigCellGeometryTools.h"
#include "RigContourMapCalculator.h"
#include "RigContourMapGrid.h"
#include "RigFemAddressDefines.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigHexIntersectionTools.h"

#include "RivFemElmVisibilityCalculator.h"

#include "cvfVector3.h"

#include <algorithm>
#include <array>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechContourMapProjection::RigGeoMechContourMapProjection( RigGeoMechCaseData&      caseData,
                                                                const RigContourMapGrid& contourMapGrid,
                                                                bool                     limitToPorePressureRegions,
                                                                double                   paddingAroundPorePressureRegion )
    : RigContourMapProjection( contourMapGrid )
    , m_caseData( caseData )
    , m_limitToPorePressureRegions( limitToPorePressureRegions )
    , m_paddingAroundPorePressureRegion( paddingAroundPorePressureRegion )
    , m_kLayers( 0u )
{
    m_femPart     = m_caseData.femParts()->part( 0 );
    m_femPartGrid = m_femPart->getOrCreateStructGrid();
    m_kLayers     = m_femPartGrid->cellCountK();
    m_femPart->ensureIntersectionSearchTreeIsBuilt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigGeoMechContourMapProjection::calculateExpandedPorBarBBox( RigGeoMechCaseData& caseData,
                                                                              const std::string&  resultComponentName,
                                                                              int                 timeStep,
                                                                              int                 frameIndex,
                                                                              double              paddingAroundPorePressureRegion )
{
    RigFemResultAddress          porBarAddr( RigFemResultPosEnum::RIG_ELEMENT_NODAL, RigFemAddressDefines::porBar(), resultComponentName );
    RigFemPartResultsCollection* resultCollection = caseData.femPartResults();

    const std::vector<float>& resultValues = resultCollection->resultValues( porBarAddr, 0, timeStep, frameIndex );
    cvf::BoundingBox          boundingBox;

    if ( resultValues.empty() )
    {
        return boundingBox;
    }

    auto femPart     = caseData.femParts()->part( 0 );
    auto femPartGrid = femPart->getOrCreateStructGrid();
    for ( int i = 0; i < femPart->elementCount(); ++i )
    {
        size_t resValueIdx = femPart->elementNodeResultIdx( (int)i, 0 );
        CVF_ASSERT( resValueIdx < resultValues.size() );
        double scalarValue   = resultValues[resValueIdx];
        bool   validPorValue = scalarValue != std::numeric_limits<double>::infinity();

        if ( validPorValue )
        {
            std::array<cvf::Vec3d, 8> hexCorners;
            femPartGrid->cellCornerVertices( i, hexCorners.data() );
            for ( size_t c = 0; c < 8; ++c )
            {
                boundingBox.add( hexCorners[c] );
            }
        }
    }

    cvf::Vec3d boxMin    = boundingBox.min();
    cvf::Vec3d boxMax    = boundingBox.max();
    cvf::Vec3d boxExtent = boundingBox.extent();
    boxMin.x() -= boxExtent.x() * 0.5 * paddingAroundPorePressureRegion;
    boxMin.y() -= boxExtent.y() * 0.5 * paddingAroundPorePressureRegion;
    boxMax.x() += boxExtent.x() * 0.5 * paddingAroundPorePressureRegion;
    boxMax.y() += boxExtent.y() * 0.5 * paddingAroundPorePressureRegion;
    return cvf::BoundingBox( boxMin, boxMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<bool> RigGeoMechContourMapProjection::getMapCellVisibility( int                                            viewStepIndex,
                                                                        RigContourMapCalculator::ResultAggregationType resultAggregation )
{
    m_mapCellVisibility = getMapCellVisibility( m_currentResultAddr, viewStepIndex, resultAggregation );
    return m_mapCellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<bool> RigGeoMechContourMapProjection::getMapCellVisibility( RigFemResultAddress                            resAddr,
                                                                        int                                            viewStepIndex,
                                                                        RigContourMapCalculator::ResultAggregationType resultAggregation )
{
    cvf::Vec2ui                            nCellsIJ = numberOfElementsIJ();
    std::vector<std::vector<unsigned int>> distanceImage( nCellsIJ.x(), std::vector<unsigned int>( nCellsIJ.y(), 0u ) );

    std::vector<bool> mapCellVisibility;

    if ( m_limitToPorePressureRegions )
    {
        resAddr = RigFemAddressDefines::elementNodalPorBarAddress();
    }

    std::vector<double> cellResults = generateResultsFromAddress( resAddr, mapCellVisibility, resultAggregation, viewStepIndex );

    mapCellVisibility.resize( numberOfCells(), true );
    CVF_ASSERT( mapCellVisibility.size() == cellResults.size() );

    {
        cvf::BoundingBox validResBoundingBox;
        for ( size_t cellIndex = 0; cellIndex < cellResults.size(); ++cellIndex )
        {
            cvf::Vec2ui ij = m_contourMapGrid.ijFromCellIndex( cellIndex );
            if ( cellResults[cellIndex] != std::numeric_limits<double>::infinity() )
            {
                distanceImage[ij.x()][ij.y()] = 1u;
                validResBoundingBox.add( cvf::Vec3d( m_contourMapGrid.cellCenterPosition( ij.x(), ij.y() ), 0.0 ) );
            }
            else
            {
                mapCellVisibility[cellIndex] = false;
            }
        }

        if ( m_limitToPorePressureRegions && m_paddingAroundPorePressureRegion > 0.0 )
        {
            RiaImageTools::distanceTransform2d( distanceImage );

            cvf::Vec3d porExtent   = validResBoundingBox.extent();
            double     radius      = std::max( porExtent.x(), porExtent.y() ) * 0.25;
            double     expansion   = m_paddingAroundPorePressureRegion * radius;
            size_t     cellPadding = std::ceil( expansion / m_contourMapGrid.sampleSpacing() );
            for ( size_t cellIndex = 0; cellIndex < cellResults.size(); ++cellIndex )
            {
                if ( !mapCellVisibility[cellIndex] )
                {
                    cvf::Vec2ui ij = m_contourMapGrid.ijFromCellIndex( cellIndex );
                    if ( distanceImage[ij.x()][ij.y()] < cellPadding * cellPadding )
                    {
                        mapCellVisibility[cellIndex] = true;
                    }
                }
            }
        }
    }
    return mapCellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechContourMapProjection::generateAndSaveResults( RigFemResultAddress                            resultAddress,
                                                             RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                             int                                            viewerStepIndex )
{
    m_aggregatedResults = generateResultsFromAddress( resultAddress, m_mapCellVisibility, resultAggregation, viewerStepIndex );
    m_currentResultAddr = resultAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechContourMapProjection::generateResultsFromAddress( RigFemResultAddress      resultAddress,
                                                                                const std::vector<bool>& mapCellVisibility,
                                                                                RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                                                int viewerStepIndex ) const
{
    RigFemPartResultsCollection* resultCollection  = m_caseData.femPartResults();
    size_t                       nCells            = numberOfCells();
    std::vector<double>          aggregatedResults = std::vector<double>( nCells, std::numeric_limits<double>::infinity() );

    auto [stepIdx, frameIdx] = m_caseData.femPartResults()->stepListIndexToTimeStepAndDataFrameIndex( viewerStepIndex );

    bool wasInvalid = false;
    if ( !resultAddress.isValid() )
    {
        wasInvalid    = true;
        resultAddress = RigFemAddressDefines::elementNodalPorBarAddress();
    }

    if ( resultAddress.fieldName == "PP" )
    {
        resultAddress.fieldName = RigFemAddressDefines::porBar(); // More likely to be in memory than POR
    }
    if ( resultAddress.fieldName == RigFemAddressDefines::porBar() )
    {
        resultAddress.resultPosType = RIG_ELEMENT_NODAL;
    }
    else if ( resultAddress.resultPosType == RIG_FORMATION_NAMES )
    {
        resultAddress.resultPosType = RIG_ELEMENT_NODAL; // formation indices are stored per element node result.
    }

    std::vector<float> resultValuesF = resultCollection->resultValues( resultAddress, 0, stepIdx, frameIdx );
    if ( resultValuesF.empty() ) return aggregatedResults;

    std::vector<double> resultValues = gridCellValues( resultAddress, resultValuesF );

    if ( wasInvalid )
    {
        // For invalid result addresses we just use the POR-Bar result to get the reservoir region
        // And display a dummy 0-result in the region.
        for ( double& value : resultValues )
        {
            if ( value != std::numeric_limits<double>::infinity() )
            {
                value = 0.0;
            }
        }
    }

#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( nCells ); ++index )
    {
        if ( mapCellVisibility.empty() || mapCellVisibility[index] )
        {
            cvf::Vec2ui ij           = m_contourMapGrid.ijFromCellIndex( index );
            aggregatedResults[index] = calculateValueInMapCell( ij.x(), ij.y(), resultValues, resultAggregation );
        }
    }

    return aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigGeoMechContourMapProjection::findIntersectingCells( const cvf::BoundingBox& bbox ) const
{
    return m_femPart->findIntersectingElementsWithExistingSearchTree( bbox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGeoMechContourMapProjection::kLayer( size_t globalCellIdx ) const
{
    size_t i, j, k;
    m_femPartGrid->ijkFromCellIndex( globalCellIdx, &i, &j, &k );
    return k;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGeoMechContourMapProjection::kLayers() const
{
    return m_kLayers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechContourMapProjection::calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;
    m_femPartGrid->cellCornerVertices( globalCellIdx, hexCorners.data() );

    cvf::BoundingBox          overlapBBox;
    std::array<cvf::Vec3d, 8> overlapCorners;
    if ( RigCellGeometryTools::estimateHexOverlapWithBoundingBox( hexCorners, bbox, &overlapCorners, &overlapBBox ) )
    {
        double overlapVolume = RigCellGeometryTools::calculateCellVolume( overlapCorners );
        return overlapVolume;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechContourMapProjection::calculateRayLengthInCell( size_t            globalCellIdx,
                                                                 const cvf::Vec3d& highestPoint,
                                                                 const cvf::Vec3d& lowestPoint ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const std::vector<cvf::Vec3f>& nodeCoords    = m_femPart->nodes().coordinates;
    const int*                     cornerIndices = m_femPart->connectivities( globalCellIdx );

    hexCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
    hexCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
    hexCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
    hexCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
    hexCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
    hexCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
    hexCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
    hexCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );

    std::vector<HexIntersectionInfo> intersections;

    if ( RigHexIntersectionTools::lineHexCellIntersection( highestPoint, lowestPoint, hexCorners.data(), 0, &intersections ) )
    {
        double lengthInCell = ( intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint ).length();
        return lengthInCell;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechContourMapProjection::getParameterWeightForCell( size_t globalCellIdx, const std::vector<double>& parameterWeights ) const
{
    if ( parameterWeights.empty() ) return 1.0;

    return parameterWeights[globalCellIdx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechContourMapProjection::gridCellValues( RigFemResultAddress resAddr, std::vector<float>& resultValues ) const
{
    std::vector<double> gridCellValues( m_femPart->elementCount(), std::numeric_limits<double>::infinity() );
    for ( size_t globalCellIdx = 0; globalCellIdx < static_cast<size_t>( m_femPart->elementCount() ); ++globalCellIdx )
    {
        RigElementType elmType = m_femPart->elementType( globalCellIdx );
        if ( !RigFemTypes::is8NodeElement( elmType ) ) continue;

        if ( resAddr.resultPosType == RIG_ELEMENT )
        {
            gridCellValues[globalCellIdx] = static_cast<double>( resultValues[globalCellIdx] );
        }
        else if ( resAddr.resultPosType == RIG_ELEMENT_NODAL )
        {
            RiaWeightedMeanCalculator<float> cellAverage;
            for ( int i = 0; i < 8; ++i )
            {
                size_t gridResultValueIdx =
                    m_femPart->resultValueIdxFromResultPosType( resAddr.resultPosType, static_cast<int>( globalCellIdx ), i );
                cellAverage.addValueAndWeight( resultValues[gridResultValueIdx], 1.0 );
            }

            gridCellValues[globalCellIdx] = static_cast<double>( cellAverage.weightedMean() );
        }
        else
        {
            RiaWeightedMeanCalculator<float> cellAverage;
            const int*                       elmNodeIndices = m_femPart->connectivities( globalCellIdx );
            for ( int i = 0; i < 8; ++i )
            {
                cellAverage.addValueAndWeight( resultValues[elmNodeIndices[i]], 1.0 );
            }
            gridCellValues[globalCellIdx] = static_cast<double>( cellAverage.weightedMean() );
        }
    }
    return gridCellValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechContourMapProjection::isCellActive( size_t globalCellIdx ) const
{
    // For GeoMech grids, all cells are considered active
    return true;
}
