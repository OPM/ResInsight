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

#include "RigFault.h"
#include "RigGriddedPart3d.h"
#include "RigMainGrid.h"

#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimUserDefinedIndexFilter.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModelGenerator::RigFaultReactivationModelGenerator( cvf::Vec3d position, cvf::Vec3d normal )
    : m_startPosition( position )
    , m_normal( normal )
    , m_bufferAboveFault( 0.0 )
    , m_bufferBelowFault( 0.0 )
    , m_startDepth( 0.0 )
    , m_depthBelowFault( 100.0 )
    , m_horzExtentFromFault( 1000.0 )
    , m_modelThickness( 100.0 )
    , m_useLocalCoordinates( false )
{
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
void RigFaultReactivationModelGenerator::setFaultBufferDepth( double aboveFault, double belowFault )
{
    m_bufferAboveFault = aboveFault;
    m_bufferBelowFault = belowFault;
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
void RigFaultReactivationModelGenerator::setModelGriddingOptions( double maxCellHeight,
                                                                  double cellSizeFactor,
                                                                  int    noOfCellsHorzFront,
                                                                  int    noOfCellsHorzBack )
{
    m_maxCellHeight      = maxCellHeight;
    m_cellSizeFactor     = cellSizeFactor;
    m_noOfCellsHorzFront = noOfCellsHorzFront;
    m_noOfCellsHorzBack  = noOfCellsHorzBack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3d, cvf::Vec3d> RigFaultReactivationModelGenerator::modelLocalNormalsXY()
{
    cvf::Vec3d xNormal = m_normal ^ cvf::Vec3d::Z_AXIS;
    xNormal.z()        = 0.0;
    xNormal.normalize();

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
const std::array<int, 4> RigFaultReactivationModelGenerator::faceIJCornerIndexes( cvf::StructGridInterface::FaceType face )
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
cvf::Vec3d RigFaultReactivationModelGenerator::lineIntersect( const cvf::Plane& plane, cvf::Vec3d lineA, cvf::Vec3d lineB )
{
    double dist = 0.0;
    return caf::HexGridIntersectionTools::planeLineIntersectionForMC( plane, lineA, lineB, &dist );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFaultReactivationModelGenerator::oppositeStartCellIndex( const std::vector<size_t>          cellIndexColumn,
                                                                   cvf::StructGridInterface::FaceType face )
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
                qDebug() << "Found opposite start cell: " + QString::number( faultFace.m_oppositeReservoirCellIndex );
                bFoundOppositeCell = true;
                oppositeCellIdx    = faultFace.m_oppositeReservoirCellIndex;
                break;
            }
            else if ( ( faultFace.m_nativeFace == oppositeStartFace ) && ( faultFace.m_oppositeReservoirCellIndex == backCellIdx ) )
            {
                qDebug() << "Found backwards opposite start cell: " + QString::number( faultFace.m_nativeReservoirCellIndex );
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
void RigFaultReactivationModelGenerator::addFilter( QString name, std::vector<size_t> cells )
{
    RimEclipseView* view = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    if ( view == nullptr ) return;

    auto cellFilters = view->cellFilterCollection();
    if ( cellFilters == nullptr ) return;

    auto eCase  = cellFilters->firstAncestorOfType<RimEclipseCase>();
    auto filter = cellFilters->addNewUserDefinedIndexFilter( eCase, cells );
    filter->setName( name );
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
std::pair<std::array<cvf::Vec3d, 12>, std::array<cvf::Vec3d, 12>> RigFaultReactivationModelGenerator::generatePointsFrontBack()
{
    std::array<cvf::Vec3d, 24> points;

    double top_depth    = -m_startDepth;
    double bottom_depth = m_bottomFault.z() - m_depthBelowFault;

    cvf::Vec3d edge_front = m_startPosition + m_horzExtentFromFault * m_normal;
    cvf::Vec3d edge_back  = m_startPosition - m_horzExtentFromFault * m_normal;

    points[8]     = m_bottomFault;
    points[8].z() = bottom_depth;

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
    std::vector<size_t> backMap  = { 8, 9, 10, 12, 14, 15, 16, 17, 18, 20, 22, 23 };

    for ( int i = 0; i < 12; i++ )
    {
        frontPoints[i] = points[frontMap[i]];
        backPoints[i]  = points[backMap[i]];
    }

    return std::make_pair( frontPoints, backPoints );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::generateGeometry( size_t startCellIndex, cvf::StructGridInterface::FaceType startFace )
{
    std::vector<size_t> cellColumnBack;
    std::vector<size_t> cellColumnFront;
    size_t              i, j, k;

    // build column of cells behind fault
    m_grid->ijkFromCellIndexUnguarded( startCellIndex, &i, &j, &k );
    cellColumnBack.push_back( startCellIndex ); // want the user clicked cell to be the first in the list for later

    for ( auto kLayer = 0; kLayer < m_grid->cellCountK(); kLayer++ )
    {
        auto cellIdx = m_grid->cellIndexFromIJKUnguarded( i, j, kLayer );

        if ( cellIdx != startCellIndex ) cellColumnBack.push_back( cellIdx );
    }

    // build cell column of cells in front of fault, opposite to the cell column behind the fault
    auto   oppositeStartFace = cvf::StructGridInterface::oppositeFace( startFace );
    size_t oppositeCellIdx   = oppositeStartCellIndex( cellColumnBack, startFace );

    m_grid->ijkFromCellIndexUnguarded( oppositeCellIdx, &i, &j, &k );
    for ( auto kLayer = 0; kLayer < m_grid->cellCountK(); kLayer++ )
    {
        auto cellIdx = m_grid->cellIndexFromIJKUnguarded( i, j, kLayer );

        cellColumnFront.push_back( cellIdx );
    }

    addFilter( "In front of fault column", cellColumnFront );
    addFilter( "Behind fault column", cellColumnBack );

    auto zPositionsBack  = elementLayers( startFace, cellColumnBack );
    auto zPositionsFront = elementLayers( oppositeStartFace, cellColumnFront );

    // add extra fault buffer above the fault, starting at the deepest top-most cell on either side of the fault

    double front_top    = zPositionsFront.begin()->first;
    double back_top     = zPositionsBack.begin()->first;
    m_topReservoirFront = zPositionsFront.begin()->second;
    m_topReservoirBack  = zPositionsBack.begin()->second;

    cvf::Vec3d top_point = m_topReservoirFront;

    if ( front_top > back_top )
    {
        top_point = extrapolatePoint( ( ++zPositionsBack.begin() )->second, zPositionsBack.begin()->second, m_bufferAboveFault );
    }
    else if ( back_top > front_top )
    {
        top_point = extrapolatePoint( ( ++zPositionsFront.begin() )->second, zPositionsFront.begin()->second, m_bufferAboveFault );
    }

    if ( front_top != back_top )
    {
        zPositionsBack[top_point.z()]  = top_point;
        zPositionsFront[top_point.z()] = top_point;
    }
    m_topFault = top_point;

    // add extra fault buffer below the fault, starting at the shallowest bottom-most cell on either side of the fault

    double front_bottom    = zPositionsFront.rbegin()->first;
    double back_bottom     = zPositionsBack.rbegin()->first;
    m_bottomReservoirFront = zPositionsFront.rbegin()->second;
    m_bottomReservoirBack  = zPositionsBack.rbegin()->second;

    cvf::Vec3d bottom_point = m_bottomReservoirFront;

    if ( front_bottom > back_bottom )
    {
        bottom_point = extrapolatePoint( ( ++zPositionsFront.rbegin() )->second, zPositionsFront.rbegin()->second, m_bufferBelowFault );
    }
    else if ( back_bottom > front_bottom )
    {
        bottom_point = extrapolatePoint( ( ++zPositionsBack.rbegin() )->second, zPositionsBack.rbegin()->second, m_bufferBelowFault );
    }

    if ( back_bottom != front_bottom )
    {
        zPositionsBack[bottom_point.z()]  = bottom_point;
        zPositionsFront[bottom_point.z()] = bottom_point;
    }
    m_bottomFault = bottom_point;

    // TODO - spilt layers in zPositions that are larger than the user given limit m_maxCellHeight

    std::vector<cvf::Vec3d> frontReservoirLayers;
    for ( auto& kvp : zPositionsFront )
        frontReservoirLayers.push_back( kvp.second );

    std::vector<cvf::Vec3d> backReservoirLayers;
    for ( auto& kvp : zPositionsBack )
        backReservoirLayers.push_back( kvp.second );

    auto [frontPartPoints, backPartPoints] = generatePointsFrontBack();

    RigGriddedPart3d frontPart;
    RigGriddedPart3d backPart;

    frontPart.generateGeometry( frontPartPoints, frontReservoirLayers, m_maxCellHeight, m_cellSizeFactor, m_noOfCellsHorzFront, m_modelThickness );
    backPart.generateGeometry( backPartPoints, backReservoirLayers, m_maxCellHeight, m_cellSizeFactor, m_noOfCellsHorzBack, m_modelThickness );

    frontPart.generateLocalNodes( m_localCoordTransform );
    backPart.generateLocalNodes( m_localCoordTransform );

    frontPart.setUseLocalCoordinates( m_useLocalCoordinates );
    backPart.setUseLocalCoordinates( m_useLocalCoordinates );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<double, cvf::Vec3d> RigFaultReactivationModelGenerator::elementLayers( cvf::StructGridInterface::FaceType face,
                                                                                const std::vector<size_t>&         cellIndexColumn )
{
    cvf::Plane modelPlane;
    modelPlane.setFromPointAndNormal( m_startPosition, m_normal );

    auto cornerIndexes = faceIJCornerIndexes( face );

    std::map<double, cvf::Vec3d> zPositions;

    for ( auto cellIdx : cellIndexColumn )
    {
        RigCell cell    = m_grid->cell( cellIdx );
        auto    corners = cell.faceCorners( face );

        cvf::Vec3d intersect1 = lineIntersect( modelPlane, corners[cornerIndexes[0]], corners[cornerIndexes[1]] );
        cvf::Vec3d intersect2 = lineIntersect( modelPlane, corners[cornerIndexes[2]], corners[cornerIndexes[3]] );

        zPositions[intersect1.z()] = intersect1;
        zPositions[intersect2.z()] = intersect2;

        qDebug() << "Intersect candidate 1 at depth " + QString::number( intersect1.z() );
        qDebug() << "Intersect candidate 2 at depth " + QString::number( intersect2.z() );
    }

    return zPositions;
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
