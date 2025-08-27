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

#include "RifReaderEclipseWell.h"

#include "RiaEclipseUnitTools.h"

#include "RifEclipseRestartDataAccess.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "cafProgressInfo.h"

#include "cvfTrace.h"

#include "ert/ecl_well/well_conn.h"
#include "ert/ecl_well/well_info.h"
#include "ert/ecl_well/well_segment.h"
#include "ert/ecl_well/well_segment_collection.h"
#include "ert/ecl_well/well_state.h"

#include <array>
#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Helper struct to store info on how a well-to-grid connection contributes to the position of
/// well segments without any connections.
//--------------------------------------------------------------------------------------------------
struct SegmentPositionContribution
{
    SegmentPositionContribution( int        connectionSegmentId,
                                 cvf::Vec3d connectionPosition,
                                 double     lengthFromConnection,
                                 bool       isInsolating,
                                 int        segmentIdUnder,
                                 int        segmentIdAbove,
                                 bool       isFromAbove )
        : m_connectionSegmentId( connectionSegmentId )
        , m_lengthFromConnection( lengthFromConnection )
        , m_isInsolating( isInsolating )
        , m_connectionPosition( connectionPosition )
        , m_segmentIdUnder( segmentIdUnder )
        , m_segmentIdAbove( segmentIdAbove )
        , m_isFromAbove( isFromAbove )
    {
    }

    int        m_connectionSegmentId;
    double     m_lengthFromConnection;
    bool       m_isInsolating;
    cvf::Vec3d m_connectionPosition;
    int        m_segmentIdUnder;
    int        m_segmentIdAbove;
    bool       m_isFromAbove;
};

