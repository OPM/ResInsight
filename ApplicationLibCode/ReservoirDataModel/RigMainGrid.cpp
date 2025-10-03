/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigMainGrid.h"

#include "RiaLogging.h"
#include "RiaOpenMPTools.h"
#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigHexIntersectionTools.h"
#include "RigNNCData.h"

#include "cvfAssert.h"
#include "cvfBoundingBoxTree.h"

#include <array>

RigMainGrid::RigMainGrid()
    : RigGridBase( this )
{
    m_displayModelOffset = cvf::Vec3d::ZERO;

    m_gridIndex = 0;
    m_gridId    = 0;
    m_gridIdToIndexMapping.push_back( 0 );

    m_flipXAxis = false;
    m_flipYAxis = false;

    m_useMapAxes   = false;
    m_mapAxes      = defaultMapAxes();
    m_dualPorosity = false;

    m_isFaceNormalsOutwards         = true;
    m_isFaceNormalsOutwardsComputed = false;
}

RigMainGrid::~RigMainGrid()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>& RigMainGrid::nodes()
{
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigMainGrid::nodes() const
{
    return m_nodes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::totalCellCount() const
{
    return m_cells.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCell>& RigMainGrid::reservoirCells()
{
    return m_cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCell>& RigMainGrid::reservoirCells() const
{
    return m_cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridAndGridLocalIdxFromGlobalCellIdx( size_t globalCellIdx, size_t* gridLocalCellIdx )
{
    CVF_ASSERT( globalCellIdx < totalCellCount() );

    const RigCell& cell     = this->cell( globalCellIdx );
    RigGridBase*   hostGrid = cell.hostGrid();
    CVF_ASSERT( hostGrid );

    if ( gridLocalCellIdx )
    {
        *gridLocalCellIdx = cell.gridLocalCellIndex();
    }

    return hostGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigMainGrid::gridAndGridLocalIdxFromGlobalCellIdx( size_t globalCellIdx, size_t* gridLocalCellIdx ) const
{
    CVF_ASSERT( globalCellIdx < totalCellCount() );

    const RigCell&     cell     = this->cell( globalCellIdx );
    const RigGridBase* hostGrid = cell.hostGrid();
    CVF_ASSERT( hostGrid );

    if ( gridLocalCellIdx )
    {
        *gridLocalCellIdx = cell.gridLocalCellIndex();
    }

    return hostGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigMainGrid::cellByGridAndGridLocalCellIdx( size_t gridIdx, size_t gridLocalCellIdx ) const
{
    return gridByIndex( gridIdx )->cell( gridLocalCellIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::reservoirCellIndexByGridAndGridLocalCellIndex( size_t gridIdx, size_t gridLocalCellIdx ) const
{
    return gridByIndex( gridIdx )->reservoirCellIndex( gridLocalCellIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::findReservoirCellIndexFromPoint( const cvf::Vec3d& point ) const
{
    cvf::BoundingBox pointBBox;
    pointBBox.add( point );

    std::vector<size_t> cellIndices = m_mainGrid->findIntersectingCells( pointBBox );

    for ( size_t cellIndex : cellIndices )
    {
        std::array<cvf::Vec3d, 8> hexCorners = m_mainGrid->cellCornerVertices( cellIndex );
        if ( RigHexIntersectionTools::isPointInCell( point, hexCorners ) )
        {
            return cellIndex;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::addLocalGrid( RigLocalGrid* localGrid )
{
    CVF_ASSERT( localGrid && localGrid->gridId() != cvf::UNDEFINED_INT ); // The grid ID must be set.
    CVF_ASSERT( localGrid->gridId() >= 0 ); // We cant handle negative ID's if they exist.

    m_localGrids.push_back( localGrid );
    localGrid->setGridIndex( m_localGrids.size() ); // Maingrid itself has grid index 0

    if ( m_gridIdToIndexMapping.size() <= static_cast<size_t>( localGrid->gridId() ) )
    {
        m_gridIdToIndexMapping.resize( localGrid->gridId() + 1, cvf::UNDEFINED_SIZE_T );
    }

    m_gridIdToIndexMapping[localGrid->gridId()] = localGrid->gridIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::gridCountOnFile() const
{
    size_t gridCount = 1;

    for ( const auto& grid : m_localGrids )
    {
        if ( !grid->isTempGrid() )
        {
            gridCount++;
        }
    }

    return gridCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::gridCount() const
{
    return m_localGrids.size() + 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubGridsParentGridPointer()
{
    if ( !m_localGrids.empty() && m_localGrids[0]->parentGrid() == nullptr )
    {
        initSubGridParentPointer();
        size_t i;
        for ( i = 0; i < m_localGrids.size(); ++i )
        {
            m_localGrids[i]->initSubGridParentPointer();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::initAllSubCellsMainGridCellIndex()
{
    initSubCellsMainGridCellIndex();
    size_t i;
    for ( i = 0; i < m_localGrids.size(); ++i )
    {
        m_localGrids[i]->initSubCellsMainGridCellIndex();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigMainGrid::displayModelOffset() const
{
    return m_displayModelOffset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setDisplayModelOffset( cvf::Vec3d offset )
{
    m_displayModelOffset = offset;
}

//--------------------------------------------------------------------------------------------------
/// Initialize pointers from grid to parent grid
/// Compute cell ranges for active and valid cells
/// Compute bounding box in world coordinates based on node coordinates
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeCachedData( std::string* aabbTreeInfo )
{
    initAllSubGridsParentGridPointer();
    initAllSubCellsMainGridCellIndex();

    m_cellSearchTree = nullptr;

    const double maxNumberOfLeafNodes = 4000000;
    const double factor               = std::ceil( cellCount() / maxNumberOfLeafNodes );
    const size_t cellsPerBoundingBox  = std::max( size_t( 1 ), static_cast<size_t>( factor ) );

    if ( cellsPerBoundingBox > 1 )
    {
        buildCellSearchTreeOptimized( cellsPerBoundingBox );
    }
    else
    {
        buildCellSearchTree();
    }

    if ( aabbTreeInfo )
    {
        *aabbTreeInfo += "Cells per bounding box : " + std::to_string( cellsPerBoundingBox ) + "\n";
        *aabbTreeInfo += m_cellSearchTree->info();
    }

    computeBoundingBox();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridByIndex( size_t localGridIndex )
{
    if ( localGridIndex == 0 ) return this;
    CVF_ASSERT( localGridIndex - 1 < m_localGrids.size() );
    return m_localGrids[localGridIndex - 1].p();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with index \a localGridIndex. Main Grid itself has index 0. First LGR starts on 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigMainGrid::gridByIndex( size_t localGridIndex ) const
{
    if ( localGridIndex == 0 ) return this;
    CVF_ASSERT( localGridIndex - 1 < m_localGrids.size() );
    return m_localGrids[localGridIndex - 1].p();
}

//--------------------------------------------------------------------------------------------------
/// Returns the grid with the given name. Main Grid itself could be retreived by using name ""
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridByName( const std::string& name )
{
    if ( name.empty() ) return this;

    for ( auto& grid : m_localGrids )
    {
        if ( grid->gridName() == name ) return grid.p();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setFlipAxis( bool flipXAxis, bool flipYAxis )
{
    bool needFlipX = false;
    bool needFlipY = false;

    if ( m_flipXAxis != flipXAxis )
    {
        needFlipX = true;
    }

    if ( m_flipYAxis != flipYAxis )
    {
        needFlipY = true;
    }

    if ( needFlipX || needFlipY )
    {
        for ( size_t i = 0; i < m_nodes.size(); i++ )
        {
            if ( needFlipX )
            {
                m_nodes[i].x() *= -1.0;
            }

            if ( needFlipY )
            {
                m_nodes[i].y() *= -1.0;
            }
        }

        m_flipXAxis = flipXAxis;
        m_flipYAxis = flipYAxis;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridBase* RigMainGrid::gridById( int localGridId )
{
    CVF_ASSERT( localGridId >= 0 && static_cast<size_t>( localGridId ) < m_gridIdToIndexMapping.size() );
    return gridByIndex( m_gridIdToIndexMapping[localGridId] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigMainGrid::totalTemporaryGridCellCount() const
{
    size_t cellCount = 0;

    for ( const auto& grid : m_localGrids )
    {
        if ( grid->isTempGrid() )
        {
            cellCount += grid->cellCount();
        }
    }

    return cellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigNNCData* RigMainGrid::nncData()
{
    if ( m_nncData.isNull() )
    {
        m_nncData = new RigNNCData;
    }

    return m_nncData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setFaults( const cvf::Collection<RigFault>& faults )
{
    m_faults = faults;

#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( m_faults.size() ); i++ )
    {
        m_faults[i]->computeFaultFacesFromCellRanges( mainGrid() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Collection<RigFault>& RigMainGrid::faults() const
{
    return m_faults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Collection<RigFault>& RigMainGrid::faults()
{
    return m_faults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::hasFaultWithName( const QString& name ) const
{
    for ( auto fault : m_faults )
    {
        if ( fault->name() == name )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeBoundingBox()
{
    m_boundingBox.reset();

    const int numberOfThreads = RiaOpenMPTools::availableThreadCount();

    std::vector<cvf::BoundingBox> threadBoundingBoxes( numberOfThreads );

#pragma omp parallel
    {
        int myThread = RiaOpenMPTools::currentThreadIndex();

        // NB! We are inside a parallel section, do not use "parallel for" here
#pragma omp for
        for ( long i = 0; i < static_cast<long>( m_nodes.size() ); i++ )
        {
            threadBoundingBoxes[myThread].add( m_nodes[i] );
        }
    }

    for ( int i = 0; i < numberOfThreads; i++ )
    {
        m_boundingBox.add( threadBoundingBoxes[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::calculateFaults( const RigActiveCellInfo* activeCellInfo )
{
    if ( hasFaultWithName( RiaResultNames::undefinedGridFaultName() ) &&
         hasFaultWithName( RiaResultNames::undefinedGridFaultWithInactiveName() ) )
    {
        // RiaLogging::debug(QString("Calculate faults already run for grid."));

        return;
    }

    m_faultsPrCellAcc = new RigFaultsPrCellAccumulator( totalCellCount() );

    // Spread fault idx'es on the cells from the faults
    for ( size_t fIdx = 0; fIdx < m_faults.size(); ++fIdx )
    {
        m_faults[fIdx]->accumulateFaultsPrCell( m_faultsPrCellAcc.p(), static_cast<int>( fIdx ) );
    }

    // Find the geometrical faults that is in addition: Has no user defined (eclipse) fault assigned.
    // Separate the grid faults that has an inactive cell as member

    RigFault* unNamedFault = new RigFault;
    unNamedFault->setName( RiaResultNames::undefinedGridFaultName() );
    int unNamedFaultIdx = static_cast<int>( m_faults.size() );
    m_faults.push_back( unNamedFault );

    RigFault* unNamedFaultWithInactive = new RigFault;
    unNamedFaultWithInactive->setName( RiaResultNames::undefinedGridFaultWithInactiveName() );
    int unNamedFaultWithInactiveIdx = static_cast<int>( m_faults.size() );
    m_faults.push_back( unNamedFaultWithInactive );

    const std::vector<cvf::Vec3d>& vxs = m_mainGrid->nodes();

    std::vector<RigFault::FaultFace>& unNamedFaultFaces         = unNamedFault->faultFaces();
    std::vector<RigFault::FaultFace>& unNamedFaultFacesInactive = unNamedFaultWithInactive->faultFaces();
    for ( int gcIdx = 0; gcIdx < static_cast<int>( totalCellCount() ); ++gcIdx )
    {
        addUnNamedFaultFaces( gcIdx,
                              activeCellInfo,
                              vxs,
                              unNamedFaultIdx,
                              unNamedFaultWithInactiveIdx,
                              unNamedFaultFaces,
                              unNamedFaultFacesInactive,
                              m_faultsPrCellAcc.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::addUnNamedFaultFaces( int                               gcIdx,
                                        const RigActiveCellInfo*          activeCellInfo,
                                        const std::vector<cvf::Vec3d>&    vxs,
                                        int                               unNamedFaultIdx,
                                        int                               unNamedFaultWithInactiveIdx,
                                        std::vector<RigFault::FaultFace>& unNamedFaultFaces,
                                        std::vector<RigFault::FaultFace>& unNamedFaultFacesInactive,
                                        RigFaultsPrCellAccumulator*       faultsPrCellAcc ) const
{
    if ( cell( gcIdx ).isInvalid() )
    {
        return;
    }

    size_t neighborReservoirCellIdx;
    size_t neighborGridCellIdx;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    const RigGridBase* hostGrid                 = nullptr;
    bool               firstNO_FAULTFaceForCell = true;
    bool               isCellActive             = true;

    char upperLimitForFaceType = cvf::StructGridInterface::FaceType::POS_K;

    // Compare only I and J faces
    for ( char faceIdx = 0; faceIdx < upperLimitForFaceType; ++faceIdx )
    {
        cvf::StructGridInterface::FaceType face = cvf::StructGridInterface::FaceType( faceIdx );

        // For faces that has no used defined Fault assigned:

        if ( m_faultsPrCellAcc->faultIdx( gcIdx, face ) == RigFaultsPrCellAccumulator::NO_FAULT )
        {
            // Find neighbor cell
            if ( firstNO_FAULTFaceForCell ) // To avoid doing this for every face, and only when detecting a NO_FAULT
            {
                size_t gridLocalCellIndex;
                hostGrid = gridAndGridLocalIdxFromGlobalCellIdx( gcIdx, &gridLocalCellIndex );

                hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k );
                isCellActive = activeCellInfo->isActive( gcIdx );

                firstNO_FAULTFaceForCell = false;
            }

            if ( !hostGrid->cellIJKNeighbor( i, j, k, face, &neighborGridCellIdx ) )
            {
                continue;
            }

            neighborReservoirCellIdx = hostGrid->reservoirCellIndex( neighborGridCellIdx );
            if ( cell( neighborReservoirCellIdx ).isInvalid() )
            {
                continue;
            }

            bool isNeighborCellActive = activeCellInfo->isActive( neighborReservoirCellIdx );

            double tolerance = 1e-6;

            std::array<size_t, 4> faceIdxs;
            cell( gcIdx ).faceIndices( face, &faceIdxs );
            std::array<size_t, 4> nbFaceIdxs;
            cell( neighborReservoirCellIdx ).faceIndices( StructGridInterface::oppositeFace( face ), &nbFaceIdxs );

            bool sharedFaceVertices = true;
            if ( sharedFaceVertices && vxs[faceIdxs[0]].pointDistance( vxs[nbFaceIdxs[0]] ) > tolerance ) sharedFaceVertices = false;
            if ( sharedFaceVertices && vxs[faceIdxs[1]].pointDistance( vxs[nbFaceIdxs[3]] ) > tolerance ) sharedFaceVertices = false;
            if ( sharedFaceVertices && vxs[faceIdxs[2]].pointDistance( vxs[nbFaceIdxs[2]] ) > tolerance ) sharedFaceVertices = false;
            if ( sharedFaceVertices && vxs[faceIdxs[3]].pointDistance( vxs[nbFaceIdxs[1]] ) > tolerance ) sharedFaceVertices = false;

            if ( sharedFaceVertices )
            {
                continue;
            }

            // To avoid doing this calculation for the opposite face
            int faultIdx = unNamedFaultIdx;
            if ( !( isCellActive && isNeighborCellActive ) ) faultIdx = unNamedFaultWithInactiveIdx;

            faultsPrCellAcc->setFaultIdx( gcIdx, face, faultIdx );
            faultsPrCellAcc->setFaultIdx( neighborReservoirCellIdx, StructGridInterface::oppositeFace( face ), faultIdx );

            // Add as fault face only if the grid index is less than the neighbors

            if ( static_cast<size_t>( gcIdx ) < neighborReservoirCellIdx )
            {
                RigFault::FaultFace ff( gcIdx, cvf::StructGridInterface::FaceType( faceIdx ), neighborReservoirCellIdx );
                if ( isCellActive && isNeighborCellActive )
                {
                    unNamedFaultFaces.push_back( ff );
                }
                else
                {
                    unNamedFaultFacesInactive.push_back( ff );
                }
            }
            else
            {
                CVF_FAIL_MSG( "Found fault with global neighbor index less than the native index. " );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::distributeNNCsToFaults()
{
    if ( m_faultsPrCellAcc.isNull() ) return;

    const RigConnectionContainer& nncs = nncData()->allConnections();
    for ( size_t nncIdx = 0; nncIdx < nncs.size(); ++nncIdx )
    {
        // Find the fault for each side of the nnc
        const RigConnection& conn  = nncs[nncIdx];
        int                  fIdx1 = RigFaultsPrCellAccumulator::NO_FAULT;
        int                  fIdx2 = RigFaultsPrCellAccumulator::NO_FAULT;

        if ( conn.face() != StructGridInterface::NO_FACE )
        {
            fIdx1 = m_faultsPrCellAcc->faultIdx( conn.c1GlobIdx(), conn.face() );
            fIdx2 = m_faultsPrCellAcc->faultIdx( conn.c2GlobIdx(), StructGridInterface::oppositeFace( conn.face() ) );
        }

        if ( fIdx1 < 0 && fIdx2 < 0 )
        {
            cvf::String lgrString( "Same Grid" );
            if ( cell( conn.c1GlobIdx() ).hostGrid() != cell( conn.c2GlobIdx() ).hostGrid() )
            {
                lgrString = "Different Grid";
            }

            // cvf::Trace::show("NNC: No Fault for NNC C1: " + cvf::String((int)conn.m_c1GlobIdx) + " C2: " +
            // cvf::String((int)conn.m_c2GlobIdx) + " Grid: " + lgrString);
        }

        if ( fIdx1 >= 0 )
        {
            // Add the connection to both, if they are different.
            m_faults[fIdx1]->connectionIndices().push_back( nncIdx );
        }

        if ( fIdx2 != fIdx1 )
        {
            if ( fIdx2 >= 0 )
            {
                m_faults[fIdx2]->connectionIndices().push_back( nncIdx );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// The cell is normally inverted due to Depth becoming -Z at import,
/// but if (only) one of the flipX/Y is done, the cell is back to normal
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::isFaceNormalsOutwards() const
{
    if ( !m_isFaceNormalsOutwardsComputed )
    {
        std::vector<size_t> reservoirCellIndices;
        reservoirCellIndices.resize( totalCellCount() );
        std::iota( reservoirCellIndices.begin(), reservoirCellIndices.end(), 0 );

        computeFaceNormalsDirection( reservoirCellIndices );
    }

    return m_isFaceNormalsOutwards;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::computeFaceNormalsDirection( const std::vector<size_t>& reservoirCellIndices ) const
{
    auto isValidAndFaceNormalDir =
        []( const double ijSize, const double kSize, const RigCell& cell, cvf::StructGridInterface::FaceType face ) -> std::pair<bool, bool>
    {
        const cvf::Vec3d cellCenter = cell.center();
        const cvf::Vec3d faceCenter = cell.faceCenter( face );
        const cvf::Vec3d faceNormal = cell.faceNormalWithAreaLength( face );

        if ( ( faceCenter - cellCenter ).length() > 0.2 * ijSize && ( faceNormal.length() > ( 0.2 * ijSize * 0.2 * kSize ) ) )
        {
            // Cell is assumed ok to use, so calculate whether the normals are outwards or inwards
            if ( ( faceCenter - cellCenter ) * faceNormal >= 0 )
            {
                return { true, true };
            }

            return { true, false };
        }

        return { false, false };
    };

    double ijSize = characteristicIJCellSize();

    cvf::Vec3d   cellSize             = characteristicCellSizes();
    const double characteristicVolume = cellSize.x() * cellSize.y() * cellSize.z();

    for ( const auto& index : reservoirCellIndices )
    {
        const auto& cell = this->cell( index );
        if ( !cell.isInvalid() )
        {
            // Some cells can be very twisted and distorted. Use a volume criteria to find a reasonably regular cell.
            const double cellVolume = cell.volume();
            if ( cellVolume < characteristicVolume * 0.8 ) continue;

            auto [isValid1, direction1] = isValidAndFaceNormalDir( ijSize, cellSize.z(), cell, cvf::StructGridInterface::FaceType::NEG_I );
            auto [isValid2, direction2] = isValidAndFaceNormalDir( ijSize, cellSize.z(), cell, cvf::StructGridInterface::FaceType::POS_I );
            auto [isValid3, direction3] = isValidAndFaceNormalDir( ijSize, cellSize.z(), cell, cvf::StructGridInterface::FaceType::NEG_J );
            auto [isValid4, direction4] = isValidAndFaceNormalDir( ijSize, cellSize.z(), cell, cvf::StructGridInterface::FaceType::POS_J );

            if ( !isValid1 || !isValid2 || !isValid3 || !isValid4 ) continue;

            if ( direction1 && direction2 && direction3 && direction4 )
            {
                // All face normals pointing outwards
                m_isFaceNormalsOutwards         = true;
                m_isFaceNormalsOutwardsComputed = true;
                return;
            }

            if ( !direction1 && !direction2 && !direction3 && !direction4 )
            {
                // All cell face normals pointing inwards
                m_isFaceNormalsOutwards         = false;
                m_isFaceNormalsOutwardsComputed = true;
                return;
            }
        }
    }

    // If this code is reached, it was not possible to get a consistent answer on the direction of a cell surface
    // normal. Set a default direction for face normals.
    m_isFaceNormalsOutwards         = true;
    m_isFaceNormalsOutwardsComputed = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFault* RigMainGrid::findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex, cvf::StructGridInterface::FaceType face ) const
{
    if ( m_faultsPrCellAcc.isNull() ) return nullptr;

    if ( face == cvf::StructGridInterface::NO_FACE ) return nullptr;

    int faultIdx = m_faultsPrCellAcc->faultIdx( reservoirCellIndex, face );
    if ( faultIdx != RigFaultsPrCellAccumulator::NO_FAULT )
    {
        return m_faults.at( faultIdx );
    }

#if 0
    for (size_t i = 0; i < m_faults.size(); i++)
    {
        const RigFault* rigFault = m_faults.at(i);
        const std::vector<RigFault::FaultFace>& faultFaces = rigFault->faultFaces();

        for (size_t fIdx = 0; fIdx < faultFaces.size(); fIdx++)
        {
            if (faultFaces[fIdx].m_nativeReservoirCellIndex == cellIndex)
            {
                if (face == faultFaces[fIdx].m_nativeFace )
                {
                    return rigFault;
                }
            }

            if (faultFaces[fIdx].m_oppositeReservoirCellIndex == cellIndex)
            {
                if (face == cvf::StructGridInterface::oppositeFace(faultFaces[fIdx].m_nativeFace))
                {
                    return rigFault;
                }
            }
        }
    }
#endif
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigMainGrid::findIntersectingCells( const cvf::BoundingBox& inputBB ) const
{
    CVF_ASSERT( m_cellSearchTree.notNull() );
    std::vector<size_t> cellIndices;
    m_cellSearchTree->findIntersections( inputBB, &cellIndices );
    return cellIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::buildCellSearchTree()
{
    if ( m_cellSearchTree.isNull() )
    {
        // build tree

        size_t cellCount = totalCellCount();

        std::vector<size_t>           cellIndicesForBoundingBoxes;
        std::vector<cvf::BoundingBox> cellBoundingBoxes;

#pragma omp parallel
        {
            int    numberOfThreads = RiaOpenMPTools::availableThreadCount();
            size_t threadCellCount = std::ceil( cellCount / static_cast<double>( numberOfThreads ) );

            std::vector<size_t>           threadIndicesForBoundingBoxes;
            std::vector<cvf::BoundingBox> threadBoundingBoxes;

            threadIndicesForBoundingBoxes.reserve( threadCellCount );
            threadBoundingBoxes.reserve( threadCellCount );

#pragma omp for
            for ( int cIdx = 0; cIdx < (int)cellCount; ++cIdx )
            {
                auto& cell = this->cell( cIdx );
                if ( cell.isInvalid() ) continue;

                const std::array<size_t, 8>& cellIndices = cell.cornerIndices();

                cvf::BoundingBox cellBB;
                for ( size_t i : cellIndices )
                {
                    cellBB.add( m_nodes[i] );
                }

                if ( cellBB.isValid() )
                {
                    threadIndicesForBoundingBoxes.emplace_back( cIdx );
                    threadBoundingBoxes.emplace_back( cellBB );
                }
            }

            threadIndicesForBoundingBoxes.shrink_to_fit();
            threadBoundingBoxes.shrink_to_fit();

#pragma omp critical
            {
                cellIndicesForBoundingBoxes.insert( cellIndicesForBoundingBoxes.end(),
                                                    threadIndicesForBoundingBoxes.begin(),
                                                    threadIndicesForBoundingBoxes.end() );

                cellBoundingBoxes.insert( cellBoundingBoxes.end(), threadBoundingBoxes.begin(), threadBoundingBoxes.end() );
            }
        }
        m_cellSearchTree = new cvf::BoundingBoxTree;
        m_cellSearchTree->buildTreeFromBoundingBoxes( cellBoundingBoxes, &cellIndicesForBoundingBoxes );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::buildCellSearchTreeOptimized( size_t cellsPerBoundingBox )
{
    int threadCount = RiaOpenMPTools::availableThreadCount();

    std::vector<std::vector<std::vector<int>>> threadCellIndicesForBoundingBoxes( threadCount );
    std::vector<std::vector<cvf::BoundingBox>> threadCellBoundingBoxes( threadCount );

#pragma omp parallel
    {
        int myThread = RiaOpenMPTools::currentThreadIndex();

#pragma omp for
        for ( int i = 0; i < static_cast<int>( cellCountI() ); i++ )
        {
            for ( size_t j = 0; j < cellCountJ(); j++ )
            {
                size_t k = 0;
                while ( k < cellCountK() )
                {
                    size_t kCount = 0;

                    std::vector<int> aggregatedCellIndices;
                    cvf::BoundingBox accumulatedBB;

                    while ( ( kCount < cellsPerBoundingBox ) && ( k + kCount < cellCountK() ) )
                    {
                        size_t      cellIdx = cellIndexFromIJK( i, j, k + kCount );
                        const auto& rigCell = cell( cellIdx );

                        if ( !rigCell.isInvalid() )
                        {
                            aggregatedCellIndices.push_back( static_cast<int>( cellIdx ) );

                            // Add all cells in sub grid contained in this main grid cell
                            if ( auto subGrid = rigCell.subGrid() )
                            {
                                for ( size_t localIdx = 0; localIdx < subGrid->cellCount(); localIdx++ )
                                {
                                    const auto& localCell = subGrid->cell( localIdx );
                                    if ( localCell.mainGridCellIndex() == cellIdx )
                                    {
                                        aggregatedCellIndices.push_back( static_cast<int>( subGrid->reservoirCellIndex( localIdx ) ) );
                                    }
                                }
                            }

                            const std::array<size_t, 8>& cellIndices = rigCell.cornerIndices();

                            cvf::BoundingBox cellBB;
                            for ( size_t i : cellIndices )
                            {
                                cellBB.add( m_nodes[i] );
                            }

                            if ( cellBB.isValid() ) accumulatedBB.add( cellBB );
                        }
                        kCount++;
                    }

                    k += kCount;
                    kCount = 0;

                    threadCellIndicesForBoundingBoxes[myThread].emplace_back( aggregatedCellIndices );
                    threadCellBoundingBoxes[myThread].emplace_back( accumulatedBB );
                }
            }
        }
    }

    std::vector<std::vector<int>> cellIndicesForBoundingBoxes;
    std::vector<cvf::BoundingBox> cellBoundingBoxes;

    for ( auto i = 0; i < threadCount; i++ )
    {
        cellIndicesForBoundingBoxes.insert( cellIndicesForBoundingBoxes.end(),
                                            threadCellIndicesForBoundingBoxes[i].begin(),
                                            threadCellIndicesForBoundingBoxes[i].end() );

        cellBoundingBoxes.insert( cellBoundingBoxes.end(), threadCellBoundingBoxes[i].begin(), threadCellBoundingBoxes[i].end() );
    }

    m_cellSearchTree = new cvf::BoundingBoxTree;
    m_cellSearchTree->buildTreeFromBoundingBoxesOptimized( cellBoundingBoxes, cellIndicesForBoundingBoxes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigMainGrid::boundingBox() const
{
    return m_boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::isTempGrid() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RigMainGrid::associatedWellPathName() const
{
    static const std::string EMPTY_STRING;
    return EMPTY_STRING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setUseMapAxes( bool useMapAxes )
{
    m_useMapAxes = useMapAxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::useMapAxes() const
{
    return m_useMapAxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setMapAxes( const std::array<double, 6>& mapAxes )
{
    m_mapAxes = mapAxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::array<double, 6>& RigMainGrid::mapAxes() const
{
    return m_mapAxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<float, 6> RigMainGrid::mapAxesF() const
{
    std::array<float, 6> floatAxes;
    for ( size_t i = 0; i < 6; ++i )
    {
        floatAxes[i] = (float)m_mapAxes[i];
    }
    return floatAxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RigMainGrid::mapAxisTransform() const
{
    cvf::Mat4d mapAxisTrans;
    if ( m_useMapAxes )
    {
        cvf::Vec3d origin( m_mapAxes[2], m_mapAxes[3], 0.0 );
        cvf::Vec3d xAxis = cvf::Vec3d( m_mapAxes[4] - origin[0], m_mapAxes[5] - origin[1], 0.0 ).getNormalized();
        cvf::Vec3d yAxis = cvf::Vec3d( m_mapAxes[0] - origin[0], m_mapAxes[1] - origin[1], 0.0 ).getNormalized();
        cvf::Vec3d zAxis( 0.0, 0.0, 1.0 );
        mapAxisTrans = cvf::Mat4d::fromCoordSystemAxes( &xAxis, &yAxis, &zAxis );
        mapAxisTrans.setTranslation( origin );
        mapAxisTrans.invert();
    }
    else
    {
        mapAxisTrans.setIdentity();
    }
    return mapAxisTrans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMainGrid::isDualPorosity() const
{
    return m_dualPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMainGrid::setDualPorosity( bool enable )
{
    m_dualPorosity = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, 6> RigMainGrid::defaultMapAxes()
{
    const double origin[2] = { 0.0, 0.0 };
    const double xPoint[2] = { 1.0, 0.0 };
    const double yPoint[2] = { 0.0, 1.0 };

    // Order (see Elipse Reference Manual for keyword MAPAXES): Y_x, Y_y, O_x, O_y, X_x, X_y
    return { yPoint[0], yPoint[1], origin[0], origin[1], xPoint[0], xPoint[1] };
}

//--------------------------------------------------------------------------------------------------
// Invalidate all cells with I > iLimit (0 based index)
//--------------------------------------------------------------------------------------------------
void RigMainGrid::invalidateCellsAboveI( size_t iLimit )
{
    const auto cells = cellCount();
    for ( size_t i = iLimit + 1; i < cellCountI(); i++ )
    {
        for ( size_t j = 0; j < cellCountJ(); j++ )
        {
            for ( size_t k = 0; k < cellCountK(); k++ )
            {
                const auto cellIdx = cellIndexFromIJK( i, j, k );
                if ( cellIdx < cells )
                {
                    RigCell& c = cell( cellIdx );
                    c.setInvalid( true );
                }
            }
        }
    }
}
