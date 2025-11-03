/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RigMswDataFormatter.h"

#include "RifTextDataTableFormatter.h"

#include "RigMswTableData.h"
#include "RigMswUnifiedData.h"

namespace
{
//--------------------------------------------------------------------------------------------------
/// Helper function to format WELSEGS segment data rows
//--------------------------------------------------------------------------------------------------
template <typename RowContainer>
void formatWelsegsRows( RifTextDataTableFormatter& formatter, const RowContainer& rows )
{
    for ( const auto& row : rows )
    {
        formatter.add( row.segmentNumber );
        formatter.add( row.segmentNumber );
        formatter.add( row.branchNumber );
        formatter.add( row.outletSegmentNumber );
        formatter.add( row.length );

        formatter.addOptionalValue( row.depth );
        formatter.addOptionalValue( row.diameter );
        formatter.addOptionalValue( row.roughness );

        if ( !row.description.empty() )
        {
            formatter.addOptionalComment( QString::fromStdString( row.description ) );
        }
        formatter.rowCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to format COMPSEGS data rows
//--------------------------------------------------------------------------------------------------
template <typename RowContainer>
void formatCompsegsRows( RifTextDataTableFormatter& formatter, const RowContainer& rows, bool isLgrData )
{
    for ( const auto& row : rows )
    {
        if ( isLgrData )
        {
            formatter.addStdString( row.gridName );
        }

        formatter.add( row.cellI );
        formatter.add( row.cellJ );
        formatter.add( row.cellK );
        formatter.add( row.branchNumber );
        formatter.add( row.startLength );
        formatter.add( row.endLength );

        formatter.addOptionalValue( row.direction );
        formatter.addOptionalValue( row.endRange );
        formatter.addOptionalValue( row.connectionDepth );

        formatter.rowCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to format WSEGVALV data rows
//--------------------------------------------------------------------------------------------------
template <typename RowContainer>
void formatWsegvalvRows( RifTextDataTableFormatter& formatter, const RowContainer& rows )
{
    for ( const auto& row : rows )
    {
        formatter.addStdString( row.wellName );
        formatter.add( row.segmentNumber );
        formatter.add( row.flowCoefficient );
        formatter.addOptionalValue( row.area );

        formatter.rowCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to format WSEGAICD data rows
//--------------------------------------------------------------------------------------------------
template <typename RowContainer>
void formatWsegaicdRows( RifTextDataTableFormatter& formatter, const RowContainer& rows )
{
    for ( const auto& row : rows )
    {
        formatter.addStdString( row.wellName );
        formatter.add( row.segmentNumber );
        formatter.add( row.flowCoefficient );
        formatter.addOptionalValue( row.area );
        formatter.addOptionalValue( row.oilViscosityParameter );
        formatter.addOptionalValue( row.waterViscosityParameter );
        formatter.addOptionalValue( row.gasViscosityParameter );
        formatter.addStdString( row.deviceType );

        formatter.rowCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to create COMPSEGS headers
//--------------------------------------------------------------------------------------------------
std::vector<RifTextDataTableColumn> createCompsegsHeader( bool isLgrData )
{
    if ( isLgrData )
    {
        return { RifTextDataTableColumn( "Grid" ),
                 RifTextDataTableColumn( "I" ),
                 RifTextDataTableColumn( "J" ),
                 RifTextDataTableColumn( "K" ),
                 RifTextDataTableColumn( "Branch no" ),
                 RifTextDataTableColumn( "Start Length" ),
                 RifTextDataTableColumn( "End Length" ),
                 RifTextDataTableColumn( "Dir Pen" ),
                 RifTextDataTableColumn( "End Range" ),
                 RifTextDataTableColumn( "Connection Depth" ) };
    }
    else
    {
        return { RifTextDataTableColumn( "I" ),
                 RifTextDataTableColumn( "J" ),
                 RifTextDataTableColumn( "K" ),
                 RifTextDataTableColumn( "Branch no" ),
                 RifTextDataTableColumn( "Start Length" ),
                 RifTextDataTableColumn( "End Length" ),
                 RifTextDataTableColumn( "Dir Pen" ),
                 RifTextDataTableColumn( "End Range" ),
                 RifTextDataTableColumn( "Connection Depth" ) };
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to create WSEGVALV headers
//--------------------------------------------------------------------------------------------------
std::vector<RifTextDataTableColumn> createWsegvalvHeader()
{
    return { RifTextDataTableColumn( "Well" ),
             RifTextDataTableColumn( "Seg No" ),
             RifTextDataTableColumn( "Cv" ),
             RifTextDataTableColumn( "Ac", RifTextDataTableDoubleFormatting( RIF_CONSISE, 4 ) ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper function to create WSEGAICD headers
//--------------------------------------------------------------------------------------------------
std::vector<RifTextDataTableColumn> createWsegaicdHeader()
{
    return { RifTextDataTableColumn( "Well" ),
             RifTextDataTableColumn( "Seg No" ),
             RifTextDataTableColumn( "Flow Coeff" ),
             RifTextDataTableColumn( "Area" ),
             RifTextDataTableColumn( "Oil Visc" ),
             RifTextDataTableColumn( "Water Visc" ),
             RifTextDataTableColumn( "Gas Visc" ),
             RifTextDataTableColumn( "Device Type" ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper function to create WELSEGS segment headers
//--------------------------------------------------------------------------------------------------
std::vector<RifTextDataTableColumn> createWelsegsSegmentHeader()
{
    return { RifTextDataTableColumn( "First Seg" ),
             RifTextDataTableColumn( "Last Seg" ),
             RifTextDataTableColumn( "Branch No" ),
             RifTextDataTableColumn( "Outlet Seg" ),
             RifTextDataTableColumn( "Length" ),
             RifTextDataTableColumn( "Depth Change" ),
             RifTextDataTableColumn( "Diam" ),
             RifTextDataTableColumn( "Rough", RifTextDataTableDoubleFormatting( RIF_FLOAT, 7 ) ) };
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// Format WELSEGS table for a single well
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWelsegsTable( RifTextDataTableFormatter& formatter, const RigMswTableData& tableData )
{
    if ( !tableData.hasWelsegsData() ) return;

    const auto& welsegsHeader = tableData.welsegsHeader();
    formatter.keyword( "WELSEGS" );
    std::vector<RifTextDataTableColumn> tableHeader = {
        RifTextDataTableColumn( "Well" ),
        RifTextDataTableColumn( "Dep 1" ),
        RifTextDataTableColumn( "Tlen 1" ),
        RifTextDataTableColumn( "Vol 1" ),
        RifTextDataTableColumn( "Len&Dep" ),
        RifTextDataTableColumn( "PresDrop" ),
    };
    formatter.header( tableHeader );

    // Write header row
    formatter.addStdString( welsegsHeader.wellName );
    formatter.add( welsegsHeader.topTVD );
    formatter.add( welsegsHeader.topMD );
    formatter.addOptionalValue( welsegsHeader.volume );
    formatter.addStdString( welsegsHeader.lengthAndDepthText );
    formatter.addStdString( welsegsHeader.pressureDropText );
    formatter.rowCompleted();

    // Column headers for segment data
    auto segmentHeader = createWelsegsSegmentHeader();
    formatter.header( segmentHeader );

    // Write segment data using helper function
    formatWelsegsRows( formatter, tableData.welsegsData() );
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format COMPSEGS table for a single well
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatCompsegsTable( RifTextDataTableFormatter& formatter, const RigMswTableData& tableData, bool isLgrData )
{
    std::vector<CompsegsRow> rows;
    if ( isLgrData )
    {
        rows = tableData.lgrCompsegsData();
    }
    else
    {
        rows = tableData.mainGridCompsegsData();
    }

    if ( rows.empty() ) return;

    formatter.keyword( "COMPSEGS" );

    // Add well name
    {
        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "Name" ) };
        formatter.header( header );
        formatter.addStdString( tableData.wellName() );
        formatter.rowCompleted();
    }

    auto header = createCompsegsHeader( isLgrData );
    formatter.header( header );

    formatCompsegsRows( formatter, rows, isLgrData );
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format WSEGVALV table for a single well
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWsegvalvTable( RifTextDataTableFormatter& formatter, const RigMswTableData& tableData )
{
    if ( !tableData.hasWsegvalvData() ) return;

    formatter.keyword( "WSEGVALV" );
    auto header = createWsegvalvHeader();
    formatter.header( header );

    formatWsegvalvRows( formatter, tableData.wsegvalvData() );
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format WSEGAICD table for a single well
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWsegaicdTable( RifTextDataTableFormatter& formatter, const RigMswTableData& tableData )
{
    if ( !tableData.hasWsegaicdData() ) return;

    formatter.keyword( "WSEGAICD" );
    auto header = createWsegaicdHeader();
    formatter.header( header );

    formatWsegaicdRows( formatter, tableData.wsegaicdData() );
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format WELSEGS table for unified data (multiple wells)
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWelsegsTable( RifTextDataTableFormatter& formatter, const RigMswUnifiedDataWIP& unifiedData )
{
    if ( unifiedData.isEmpty() ) return;

    auto headers = unifiedData.getAllWelsegsHeaders();
    auto rows    = unifiedData.getAllWelsegsRows();

    if ( headers.empty() && rows.empty() ) return;

    formatter.keyword( "WELSEGS" );
    std::vector<RifTextDataTableColumn> unifiedWelsegsHeader = { RifTextDataTableColumn( "Well" ),
                                                                 RifTextDataTableColumn( "Dep 1" ),
                                                                 RifTextDataTableColumn( "Tlen 1" ),
                                                                 RifTextDataTableColumn( "Vol 1" ),
                                                                 RifTextDataTableColumn( "Len&Dep" ),
                                                                 RifTextDataTableColumn( "PresDrop" ) };
    formatter.header( unifiedWelsegsHeader );

    // Write headers for all wells
    for ( const auto& header : headers )
    {
        formatter.addStdString( header.wellName );
        formatter.add( header.topTVD );
        formatter.add( header.topMD );
        formatter.addOptionalValue( header.volume );
        formatter.addStdString( header.lengthAndDepthText );
        formatter.addStdString( header.pressureDropText );
        formatter.rowCompleted();
    }

    // Column headers for segment data
    auto segmentHeader = createWelsegsSegmentHeader();
    formatter.header( segmentHeader );

    // Write all segment data using helper function
    formatWelsegsRows( formatter, rows );

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format WSEGVALV table for unified data (multiple wells)
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWsegvalvTable( RifTextDataTableFormatter& formatter, const RigMswUnifiedDataWIP& unifiedData )
{
    auto rows = unifiedData.getAllWsegvalvRows();
    if ( rows.empty() ) return;

    formatter.keyword( "WSEGVALV" );
    auto header = createWsegvalvHeader();
    formatter.header( header );

    formatWsegvalvRows( formatter, rows );

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format WSEGAICD table for unified data (multiple wells)
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWsegaicdTable( RifTextDataTableFormatter& formatter, const RigMswUnifiedDataWIP& unifiedData )
{
    auto rows = unifiedData.getAllWsegaicdRows();
    if ( rows.empty() ) return;

    formatter.keyword( "WSEGAICD" );
    auto header = createWsegaicdHeader();
    formatter.header( header );

    formatWsegaicdRows( formatter, rows );

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format all MSW tables for a single well
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatMswTables( RifTextDataTableFormatter& formatter, const RigMswTableData& tableData )
{
    formatWelsegsTable( formatter, tableData );
    formatCompsegsTable( formatter, tableData, false ); // Main grid

    if ( tableData.hasLgrData() )
    {
        formatCompsegsTable( formatter, tableData, true ); // LGR data
    }

    formatWsegvalvTable( formatter, tableData );
    formatWsegaicdTable( formatter, tableData );
}

//--------------------------------------------------------------------------------------------------
/// Format all MSW tables for unified data (multiple wells)
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatMswTables( RifTextDataTableFormatter& formatter, const RigMswUnifiedDataWIP& unifiedData )
{
    formatWelsegsTable( formatter, unifiedData );

    for ( const auto& wellData : unifiedData.wellDataList() )
    {
        bool isLgrData = false;
        formatCompsegsTable( formatter, wellData, isLgrData ); // Main grid, LGR is handeled separately
    }

    formatWsegvalvTable( formatter, unifiedData );
    formatWsegaicdTable( formatter, unifiedData );
}