size_t RifReaderEclipseWell::localGridCellIndexFromErtConnection( const RigGridBase*    grid,
                                                                  const well_conn_type* ert_connection,
                                                                  const char*           wellNameForErrorMsgs )
{
    CVF_ASSERT( ert_connection );
    CVF_ASSERT( grid );

    int cellI = well_conn_get_i( ert_connection );
    int cellJ = well_conn_get_j( ert_connection );
    int cellK = well_conn_get_k( ert_connection );

    // If a well is defined in fracture region, the K-value is from (cellCountK - 1) -> cellCountK*2 - 1
    // Adjust K so index is always in valid grid region
    if ( cellK >= static_cast<int>( grid->cellCountK() ) )
    {
        cellK -= static_cast<int>( grid->cellCountK() );
    }

    // See description for keyword ICON at page 54/55 of Rile Formats Reference Manual 2010.2
    /*
    Integer completion data array
    ICON(NICONZ,NCWMAX,NWELLS) with dimensions
    defined by INTEHEAD. The following items are required for each completion in each well:
    Item 1 - Well connection index ICON(1,IC,IWELL) = IC (set to -IC if connection is not in current LGR)
    Item 2 - I-coordinate (<= 0 if not in this LGR)
    Item 3 - J-coordinate (<= 0 if not in this LGR)
    Item 4 - K-coordinate (<= 0 if not in this LGR)
    Item 6 - Connection status > 0 open, <= 0 shut
    Item 14 - Penetration direction (1=x, 2=y, 3=z, 4=fractured in x-direction, 5=fractured in y-direction)
    If undefined or zero, assume Z
    Item 15 - Segment number containing connection (for multi-segment wells, =0 for ordinary wells)
    Undefined items in this array may be set to zero.
    */

    // The K value might also be -1. It is not yet known why, or what it is supposed to mean,
    // but for now we will interpret as 0.
    // TODO: Ask Joakim Haave regarding this.
    if ( cellK < 0 )
    {
        // cvf::Trace::show("Well Connection for grid " + cvf::String(grid->gridName()) + "\n - Detected negative K
        // value (K=" + cvf::String(cellK) + ") for well : " + cvf::String(wellName) + " K clamped to 0");

        cellK = 0;
    }

    // Introduced based on discussion with H�kon H�gst�l 08.09.2016
    if ( cellK >= static_cast<int>( grid->cellCountK() ) )
    {
        int maxCellK = static_cast<int>( grid->cellCountK() );
        if ( wellNameForErrorMsgs )
        {
            cvf::Trace::show( "Well Connection for grid " + cvf::String( grid->gridName() ) +
                              "\n - Ignored connection with invalid K value (K=" + cvf::String( cellK ) +
                              ", max K = " + cvf::String( maxCellK ) + ") for well : " + cvf::String( wellNameForErrorMsgs ) );
        }
        return cvf::UNDEFINED_SIZE_T;
    }

    if ( ( cellI < 0 ) || ( cellJ < 0 ) )
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    return grid->cellIndexFromIJK( cellI, cellJ, cellK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RifReaderEclipseWell::createWellResultPoint( const RigEclipseCaseData* eCaseData,
                                                                const RigGridBase*        grid,
                                                                const well_conn_type*     ert_connection,
                                                                const well_segment_type*  segment,
                                                                const char*               wellName )
{
    CVF_ASSERT( ert_connection );
    CVF_ASSERT( grid );

    size_t gridCellIndex = localGridCellIndexFromErtConnection( grid, ert_connection, wellName );

    bool   isCellOpen       = well_conn_open( ert_connection );
    double volumeRate       = well_conn_get_volume_rate( ert_connection );
    double oilRate          = well_conn_get_oil_rate( ert_connection );
    double gasRate          = well_conn_get_gas_rate( ert_connection );
    double waterRate        = well_conn_get_water_rate( ert_connection );
    double connectionFactor = well_conn_get_connection_factor( ert_connection );

    RigWellResultPoint resultPoint;

    if ( ( grid->cellCount() == 0 ) || ( gridCellIndex > grid->cellCount() - 1 ) )
    {
        return resultPoint;
    }

    const RigCell& c = grid->cell( gridCellIndex );
    if ( c.isInvalid() )
    {
        return resultPoint;
    }

    if ( gridCellIndex != cvf::UNDEFINED_SIZE_T )
    {
        int branchId = -1, segmentId = -1, outletBranchId = -1, outletSegmentId = -1;

        if ( segment )
        {
            branchId  = well_segment_get_branch_id( segment );
            segmentId = well_segment_get_id( segment );

            auto outletSegment = well_segment_get_outlet( segment );
            if ( outletSegment )
            {
                outletBranchId  = well_segment_get_branch_id( outletSegment );
                outletSegmentId = well_segment_get_id( outletSegment );
            }
        }

        resultPoint.setGridIndex( grid->gridIndex() );
        resultPoint.setGridCellIndex( gridCellIndex );

        resultPoint.setIsOpen( isCellOpen );

        resultPoint.setSegmentData( branchId, segmentId );
        resultPoint.setOutletSegmentData( outletBranchId, outletSegmentId );

        const double adjustedGasRate = RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( eCaseData->unitsType(), gasRate );
        resultPoint.setFlowData( volumeRate, oilRate, adjustedGasRate, waterRate );

        resultPoint.setConnectionFactor( connectionFactor );

        if ( auto ijk = grid->ijkFromCellIndex( gridCellIndex ) )
        {
            resultPoint.setIjk( ijk->toOneBased() );
        }
    }

    return resultPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RifReaderEclipseWell::createWellResultPoint( const RigEclipseCaseData* eCaseData,
                                                                const RigGridBase*        grid,
                                                                const well_conn_type*     ert_connection,
                                                                const char*               wellName )
{
    return createWellResultPoint( eCaseData, grid, ert_connection, nullptr, wellName );
}

//--------------------------------------------------------------------------------------------------
/// Inverse distance interpolation of the supplied points and distance weights for
/// the contributing points which are closest above, and closest below
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RifReaderEclipseWell::interpolate3DPosition( const std::vector<SegmentPositionContribution>& positions )
{
    std::vector<SegmentPositionContribution> filteredPositions;
    filteredPositions.reserve( positions.size() );

    double                                   minDistFromContribAbove = HUGE_VAL;
    double                                   minDistFromContribBelow = HUGE_VAL;
    std::vector<SegmentPositionContribution> contrFromAbove;
    std::vector<SegmentPositionContribution> contrFromBelow;

    for ( size_t i = 0; i < positions.size(); i++ )
    {
        if ( positions[i].m_connectionPosition != cvf::Vec3d::UNDEFINED )
        {
            if ( positions[i].m_isFromAbove && positions[i].m_lengthFromConnection < minDistFromContribAbove )
            {
                if ( !contrFromAbove.empty() )
                    contrFromAbove[0] = positions[i];
                else
                    contrFromAbove.push_back( positions[i] );

                minDistFromContribAbove = positions[i].m_lengthFromConnection;
            }

            if ( !positions[i].m_isFromAbove && positions[i].m_lengthFromConnection < minDistFromContribBelow )
            {
                if ( !contrFromBelow.empty() )
                    contrFromBelow[0] = positions[i];
                else
                    contrFromBelow.push_back( positions[i] );

                minDistFromContribBelow = positions[i].m_lengthFromConnection;
            }
        }
    }

    filteredPositions = contrFromAbove;
    filteredPositions.insert( filteredPositions.end(), contrFromBelow.begin(), contrFromBelow.end() );

    std::vector<double> nominators( filteredPositions.size(), 0.0 );

    double     denominator       = 0.0;
    cvf::Vec3d interpolatedValue = cvf::Vec3d::ZERO;

    for ( size_t i = 0; i < filteredPositions.size(); i++ )
    {
#if 0 // Pure average test
        nominators[i] = 1.0;
#else
        double distance = filteredPositions[i].m_lengthFromConnection;

        if ( distance < 1e-6 )
        {
            return filteredPositions[i].m_connectionPosition;
        }
        else if ( distance < 1.0 )
        {
            // distance = 1.0;
        }

        distance      = 1.0 / distance;
        nominators[i] = distance;
        denominator += distance;

#endif
    }
#if 0 // Pure average test
    denominator = positions.size(); // Pure average test
#endif
    for ( size_t i = 0; i < filteredPositions.size(); i++ )
    {
        interpolatedValue += ( nominators[i] / denominator ) * filteredPositions[i].m_connectionPosition;
    }

    return interpolatedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseWell::propagatePosContribDownwards( std::map<int, std::vector<SegmentPositionContribution>>& segmentIdToPositionContrib,
                                                         const well_segment_collection_type*      allErtSegments,
                                                         int                                      ertSegmentId,
                                                         std::vector<SegmentPositionContribution> posContrib )
{
    std::map<int, std::vector<SegmentPositionContribution>>::iterator posContribIt;
    posContribIt = segmentIdToPositionContrib.find( ertSegmentId );

    if ( posContribIt != segmentIdToPositionContrib.end() )
    {
        // Create a set of the segments below this, that has to be followed.

        std::set<int> segmentIdsBelow;
        for ( size_t i = 0; i < posContribIt->second.size(); ++i )
        {
            segmentIdsBelow.insert( posContribIt->second[i].m_segmentIdUnder );
        }

        // Get the segment length to add to the contributions

        well_segment_type* segment      = well_segment_collection_get( allErtSegments, posContribIt->first );
        double             sementLength = well_segment_get_length( segment );

        // If we do not have the contribution represented, add it, and accumulate the length
        // If it is already present, do not touch
        for ( size_t i = 0; i < posContrib.size(); ++i )
        {
            bool foundContribution = false;
            for ( size_t j = 0; j < posContribIt->second.size(); ++j )
            {
                if ( posContribIt->second[j].m_connectionSegmentId == posContrib[i].m_connectionSegmentId )
                {
                    foundContribution = true;
                    break;
                }
            }

            if ( !foundContribution )
            {
                posContrib[i].m_lengthFromConnection += sementLength;
                posContrib[i].m_isFromAbove = true;
                posContribIt->second.push_back( posContrib[i] );
            }
            posContrib[i].m_segmentIdAbove = ertSegmentId;
        }

        for ( std::set<int>::iterator it = segmentIdsBelow.begin(); it != segmentIdsBelow.end(); ++it )
        {
            propagatePosContribDownwards( segmentIdToPositionContrib, allErtSegments, ( *it ), posContrib );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper class to determine whether a well connection is present in a sub cell
//  for a specific well. Connections must be tested from innermost lgr to outermost since
//  it accumulates the outer cells having subcell connections as it goes.
//--------------------------------------------------------------------------------------------------
class WellResultPointHasSubCellConnectionCalculator
{
public:
    explicit WellResultPointHasSubCellConnectionCalculator( const RigMainGrid* mainGrid, well_state_type* ert_well_state )
        : m_mainGrid( mainGrid )
    {
        int lastGridNr = static_cast<int>( m_mainGrid->gridCountOnFile() ) - 1;

        for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
        {
            const well_conn_type* ert_wellhead = well_state_iget_wellhead( ert_well_state, static_cast<int>( gridNr ) );
            if ( ert_wellhead )
            {
                size_t localGridCellidx =
                    RifReaderEclipseWell::localGridCellIndexFromErtConnection( m_mainGrid->gridByIndex( gridNr ), ert_wellhead, nullptr );
                insertTheParentCells( gridNr, localGridCellidx );
            }

            std::string                      gridname = gridNr == 0 ? ECL_GRID_GLOBAL_GRID : m_mainGrid->gridByIndex( gridNr )->gridName();
            const well_conn_collection_type* connections = well_state_get_grid_connections( ert_well_state, gridname.data() );

            if ( connections )
            {
                int connectionCount = well_conn_collection_get_size( connections );
                if ( connectionCount )
                {
                    for ( int connIdx = 0; connIdx < connectionCount; connIdx++ )
                    {
                        well_conn_type* ert_connection = well_conn_collection_iget( connections, connIdx );

                        size_t localGridCellidx = RifReaderEclipseWell::localGridCellIndexFromErtConnection( m_mainGrid->gridByIndex( gridNr ),
                                                                                                             ert_connection,
                                                                                                             nullptr );
                        insertTheParentCells( gridNr, localGridCellidx );
                    }
                }
            }
        }
    }

    bool hasSubCellConnection( const RigWellResultPoint& wellResultPoint )
    {
        if ( !wellResultPoint.isCell() ) return false;

        size_t gridIndex     = wellResultPoint.gridIndex();
        size_t gridCellIndex = wellResultPoint.cellIndex();

        size_t reservoirCellIdx = m_mainGrid->reservoirCellIndexByGridAndGridLocalCellIndex( gridIndex, gridCellIndex );

        return m_gridCellsWithSubCellWellConnections.count( reservoirCellIdx ) != 0;
    }

private:
    void insertTheParentCells( size_t gridIndex, size_t gridCellIndex )
    {
        if ( gridCellIndex == cvf::UNDEFINED_SIZE_T ) return;

        // Traverse parent gridcells, and add them to the map

        while ( gridIndex > 0 ) // is lgr
        {
            const RigCell& connectionCell = m_mainGrid->cellByGridAndGridLocalCellIdx( gridIndex, gridCellIndex );
            if ( connectionCell.isInvalid() ) break;

            RigGridBase* hostGrid = connectionCell.hostGrid();

            RigLocalGrid* lgrHost = static_cast<RigLocalGrid*>( hostGrid );
            gridIndex             = lgrHost->parentGrid()->gridIndex();
            gridCellIndex         = connectionCell.parentCellIndex();

            size_t parentReservoirCellIdx = m_mainGrid->reservoirCellIndexByGridAndGridLocalCellIndex( gridIndex, gridCellIndex );
            m_gridCellsWithSubCellWellConnections.insert( parentReservoirCellIdx );
        }
    }

    std::set<size_t>   m_gridCellsWithSubCellWellConnections;
    const RigMainGrid* m_mainGrid;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseWell::readWellCells( RifEclipseRestartDataAccess* restartDataAccess,
                                          RigEclipseCaseData*          eclipseCaseData,
                                          std::vector<QDateTime>       filteredTimeSteps,
                                          std::vector<std::string>     gridNames,
                                          bool                         importCompleteMswData )
{
    CVF_ASSERT( eclipseCaseData );

    if ( restartDataAccess == nullptr ) return;

    well_info_type* ert_well_info = well_info_alloc( gridNames );
    if ( !ert_well_info ) return;

    restartDataAccess->readWellData( ert_well_info, importCompleteMswData );

    std::vector<double>    daysSinceSimulationStart;
    std::vector<QDateTime> timeSteps;
    restartDataAccess->timeSteps( &timeSteps, &daysSinceSimulationStart );
    std::vector<int> reportNumbers = restartDataAccess->reportNumbers();

    bool sameCount = false;
    if ( timeSteps.size() == reportNumbers.size() )
    {
        sameCount = true;
    }

    std::vector<RigGridBase*> grids;
    eclipseCaseData->allGrids( &grids );

    cvf::Collection<RigSimWellData> wells;
    caf::ProgressInfo               progress( well_info_get_num_wells( ert_well_info ), "" );

    int wellIdx;
    for ( wellIdx = 0; wellIdx < well_info_get_num_wells( ert_well_info ); wellIdx++ )
    {
        const char* wellName = well_info_iget_well_name( ert_well_info, wellIdx );
        CVF_ASSERT( wellName );

        cvf::ref<RigSimWellData> simWellData = new RigSimWellData;
        simWellData->m_wellName              = wellName;

        well_ts_type* ert_well_time_series = well_info_get_ts( ert_well_info, wellName );
        int           timeStepCount        = well_ts_get_size( ert_well_time_series );

        simWellData->m_wellCellsTimeSteps.resize( timeStepCount );

        int timeIdx;
        for ( timeIdx = 0; timeIdx < timeStepCount; timeIdx++ )
        {
            well_state_type* ert_well_state = well_ts_iget_state( ert_well_time_series, timeIdx );

            RigWellResultFrame& wellResFrame = simWellData->m_wellCellsTimeSteps[timeIdx];

            // Build timestamp for well
            bool haveFoundTimeStamp = false;

            if ( sameCount )
            {
                int reportNr = well_state_get_report_nr( ert_well_state );

                for ( size_t i = 0; i < reportNumbers.size(); i++ )
                {
                    if ( reportNumbers[i] == reportNr )
                    {
                        wellResFrame.setTimestamp( timeSteps[i] );
                        haveFoundTimeStamp = true;
                    }
                }
            }

            if ( !haveFoundTimeStamp )
            {
                // This fallback will not work for timesteps before 1970.

                // Also see RifEclipseOutputFileAccess::timeStepsText for accessing time_t structures
                time_t stepTime = well_state_get_sim_time( ert_well_state );
                wellResFrame.setTimestamp( QDateTime::fromSecsSinceEpoch( stepTime, Qt::UTC ) );
            }

            // Production type
            well_type_enum ert_well_type = well_state_get_type( ert_well_state );
            if ( ert_well_type == ECL_WELL_PRODUCER )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::PRODUCER );
            }
            else if ( ert_well_type == ECL_WELL_WATER_INJECTOR )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::WATER_INJECTOR );
            }
            else if ( ert_well_type == ECL_WELL_GAS_INJECTOR )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::GAS_INJECTOR );
            }
            else if ( ert_well_type == ECL_WELL_OIL_INJECTOR )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::OIL_INJECTOR );
            }
            else
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE );
            }

            wellResFrame.setIsOpen( well_state_is_open( ert_well_state ) );

            if ( importCompleteMswData && well_state_is_MSW( ert_well_state ) )
            {
                simWellData->setMultiSegmentWell( true );

                // how do we handle LGR-s ?
                // 1. Create separate visual branches for each Grid, with its own wellhead
                // 2. Always use the connections to the grid with the highest number (innermost LGR).
                // 3. Handle both and switch between them according to visual settings of grid visualization
                // Will there ever exist connections to different grids for the same segment ?
                // We have currently selected 2.

                // Set the wellhead

                int lastGridNr = static_cast<int>( grids.size() ) - 1;
                for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                {
                    //  If several grids have a wellhead definition for this well, we use the last one.
                    // (Possibly the innermost LGR)

                    const well_conn_type* ert_wellhead = well_state_iget_wellhead( ert_well_state, static_cast<int>( gridNr ) );
                    if ( ert_wellhead )
                    {
                        auto wellHead = RifReaderEclipseWell::createWellResultPoint( eclipseCaseData, grids[gridNr], ert_wellhead, wellName );

                        // HACK: Ert returns open as "this is equally wrong as closed for well heads".
                        // Well heads are not open jfr mail communication with HHGS and JH Statoil 07.01.2016
                        wellHead.setIsOpen( false );
                        wellResFrame.setWellHead( wellHead );
                        break;
                    }
                }

                well_branch_collection_type*                            branches    = well_state_get_branches( ert_well_state );
                int                                                     branchCount = well_branch_collection_get_size( branches );
                std::map<int, std::vector<SegmentPositionContribution>> segmentIdToPositionContrib;
                std::vector<int>                                        upperSegmentIdsOfUnpositionedSegementGroup;

                // Create copy of well result branches for modification
                std::vector<RigWellResultBranch> wellResultBranches = wellResFrame.wellResultBranches();
                wellResultBranches.resize( branchCount );

                // For each branch, go from bottom segment upwards and transfer their connections to WellResultpoints.
                // If they have no connections, create a resultpoint representing their bottom position, which will
                // receive an actual position at a later stage.
                // I addition, distribute contributions for calculating segment bottom positions from bottom and up.

                for ( int bIdx = 0; bIdx < well_branch_collection_get_size( branches ); bIdx++ )
                {
                    RigWellResultBranch& wellResultBranch = wellResultBranches[bIdx];

                    const well_segment_type* segment = well_branch_collection_iget_start_segment( branches, bIdx );

                    int branchId = well_segment_get_branch_id( segment );
                    wellResultBranch.setErtBranchId( branchId );

                    // Data for segment position calculation
                    int        lastConnectionSegmentId     = -1;
                    cvf::Vec3d lastConnectionPos           = cvf::Vec3d::UNDEFINED;
                    cvf::Vec3d lastConnectionCellCorner    = cvf::Vec3d::UNDEFINED;
                    double     lastConnectionCellSize      = 0;
                    double     accLengthFromLastConnection = 0;
                    int        segmentIdBelow              = -1;
                    bool       segmentBelowHasConnections  = false;

                    while ( segment && branchId == well_segment_get_branch_id( segment ) )
                    {
                        // Loop backwards, making us select the connection in the innermost lgr as the truth
                        bool segmentHasConnections = false;

                        for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                        {
                            std::string gridName = ertGridName( eclipseCaseData, gridNr );

                            // If this segment has connections in any grid, transfer the innermost ones

                            if ( well_segment_has_grid_connections( segment, gridName.data() ) )
                            {
                                const well_conn_collection_type* connections     = well_segment_get_connections( segment, gridName.data() );
                                int                              connectionCount = well_conn_collection_get_size( connections );

                                // Loop backwards to put the deepest connections first in the array. (The segments are
                                // also traversed deep to shallow)
                                for ( int connIdx = connectionCount - 1; connIdx >= 0; connIdx-- )
                                {
                                    well_conn_type* ert_connection = well_conn_collection_iget( connections, connIdx );
                                    wellResultBranch.addBranchResultPoint( RifReaderEclipseWell::createWellResultPoint( eclipseCaseData,
                                                                                                                        grids[gridNr],
                                                                                                                        ert_connection,
                                                                                                                        segment,
                                                                                                                        wellName ) );
                                }

                                segmentHasConnections = true;

                                // Prepare data for segment position calculation

                                well_conn_type*    ert_connection = well_conn_collection_iget( connections, 0 );
                                RigWellResultPoint point =
                                    RifReaderEclipseWell::createWellResultPoint( eclipseCaseData, grids[gridNr], ert_connection, segment, wellName );
                                lastConnectionPos                  = grids[gridNr]->cell( point.cellIndex() ).center();
                                std::array<cvf::Vec3d, 8> cellVxes = grids[gridNr]->cellCornerVertices( point.cellIndex() );
                                lastConnectionCellCorner           = cellVxes[0];
                                lastConnectionCellSize             = ( lastConnectionPos - cellVxes[0] ).length();

                                lastConnectionSegmentId     = well_segment_get_id( segment );
                                accLengthFromLastConnection = well_segment_get_length( segment ) / ( connectionCount + 1 );
                                if ( !segmentBelowHasConnections ) upperSegmentIdsOfUnpositionedSegementGroup.push_back( segmentIdBelow );

                                break; // Stop looping over grids
                            }
                        }

                        // If the segment did not have connections at all, we need to create a resultpoint representing
                        // the bottom of the segment and store it as an unpositioned segment

                        if ( !segmentHasConnections )
                        {
                            RigWellResultPoint data;
                            data.setSegmentData( branchId, well_segment_get_id( segment ) );

                            wellResultBranch.addBranchResultPoint( data );

                            // Store data for segment position calculation
                            bool isAnInsolationContribution = accLengthFromLastConnection < lastConnectionCellSize;

                            segmentIdToPositionContrib[well_segment_get_id( segment )].push_back(
                                SegmentPositionContribution( lastConnectionSegmentId,
                                                             lastConnectionPos,
                                                             accLengthFromLastConnection,
                                                             isAnInsolationContribution,
                                                             segmentIdBelow,
                                                             -1,
                                                             false ) );
                            accLengthFromLastConnection += well_segment_get_length( segment );
                        }

                        segmentIdBelow             = well_segment_get_id( segment );
                        segmentBelowHasConnections = segmentHasConnections;

                        if ( well_segment_get_outlet_id( segment ) == -1 )
                        {
                            segment = nullptr;
                        }
                        else
                        {
                            segment = well_segment_get_outlet( segment );
                        }
                    }

                    // Add resultpoint representing the outlet segment (bottom), if not the branch ends at the wellhead.

                    const well_segment_type* outletSegment = segment;

                    if ( outletSegment )
                    {
                        bool outletSegmentHasConnections = false;

                        for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                        {
                            std::string gridName = ertGridName( eclipseCaseData, gridNr );

                            // If this segment has connections in any grid, use the deepest innermost one

                            if ( well_segment_has_grid_connections( outletSegment, gridName.data() ) )
                            {
                                const well_conn_collection_type* connections = well_segment_get_connections( outletSegment, gridName.data() );
                                int connectionCount = well_conn_collection_get_size( connections );

                                // Select the deepest connection
                                well_conn_type* ert_connection = well_conn_collection_iget( connections, connectionCount - 1 );

                                auto resultPoint = RifReaderEclipseWell::createWellResultPoint( eclipseCaseData,
                                                                                                grids[gridNr],
                                                                                                ert_connection,
                                                                                                outletSegment,
                                                                                                wellName );
                                // This result point is only supposed to be used to indicate connection to a parent well
                                // Clear all flow in this result point
                                resultPoint.clearAllFlow();

                                wellResultBranch.addBranchResultPoint( resultPoint );

                                outletSegmentHasConnections = true;
                                break; // Stop looping over grids
                            }
                        }

                        if ( !outletSegmentHasConnections )
                        {
                            // Store the result point

                            RigWellResultPoint data;
                            data.setSegmentData( well_segment_get_branch_id( outletSegment ), well_segment_get_id( outletSegment ) );
                            wellResultBranch.addBranchResultPoint( data );

                            // Store data for segment position calculation,
                            // and propagate it upwards until we meet a segment with connections

                            bool isAnInsolationContribution = accLengthFromLastConnection < lastConnectionCellSize;

                            cvf::Vec3d lastConnectionPosWOffset = lastConnectionPos;
                            if ( isAnInsolationContribution )
                                lastConnectionPosWOffset += 0.4 * ( lastConnectionCellCorner - lastConnectionPos );

                            segmentIdToPositionContrib[well_segment_get_id( outletSegment )].push_back(
                                SegmentPositionContribution( lastConnectionSegmentId,
                                                             lastConnectionPosWOffset,
                                                             accLengthFromLastConnection,
                                                             isAnInsolationContribution,
                                                             segmentIdBelow,
                                                             -1,
                                                             false ) );

                            /// Loop further to add this position contribution until a segment with connections is found

                            accLengthFromLastConnection += well_segment_get_length( outletSegment );
                            segmentIdBelow = well_segment_get_id( outletSegment );

                            const well_segment_type* aboveOutletSegment = nullptr;

                            if ( well_segment_get_outlet_id( outletSegment ) == -1 )
                            {
                                aboveOutletSegment = nullptr;
                            }
                            else
                            {
                                aboveOutletSegment = well_segment_get_outlet( outletSegment );
                            }

                            while ( aboveOutletSegment )
                            {
                                // Loop backwards, just because we do that the other places
                                bool segmentHasConnections = false;

                                for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                                {
                                    std::string gridName = ertGridName( eclipseCaseData, gridNr );

                                    // If this segment has connections in any grid, stop traversal

                                    if ( well_segment_has_grid_connections( aboveOutletSegment, gridName.data() ) )
                                    {
                                        segmentHasConnections = true;
                                        break;
                                    }
                                }

                                if ( !segmentHasConnections )
                                {
                                    segmentIdToPositionContrib[well_segment_get_id( aboveOutletSegment )].push_back(
                                        SegmentPositionContribution( lastConnectionSegmentId,
                                                                     lastConnectionPos,
                                                                     accLengthFromLastConnection,
                                                                     isAnInsolationContribution,
                                                                     segmentIdBelow,
                                                                     -1,
                                                                     false ) );
                                    accLengthFromLastConnection += well_segment_get_length( aboveOutletSegment );
                                }
                                else
                                {
                                    break; // We have found a segment with connections. We do not need to propagate
                                           // position contributions further
                                }

                                segmentIdBelow = well_segment_get_id( aboveOutletSegment );

                                if ( well_segment_get_outlet_id( aboveOutletSegment ) == -1 )
                                {
                                    aboveOutletSegment = nullptr;
                                }
                                else
                                {
                                    aboveOutletSegment = well_segment_get_outlet( aboveOutletSegment );
                                }
                            }
                        }
                    }
                    else
                    {
                        // Add wellhead as result point Nope. Not Yet, but it is a good idea.
                        // The centerline calculations would be a bit simpler, I think.
                    }

                    // Reverse the order of the result points in this branch, making the deepest come last
                    auto branchResultPoints = wellResultBranch.branchResultPoints();
                    std::reverse( branchResultPoints.begin(), branchResultPoints.end() );
                    wellResultBranch.setBranchResultPoints( branchResultPoints );
                } // End of the branch loop

                // Set modified copy back to frame
                wellResFrame.setWellResultBranches( wellResultBranches );

                // Propagate position contributions from connections above unpositioned segments downwards

                well_segment_collection_type* allErtSegments = well_state_get_segments( ert_well_state );

                bool isWellHead = true;
                for ( const auto& wellResultBranch : wellResFrame.wellResultBranches() )
                {
                    bool previousResultPointWasCell = isWellHead;

                    // Go downwards until we find a none-cell result point just after a cell result point
                    // When we do, start propagating

                    for ( size_t rpIdx = 0; rpIdx < wellResultBranch.branchResultPoints().size(); ++rpIdx )
                    {
                        const RigWellResultPoint resPoint = wellResultBranch.branchResultPoints()[rpIdx];
                        if ( resPoint.isCell() )
                        {
                            previousResultPointWasCell = true;
                        }
                        else
                        {
                            if ( previousResultPointWasCell )
                            {
                                RigWellResultPoint prevResPoint;
                                if ( isWellHead && rpIdx == 0 )
                                {
                                    prevResPoint = wellResFrame.wellHead();
                                }
                                else if ( rpIdx > 0 )
                                {
                                    prevResPoint = wellResultBranch.branchResultPoints()[rpIdx - 1];
                                }

                                if ( !prevResPoint.isCell() )
                                {
                                    // When importing only active cells, this situation can occur if the well head is a inactive cell.
                                    continue;
                                }

                                cvf::Vec3d lastConnectionPos = grids[prevResPoint.gridIndex()]->cell( prevResPoint.cellIndex() ).center();

                                SegmentPositionContribution
                                    posContrib( prevResPoint.segmentId(), lastConnectionPos, 0.0, false, -1, prevResPoint.segmentId(), true );

                                int ertSegmentId = resPoint.segmentId();

                                std::map<int, std::vector<SegmentPositionContribution>>::iterator posContribIt;
                                posContribIt = segmentIdToPositionContrib.find( ertSegmentId );
                                CVF_ASSERT( posContribIt != segmentIdToPositionContrib.end() );

                                std::vector<SegmentPositionContribution> posContributions = posContribIt->second;
                                for ( size_t i = 0; i < posContributions.size(); ++i )
                                {
                                    posContributions[i].m_segmentIdAbove = prevResPoint.segmentId();
                                }
                                posContributions.push_back( posContrib );

                                propagatePosContribDownwards( segmentIdToPositionContrib, allErtSegments, ertSegmentId, posContributions );
                            }

                            previousResultPointWasCell = false;
                        }
                    }

                    isWellHead = false;
                }

                // Calculate the bottom position of all the unpositioned segments
                // Then do the calculation based on the refined contributions

                std::map<int, std::vector<SegmentPositionContribution>>::iterator posContribIt = segmentIdToPositionContrib.begin();
                std::map<int, cvf::Vec3d>                                         bottomPositions;
                while ( posContribIt != segmentIdToPositionContrib.end() )
                {
                    bottomPositions[posContribIt->first] = interpolate3DPosition( posContribIt->second );
                    ++posContribIt;
                }

                // Copy content and distribute the positions to the result points stored in the wellResultBranch.branchResultPoints()
                // set updated copy back to frame

                std::vector<RigWellResultBranch> newWellResultBranches = wellResFrame.wellResultBranches();
                for ( auto& wellResultBranch : newWellResultBranches )
                {
                    RigWellResultBranch& newWellResultBranch = wellResultBranch;
                    for ( auto& resultPoint : newWellResultBranch.branchResultPoints() )
                    {
                        if ( !resultPoint.isCell() )
                        {
                            resultPoint.setBottomPosition( bottomPositions[resultPoint.segmentId()] );
                        }
                    }
                }
                wellResFrame.setWellResultBranches( newWellResultBranches );
            } // End of the MSW section
            else
            {
                // Code handling None-MSW Wells ... Normal wells that is.

                WellResultPointHasSubCellConnectionCalculator subCellConnCalc( eclipseCaseData->mainGrid(), ert_well_state );
                int                                           lastGridNr = static_cast<int>( grids.size() ) - 1;
                for ( int gridNr = 0; gridNr <= lastGridNr; ++gridNr )
                {
                    const well_conn_type* ert_wellhead = well_state_iget_wellhead( ert_well_state, static_cast<int>( gridNr ) );
                    if ( ert_wellhead )
                    {
                        RigWellResultPoint wellHeadRp =
                            RifReaderEclipseWell::createWellResultPoint( eclipseCaseData, grids[gridNr], ert_wellhead, wellName );
                        // HACK: Ert returns open as "this is equally wrong as closed for well heads".
                        // Well heads are not open jfr mail communication with HHGS and JH Statoil 07.01.2016
                        wellHeadRp.setIsOpen( false );

                        if ( !subCellConnCalc.hasSubCellConnection( wellHeadRp ) ) wellResFrame.setWellHead( wellHeadRp );
                    }

                    const well_conn_collection_type* connections =
                        well_state_get_grid_connections( ert_well_state, ertGridName( eclipseCaseData, gridNr ).data() );

                    // Import all well result cells for all connections
                    if ( connections )
                    {
                        int connectionCount = well_conn_collection_get_size( connections );
                        if ( connectionCount )
                        {
                            RigWellResultBranch wellResultBranch;
                            wellResultBranch.setErtBranchId( 0 ); // Normal wells have only one branch

                            std::vector<RigWellResultPoint> branchResultPoints = wellResultBranch.branchResultPoints();
                            const size_t                    existingCellCount  = branchResultPoints.size();
                            branchResultPoints.resize( existingCellCount + connectionCount );

                            for ( int connIdx = 0; connIdx < connectionCount; connIdx++ )
                            {
                                well_conn_type*    ert_connection = well_conn_collection_iget( connections, connIdx );
                                RigWellResultPoint wellRp =
                                    RifReaderEclipseWell::createWellResultPoint( eclipseCaseData, grids[gridNr], ert_connection, wellName );

                                if ( !subCellConnCalc.hasSubCellConnection( wellRp ) )
                                {
                                    branchResultPoints[existingCellCount + connIdx] = wellRp;
                                }
                            }
                            wellResultBranch.setBranchResultPoints( branchResultPoints );
                            wellResFrame.addWellResultBranch( wellResultBranch );
                        }
                    }
                }
            }
        }

        simWellData->computeMappingFromResultTimeIndicesToWellTimeIndices( filteredTimeSteps );

        wells.push_back( simWellData.p() );

        progress.incrementProgress();
    }

    well_info_free( ert_well_info );

    eclipseCaseData->setSimWellData( wells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifReaderEclipseWell::ertGridName( const RigEclipseCaseData* eCaseData, size_t gridNr )
{
    std::string gridName;
    if ( gridNr == 0 )
    {
        gridName = ECL_GRID_GLOBAL_GRID;
    }
    else
    {
        CVF_ASSERT( eCaseData );
        CVF_ASSERT( eCaseData->gridCount() > gridNr );
        gridName = eCaseData->grid( gridNr )->gridName();
    }

    return gridName;
}
