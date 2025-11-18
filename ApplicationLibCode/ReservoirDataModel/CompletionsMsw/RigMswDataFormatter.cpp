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
void formatWelsegsRows( RifTextDataTableFormatter& formatter, const std::vector<WelsegsRow>& rows )
{
    for ( const auto& row : rows )
    {
        formatter.add( row.segment1 );
        formatter.add( row.segment2 );
        formatter.add( row.branch );
        formatter.add( row.joinSegment );
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
void formatCompsegsRows( RifTextDataTableFormatter& formatter, const std::vector<CompsegsRow>& rows, bool isLgrData )
{
    for ( const auto& row : rows )
    {
        if ( isLgrData )
        {
            formatter.addStdString( row.gridName );
        }

        formatter.add( row.i );
        formatter.add( row.j );
        formatter.add( row.k );
        formatter.add( row.branch );
        formatter.add( row.distanceStart );
        formatter.add( row.distanceEnd );

        formatter.rowCompleted();
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Helper function to format WSEGVALV data rows
//--------------------------------------------------------------------------------------------------
void formatWsegvalvRows( RifTextDataTableFormatter& formatter, const std::vector<WsegvalvRow>& rows )
{
    for ( const auto& row : rows )
    {
        formatter.addStdString( row.well );
        formatter.add( row.segmentNumber );
        formatter.add( row.cv );
        formatter.add( row.area );

        formatter.rowCompleted();
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper function to format WSEGAICD data rows
//--------------------------------------------------------------------------------------------------
void formatWsegaicdRows( RifTextDataTableFormatter& formatter, const std::vector<WsegaicdRow>& rows )
{
    for ( const auto& row : rows )
    {
        formatter.addStdString( row.well );
        formatter.add( row.segment1 );
        formatter.add( row.segment2 );
        formatter.add( row.strength );
        formatter.addOptionalValue( row.length );
        formatter.addOptionalValue( row.densityCali );
        formatter.addOptionalValue( row.viscosityCali );
        formatter.addOptionalValue( row.criticalValue );
        formatter.addOptionalValue( row.widthTrans );
        formatter.addOptionalValue( row.maxViscRatio );

        if ( row.methodScalingFactor.has_value() )
        {
            formatter.add( row.methodScalingFactor.value() );
        }
        else
        {
            formatter.add( formatter.defaultMarker() );
        }

        formatter.add( row.maxAbsRate );
        formatter.add( row.flowRateExponent );
        formatter.add( row.viscExponent );

        if ( row.status.has_value() )
        {
            formatter.addStdString( row.status.value() );
        }
        else
        {
            formatter.addStdString( "OPEN" );
        }

        formatter.addOptionalValue( row.oilFlowFraction );
        formatter.addOptionalValue( row.waterFlowFraction );
        formatter.addOptionalValue( row.gasFlowFraction );
        formatter.addOptionalValue( row.oilViscFraction );
        formatter.addOptionalValue( row.waterViscFraction );
        formatter.addOptionalValue( row.gasViscFraction );

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
    formatter.addStdString( welsegsHeader.well );
    formatter.add( welsegsHeader.topDepth );
    formatter.add( welsegsHeader.topLength );
    formatter.addOptionalValue( welsegsHeader.wellboreVolume );
    formatter.addStdString( welsegsHeader.infoType );

    if ( welsegsHeader.pressureComponents.has_value() )
    {
        formatter.addStdString( "'" + welsegsHeader.pressureComponents.value() + "'" );
    }
    else
    {
        formatter.add( formatter.defaultMarker() );
    }

    formatter.rowCompleted();

    // Column headers for segment data
    auto segmentHeader = createWelsegsSegmentHeader();
    formatter.header( segmentHeader );

    // Write segment data using helper function
    formatWelsegsRows( formatter, tableData.welsegsData() );
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::addWsegaicdHeader( RifTextDataTableFormatter& formatter )
{
    // Write out header for AICD table

    std::vector<QString> columnDescriptions = { "Well Name",
                                                "Segment Number",
                                                "Segment Number",
                                                "Strength of AICD",
                                                "Flow Scaling Factor for AICD",
                                                "Density of Calibration Fluid",
                                                "Viscosity of Calibration Fluid",
                                                "Critical water in liquid fraction for emulsions viscosity model",
                                                "Emulsion viscosity transition region",
                                                "Max ratio of emulsion viscosity to continuous phase viscosity",
                                                "Flow scaling factor method",
                                                "Maximum flow rate for AICD device",
                                                "Volume flow rate exponent, x",
                                                "Viscosity function exponent, y",
                                                "Device OPEN/SHUT",
                                                "Exponent of the oil flowing fraction in the density mixture calculation",
                                                "Exponent of the water flowing fraction in the density mixture calculation",
                                                "Exponent of the gas flowing fraction in the density mixture calculation",
                                                "Exponent of the oil flowing fraction in the density viscosity calculation",
                                                "Exponent of the water flowing fraction in the density viscosity calculation",
                                                "Exponent of the gas flowing fraction in the density viscosity calculation" };

    formatter.keyword( "WSEGAICD" );
    formatter.comment( "Column Overview:" );
    for ( size_t i = 0; i < columnDescriptions.size(); ++i )
    {
        formatter.comment( QString( "%1: %2" ).arg( i + 1, 2, 10, QChar( '0' ) ).arg( columnDescriptions[i] ) );
    }

    std::vector<RifTextDataTableColumn> header;
    for ( size_t i = 1; i <= 21; ++i )
    {
        QString                cName = QString( "%1" ).arg( i, 2, 10, QChar( '0' ) );
        RifTextDataTableColumn col( cName, RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_CONSISE ), RIGHT );
        header.push_back( col );
    }
    formatter.header( header );
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

    RifTextDataTableFormatter tightFormatter( formatter );
    tightFormatter.setColumnSpacing( 1 );
    tightFormatter.setTableRowPrependText( "   " );

    addWsegaicdHeader( tightFormatter );

    formatWsegaicdRows( tightFormatter, tableData.wsegaicdData() );
    tightFormatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// Format WELSEGS table for unified data (multiple wells)
//--------------------------------------------------------------------------------------------------
void RigMswDataFormatter::formatWelsegsTable( RifTextDataTableFormatter& formatter, const RigMswUnifiedDataWIP& unifiedData )
{
    // TODO: Handle multiple WELSEGS headers?
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

    RifTextDataTableFormatter tightFormatter( formatter );
    tightFormatter.setColumnSpacing( 1 );
    tightFormatter.setTableRowPrependText( "   " );

    addWsegaicdHeader( tightFormatter );

    formatWsegaicdRows( tightFormatter, unifiedData.getAllWsegaicdRows() );
    tightFormatter.tableCompleted();
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
        formatCompsegsTable( formatter, wellData, isLgrData ); // Main grid, LGR is handled separately
    }

    formatWsegvalvTable( formatter, unifiedData );
    formatWsegaicdTable( formatter, unifiedData );
}
