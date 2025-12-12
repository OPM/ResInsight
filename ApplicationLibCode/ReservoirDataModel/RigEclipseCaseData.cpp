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

#include "RigEclipseCaseData.h"

#include "RifReaderEclipseOutput.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigEquil.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "Well/RigSimWellData.h"
#include "Well/RigSimulationWellCenterLineCalculator.h"
#include "Well/RigSimulationWellCoordsAndMD.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include <array>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData::RigEclipseCaseData( RimEclipseCase* ownerCase )
    : m_hasParsedDeckForEquilData( false )
{
    m_mainGrid  = std::make_shared<RigMainGrid>();
    m_ownerCase = ownerCase;

    m_matrixModelResults   = std::make_shared<RigCaseCellResultsData>( this, RiaDefines::PorosityModelType::MATRIX_MODEL );
    m_fractureModelResults = std::make_shared<RigCaseCellResultsData>( this, RiaDefines::PorosityModelType::FRACTURE_MODEL );

    m_activeCellInfo         = std::make_shared<RigActiveCellInfo>();
    m_fractureActiveCellInfo = std::make_shared<RigActiveCellInfo>();

    m_matrixModelResults->setActiveCellInfo( m_activeCellInfo.get() );
    m_fractureModelResults->setActiveCellInfo( m_fractureActiveCellInfo.get() );

    m_unitsType = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData::~RigEclipseCaseData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RigEclipseCaseData::mainGrid()
{
    return m_mainGrid.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RigEclipseCaseData::mainGrid() const
{
    return m_mainGrid.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setMainGrid( std::shared_ptr<RigMainGrid> mainGrid )
{
    m_mainGrid = mainGrid;

    m_matrixModelResults->setMainGrid( m_mainGrid.get() );
    m_fractureModelResults->setMainGrid( m_mainGrid.get() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGridBase*> RigEclipseCaseData::allGrids()
{
    if ( !m_mainGrid ) return {};

    std::vector<RigGridBase*> grids;
    for ( size_t i = 0; i < m_mainGrid->gridCount(); i++ )
    {
        grids.push_back( m_mainGrid->gridByIndex( i ) );
    }

    return grids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigGridBase*> RigEclipseCaseData::allGrids() const
{
    if ( !m_mainGrid ) return {};

    std::vector<const RigGridBase*> grids;
    for ( size_t i = 0; i < m_mainGrid->gridCount(); i++ )
    {
        grids.push_back( m_mainGrid->gridByIndex( i ) );
    }

    return grids;
}

//--------------------------------------------------------------------------------------------------
/// Get grid by index. The main grid has index 0, so the first lgr has index 1
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigEclipseCaseData::grid( size_t index ) const
{
    CVF_ASSERT( m_mainGrid != nullptr );
    return m_mainGrid->gridByIndex( index );
}

//--------------------------------------------------------------------------------------------------
/// Get grid by index. The main grid has index 0, so the first lgr has index 1
//--------------------------------------------------------------------------------------------------
RigGridBase* RigEclipseCaseData::grid( size_t index )
{
    CVF_ASSERT( m_mainGrid != nullptr );
    return m_mainGrid->gridByIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGridBase* RigEclipseCaseData::grid( const QString& gridName ) const
{
    if ( !m_mainGrid )
    {
        return nullptr;
    }

    if ( gridName.isEmpty() )
    {
        return m_mainGrid.get();
    }

    for ( size_t i = 0; i < m_mainGrid->gridCount(); i++ )
    {
        const RigGridBase* grid = m_mainGrid->gridByIndex( i );
        if ( QString::fromStdString( grid->gridName() ) == gridName ) return grid;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseCaseData::gridCount() const
{
    CVF_ASSERT( m_mainGrid != nullptr );
    return m_mainGrid->gridCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeWellCellsPrGrid()
{
    // If we have computed this already, return
    if ( !m_wellCellsInGrid.empty() ) return;

    auto grids = allGrids();

    //  Allocate and initialize the arrays
    m_wellCellsInGrid.resize( grids.size() );
    m_gridCellToResultWellIndex.resize( grids.size() );

    for ( size_t gIdx = 0; gIdx < grids.size(); ++gIdx )
    {
        if ( m_wellCellsInGrid[gIdx].isNull() || m_wellCellsInGrid[gIdx]->size() != grids[gIdx]->cellCount() )
        {
            m_wellCellsInGrid[gIdx] = new cvf::UByteArray;
            m_wellCellsInGrid[gIdx]->resize( grids[gIdx]->cellCount() );

            m_gridCellToResultWellIndex[gIdx] = new cvf::UIntArray;
            m_gridCellToResultWellIndex[gIdx]->resize( grids[gIdx]->cellCount() );
        }
        m_wellCellsInGrid[gIdx]->setAll( false );
        m_gridCellToResultWellIndex[gIdx]->setAll( cvf::UNDEFINED_UINT );
    }

    // Fill arrays with data
    for ( size_t wIdx = 0; wIdx < m_simWellData.size(); ++wIdx )
    {
        for ( const RigWellResultFrame& wellCells : m_simWellData[wIdx]->m_wellCellsTimeSteps )
        {
            // Well result branches
            for ( const auto& wellSegment : wellCells.wellResultBranches() )
            {
                for ( const auto& resultPoint : wellSegment.branchResultPoints() )
                {
                    size_t gridIndex     = resultPoint.gridIndex();
                    size_t gridCellIndex = resultPoint.cellIndex();
                    if ( gridIndex < m_wellCellsInGrid.size() && gridCellIndex < m_wellCellsInGrid[gridIndex]->size() )
                    {
                        // NOTE : We do not check if the grid cell is active as we do for well head.
                        // If we add test for active cell, thorough testing and verification of the new behavior must
                        // be addressed
                        m_wellCellsInGrid[gridIndex]->set( gridCellIndex, true );
                        m_gridCellToResultWellIndex[gridIndex]->set( gridCellIndex, static_cast<cvf::uint>( wIdx ) );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setSimWellData( const cvf::Collection<RigSimWellData>& data )
{
    m_simWellData = data;
    m_wellCellsInGrid.clear();
    m_gridCellToResultWellIndex.clear();

    computeWellCellsPrGrid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RigEclipseCaseData::findSortedWellNames() const
{
    std::set<QString> sortedWellNames;

    const cvf::Collection<RigSimWellData>& simWellData = wellResults();

    for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
    {
        sortedWellNames.insert( simWellData[wIdx]->m_wellName );
    }

    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigSimWellData* RigEclipseCaseData::findSimWellData( QString wellName ) const
{
    for ( size_t wIdx = 0; wIdx < m_simWellData.size(); ++wIdx )
    {
        if ( m_simWellData[wIdx]->m_wellName == wellName )
        {
            return m_simWellData[wIdx].p();
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::UByteArray* RigEclipseCaseData::wellCellsInGrid( size_t gridIndex )
{
    computeWellCellsPrGrid();
    CVF_ASSERT( gridIndex < m_wellCellsInGrid.size() );

    return m_wellCellsInGrid[gridIndex].p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::UIntArray* RigEclipseCaseData::gridCellToResultWellIndex( size_t gridIndex )
{
    computeWellCellsPrGrid();
    CVF_ASSERT( gridIndex < m_gridCellToResultWellIndex.size() );

    return m_gridCellToResultWellIndex[gridIndex].p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigEclipseCaseData::cellFromWellResultCell( const RigWellResultPoint& wellResultPoint ) const
{
    CVF_ASSERT( wellResultPoint.isCell() );

    size_t gridIndex     = wellResultPoint.gridIndex();
    size_t gridCellIndex = wellResultPoint.cellIndex();

    return grid( gridIndex )->cell( gridCellIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigEclipseCaseData::findSharedSourceFace( cvf::StructGridInterface::FaceType& sharedSourceFace,
                                               const RigWellResultPoint&           sourceWellCellResult,
                                               const RigWellResultPoint&           otherWellCellResult ) const
{
    size_t gridIndex     = sourceWellCellResult.gridIndex();
    size_t gridCellIndex = sourceWellCellResult.cellIndex();

    size_t otherGridIndex     = otherWellCellResult.gridIndex();
    size_t otherGridCellIndex = otherWellCellResult.cellIndex();

    if ( gridIndex != otherGridIndex ) return false;

    const RigGridBase* grid = this->grid( gridIndex );
    size_t             i, j, k;
    grid->ijkFromCellIndex( gridCellIndex, &i, &j, &k );

    for ( cvf::StructGridInterface::FaceType sourceFace : cvf::StructGridInterface::validFaceTypes() )
    {
        size_t ni, nj, nk;
        RigGridBase::neighborIJKAtCellFace( i, j, k, sourceFace, &ni, &nj, &nk );

        if ( grid->isCellValid( ni, nj, nk ) )
        {
            size_t neighborCellIndex = grid->cellIndexFromIJK( ni, nj, nk );

            if ( neighborCellIndex == otherGridCellIndex )
            {
                sharedSourceFace = sourceFace;
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Helper class used to find min/max range for valid and active cells
//--------------------------------------------------------------------------------------------------
class CellRangeBB
{
public:
    CellRangeBB()
        : m_min( cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T )
        , m_max( 0, 0, 0 )
    {
    }

    void add( size_t i, size_t j, size_t k )
    {
        if ( i < m_min.x() ) m_min.x() = i;
        if ( j < m_min.y() ) m_min.y() = j;
        if ( k < m_min.z() ) m_min.z() = k;

        if ( i > m_max.x() ) m_max.x() = i;
        if ( j > m_max.y() ) m_max.y() = j;
        if ( k > m_max.z() ) m_max.z() = k;
    }

public:
    caf::VecIjk0 m_min;
    caf::VecIjk0 m_max;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellIJKBBox()
{
    if ( m_mainGrid != nullptr && m_activeCellInfo != nullptr && m_fractureActiveCellInfo != nullptr )
    {
        CellRangeBB matrixModelActiveBB;
        CellRangeBB fractureModelActiveBB;

        for ( size_t idx = 0; idx < m_mainGrid->cellCount(); idx++ )
        {
            size_t i, j, k;
            m_mainGrid->ijkFromCellIndex( idx, &i, &j, &k );

            if ( !m_mainGrid->isCellValid( i, j, k ) ) continue;

            if ( m_activeCellInfo->isActive( idx ) )
            {
                matrixModelActiveBB.add( i, j, k );
            }

            if ( m_fractureActiveCellInfo->isActive( idx ) )
            {
                fractureModelActiveBB.add( i, j, k );
            }
        }
        m_activeCellInfo->setIjkBoundingBox( matrixModelActiveBB.m_min, matrixModelActiveBB.m_max );
        m_fractureActiveCellInfo->setIjkBoundingBox( fractureModelActiveBB.m_min, fractureModelActiveBB.m_max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellBoundingBoxes( bool useOptimizedVersion )
{
    computeActiveCellIJKBBox();

    if ( useOptimizedVersion )
        computeActiveCellsGeometryBoundingBoxOptimized();
    else
        computeActiveCellsGeometryBoundingBoxSlow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigEclipseCaseData::simulationWellNames() const
{
    std::vector<QString> wellNames;
    for ( const auto& wellResult : wellResults() )
    {
        wellNames.push_back( wellResult->m_wellName );
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigEclipseCaseData::hasSimulationWell( const QString& simWellName ) const
{
    const auto wellNames = simulationWellNames();
    return std::find( wellNames.begin(), wellNames.end(), simWellName ) != wellNames.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RigWellPath*>
    RigEclipseCaseData::simulationWellBranches( const QString& simWellName, bool includeAllCellCenters, bool useAutoDetectionOfBranches ) const
{
    std::vector<const RigWellPath*> branches;

    if ( simWellName.isEmpty() || simWellName.toUpper() == "NONE" )
    {
        return branches;
    }

    const RigSimWellData* simWellData = findSimWellData( simWellName );
    if ( !simWellData ) return branches;

    std::tuple<QString, bool, bool> simWellSeachItem = std::make_tuple( simWellName, includeAllCellCenters, useAutoDetectionOfBranches );

    if ( m_simWellBranchCache.find( simWellSeachItem ) == m_simWellBranchCache.end() )
    {
        const auto simWellBranches = RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineForTimeStep( this,
                                                                                                                    simWellData,
                                                                                                                    -1,
                                                                                                                    useAutoDetectionOfBranches,
                                                                                                                    includeAllCellCenters );

        m_simWellBranchCache.insert( std::make_pair( simWellSeachItem, cvf::Collection<RigWellPath>() ) );

        for ( const auto& [coords, wellCells] : simWellBranches )
        {
            auto wellMdCalculator = RigSimulationWellCoordsAndMD( coords );

            cvf::ref<RigWellPath> newWellPath = new RigWellPath( wellMdCalculator.wellPathPoints(), wellMdCalculator.measuredDepths() );

            m_simWellBranchCache[simWellSeachItem].push_back( newWellPath.p() );
        }
    }

    for ( const auto& branch : m_simWellBranchCache[simWellSeachItem] )
    {
        branches.push_back( branch.p() );
    }

    return branches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setVirtualPerforationTransmissibilities(
    std::shared_ptr<RigVirtualPerforationTransmissibilities> virtualPerforationTransmissibilities )
{
    m_virtualPerforationTransmissibilities = virtualPerforationTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigVirtualPerforationTransmissibilities* RigEclipseCaseData::virtualPerforationTransmissibilities() const
{
    return m_virtualPerforationTransmissibilities.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigVirtualPerforationTransmissibilities> RigEclipseCaseData::virtualPerforationTransmissibilitiesShared()
{
    return m_virtualPerforationTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<const RigVirtualPerforationTransmissibilities> RigEclipseCaseData::virtualPerforationTransmissibilitiesShared() const
{
    return m_virtualPerforationTransmissibilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::ensureDeckIsParsedForEquilData( const QString& dataDeckFile, const QString& includeFileAbsolutePathPrefix )
{
    if ( !m_hasParsedDeckForEquilData )
    {
        RifReaderEclipseOutput::importEquilData( dataDeckFile, includeFileAbsolutePathPrefix, this );

        m_hasParsedDeckForEquilData = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEquil> RigEclipseCaseData::equilData() const
{
    return m_equil;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setEquilData( const std::vector<RigEquil>& equilObjects )
{
    m_equil = equilObjects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RigEclipseCaseData::activeCellInfo( RiaDefines::PorosityModelType porosityModel )
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_activeCellInfo.get();
    }

    return m_fractureActiveCellInfo.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigEclipseCaseData::activeCellInfo( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_activeCellInfo.get();
    }

    return m_fractureActiveCellInfo.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigActiveCellInfo> RigEclipseCaseData::activeCellInfoShared( RiaDefines::PorosityModelType porosityModel )
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_activeCellInfo;
    }

    return m_fractureActiveCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<const RigActiveCellInfo> RigEclipseCaseData::activeCellInfoShared( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_activeCellInfo;
    }

    return m_fractureActiveCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setActiveCellInfo( RiaDefines::PorosityModelType porosityModel, std::shared_ptr<RigActiveCellInfo> activeCellInfo )
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        m_activeCellInfo = activeCellInfo;
        m_matrixModelResults->setActiveCellInfo( m_activeCellInfo.get() );
    }
    else
    {
        m_fractureActiveCellInfo = activeCellInfo;
        m_fractureModelResults->setActiveCellInfo( m_fractureActiveCellInfo.get() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigEclipseCaseData::hasFractureResults() const
{
    return activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL ) &&
           activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL )->reservoirActiveCellCount() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellsGeometryBoundingBoxSlow()
{
    if ( !m_activeCellInfo || !m_fractureActiveCellInfo )
    {
        return;
    }

    if ( !m_mainGrid )
    {
        cvf::BoundingBox bb;
        m_activeCellInfo->setGeometryBoundingBox( bb );
        m_fractureActiveCellInfo->setGeometryBoundingBox( bb );
        return;
    }

    RigActiveCellInfo* activeInfos[2];
    activeInfos[0] = m_fractureActiveCellInfo.get();
    activeInfos[1] = m_activeCellInfo.get(); // Last, to make this bb.min become display offset

    cvf::BoundingBox bb;
    for ( int acIdx = 0; acIdx < 2; ++acIdx )
    {
        bb.reset();
        if ( m_mainGrid->nodes().empty() )
        {
            bb.add( cvf::Vec3d::ZERO );
        }
        else
        {
            for ( size_t i = 0; i < m_mainGrid->cellCount(); i++ )
            {
                if ( activeInfos[acIdx]->isActive( i ) )
                {
                    std::array<cvf::Vec3d, 8> hexCorners = m_mainGrid->cellCornerVertices( i );
                    for ( const auto& corner : hexCorners )
                    {
                        bb.add( corner );
                    }
                }
            }
        }

        activeInfos[acIdx]->setGeometryBoundingBox( bb );
    }

    // This design choice is unfortunate, as the bounding box of active cells can be computed in different ways.
    // Must keep the code to make sure existing projects display 3D model at the same location in the scene.
    m_mainGrid->setDisplayModelOffset( bb.min() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::computeActiveCellsGeometryBoundingBoxOptimized()
{
    if ( !m_activeCellInfo || !m_fractureActiveCellInfo )
    {
        return;
    }

    if ( !m_mainGrid )
    {
        cvf::BoundingBox bb;
        m_activeCellInfo->setGeometryBoundingBox( bb );
        m_fractureActiveCellInfo->setGeometryBoundingBox( bb );
        return;
    }

    RigActiveCellInfo* activeInfos[2];
    activeInfos[0] = m_fractureActiveCellInfo.get();
    activeInfos[1] = m_activeCellInfo.get();

    cvf::BoundingBox bb;
    for ( int acIdx = 0; acIdx < 2; ++acIdx )
    {
        bb.reset();
        if ( m_mainGrid->nodes().empty() )
        {
            bb.add( cvf::Vec3d::ZERO );
        }
        else
        {
            // Use the top and bottom layer of active cells to compute the bounding box

            auto [minBB, maxBB] = activeInfos[acIdx]->ijkBoundingBox();

            for ( auto k : { minBB.z(), maxBB.z() } )
            {
                for ( size_t i = minBB.x(); i <= maxBB.x(); i++ )
                {
                    for ( size_t j = minBB.y(); j <= maxBB.y(); j++ )
                    {
                        if ( m_mainGrid->isCellValid( i, j, k ) )
                        {
                            size_t cellIndex = m_mainGrid->cellIndexFromIJK( i, j, k );

                            std::array<cvf::Vec3d, 8> hexCorners = m_mainGrid->cellCornerVertices( cellIndex );
                            for ( const auto& corner : hexCorners )
                            {
                                bb.add( corner );
                            }
                        }
                    }
                }
            }
        }

        if ( m_mainGrid->totalTemporaryGridCellCount() > 0 )
        {
            // When we have LGRs for radial grids, we must loop over all local grids. The local grids can contain coordinates not present
            // in the main grid

            for ( size_t i = 0; i < m_mainGrid->gridCount(); i++ )
            {
                auto localGrid = m_mainGrid->gridByIndex( i );
                for ( size_t localCellIndex = 0; localCellIndex < localGrid->cellCount(); localCellIndex++ )
                {
                    size_t globalCellIndex = localGrid->reservoirCellIndex( localCellIndex );

                    if ( globalCellIndex < activeInfos[acIdx]->reservoirCellCount() && activeInfos[acIdx]->isActive( globalCellIndex ) )
                    {
                        std::array<cvf::Vec3d, 8> hexCorners = localGrid->cellCornerVertices( localCellIndex );
                        for ( const auto& corner : hexCorners )
                        {
                            bb.add( corner );
                        }
                    }
                }
            }
        }

        activeInfos[acIdx]->setGeometryBoundingBox( bb );
    }

    auto bbMainGrid = m_mainGrid->boundingBox();

    // Use center of bounding box as display offset. This point will be stable and independent of the active cell bounding box.
    m_mainGrid->setDisplayModelOffset( bbMainGrid.center() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setActiveFormationNames( RigFormationNames* activeFormationNames )
{
    m_matrixModelResults->setActiveFormationNames( activeFormationNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFormationNames* RigEclipseCaseData::activeFormationNames() const
{
    return m_matrixModelResults->activeFormationNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<QString> RigEclipseCaseData::formationNames() const
{
    if ( activeFormationNames() )
    {
        return activeFormationNames()->formationNames();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigAllanDiagramData* RigEclipseCaseData::allanDiagramData()
{
    return m_matrixModelResults->allanDiagramData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RigCaseCellResultsData> RigEclipseCaseData::results( RiaDefines::PorosityModelType porosityModel )
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_matrixModelResults;
    }

    return m_fractureModelResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<const RigCaseCellResultsData> RigEclipseCaseData::results( RiaDefines::PorosityModelType porosityModel ) const
{
    if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_matrixModelResults;
    }

    return m_fractureModelResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigEclipseCaseData::resultValues( RiaDefines::PorosityModelType porosityModel,
                                                             RiaDefines::ResultCatType     type,
                                                             const QString&                resultName,
                                                             size_t                        timeStepIndex )
{
    std::shared_ptr<RigCaseCellResultsData> gridCellResults = results( porosityModel );

    const std::vector<double>* swatResults = nullptr;
    if ( gridCellResults->ensureKnownResultLoaded( RigEclipseResultAddress( type, resultName ) ) )
    {
        swatResults = &( gridCellResults->cellScalarResults( RigEclipseResultAddress( type, resultName ), timeStepIndex ) );
    }

    return swatResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RigEclipseCaseData::availablePhases() const
{
    return m_availablePhases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseCaseData::setAvailablePhases( const std::set<RiaDefines::PhaseType>& phases )
{
    m_availablePhases = phases;
}
