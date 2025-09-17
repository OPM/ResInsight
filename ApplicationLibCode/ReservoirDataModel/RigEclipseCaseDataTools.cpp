/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RigEclipseCaseDataTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigSimWellData.h"
#include "Well/RigSimulationWellCenterLineCalculator.h"

#include "cvfBoundingBox.h"
#include "cvfObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigEclipseCaseDataTools::firstProducer( RigEclipseCaseData* eclipseCaseData )
{
    if ( !eclipseCaseData ) return {};

    auto caseCellResultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !caseCellResultsData ) return {};

    auto timeStepCount = caseCellResultsData->timeStepDates().size();
    if ( timeStepCount == 0 ) return {};

    auto simWells = eclipseCaseData->wellResults();
    for ( const auto& well : simWells )
    {
        if ( well->wellProductionType( timeStepCount - 1 ) == RiaDefines::WellProductionType::PRODUCER )
        {
            return well->m_wellName;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigEclipseCaseDataTools::wellBoundingBoxInDomainCoords( RigEclipseCaseData*   eclipseCaseData,
                                                                         const RigSimWellData* simWellData,
                                                                         int                   timeStepIndex,
                                                                         bool                  isAutoDetectingBranches,
                                                                         bool                  isUsingCellCenterForPipe )
{
    if ( !eclipseCaseData || !simWellData ) return cvf::BoundingBox();

    auto simWellBranches = RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineForTimeStep( eclipseCaseData,
                                                                                                          simWellData,
                                                                                                          timeStepIndex,
                                                                                                          isAutoDetectingBranches,
                                                                                                          isUsingCellCenterForPipe );

    auto [coords, wellCells] = RigSimulationWellCenterLineCalculator::extractBranchData( simWellBranches );

    cvf::BoundingBox bb;
    for ( const auto& branchCoords : coords )
    {
        for ( const auto& coord : branchCoords )
        {
            bb.add( coord );
        }
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RigEclipseCaseDataTools::wellBoundingBoxIjk( RigEclipseCaseData*   eclipseCaseData,
                                                                                 const RigSimWellData* simWellData,
                                                                                 int                   timeStepIndex,
                                                                                 bool                  isAutoDetectingBranches,
                                                                                 bool                  isUsingCellCenterForPipe )
{
    if ( !eclipseCaseData || !simWellData ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    cvf::BoundingBox domainBB =
        wellBoundingBoxInDomainCoords( eclipseCaseData, simWellData, timeStepIndex, isAutoDetectingBranches, isUsingCellCenterForPipe );

    if ( !domainBB.isValid() ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    auto mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    // Convert domain bounding box min/max to cell indices
    cvf::Vec3d minPoint = domainBB.min();
    cvf::Vec3d maxPoint = domainBB.max();

    size_t minCellIndex = mainGrid->findReservoirCellIndexFromPoint( minPoint );
    size_t maxCellIndex = mainGrid->findReservoirCellIndexFromPoint( maxPoint );

    if ( minCellIndex == cvf::UNDEFINED_SIZE_T || maxCellIndex == cvf::UNDEFINED_SIZE_T )
        return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    // Convert cell indices to IJK coordinates
    auto ijkFromCellIndex = []( RigMainGrid* mainGrid, size_t index )
    {
        if ( auto ijkOpt = mainGrid->ijkFromCellIndex( index ); ijkOpt.has_value() )
            return cvf::Vec3st( ijkOpt->i(), ijkOpt->j(), ijkOpt->k() );
        return cvf::Vec3st::UNDEFINED;
    };

    cvf::Vec3st minIjk = ijkFromCellIndex( mainGrid, minCellIndex );
    cvf::Vec3st maxIjk = ijkFromCellIndex( mainGrid, maxCellIndex );

    if ( minIjk.isUndefined() || maxIjk.isUndefined() ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    if ( minIjk.x() > maxIjk.x() || minIjk.y() > maxIjk.y() || minIjk.z() > maxIjk.z() )
    {
        std::swap( minIjk, maxIjk );
    }

    return { minIjk, maxIjk };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RigEclipseCaseDataTools::wellsBoundingBoxIjk( RigEclipseCaseData*                       eclipseCaseData,
                                                                                  const std::vector<const RigSimWellData*>& simWells,
                                                                                  int                                       timeStepIndex,
                                                                                  bool isAutoDetectingBranches,
                                                                                  bool isUsingCellCenterForPipe )
{
    if ( !eclipseCaseData || simWells.empty() ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    cvf::Vec3st globalMin = cvf::Vec3st::UNDEFINED;
    cvf::Vec3st globalMax = cvf::Vec3st::UNDEFINED;

    for ( const auto& well : simWells )
    {
        if ( !well ) continue;

        auto [minIjk, maxIjk] = wellBoundingBoxIjk( eclipseCaseData, well, timeStepIndex, isAutoDetectingBranches, isUsingCellCenterForPipe );

        if ( !minIjk.isUndefined() && !maxIjk.isUndefined() )
        {
            if ( globalMin.isUndefined() && globalMax.isUndefined() )
            {
                globalMin = minIjk;
                globalMax = maxIjk;
            }
            else
            {
                globalMin.x() = std::min( globalMin.x(), minIjk.x() );
                globalMin.y() = std::min( globalMin.y(), minIjk.y() );
                globalMin.z() = std::min( globalMin.z(), minIjk.z() );

                globalMax.x() = std::max( globalMax.x(), maxIjk.x() );
                globalMax.y() = std::max( globalMax.y(), maxIjk.y() );
                globalMax.z() = std::max( globalMax.z(), maxIjk.z() );
            }
        }
    }

    return { globalMin, globalMax };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RigEclipseCaseDataTools::expandBoundingBoxIjk( RigEclipseCaseData* eclipseCaseData,
                                                                                   const cvf::Vec3st&  minIjk,
                                                                                   const cvf::Vec3st&  maxIjk,
                                                                                   size_t              numPadding )
{
    if ( !eclipseCaseData || minIjk.isUndefined() || maxIjk.isUndefined() ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    auto mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return { cvf::Vec3st::UNDEFINED, cvf::Vec3st::UNDEFINED };

    // Calculate expanded bounds with padding, ensuring we stay within grid bounds
    size_t expandedMinI = ( minIjk.x() >= numPadding ) ? ( minIjk.x() - numPadding ) : 0;
    size_t expandedMinJ = ( minIjk.y() >= numPadding ) ? ( minIjk.y() - numPadding ) : 0;
    size_t expandedMinK = ( minIjk.z() >= numPadding ) ? ( minIjk.z() - numPadding ) : 0;

    size_t expandedMaxI = std::min( maxIjk.x() + numPadding, mainGrid->cellCountI() - 1 );
    size_t expandedMaxJ = std::min( maxIjk.y() + numPadding, mainGrid->cellCountJ() - 1 );
    size_t expandedMaxK = std::min( maxIjk.z() + numPadding, mainGrid->cellCountK() - 1 );

    return { cvf::Vec3st( expandedMinI, expandedMinJ, expandedMinK ), cvf::Vec3st( expandedMaxI, expandedMaxJ, expandedMaxK ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RigEclipseCaseDataTools::createVisibilityFromIjkBounds( RigEclipseCaseData* eclipseCaseData,
                                                                                  const cvf::Vec3st&  minIjk,
                                                                                  const cvf::Vec3st&  maxIjk )
{
    if ( !eclipseCaseData || minIjk.isUndefined() || maxIjk.isUndefined() ) return nullptr;

    auto mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return nullptr;

    // Create visibility array sized for the total number of reservoir cells
    size_t                    totalCellCount = mainGrid->cellCount();
    cvf::ref<cvf::UByteArray> visibility     = new cvf::UByteArray( totalCellCount );
    visibility->setAll( false ); // Initialize all cells as invisible

    // Mark cells within the IJK bounds as visible
    for ( size_t i = minIjk.x(); i <= maxIjk.x() && i < mainGrid->cellCountI(); ++i )
    {
        for ( size_t j = minIjk.y(); j <= maxIjk.y() && j < mainGrid->cellCountJ(); ++j )
        {
            for ( size_t k = minIjk.z(); k <= maxIjk.z() && k < mainGrid->cellCountK(); ++k )
            {
                size_t cellIndex = mainGrid->cellIndexFromIJK( i, j, k );
                if ( cellIndex < totalCellCount )
                {
                    visibility->set( cellIndex, true );
                }
            }
        }
    }

    return visibility;
}
