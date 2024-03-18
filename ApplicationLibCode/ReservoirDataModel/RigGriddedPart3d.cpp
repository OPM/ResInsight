/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RigGriddedPart3d.h"

#include "RigActiveCellInfo.h"
#include "RigMainGrid.h"

#include "RimFaultReactivationDataAccess.h"
#include "RimFaultReactivationEnums.h"

#include "cvfBoundingBox.h"
#include "cvfPlane.h"
#include "cvfTextureImage.h"

#include "cafLine.h"

#include <cmath>
#include <map>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGriddedPart3d::RigGriddedPart3d()
    : m_useLocalCoordinates( false )
    , m_topHeight( 0.0 )
    , m_faultSafetyDistance( 1.0 )
    , m_nVertElements( 0 )
    , m_nHorzElements( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGriddedPart3d::~RigGriddedPart3d()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::reset()
{
    m_boundaryElements.clear();
    m_boundaryNodes.clear();
    m_borderSurfaceElements.clear();
    m_nodes.clear();
    m_dataNodes.clear();
    m_localNodes.clear();
    m_elementIndices.clear();
    m_meshLines.clear();
    m_elementSets.clear();
    m_nVertElements = 0;
    m_nHorzElements = 0;
    m_topHeight     = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGriddedPart3d::Regions> RigGriddedPart3d::allRegions()
{
    return { Regions::LowerUnderburden, Regions::UpperUnderburden, Regions::Reservoir, Regions::LowerOverburden, Regions::UpperOverburden };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGriddedPart3d::stepVector( cvf::Vec3d start, cvf::Vec3d stop, int nSteps )
{
    cvf::Vec3d vec = stop - start;
    return vec.getNormalized() * ( vec.length() / nSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGriddedPart3d::generateConstantLayers( double zFrom, double zTo, double maxSize )
{
    std::vector<double> layers;

    double diff = zTo - zFrom;

    if ( diff == 0.0 ) return layers;

    if ( std::abs( diff ) <= maxSize )
    {
        layers.push_back( std::min( zFrom, zTo ) );
        return layers;
    }

    double steps = std::abs( diff / maxSize );

    int nSteps = (int)std::ceil( steps );

    double stepSize = diff / nSteps;

    for ( int i = 0; i < nSteps; i++ )
    {
        layers.push_back( zFrom + stepSize * i );
    }

    std::sort( layers.begin(), layers.end() );

    return layers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGriddedPart3d::generateGrowingLayers( double zFrom, double zTo, double maxSize, double growfactor )
{
    std::vector<double> layers;

    double diff = zTo - zFrom;
    if ( std::abs( diff ) <= maxSize )
    {
        layers.push_back( std::min( zFrom, zTo ) );
        return layers;
    }

    double startHeight = maxSize;
    double curDepth    = zFrom;

    if ( zTo < zFrom )
    {
        while ( curDepth > zTo )
        {
            layers.push_back( curDepth );
            curDepth -= startHeight;
            startHeight *= growfactor;
        }
    }
    else if ( zTo > zFrom )
    {
        while ( curDepth < zTo )
        {
            layers.push_back( curDepth );
            curDepth += startHeight;
            startHeight *= growfactor;
        }
    }

    if ( std::abs( zTo - layers.back() ) < maxSize ) layers.pop_back();
    layers.push_back( zTo );

    std::sort( layers.begin(), layers.end() );

    return layers;
}

//--------------------------------------------------------------------------------------------------
///  Point index in input
///
///
///      5 ------| 11                          *
///        |  OU |           Overburden Upper  *
///      4 |------\10                          *
///        |  OL   \         Overburden Lower  *
///      3 |--------\  9                       *
///        |         \                         *
///        |   R      \      Reservoir         *
///      2 |___________\ 8                     *
///        |    UU      \    Underburden Upper *
///      1 |-------------\7                    *
///        |             |                     *
///        |    UL       |   Underburden Lower *
///        |             |                     *
///      0 -------------- 6                    *
///
/// Assumes horizontal lines are parallel
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::generateGeometry( const std::array<cvf::Vec3d, 12>&    inputPoints,
                                         const std::vector<double>&           reservoirZ,
                                         double                               maxCellHeight,
                                         double                               cellSizeFactor,
                                         const std::vector<double>&           horizontalPartition,
                                         const std::vector<caf::Line<double>> faultLines,
                                         const std::vector<cvf::Vec3d>&       thicknessVectors,
                                         double                               topHeight,
                                         int                                  nFaultZoneCells )
{
    reset();

    m_topHeight = topHeight;

    std::map<Regions, std::vector<double>> layersPerRegion;

    layersPerRegion[Regions::LowerUnderburden] = generateGrowingLayers( inputPoints[1].z(), inputPoints[0].z(), maxCellHeight, cellSizeFactor );
    layersPerRegion[Regions::UpperUnderburden] = generateConstantLayers( inputPoints[1].z(), inputPoints[2].z(), maxCellHeight );
    layersPerRegion[Regions::Reservoir]        = reservoirZ;

    layersPerRegion[Regions::LowerOverburden] = generateConstantLayers( inputPoints[3].z(), inputPoints[4].z(), maxCellHeight );
    layersPerRegion[Regions::UpperOverburden] = generateGrowingLayers( inputPoints[4].z(), inputPoints[5].z(), maxCellHeight, cellSizeFactor );

    layersPerRegion[Regions::LowerUnderburden].pop_back(); // to avoid overlap with bottom of next region
    layersPerRegion[Regions::Reservoir].pop_back(); // to avoid overlap with bottom of next region

    m_boundaryNodes[Boundary::Bottom]    = {};
    m_boundaryNodes[Boundary::FarSide]   = {};
    m_boundaryNodes[Boundary::Fault]     = {};
    m_boundaryNodes[Boundary::Reservoir] = {};

    size_t       nVertCells = 0;
    const size_t nHorzCells = horizontalPartition.size() - 1;

    for ( auto region : allRegions() )
    {
        nVertCells += layersPerRegion[region].size();
    }

    const int nThicknessCells = 2;

    size_t reserveSize = ( nVertCells + 1 ) * ( nHorzCells + 1 ) * ( nThicknessCells + 1 );
    m_nodes.reserve( reserveSize );
    m_dataNodes.reserve( reserveSize );

    m_nHorzElements = (int)nHorzCells;
    m_nVertElements = (int)nVertCells - 1;

    unsigned int nodeIndex = 0;
    unsigned int layer     = 0;

    cvf::Vec3d fromPos;
    cvf::Vec3d toPos;
    cvf::Vec3d fromStep;
    cvf::Vec3d toStep;

    for ( auto region : allRegions() )
    {
        switch ( region )
        {
            case Regions::LowerUnderburden:
                fromPos  = inputPoints[0];
                toPos    = inputPoints[6];
                fromStep = cvf::Vec3d( 0, 0, 0 );
                toStep   = cvf::Vec3d( 0, 0, 0 );
                break;

            case Regions::UpperUnderburden:
                fromPos  = inputPoints[1];
                toPos    = inputPoints[7];
                fromStep = stepVector( inputPoints[1], inputPoints[2], (int)layersPerRegion[region].size() );
                toStep   = stepVector( inputPoints[7], inputPoints[8], (int)layersPerRegion[region].size() );
                break;

            case Regions::Reservoir:
                fromPos = inputPoints[2];
                toPos   = inputPoints[8];
                break;

            case Regions::LowerOverburden:
                fromPos  = inputPoints[3];
                toPos    = inputPoints[9];
                fromStep = stepVector( inputPoints[3], inputPoints[4], (int)layersPerRegion[region].size() );
                toStep   = stepVector( inputPoints[9], inputPoints[10], (int)layersPerRegion[region].size() );
                break;

            case Regions::UpperOverburden:
                fromPos  = inputPoints[4];
                toPos    = inputPoints[10];
                fromStep = cvf::Vec3d( 0, 0, 0 );
                toStep   = cvf::Vec3d( 0, 0, 0 );
                break;
        }

        for ( int v = 0; v < (int)layersPerRegion[region].size(); v++, layer++ )
        {
            if ( ( region == Regions::LowerUnderburden ) || ( region == Regions::UpperOverburden ) )
            {
                fromPos.z() = layersPerRegion[region][v];
                toPos.z()   = layersPerRegion[region][v];
            }
            else if ( region == Regions::Reservoir )
            {
                fromPos.z() = reservoirZ[v];
                cvf::Plane zPlane;
                zPlane.setFromPointAndNormal( fromPos, cvf::Vec3d::Z_AXIS );
                zPlane.intersect( faultLines[1].start(), faultLines[1].end(), &toPos );
            }

            cvf::Vec3d stepHorz = toPos - fromPos;
            cvf::Vec3d p;
            cvf::Vec3d safetyOffset = fromPos - toPos;
            safetyOffset.normalize();
            safetyOffset *= m_faultSafetyDistance;

            m_meshLines.push_back( { fromPos, toPos } );

            for ( int h = 0; h <= (int)nHorzCells; h++ )
            {
                p = toPos - horizontalPartition[h] * stepHorz;

                for ( int t = 0; t <= nThicknessCells; t++, nodeIndex++ )
                {
                    auto nodePoint = p + thicknessVectors[t];

                    // adjust points along the fault line inside the reservoir to make sure they end up at the fault
                    if ( ( h == (int)nHorzCells ) &&
                         ( ( region == Regions::Reservoir ) || region == Regions::LowerOverburden || region == Regions::UpperUnderburden ) )
                    {
                        cvf::Plane zPlane;
                        zPlane.setFromPointAndNormal( p, cvf::Vec3d::Z_AXIS );
                        zPlane.intersect( faultLines[t].start(), faultLines[t].end(), &nodePoint );
                    }

                    m_nodes.push_back( nodePoint );

                    // move nodes at fault used for data extraction a bit away from the fault
                    if ( h == (int)nHorzCells )
                    {
                        m_dataNodes.push_back( p + safetyOffset );
                    }
                    else
                    {
                        m_dataNodes.push_back( p );
                    }

                    if ( layer == 0 )
                    {
                        m_boundaryNodes[Boundary::Bottom].push_back( nodeIndex );
                    }
                    if ( h == 0 )
                    {
                        m_boundaryNodes[Boundary::FarSide].push_back( nodeIndex );
                    }
                    else if ( h == (int)nHorzCells )
                    {
                        m_boundaryNodes[Boundary::Fault].push_back( nodeIndex );

                        if ( region == Regions::Reservoir )
                        {
                            m_boundaryNodes[Boundary::Reservoir].push_back( nodeIndex );
                        }
                    }
                }
            }

            if ( region != Regions::Reservoir )
            {
                fromPos += fromStep;
                toPos += toStep;
            }
        }
    }

    // ** generate elements of type hex8

    m_elementIndices.resize( (size_t)( ( nVertCells - 1 ) * nHorzCells * nThicknessCells ) );

    m_borderSurfaceElements[RimFaultReactivation::BorderSurface::Seabed]       = {};
    m_borderSurfaceElements[RimFaultReactivation::BorderSurface::UpperSurface] = {};
    m_borderSurfaceElements[RimFaultReactivation::BorderSurface::FaultSurface] = {};
    m_borderSurfaceElements[RimFaultReactivation::BorderSurface::LowerSurface] = {};

    m_elementSets[ElementSets::OverBurden]     = {};
    m_elementSets[ElementSets::Reservoir]      = {};
    m_elementSets[ElementSets::IntraReservoir] = {};
    m_elementSets[ElementSets::UnderBurden]    = {};
    m_elementSets[ElementSets::FaultZone]      = {};

    m_boundaryElements[Boundary::Bottom]  = {};
    m_boundaryElements[Boundary::FarSide] = {};
    m_boundaryElements[Boundary::Fault]   = {};

    int layerIndexOffset = 0;
    int elementIdx       = 0;
    layer                = 0;

    const int nVertCellsLower = (int)layersPerRegion[Regions::LowerUnderburden].size();
    const int nVertCellsFault = (int)( layersPerRegion[Regions::UpperUnderburden].size() + layersPerRegion[Regions::Reservoir].size() +
                                       layersPerRegion[Regions::LowerOverburden].size() );

    RimFaultReactivation::BorderSurface currentSurfaceRegion = RimFaultReactivation::BorderSurface::LowerSurface;

    const int nextLayerIdxOff = ( (int)nHorzCells + 1 ) * ( nThicknessCells + 1 );
    const int nThicknessOff   = nThicknessCells + 1;
    const int seaBedLayer     = (int)( nVertCells - 2 );

    const int nFaultZoneStart = (int)nHorzCells - nFaultZoneCells - 1;

    for ( int v = 0; v < (int)nVertCells - 1; v++ )
    {
        if ( v >= nVertCellsLower ) currentSurfaceRegion = RimFaultReactivation::BorderSurface::FaultSurface;
        if ( v >= nVertCellsLower + nVertCellsFault ) currentSurfaceRegion = RimFaultReactivation::BorderSurface::UpperSurface;

        int i = layerIndexOffset;

        for ( int h = 0; h < (int)nHorzCells; h++ )
        {
            for ( int t = 0; t < nThicknessCells; t++, elementIdx++ )
            {
                m_elementIndices[elementIdx].push_back( t + i );
                m_elementIndices[elementIdx].push_back( t + i + 1 );
                m_elementIndices[elementIdx].push_back( t + i + nThicknessOff + 1 );
                m_elementIndices[elementIdx].push_back( t + i + nThicknessOff );

                m_elementIndices[elementIdx].push_back( t + nextLayerIdxOff + i );
                m_elementIndices[elementIdx].push_back( t + nextLayerIdxOff + i + 1 );
                m_elementIndices[elementIdx].push_back( t + nextLayerIdxOff + i + nThicknessOff + 1 );
                m_elementIndices[elementIdx].push_back( t + nextLayerIdxOff + i + nThicknessOff );

                if ( v == 0 )
                {
                    m_boundaryElements[Boundary::Bottom].push_back( elementIdx );
                }
                else if ( v == seaBedLayer )
                {
                    m_borderSurfaceElements[RimFaultReactivation::BorderSurface::Seabed].push_back( elementIdx );
                }
                if ( h == 0 )
                {
                    m_boundaryElements[Boundary::FarSide].push_back( elementIdx );
                }

                bool inFaultZone = ( currentSurfaceRegion == RimFaultReactivation::BorderSurface::FaultSurface ) && ( h > nFaultZoneStart );

                if ( inFaultZone ) m_elementSets[RimFaultReactivation::ElementSets::FaultZone].push_back( elementIdx );
            }
            i += nThicknessOff;
        }

        // add elements to border surface in current region
        m_borderSurfaceElements[currentSurfaceRegion].push_back( elementIdx - 2 );
        m_borderSurfaceElements[currentSurfaceRegion].push_back( elementIdx - 1 );

        layerIndexOffset += nextLayerIdxOff;
    }

    // vertical mesh lines for 2d display
    for ( int i = 0; i < 5; i++ )
    {
        generateVerticalMeshlines( { inputPoints[i], inputPoints[i + 1], inputPoints[i + 7], inputPoints[i + 6] }, horizontalPartition );
    }
}

//--------------------------------------------------------------------------------------------------
///  Point index in input
///
///     1 ____________ 2
///      |           /
///      |          /
///      |         /
///      |        /
///      |_______/
///      0         3
///
/// Assumes 0->3 and 1->2 is parallel
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::generateVerticalMeshlines( const std::vector<cvf::Vec3d>& cornerPoints, const std::vector<double>& horzPartition )
{
    cvf::Vec3d step0to3 = cornerPoints[3] - cornerPoints[0];
    cvf::Vec3d step1to2 = cornerPoints[2] - cornerPoints[1];

    int numHorzCells = (int)horzPartition.size();

    for ( int h = 0; h < numHorzCells; h++ )
    {
        auto startP = cornerPoints[3] - horzPartition[h] * step0to3;
        auto endP   = cornerPoints[2] - horzPartition[h] * step1to2;
        m_meshLines.push_back( { startP, endP } );
    }
}

//--------------------------------------------------------------------------------------------------
/// returns node in either global or local coords depending on m_useLocalCoordinates flag
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigGriddedPart3d::nodes() const
{
    if ( m_useLocalCoordinates ) return m_localNodes;
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
/// Always returns nodes in global coordinates
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigGriddedPart3d::globalNodes() const
{
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
/// Returns nodes in global coordinates, adjusted to always extract data as if the model has no
/// thickness. Additionally, nodes closest to the fault are moved away from the fault
/// to make sure data results come from the correct side of the fault.
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigGriddedPart3d::dataNodes() const
{
    return m_dataNodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::setUseLocalCoordinates( bool useLocalCoordinates )
{
    m_useLocalCoordinates = useLocalCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGriddedPart3d::useLocalCoordinates() const
{
    return m_useLocalCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGriddedPart3d::topHeight() const
{
    return m_topHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::setFaultSafetyDistance( double distance )
{
    m_faultSafetyDistance = distance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGriddedPart3d::faultSafetyDistance() const
{
    return m_faultSafetyDistance;
}

//--------------------------------------------------------------------------------------------------
/// Output elements will be of type HEX8
///
///     7---------6
///    /|        /|
///   / |       / |
///  4---------5  |     z
///  |  3------|--2       |   y
///  | /       | /        | /
///  |/        |/         |/
///  0---------1           ----- x
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<unsigned int>>& RigGriddedPart3d::elementIndices() const
{
    return m_elementIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d> RigGriddedPart3d::elementCorners( size_t elementIndex ) const
{
    return extractCornersForElement( m_elementIndices, m_nodes, elementIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d> RigGriddedPart3d::elementDataCorners( size_t elementIndex ) const
{
    return extractCornersForElement( m_elementIndices, m_dataNodes, elementIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::pair<int, int> RigGriddedPart3d::elementCountHorzVert() const
{
    return { m_nHorzElements, m_nVertElements };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigGriddedPart3d::extractCornersForElement( const std::vector<std::vector<unsigned int>>& elementIndices,
                                                                    const std::vector<cvf::Vec3d>&                nodes,
                                                                    size_t                                        elementIndex )
{
    if ( elementIndex >= elementIndices.size() ) return {};

    std::vector<cvf::Vec3d> corners;

    for ( auto nodeIdx : elementIndices[elementIndex] )
    {
        if ( nodeIdx >= nodes.size() ) continue;
        corners.push_back( nodes[nodeIdx] );
    }

    return corners;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>>& RigGriddedPart3d::borderSurfaceElements() const
{
    return m_borderSurfaceElements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<cvf::Vec3d>>& RigGriddedPart3d::meshLines() const
{
    return m_meshLines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RimFaultReactivation::Boundary, std::vector<unsigned int>>& RigGriddedPart3d::boundaryElements() const
{
    return m_boundaryElements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RimFaultReactivation::Boundary, std::vector<unsigned int>>& RigGriddedPart3d::boundaryNodes() const
{
    return m_boundaryNodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& RigGriddedPart3d::elementSets() const
{
    return m_elementSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::generateLocalNodes( const cvf::Mat4d transform )
{
    m_localNodes.clear();

    // need to flip the Y axis for the element corners to be in an acceptable order for abaqus and the IJK numbering algorithm in resinsight
    cvf::Vec3d xAxis = { 1.0, 0.0, 0.0 };
    cvf::Vec3d yAxis = { 0.0, -1.0, 0.0 };
    cvf::Vec3d zAxis = { 0.0, 0.0, 1.0 };
    cvf::Mat4d flipY = cvf::Mat4d::fromCoordSystemAxes( &xAxis, &yAxis, &zAxis );

    for ( auto& node : m_nodes )
    {
        auto tn = node.getTransformedPoint( transform );
        m_localNodes.push_back( tn.getTransformedPoint( flipY ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::shiftNodes( const cvf::Vec3d offset )
{
    for ( int i = 0; i < (int)m_nodes.size(); i++ )
    {
        m_nodes[i] += offset;
        m_dataNodes[i] += offset;
    }

    for ( int i = 0; i < (int)m_meshLines.size(); i++ )
    {
        for ( int j = 0; j < (int)m_meshLines[i].size(); j++ )
        {
            m_meshLines[i][j] += offset;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::postProcessElementSets( const RigMainGrid* mainGrid, const RigActiveCellInfo* cellInfo )
{
    std::set<unsigned int> usedElements;

    // fault zone elements are already assigned
    for ( auto elIdx : m_elementSets[ElementSets::FaultZone] )
    {
        usedElements.insert( elIdx );
    }

    // look for overburden, starting at top going down
    updateElementSet( ElementSets::OverBurden, usedElements, mainGrid, cellInfo, m_nVertElements - 1, -1, -1 );

    // look for underburden, starting at bottom going up
    updateElementSet( ElementSets::UnderBurden, usedElements, mainGrid, cellInfo, 0, m_nVertElements, 1 );

    // remaining elements are in the reservoir
    m_elementSets[ElementSets::IntraReservoir] = {};
    m_elementSets[ElementSets::Reservoir]      = {};

    for ( unsigned int element = 0; element < m_elementIndices.size(); element++ )
    {
        if ( usedElements.contains( element ) ) continue;

        auto corners = elementDataCorners( element );
        bool bActive = false;

        size_t cellIdx = 0;
        for ( const auto& p : corners )
        {
            cellIdx = mainGrid->findReservoirCellIndexFromPoint( p );

            bActive = ( cellIdx != cvf::UNDEFINED_SIZE_T ) && ( cellInfo->isActive( cellIdx ) );
            if ( bActive ) break;
        }

        if ( bActive )
        {
            m_elementSets[ElementSets::Reservoir].push_back( element );
        }
        else
        {
            m_elementSets[ElementSets::IntraReservoir].push_back( element );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGriddedPart3d::updateElementSet( ElementSets              elSet,
                                         std::set<unsigned int>&  usedElements,
                                         const RigMainGrid*       mainGrid,
                                         const RigActiveCellInfo* cellInfo,
                                         int                      rowStart,
                                         int                      rowEnd,
                                         int                      rowInc )
{
    for ( int col = 0; col < m_nHorzElements; col++ )
    {
        for ( int row = rowStart; row != rowEnd; row += rowInc )
        {
            const unsigned int elIdx = (unsigned int)( 2 * ( ( row * m_nHorzElements ) + col ) );

            bool bStop = false;

            for ( unsigned int t = 0; t < 2; t++ )
            {
                if ( usedElements.contains( elIdx + t ) )
                {
                    bStop = true;
                    break;
                }

                auto corners = elementDataCorners( elIdx + t );

                size_t cellIdx = 0;
                for ( const auto& p : corners )
                {
                    cellIdx = mainGrid->findReservoirCellIndexFromPoint( p );

                    if ( ( cellIdx != cvf::UNDEFINED_SIZE_T ) && ( cellInfo->isActive( cellIdx ) ) )
                    {
                        bStop = true;
                        break;
                    }
                }
            }

            if ( bStop )
            {
                break;
            }
            else
            {
                m_elementSets[elSet].push_back( elIdx );
                m_elementSets[elSet].push_back( elIdx + 1 );
                usedElements.insert( elIdx );
                usedElements.insert( elIdx + 1 );
            }
        }
    }
}
