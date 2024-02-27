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

#include "RigFaultReactivationModelGenerator.h"

#include "RiaApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigFault.h"
#include "RigGriddedPart3d.h"
#include "RigMainGrid.h"

#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimUserDefinedIndexFilter.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModelGenerator::RigFaultReactivationModelGenerator( cvf::Vec3d position, cvf::Vec3d modelNormal, cvf::Vec3d modelDirection )
    : m_startPosition( position )
    , m_modelNormal( modelNormal )
    , m_modelDirection( modelDirection )
    , m_bufferAboveFault( 0.0 )
    , m_bufferBelowFault( 0.0 )
    , m_startDepth( 0.0 )
    , m_bottomDepth( 0.0 )
    , m_depthBelowFault( 100.0 )
    , m_horzExtentFromFault( 1000.0 )
    , m_modelThickness( 100.0 )
    , m_useLocalCoordinates( false )
    , m_cellSizeHeightFactor( 1.0 )
    , m_cellSizeWidthFactor( 1.0 )
    , m_minCellHeight( 0.5 )
    , m_maxCellHeight( 20.0 )
    , m_minCellWidth( 20.0 )
    , m_faultZoneCells( 0 )
{
    m_modelPlane.setFromPointAndNormal( position, modelNormal );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModelGenerator::~RigFaultReactivationModelGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setFault( const RigFault* fault )
{
    m_fault = fault;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setGrid( const RigMainGrid* grid )
{
    m_grid = grid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setActiveCellInfo( const RigActiveCellInfo* activeCellInfo )
{
    m_activeCellInfo = activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setFaultBufferDepth( double aboveFault, double belowFault, int faultZoneCells )
{
    m_bufferAboveFault = aboveFault;
    m_bufferBelowFault = belowFault;
    m_faultZoneCells   = faultZoneCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setModelSize( double startDepth, double depthBelowFault, double horzExtentFromFault )
{
    m_startDepth          = startDepth;
    m_depthBelowFault     = depthBelowFault;
    m_horzExtentFromFault = horzExtentFromFault;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setModelThickness( double thickness )
{
    m_modelThickness = thickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setUseLocalCoordinates( bool useLocalCoordinates )
{
    m_useLocalCoordinates = useLocalCoordinates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setModelGriddingOptions( double minCellHeight,
                                                                  double maxCellHeight,
                                                                  double cellSizeFactorHeight,
                                                                  double minCellWidth,
                                                                  double cellSizeFactorWidth )
{
    m_minCellHeight        = minCellHeight;
    m_maxCellHeight        = maxCellHeight;
    m_cellSizeHeightFactor = cellSizeFactorHeight;
    m_minCellWidth         = minCellWidth;
    m_cellSizeWidthFactor  = cellSizeFactorWidth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3d, cvf::Vec3d> RigFaultReactivationModelGenerator::modelLocalNormalsXY()
{
    cvf::Vec3d xNormal = m_modelDirection;
    cvf::Vec3d yNormal = xNormal ^ cvf::Vec3d::Z_AXIS;

    return std::make_pair( xNormal, yNormal );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setupLocalCoordinateTransform()
{
    auto [xNormal, yNormal] = modelLocalNormalsXY();

    m_localCoordTransform = cvf::Mat4d::fromCoordSystemAxes( &xNormal, &yNormal, &cvf::Vec3d::Z_AXIS );
    cvf::Vec3d center     = m_startPosition * -1.0;
    center.z()            = 0.0;
    center.transformPoint( m_localCoordTransform );
    m_localCoordTransform.setTranslation( center );
}

//--------------------------------------------------------------------------------------------------
/// change corner order to be consistent so that index (0,1) and (2,3) gives the lower and upper horz. lines no matter what I or J face we
/// have
//--------------------------------------------------------------------------------------------------
const std::array<int, 4> RigFaultReactivationModelGenerator::faceIJCornerIndexes( FaceType face )
{
    switch ( face )
    {
        case cvf::StructGridInterface::POS_I:
        case cvf::StructGridInterface::NEG_J:
            return { 0, 1, 3, 2 };

        case cvf::StructGridInterface::NEG_I:
        case cvf::StructGridInterface::POS_J:
            return { 0, 3, 1, 2 };

        case cvf::StructGridInterface::POS_K:
        case cvf::StructGridInterface::NEG_K:
        case cvf::StructGridInterface::NO_FACE:
        default:
            break;
    }

    CVF_ASSERT( false ); // not supported for K faces
    return { 0, 0, 0, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFaultReactivationModelGenerator::oppositeStartCellIndex( const std::vector<size_t> cellIndexColumn, FaceType face )
{
    auto   oppositeStartFace  = cvf::StructGridInterface::oppositeFace( face );
    bool   bFoundOppositeCell = false;
    size_t oppositeCellIdx    = 0;

    for ( auto backCellIdx : cellIndexColumn )
    {
        for ( auto& faultFace : m_fault->faultFaces() )
        {
            if ( ( faultFace.m_nativeFace == face ) && ( faultFace.m_nativeReservoirCellIndex == backCellIdx ) )
            {
                bFoundOppositeCell = true;
                oppositeCellIdx    = faultFace.m_oppositeReservoirCellIndex;
                break;
            }
            else if ( ( faultFace.m_nativeFace == oppositeStartFace ) && ( faultFace.m_oppositeReservoirCellIndex == backCellIdx ) )
            {
                bFoundOppositeCell = true;
                oppositeCellIdx    = faultFace.m_nativeReservoirCellIndex;
                break;
            }
        }

        if ( bFoundOppositeCell ) break;
    }

    return oppositeCellIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::updateFilters( std::vector<size_t> cellsFront, std::vector<size_t> cellsBack )
{
    RimEclipseView* view = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    if ( view == nullptr ) return;

    auto cellFilters = view->cellFilterCollection();
    if ( cellFilters == nullptr ) return;

    auto eCase       = cellFilters->firstAncestorOfType<RimEclipseCase>();
    auto frontFilter = cellFilters->addNewUserDefinedIndexFilter( eCase, cellsFront );
    frontFilter->setName( "Front" );

    auto backFilter = cellFilters->addNewUserDefinedIndexFilter( eCase, cellsBack );
    backFilter->setName( "Back" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::array<cvf::Vec3d, 12>& RigFaultReactivationModelGenerator::frontPoints() const
{
    return m_frontPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::array<cvf::Vec3d, 12>& RigFaultReactivationModelGenerator::backPoints() const
{
    return m_backPoints;
}

//--------------------------------------------------------------------------------------------------
///             <----                           fault normal                                     *
///                                                                                              *
///                15                                                                            *
///       7---------|------------ 23            top model                                        *
///        |        |           |                                                                *
///        |        |           |                                                                *
///       6|_____14_|___________| 22            top fault w/buffer                               *
///       5|-----13-\-----------| 21            top fault front                                  *
///       4|---------\-12-------| 20            top fault back                                   *
///        |          X         |               start position in fault (user selected)          *
///       3|--------11-\--------| 19            bottom fault front                               *
///       2|------------\-10----| 18            bottom fault back                                *
///       1|_____________\______| 17            bottom fault w/buffer                            *
///        |            9|      |                                                                *
///        |             |      |                                                                *
///        |             |      |                                                                *
///       0--------------|------- 16            bottom model                                     *
///                     8                                                                        *
///          front          back                                                                 *
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::generatePointsFrontBack()
{
    std::array<cvf::Vec3d, 24> points;

    double top_depth = -m_startDepth;
    m_bottomDepth    = m_bottomFault.z() - m_depthBelowFault;

    cvf::Vec3d edge_front = m_startPosition - m_horzExtentFromFault * m_modelDirection;
    cvf::Vec3d edge_back  = m_startPosition + m_horzExtentFromFault * m_modelDirection;

    points[8]     = m_bottomFault;
    points[8].z() = m_bottomDepth;

    points[9]  = m_bottomFault;
    points[10] = m_bottomReservoirBack;
    points[11] = m_bottomReservoirFront;
    points[12] = m_topReservoirBack;
    points[13] = m_topReservoirFront;
    points[14] = m_topFault;

    points[15]     = m_topFault;
    points[15].z() = top_depth;

    for ( int i = 0; i < 8; i++ )
    {
        points[i]     = edge_front;
        points[i].z() = points[i + 8].z();
    }

    for ( int i = 16; i < 24; i++ )
    {
        points[i]     = edge_back;
        points[i].z() = points[i - 8].z();
    }

    std::array<cvf::Vec3d, 12> frontPoints;
    std::array<cvf::Vec3d, 12> backPoints;

    // only return the corner points used for each part
    std::vector<size_t> frontMap = { 0, 1, 3, 5, 6, 7, 8, 9, 11, 13, 14, 15 };
    std::vector<size_t> backMap  = { 16, 17, 18, 20, 22, 23, 8, 9, 10, 12, 14, 15 };

    for ( int i = 0; i < 12; i++ )
    {
        m_frontPoints[i] = points[frontMap[i]];
        m_backPoints[i]  = points[backMap[i]];
    }

    m_horizontalPartition = partition( m_startPosition.pointDistance( edge_front ), m_minCellWidth, m_cellSizeWidthFactor );
    // we start gridding from the far edges of the model, reverse the partition
    std::reverse( m_horizontalPartition.begin(), m_horizontalPartition.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double> RigFaultReactivationModelGenerator::partition( double distance, double startSize, double sizeFactor )
{
    std::vector<double> parts;

    double d    = 0;
    double step = startSize;

    while ( d < distance )
    {
        parts.push_back( d / distance );
        d += step;
        step *= sizeFactor;
    }

    // get rid of outermost cell column if too small
    if ( distance * ( 1.0 - parts.back() ) < startSize ) parts.pop_back();

    parts.push_back( 1.0 );

    return parts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t>
    RigFaultReactivationModelGenerator::buildCellColumn( size_t startCellIndex, FaceType startFace, std::map<double, cvf::Vec3d>& layers )
{
    size_t i, j, k;

    m_grid->ijkFromCellIndexUnguarded( startCellIndex, &i, &j, &k );

    std::vector<size_t> cellColumn;

    const int    k_start = 0;
    const size_t k_stop  = m_grid->cellCountK();

    // build list of k indexes to go through, starting at the start cell and going up, then continuing down below the start cell
    std::vector<size_t> k_values;

    for ( int kLayer = (int)k; kLayer >= k_start; kLayer-- )
    {
        k_values.push_back( (size_t)kLayer );
    }
    for ( size_t kLayer = k + 1; kLayer < k_stop; kLayer++ )
    {
        k_values.push_back( kLayer );
    }

    auto [side1, side2] = sideFacesIJ( startFace );

    bool isGoingUp = true;

    for ( auto kLayer : k_values )
    {
        if ( !m_grid->isCellValid( i, j, kLayer ) ) continue;
        const auto cellIdx = m_grid->cellIndexFromIJKUnguarded( i, j, kLayer );

        RigCell cell = m_grid->cell( cellIdx );

        std::vector<RigCell> cellRow;

        cellRow.push_back( cell.neighborCell( side1 ) );
        cellRow.push_back( cell );
        cellRow.push_back( cell.neighborCell( side2 ) );

        cvf::Vec3d intersect1, intersect2;
        size_t     intersectedCell;

        auto ij_pair = findCellWithIntersection( cellRow, startFace, intersectedCell, intersect1, intersect2, isGoingUp );

        if ( intersect1.z() != intersect2.z() )
        {
            cellColumn.push_back( intersectedCell );
            if ( !intersect1.isZero() ) layers[intersect1.z()] = intersect1;
            if ( !intersect2.isZero() ) layers[intersect2.z()] = intersect2;
        }

        if ( kLayer == k )
        {
            std::reverse( cellColumn.begin(), cellColumn.end() );
            isGoingUp = false;
        }

        i = ij_pair.first;
        j = ij_pair.second;
    }

    return cellColumn;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RigFaultReactivationModelGenerator::findCellWithIntersection( const std::vector<RigCell>& cellRow,
                                                                                        FaceType                    face,
                                                                                        size_t&                     cellIndex,
                                                                                        cvf::Vec3d&                 intersect1,
                                                                                        cvf::Vec3d&                 intersect2,
                                                                                        bool                        goingUp )
{
    const auto cornerIndexes = faceIJCornerIndexes( face );

    size_t i = 0, j = 0, k = 0;

    for ( auto& cell : cellRow )
    {
        if ( cell.isInvalid() ) continue;

        auto corners = cell.faceCorners( face );

        cvf::Vec3d intersect;
        double     dist = 0.0;
        if ( caf::HexGridIntersectionTools::planeLineIntersect( m_modelPlane,
                                                                corners[cornerIndexes[0]],
                                                                corners[cornerIndexes[1]],
                                                                &intersect,
                                                                &dist,
                                                                0.001 ) )
        {
            intersect1 = intersect;

            if ( !goingUp )
            {
                cellIndex = cell.mainGridCellIndex();
                m_grid->ijkFromCellIndexUnguarded( cellIndex, &i, &j, &k );
            }
        }

        if ( caf::HexGridIntersectionTools::planeLineIntersect( m_modelPlane,
                                                                corners[cornerIndexes[2]],
                                                                corners[cornerIndexes[3]],
                                                                &intersect,
                                                                &dist,
                                                                0.001 ) )
        {
            intersect2 = intersect;

            if ( goingUp )
            {
                cellIndex = cell.mainGridCellIndex();
                m_grid->ijkFromCellIndexUnguarded( cellIndex, &i, &j, &k );
            }
        }
    }

    return { i, j };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::generateGeometry( size_t            startCellIndex,
                                                           FaceType          startFace,
                                                           RigGriddedPart3d* frontPart,
                                                           RigGriddedPart3d* backPart )
{
    // build column of cells behind fault

    std::map<double, cvf::Vec3d> layersBack;
    std::vector<size_t>          cellColumnBack = buildCellColumn( startCellIndex, startFace, layersBack );

    // find start cell and face on the opposite side of the fault, start with the user clicked cell

    std::vector<size_t> cellColumnBackSearch = { startCellIndex };
    for ( auto cidx : cellColumnBack )
    {
        if ( cidx != startCellIndex ) cellColumnBackSearch.push_back( cidx );
    }
    auto   oppositeStartFace    = cvf::StructGridInterface::oppositeFace( startFace );
    size_t oppositeStartCellIdx = oppositeStartCellIndex( cellColumnBackSearch, startFace );

    // build cell column of cells in front of fault, opposite to the cell column behind the fault

    std::map<double, cvf::Vec3d> layersFront;
    std::vector<size_t>          cellColumnFront = buildCellColumn( oppositeStartCellIdx, oppositeStartFace, layersFront );

    // add extra fault buffer below the fault, starting at the deepest bottom-most cell on either side of the fault

    double front_bottom    = layersFront.begin()->first;
    double back_bottom     = layersBack.begin()->first;
    m_bottomReservoirFront = layersFront.begin()->second;
    m_bottomReservoirBack  = layersBack.begin()->second;

    cvf::Vec3d bottom_point = m_bottomReservoirFront;

    if ( front_bottom > back_bottom )
    {
        bottom_point = extrapolatePoint( ( ++layersBack.begin() )->second, layersBack.begin()->second, m_bufferBelowFault );
    }
    else
    {
        bottom_point = extrapolatePoint( ( ++layersFront.begin() )->second, layersFront.begin()->second, m_bufferBelowFault );
    }

    m_bottomFault = bottom_point;

    // add extra fault buffer above the fault, starting at the shallowest top-most cell on either side of the fault

    double front_top    = layersFront.rbegin()->first;
    double back_top     = layersBack.rbegin()->first;
    m_topReservoirFront = layersFront.rbegin()->second;
    m_topReservoirBack  = layersBack.rbegin()->second;

    cvf::Vec3d top_point = m_topReservoirFront;
    if ( front_top > back_top )
    {
        top_point = extrapolatePoint( ( ++layersFront.rbegin() )->second, layersFront.rbegin()->second, m_bufferAboveFault );
    }
    else
    {
        top_point = extrapolatePoint( ( ++layersBack.rbegin() )->second, layersBack.rbegin()->second, m_bufferAboveFault );
    }
    m_topFault = top_point;

    // make sure layers aren't too small or too thick

    mergeTinyLayers( layersFront, m_minCellHeight );
    mergeTinyLayers( layersBack, m_minCellHeight );

    splitLargeLayers( layersFront, m_maxCellHeight );
    splitLargeLayers( layersBack, m_maxCellHeight );

    std::vector<cvf::Vec3d> frontReservoirLayers;
    for ( auto& kvp : layersFront )
        frontReservoirLayers.push_back( kvp.second );

    std::vector<cvf::Vec3d> backReservoirLayers;
    for ( auto& kvp : layersBack )
        backReservoirLayers.push_back( kvp.second );

    // generate the actual front and back grid parts

    generatePointsFrontBack();

    cvf::Vec3d                     tVec = m_modelThickness * m_modelNormal;
    std::vector<cvf::Vec3d>        thicknessVectors;
    std::vector<caf::Line<double>> faultLines;
    const std::vector<double>      thicknessFactors = { -1.0, 0.0, 1.0 };

    for ( int i = 0; i < 3; i++ )
    {
        faultLines.push_back( caf::Line<double>( m_topFault + thicknessFactors[i] * tVec, m_bottomFault + thicknessFactors[i] * tVec ) );
        thicknessVectors.push_back( thicknessFactors[i] * tVec );
    }

    frontPart->generateGeometry( m_frontPoints,
                                 frontReservoirLayers,
                                 m_maxCellHeight,
                                 m_cellSizeHeightFactor,
                                 m_horizontalPartition,
                                 faultLines,
                                 thicknessVectors,
                                 m_topReservoirFront.z(),
                                 m_faultZoneCells );

    std::reverse( faultLines.begin(), faultLines.end() );
    std::reverse( thicknessVectors.begin(), thicknessVectors.end() );

    backPart->generateGeometry( m_backPoints,
                                backReservoirLayers,
                                m_maxCellHeight,
                                m_cellSizeHeightFactor,
                                m_horizontalPartition,
                                faultLines,
                                thicknessVectors,
                                m_topReservoirBack.z(),
                                m_faultZoneCells );

    frontPart->generateLocalNodes( m_localCoordTransform );
    backPart->generateLocalNodes( m_localCoordTransform );

    frontPart->setUseLocalCoordinates( m_useLocalCoordinates );
    backPart->setUseLocalCoordinates( m_useLocalCoordinates );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::StructGridInterface::FaceType, cvf::StructGridInterface::FaceType> RigFaultReactivationModelGenerator::sideFacesIJ( FaceType face )
{
    switch ( face )
    {
        case cvf::StructGridInterface::POS_I:
        case cvf::StructGridInterface::NEG_I:
            return { cvf::StructGridInterface::NEG_J, cvf::StructGridInterface::POS_J };

        case cvf::StructGridInterface::POS_J:
        case cvf::StructGridInterface::NEG_J:
            return { cvf::StructGridInterface::NEG_I, cvf::StructGridInterface::POS_I };

        case cvf::StructGridInterface::POS_K:
        case cvf::StructGridInterface::NEG_K:
        case cvf::StructGridInterface::NO_FACE:
        default:
            break;
    }

    CVF_ASSERT( false ); // not supported for K faces
    return { cvf::StructGridInterface::NO_FACE, cvf::StructGridInterface::NO_FACE };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFaultReactivationModelGenerator::extrapolatePoint( cvf::Vec3d startPoint, cvf::Vec3d endPoint, double buffer )
{
    cvf::Vec3d direction = endPoint - startPoint;
    direction.normalize();

    return endPoint + ( buffer * direction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::mergeTinyLayers( std::map<double, cvf::Vec3d>& layers, double minHeight )
{
    std::vector<cvf::Vec3d> newLayers;

    const int nLayers = (int)layers.size();

    std::vector<double>     keys;
    std::vector<cvf::Vec3d> vals;

    for ( auto& layer : layers )
    {
        keys.push_back( layer.first );
        vals.push_back( layer.second );
    }

    // bottom layer must always be included
    newLayers.push_back( vals.front() );

    // remove any layer that is less than minHeight above the previous layer, starting at the bottom
    for ( int k = 1; k < nLayers - 1; k++ )
    {
        if ( std::abs( keys[k] - keys[k - 1] ) < minHeight )
        {
            continue;
        }
        newLayers.push_back( vals[k] );
    }
    // top layer must always be included
    newLayers.push_back( vals.back() );

    // make sure the top two layers aren't too close, if so, remove the second topmost
    const int nNewLayers = (int)newLayers.size();
    if ( nNewLayers > 2 )
    {
        if ( std::abs( newLayers[nNewLayers - 1].z() - newLayers[nNewLayers - 2].z() ) < minHeight )
        {
            newLayers.pop_back();
            newLayers.pop_back();
            newLayers.push_back( vals.back() );
        }
    }

    layers.clear();
    for ( auto& p : newLayers )
    {
        layers[p.z()] = p;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::splitLargeLayers( std::map<double, cvf::Vec3d>& layers, double maxHeight )
{
    std::vector<cvf::Vec3d> additionalPoints;

    const int nLayers = (int)layers.size();

    std::vector<double>     keys;
    std::vector<cvf::Vec3d> vals;

    for ( auto& layer : layers )
    {
        keys.push_back( layer.first );
        vals.push_back( layer.second );
    }

    for ( int k = 0; k < nLayers; k++ )
    {
        if ( k > 0 )
        {
            if ( std::abs( keys[k] - keys[k - 1] ) > maxHeight )
            {
                const auto& points = interpolateExtraPoints( vals[k - 1], vals[k], maxHeight );
                for ( auto& p : points )
                {
                    additionalPoints.push_back( p );
                }
            }
        }
    }

    for ( auto& p : additionalPoints )
    {
        layers[p.z()] = p;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d> RigFaultReactivationModelGenerator::interpolateExtraPoints( cvf::Vec3d from, cvf::Vec3d to, double maxStep )
{
    std::vector<cvf::Vec3d> points;

    const double distance = from.pointDistance( to );
    const int    nSteps   = (int)std::ceil( distance / maxStep );
    const double stepSize = distance / nSteps;

    auto stepVec = to - from;
    stepVec.normalize();
    stepVec *= stepSize;

    cvf::Vec3d p = from;

    for ( int i = 1; i < nSteps; i++ )
    {
        p += stepVec;
        points.push_back( p );
    }

    return points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d RigFaultReactivationModelGenerator::modelNormal() const
{
    return m_modelNormal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::pair<cvf::Vec3d, cvf::Vec3d> RigFaultReactivationModelGenerator::faultTopBottomPoints() const
{
    return std::make_pair( m_topFault, m_bottomFault );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RigFaultReactivationModelGenerator::depthTopBottom() const
{
    return { -m_startDepth, m_bottomDepth };
}
