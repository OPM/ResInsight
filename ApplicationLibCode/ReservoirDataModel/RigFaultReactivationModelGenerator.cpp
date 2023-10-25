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
#include "RigMainGrid.h"

#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimUserDefinedIndexFilter.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"

#include <QDebug>
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModelGenerator::RigFaultReactivationModelGenerator( cvf::Vec3d                         position,
                                                                        cvf::Vec3d                         normal,
                                                                        size_t                             cellIndex,
                                                                        cvf::StructGridInterface::FaceType face )
    : m_startPosition( position )
    , m_startFace( face )
    , m_normal( normal )
    , m_cellIndex( cellIndex )
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
void RigFaultReactivationModelGenerator::generateGeometry()
{
    size_t i, j, k;

    RimEclipseView* view = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );

    auto cellFilters = view->cellFilterCollection();
    auto eCase       = cellFilters->firstAncestorOfType<RimEclipseCase>();

    std::vector<size_t> cellColumn;

    // get ijk of start cell
    m_grid->ijkFromCellIndexUnguarded( m_cellIndex, &i, &j, &k );

    cellColumn.push_back( m_cellIndex );

    // build column of cells behind fault
    for ( auto kLayer = 0; kLayer < m_grid->cellCountK(); kLayer++ )
    {
        auto cellIdx = m_grid->cellIndexFromIJKUnguarded( i, j, kLayer );

        if ( cellIdx != m_cellIndex ) cellColumn.push_back( cellIdx );
    }

    auto filter = cellFilters->addNewUserDefinedIndexFilter( eCase, cellColumn );
    filter->setName( "Behind fault column" );

    auto   oppositeStartFace  = cvf::StructGridInterface::oppositeFace( m_startFace );
    bool   bFoundOppositeCell = false;
    size_t oppositeCellIdx    = 0;

    for ( auto backCellIdx : cellColumn )
    {
        for ( auto& faultFace : m_fault->faultFaces() )
        {
            if ( ( faultFace.m_nativeFace == m_startFace ) && ( faultFace.m_nativeReservoirCellIndex == backCellIdx ) )
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

    // build cell column for the k column that oppositeCellIdx sits in.
    std::vector<size_t> cellColumn2;

    // get ijk of start cell
    m_grid->ijkFromCellIndexUnguarded( oppositeCellIdx, &i, &j, &k );

    // build column of cells behind fault
    for ( auto kLayer = 0; kLayer < m_grid->cellCountK(); kLayer++ )
    {
        auto cellIdx = m_grid->cellIndexFromIJKUnguarded( i, j, kLayer );

        cellColumn2.push_back( cellIdx );
    }

    filter = cellFilters->addNewUserDefinedIndexFilter( eCase, cellColumn2 );
    filter->setName( "In front of fault column" );

    auto zPositionsBack  = elementLayers( m_startFace, cellColumn );
    auto zPositionsFront = elementLayers( oppositeStartFace, cellColumn2 );

    for ( auto& kvp : zPositionsFront )
    {
        qDebug() << "Front Intersect at depth " + QString::number( kvp.first );
    }

    for ( auto& kvp : zPositionsBack )
    {
        qDebug() << "Back Intersect at depth " + QString::number( kvp.first );
    }

    // add extra fault buffer above and below fault

    double front_min = zPositionsFront.begin()->first;
    double back_min  = zPositionsBack.begin()->first;

    double front_max = zPositionsFront.rbegin()->first;
    double back_max  = zPositionsBack.rbegin()->first;

    double bottom_depth = std::min( front_min, back_min ) - m_bufferBelowFault;
    double top_depth    = std::max( front_max, back_max ) + m_bufferAboveFault;

    cvf::Vec3d top_point_back = extrapolatePoint( ( zPositionsBack.begin()++ )->second, zPositionsBack.begin()->second, top_depth );
    zPositionsBack[top_point_back.z()] = top_point_back;

    cvf::Vec3d top_point_front = extrapolatePoint( ( zPositionsFront.begin()++ )->second, zPositionsFront.begin()->second, top_depth );
    zPositionsFront[top_point_front.z()] = top_point_front;

    cvf::Vec3d bottom_point_back = extrapolatePoint( ( zPositionsBack.begin()++ )->second, zPositionsBack.begin()->second, bottom_depth );
    zPositionsBack[bottom_point_back.z()] = bottom_point_back;

    cvf::Vec3d bottom_point_front = extrapolatePoint( ( zPositionsFront.rbegin()++ )->second, zPositionsFront.rbegin()->second, bottom_depth );
    zPositionsFront[bottom_point_front.z()] = bottom_point_front;
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
cvf::Vec3d RigFaultReactivationModelGenerator::extrapolatePoint( cvf::Vec3d startPoint, cvf::Vec3d endPoint, double stopDepth )
{
    return endPoint;
}
