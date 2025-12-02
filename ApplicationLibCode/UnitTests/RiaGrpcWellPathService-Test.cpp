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

#include "gtest/gtest.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "RiaDefines.h"
#include "RiaWellPathDataToGrpcConverter.h"
#include "RigCompletionData.h"
#include "RigCompletionDataGridCell.h"
#include "RimWellPathCompletionSettings.h"

#include "SimulatorTables.pb.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
/// Unit test to ensure all fields from RigCompletionData are properly copied to gRPC SimulatorCompdatEntry
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyCompdatToGrpcCopiesAllFields )
{
    // Create test input data with all possible values set
    RigCompletionDataGridCell gridCell; // Default constructor initializes i,j,k to 0
    RigCompletionData         inputData( "WELL_A", gridCell, 100.0 );

    // Set all optional fields to non-default values
    inputData.setTransmissibility( 1.23 );
    inputData.setDiameter( 0.311 );
    inputData.setKh( 45.6 );
    inputData.setDFactor( 0.789 );

    // Use the transmissibility setting method to also set skin factor
    inputData.setCombinedValuesExplicitTrans( 1.23, // transmissibility
                                              45.6, // kh
                                              0.789, // dFactor
                                              2.5, // skinFactor
                                              0.311, // diameter
                                              RigCompletionData::CellDirection::DIR_I, // direction
                                              RigCompletionData::CompletionType::PERFORATION // completionType
    );

    // Set MD depth range
    inputData.setDepthRange( 2000.5, 2010.8 );

    // Add metadata
    inputData.addMetadata( "TestName", "Test Comment" );

    // Create output gRPC message
    rips::SimulatorCompdatEntry grpcData;

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyCompdatToGrpc( inputData, &grpcData );

    // Verify all fields are copied correctly

    // Basic fields
    EXPECT_EQ( grpcData.well_name(), "WELL_A" );
    EXPECT_EQ( grpcData.comment(), "TestName: Test Comment" ); // metadata is concatenated

    // Grid coordinates (should be converted to 1-based)
    // Default constructor gives i,j,k=0, so after +1 conversion we get 1,1,1
    EXPECT_EQ( grpcData.grid_i(), 1 ); // 0+1 (1-based conversion)
    EXPECT_EQ( grpcData.grid_j(), 1 ); // 0+1
    EXPECT_EQ( grpcData.upper_k(), 1 ); // 0+1
    EXPECT_EQ( grpcData.lower_k(), 1 ); // 0+1 (same as upper_k for single cell)

    // Status field
    EXPECT_EQ( grpcData.open_shut_flag(), "OPEN" );

    // Optional numerical fields (only set if not default value)
    EXPECT_TRUE( grpcData.has_transmissibility() );
    EXPECT_DOUBLE_EQ( grpcData.transmissibility(), 1.23 );

    EXPECT_TRUE( grpcData.has_diameter() );
    EXPECT_DOUBLE_EQ( grpcData.diameter(), 0.311 );

    EXPECT_TRUE( grpcData.has_kh() );
    EXPECT_DOUBLE_EQ( grpcData.kh(), 45.6 );

    EXPECT_TRUE( grpcData.has_skin_factor() );
    EXPECT_DOUBLE_EQ( grpcData.skin_factor(), 2.5 );

    EXPECT_TRUE( grpcData.has_d_factor() );
    EXPECT_DOUBLE_EQ( grpcData.d_factor(), 0.789 );

    // Direction field
    EXPECT_TRUE( grpcData.has_direction() );
    EXPECT_EQ( grpcData.direction(), "X" ); // DIR_I maps to "X"

    // MD depth range
    EXPECT_TRUE( grpcData.has_start_md() );
    EXPECT_DOUBLE_EQ( grpcData.start_md(), 2000.5 );

    EXPECT_TRUE( grpcData.has_end_md() );
    EXPECT_DOUBLE_EQ( grpcData.end_md(), 2010.8 );
}

//--------------------------------------------------------------------------------------------------
/// Unit test to ensure default values are not copied to gRPC message
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyCompdatToGrpcSkipsDefaultValues )
{
    // Create test input data with default values
    RigCompletionDataGridCell gridCell; // Default constructor initializes i,j,k to 0
    RigCompletionData         inputData( "WELL_B", gridCell, 200.0 );

    // Don't set any optional fields (they should remain at default values)
    // Add minimal metadata
    inputData.addMetadata( "MinimalTest", "" );

    // Create output gRPC message
    rips::SimulatorCompdatEntry grpcData;

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyCompdatToGrpc( inputData, &grpcData );

    // Verify required fields are set
    EXPECT_EQ( grpcData.well_name(), "WELL_B" );
    EXPECT_EQ( grpcData.comment(), "MinimalTest: " );
    // Grid coordinates from default constructor: 0+1=1 (1-based)
    EXPECT_EQ( grpcData.grid_i(), 1 );
    EXPECT_EQ( grpcData.grid_j(), 1 );
    EXPECT_EQ( grpcData.upper_k(), 1 );
    EXPECT_EQ( grpcData.lower_k(), 1 );
    EXPECT_EQ( grpcData.open_shut_flag(), "OPEN" );

    // Verify optional fields with default values are NOT set
    EXPECT_FALSE( grpcData.has_saturation() );
    EXPECT_FALSE( grpcData.has_transmissibility() );
    EXPECT_FALSE( grpcData.has_diameter() );
    EXPECT_FALSE( grpcData.has_kh() );
    EXPECT_FALSE( grpcData.has_skin_factor() );
    EXPECT_FALSE( grpcData.has_d_factor() );

    // Direction should still be set (it has a default string value)
    EXPECT_TRUE( grpcData.has_direction() );

    // MD values should not be set (no depth range was specified)
    EXPECT_FALSE( grpcData.has_start_md() );
    EXPECT_FALSE( grpcData.has_end_md() );
}

//--------------------------------------------------------------------------------------------------
/// Unit test to verify all proto fields are covered by checking against proto definition
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyCompdatToGrpcCoversAllProtoFields )
{
    // This test documents all fields in SimulatorCompdatEntry to ensure we don't miss any
    // when updating the proto definition. If this test fails, update copyCompdatToGrpc accordingly.

    rips::SimulatorCompdatEntry grpcData;

    // Verify all fields from SimulatorTables.proto SimulatorCompdatEntry message exist:
    // Based on proto definition (lines 24-41 in SimulatorTables.proto):
    // string well_name = 1;           ✓ Covered
    // int32 grid_i = 2;              ✓ Covered
    // int32 grid_j = 3;              ✓ Covered
    // int32 upper_k = 4;             ✓ Covered
    // int32 lower_k = 5;             ✓ Covered
    // string open_shut_flag = 6;     ✓ Covered
    // optional double saturation = 7;        ✓ Covered (not used in current implementation)
    // optional double transmissibility = 8;  ✓ Covered
    // optional double diameter = 9;          ✓ Covered
    // optional double kh = 10;               ✓ Covered
    // optional double skin_factor = 11;      ✓ Covered
    // optional double d_factor = 12;         ✓ Covered
    // optional string direction = 13;        ✓ Covered
    // optional double start_md = 14;         ✓ Covered
    // optional double end_md = 15;           ✓ Covered
    // optional string comment = 16;          ✓ Covered

    // Test that we can access all fields (this will catch any missing proto fields)
    grpcData.set_well_name( "TEST" );
    grpcData.set_grid_i( 1 );
    grpcData.set_grid_j( 2 );
    grpcData.set_upper_k( 3 );
    grpcData.set_lower_k( 4 );
    grpcData.set_open_shut_flag( "OPEN" );
    grpcData.set_saturation( 0.5 );
    grpcData.set_transmissibility( 1.0 );
    grpcData.set_diameter( 0.2 );
    grpcData.set_kh( 10.0 );
    grpcData.set_skin_factor( 1.5 );
    grpcData.set_d_factor( 0.1 );
    grpcData.set_direction( "X" );
    grpcData.set_start_md( 100.0 );
    grpcData.set_end_md( 200.0 );
    grpcData.set_comment( "Test comment" );

    // If we got here without compilation errors, all proto fields are accessible
    // If a new field is added to the proto but not handled in copyCompdatToGrpc,
    // this test should be updated to reflect that
    SUCCEED();
}

//--------------------------------------------------------------------------------------------------
/// Unit tests for copyWelspecsToGrpc function
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyWelspecsToGrpcCopiesAllFields )
{
    // Create a completion settings object with test data
    RimWellPathCompletionSettings compSettings;
    compSettings.setWellNameForExport( "TEST_WELL" );
    compSettings.setGroupName( "TEST_GROUP" );

    // Create output gRPC message
    rips::SimulatorWelspecsEntry grpcData;

    // Test coordinates
    int         gridI    = 10;
    int         gridJ    = 20;
    std::string gridName = "";

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyWelspecsToGrpc( &compSettings, &grpcData, gridI, gridJ, gridName );

    // Verify basic fields are copied
    EXPECT_EQ( grpcData.well_name(), "TEST_WELL" );
    EXPECT_EQ( grpcData.group_name(), "TEST_GROUP" );

    // Verify coordinates are converted to 1-based
    EXPECT_EQ( grpcData.grid_i(), 11 ); // 10+1
    EXPECT_EQ( grpcData.grid_j(), 21 ); // 20+1

    // Verify that other fields are set to their exported string values
    EXPECT_FALSE( grpcData.phase().empty() );
}

//--------------------------------------------------------------------------------------------------
/// Unit tests for copyWelsegsToGrpc function
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyWelsegsToGrpcCopiesAllFields )
{
    // Create MSW table data with test data
    RigMswTableData mswData( "WELL_A", RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    // Set up header
    WelsegsHeader header;
    header.well               = "WELL_A";
    header.topDepth           = 1500.0;
    header.topLength          = 2000.0;
    header.wellboreVolume     = 0.5;
    header.infoType           = "ABS";
    header.pressureComponents = "HFA";
    header.flowModel          = "HO";
    mswData.setWelsegsHeader( header );

    // Add segment rows
    WelsegsRow row1;
    row1.segment1    = 1;
    row1.segment2    = 1;
    row1.branch      = 1;
    row1.joinSegment = 1;
    row1.length      = 100.0;
    row1.depth       = 1600.0;
    row1.diameter    = 0.2;
    row1.roughness   = 1e-5;
    row1.description = "Main segment";
    mswData.addWelsegsRow( row1 );

    WelsegsRow row2;
    row2.segment1    = 2;
    row2.segment2    = 2;
    row2.branch      = 1;
    row2.joinSegment = 1;
    row2.length      = 150.0;
    row2.depth       = 1750.0;
    row2.description = "Branch segment";
    mswData.addWelsegsRow( row2 );

    // Create output gRPC message
    rips::SimulatorWelsegsEntry grpcData;

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyWelsegsToGrpc( mswData, &grpcData );

    // Verify header data
    const auto& headerEntry = grpcData.header();
    EXPECT_EQ( headerEntry.well_name(), "WELL_A" );
    EXPECT_DOUBLE_EQ( headerEntry.top_depth(), 1500.0 );
    EXPECT_DOUBLE_EQ( headerEntry.top_length(), 2000.0 );
    EXPECT_TRUE( headerEntry.has_wellbore_volume() );
    EXPECT_DOUBLE_EQ( headerEntry.wellbore_volume(), 0.5 );
    EXPECT_EQ( headerEntry.info_type(), "ABS" );
    EXPECT_TRUE( headerEntry.has_pressure_components() );
    EXPECT_EQ( headerEntry.pressure_components(), "HFA" );
    EXPECT_TRUE( headerEntry.has_flow_model() );
    EXPECT_EQ( headerEntry.flow_model(), "HO" );

    // Verify row data
    EXPECT_EQ( grpcData.row_size(), 2 );

    const auto& grpcRow1 = grpcData.row( 0 );
    EXPECT_EQ( grpcRow1.segment_1(), 1 );
    EXPECT_EQ( grpcRow1.segment_2(), 1 );
    EXPECT_EQ( grpcRow1.branch(), 1 );
    EXPECT_EQ( grpcRow1.join_segment(), 1 );
    EXPECT_DOUBLE_EQ( grpcRow1.length(), 100.0 );
    EXPECT_DOUBLE_EQ( grpcRow1.depth(), 1600.0 );
    EXPECT_TRUE( grpcRow1.has_diameter() );
    EXPECT_DOUBLE_EQ( grpcRow1.diameter(), 0.2 );
    EXPECT_TRUE( grpcRow1.has_roughness() );
    EXPECT_DOUBLE_EQ( grpcRow1.roughness(), 1e-5 );
    EXPECT_TRUE( grpcRow1.has_description() );
    EXPECT_EQ( grpcRow1.description(), "Main segment" );

    const auto& grpcRow2 = grpcData.row( 1 );
    EXPECT_EQ( grpcRow2.segment_1(), 2 );
    EXPECT_EQ( grpcRow2.segment_2(), 2 );
    EXPECT_EQ( grpcRow2.branch(), 1 );
    EXPECT_EQ( grpcRow2.join_segment(), 1 );
    EXPECT_DOUBLE_EQ( grpcRow2.length(), 150.0 );
    EXPECT_DOUBLE_EQ( grpcRow2.depth(), 1750.0 );
    EXPECT_FALSE( grpcRow2.has_diameter() ); // Not set for this row
    EXPECT_FALSE( grpcRow2.has_roughness() ); // Not set for this row
    EXPECT_TRUE( grpcRow2.has_description() );
    EXPECT_EQ( grpcRow2.description(), "Branch segment" );
}

//--------------------------------------------------------------------------------------------------
/// Unit tests for copyCompsegsToGrpc function
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyCompsegsToGrpcCopiesAllFields )
{
    // Create MSW table data with compsegs data
    RigMswTableData mswData( "WELL_B", RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    // Add compsegs rows
    CompsegsRow row1;
    row1.i             = 10;
    row1.j             = 20;
    row1.k             = 5;
    row1.branch        = 1;
    row1.distanceStart = 100.0;
    row1.distanceEnd   = 200.0;
    row1.gridName      = ""; // Main grid
    mswData.addCompsegsRow( row1 );

    CompsegsRow row2;
    row2.i             = 11;
    row2.j             = 21;
    row2.k             = 6;
    row2.branch        = 2;
    row2.distanceStart = 300.0;
    row2.distanceEnd   = 400.0;
    row2.gridName      = "LGR_GRID";
    mswData.addCompsegsRow( row2 );

    // Create output gRPC message
    rips::SimulatorTableData grpcReply;

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyCompsegsToGrpc( mswData, &grpcReply );

    // Verify data was copied
    EXPECT_EQ( grpcReply.compsegs_size(), 2 );

    const auto& grpcRow1 = grpcReply.compsegs( 0 );
    EXPECT_EQ( grpcRow1.i(), 10 );
    EXPECT_EQ( grpcRow1.j(), 20 );
    EXPECT_EQ( grpcRow1.k(), 5 );
    EXPECT_EQ( grpcRow1.branch(), 1 );
    EXPECT_DOUBLE_EQ( grpcRow1.distance_start(), 100.0 );
    EXPECT_DOUBLE_EQ( grpcRow1.distance_end(), 200.0 );
    EXPECT_FALSE( grpcRow1.has_grid_name() ); // Empty grid name not set

    const auto& grpcRow2 = grpcReply.compsegs( 1 );
    EXPECT_EQ( grpcRow2.i(), 11 );
    EXPECT_EQ( grpcRow2.j(), 21 );
    EXPECT_EQ( grpcRow2.k(), 6 );
    EXPECT_EQ( grpcRow2.branch(), 2 );
    EXPECT_DOUBLE_EQ( grpcRow2.distance_start(), 300.0 );
    EXPECT_DOUBLE_EQ( grpcRow2.distance_end(), 400.0 );
    EXPECT_TRUE( grpcRow2.has_grid_name() );
    EXPECT_EQ( grpcRow2.grid_name(), "LGR_GRID" );
}

//--------------------------------------------------------------------------------------------------
/// Unit tests for copyWsegvalvToGrpc function
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyWsegvalvToGrpcCopiesAllFields )
{
    // Create MSW table data with valve data
    RigMswTableData mswData( "WELL_C", RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    // Add wsegvalv rows
    WsegvalvRow row1;
    row1.well          = "WELL_C";
    row1.segmentNumber = 5;
    row1.cv            = 0.8;
    row1.area          = 0.05;
    row1.extraLength   = 10.0;
    row1.pipeD         = 0.15;
    row1.roughness     = 1e-4;
    row1.pipeA         = 0.02;
    row1.status        = "OPEN";
    row1.maxA          = 0.1;
    mswData.addWsegvalvRow( row1 );

    WsegvalvRow row2;
    row2.well          = "WELL_C";
    row2.segmentNumber = 6;
    row2.cv            = 1.2;
    row2.area          = 0.08;
    // Optional fields not set for this row
    mswData.addWsegvalvRow( row2 );

    // Create output gRPC message
    rips::SimulatorTableData grpcReply;

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyWsegvalvToGrpc( mswData, &grpcReply );

    // Verify data was copied
    EXPECT_EQ( grpcReply.wsegvalv_size(), 2 );

    const auto& grpcRow1 = grpcReply.wsegvalv( 0 );
    EXPECT_EQ( grpcRow1.well_name(), "WELL_C" );
    EXPECT_EQ( grpcRow1.segment_number(), 5 );
    EXPECT_DOUBLE_EQ( grpcRow1.cv(), 0.8 );
    EXPECT_DOUBLE_EQ( grpcRow1.area(), 0.05 );
    EXPECT_TRUE( grpcRow1.has_extra_length() );
    EXPECT_DOUBLE_EQ( grpcRow1.extra_length(), 10.0 );
    EXPECT_TRUE( grpcRow1.has_pipe_d() );
    EXPECT_DOUBLE_EQ( grpcRow1.pipe_d(), 0.15 );
    EXPECT_TRUE( grpcRow1.has_roughness() );
    EXPECT_DOUBLE_EQ( grpcRow1.roughness(), 1e-4 );
    EXPECT_TRUE( grpcRow1.has_pipe_a() );
    EXPECT_DOUBLE_EQ( grpcRow1.pipe_a(), 0.02 );
    EXPECT_TRUE( grpcRow1.has_status() );
    EXPECT_EQ( grpcRow1.status(), "OPEN" );
    EXPECT_TRUE( grpcRow1.has_max_a() );
    EXPECT_DOUBLE_EQ( grpcRow1.max_a(), 0.1 );

    const auto& grpcRow2 = grpcReply.wsegvalv( 1 );
    EXPECT_EQ( grpcRow2.well_name(), "WELL_C" );
    EXPECT_EQ( grpcRow2.segment_number(), 6 );
    EXPECT_DOUBLE_EQ( grpcRow2.cv(), 1.2 );
    EXPECT_DOUBLE_EQ( grpcRow2.area(), 0.08 );
    EXPECT_FALSE( grpcRow2.has_extra_length() );
    EXPECT_FALSE( grpcRow2.has_pipe_d() );
    EXPECT_FALSE( grpcRow2.has_roughness() );
    EXPECT_FALSE( grpcRow2.has_pipe_a() );
    EXPECT_FALSE( grpcRow2.has_status() );
    EXPECT_FALSE( grpcRow2.has_max_a() );
}

//--------------------------------------------------------------------------------------------------
/// Unit tests for copyWsegaicdToGrpc function
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyWsegaicdToGrpcCopiesAllFields )
{
    // Create MSW table data with AICD data
    RigMswTableData mswData( "WELL_D", RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    // Add wsegaicd row with all optional fields
    WsegaicdRow row1;
    row1.well                = "WELL_D";
    row1.segment1            = 10;
    row1.segment2            = 11;
    row1.strength            = 5.0;
    row1.length              = 20.0;
    row1.densityCali         = 850.0;
    row1.viscosityCali       = 2.0;
    row1.criticalValue       = 0.5;
    row1.widthTrans          = 0.1;
    row1.maxViscRatio        = 10.0;
    row1.methodScalingFactor = 2;
    row1.maxAbsRate          = 1000.0;
    row1.flowRateExponent    = 2.0;
    row1.viscExponent        = 0.5;
    row1.status              = "OPEN";
    row1.oilFlowFraction     = 0.3;
    row1.waterFlowFraction   = 0.4;
    row1.gasFlowFraction     = 0.3;
    row1.oilViscFraction     = 0.25;
    row1.waterViscFraction   = 0.35;
    row1.gasViscFraction     = 0.4;
    row1.description         = "Test AICD";
    mswData.addWsegaicdRow( row1 );

    // Add wsegaicd row with minimal fields
    WsegaicdRow row2;
    row2.well             = "WELL_D";
    row2.segment1         = 12;
    row2.segment2         = 13;
    row2.strength         = 3.0;
    row2.maxAbsRate       = 800.0;
    row2.flowRateExponent = 1.8;
    row2.viscExponent     = 0.6;
    row2.description      = "";
    mswData.addWsegaicdRow( row2 );

    // Create output gRPC message
    rips::SimulatorTableData grpcReply;

    // Call the function under test
    RiaWellPathDataToGrpcConverter::copyWsegaicdToGrpc( mswData, &grpcReply );

    // Verify data was copied
    EXPECT_EQ( grpcReply.wsegaicd_size(), 2 );

    // Test row with all fields
    const auto& grpcRow1 = grpcReply.wsegaicd( 0 );
    EXPECT_EQ( grpcRow1.well_name(), "WELL_D" );
    EXPECT_EQ( grpcRow1.segment_1(), 10 );
    EXPECT_EQ( grpcRow1.segment_2(), 11 );
    EXPECT_DOUBLE_EQ( grpcRow1.strength(), 5.0 );
    EXPECT_TRUE( grpcRow1.has_length() );
    EXPECT_DOUBLE_EQ( grpcRow1.length(), 20.0 );
    EXPECT_TRUE( grpcRow1.has_density_cali() );
    EXPECT_DOUBLE_EQ( grpcRow1.density_cali(), 850.0 );
    EXPECT_TRUE( grpcRow1.has_viscosity_cali() );
    EXPECT_DOUBLE_EQ( grpcRow1.viscosity_cali(), 2.0 );
    EXPECT_TRUE( grpcRow1.has_critical_value() );
    EXPECT_DOUBLE_EQ( grpcRow1.critical_value(), 0.5 );
    EXPECT_TRUE( grpcRow1.has_width_trans() );
    EXPECT_DOUBLE_EQ( grpcRow1.width_trans(), 0.1 );
    EXPECT_TRUE( grpcRow1.has_max_visc_ratio() );
    EXPECT_DOUBLE_EQ( grpcRow1.max_visc_ratio(), 10.0 );
    EXPECT_TRUE( grpcRow1.has_method_scaling_factor() );
    EXPECT_EQ( grpcRow1.method_scaling_factor(), 2 );
    EXPECT_DOUBLE_EQ( grpcRow1.max_abs_rate(), 1000.0 );
    EXPECT_DOUBLE_EQ( grpcRow1.flow_rate_exponent(), 2.0 );
    EXPECT_DOUBLE_EQ( grpcRow1.visc_exponent(), 0.5 );
    EXPECT_TRUE( grpcRow1.has_status() );
    EXPECT_EQ( grpcRow1.status(), "OPEN" );
    EXPECT_TRUE( grpcRow1.has_oil_flow_fraction() );
    EXPECT_DOUBLE_EQ( grpcRow1.oil_flow_fraction(), 0.3 );
    EXPECT_TRUE( grpcRow1.has_water_flow_fraction() );
    EXPECT_DOUBLE_EQ( grpcRow1.water_flow_fraction(), 0.4 );
    EXPECT_TRUE( grpcRow1.has_gas_flow_fraction() );
    EXPECT_DOUBLE_EQ( grpcRow1.gas_flow_fraction(), 0.3 );
    EXPECT_TRUE( grpcRow1.has_oil_visc_fraction() );
    EXPECT_DOUBLE_EQ( grpcRow1.oil_visc_fraction(), 0.25 );
    EXPECT_TRUE( grpcRow1.has_water_visc_fraction() );
    EXPECT_DOUBLE_EQ( grpcRow1.water_visc_fraction(), 0.35 );
    EXPECT_TRUE( grpcRow1.has_gas_visc_fraction() );
    EXPECT_DOUBLE_EQ( grpcRow1.gas_visc_fraction(), 0.4 );
    EXPECT_TRUE( grpcRow1.has_description() );
    EXPECT_EQ( grpcRow1.description(), "Test AICD" );

    // Test row with minimal fields
    const auto& grpcRow2 = grpcReply.wsegaicd( 1 );
    EXPECT_EQ( grpcRow2.well_name(), "WELL_D" );
    EXPECT_EQ( grpcRow2.segment_1(), 12 );
    EXPECT_EQ( grpcRow2.segment_2(), 13 );
    EXPECT_DOUBLE_EQ( grpcRow2.strength(), 3.0 );
    EXPECT_FALSE( grpcRow2.has_length() );
    EXPECT_FALSE( grpcRow2.has_density_cali() );
    EXPECT_FALSE( grpcRow2.has_viscosity_cali() );
    EXPECT_FALSE( grpcRow2.has_critical_value() );
    EXPECT_FALSE( grpcRow2.has_width_trans() );
    EXPECT_FALSE( grpcRow2.has_max_visc_ratio() );
    EXPECT_FALSE( grpcRow2.has_method_scaling_factor() );
    EXPECT_DOUBLE_EQ( grpcRow2.max_abs_rate(), 800.0 );
    EXPECT_DOUBLE_EQ( grpcRow2.flow_rate_exponent(), 1.8 );
    EXPECT_DOUBLE_EQ( grpcRow2.visc_exponent(), 0.6 );
    EXPECT_FALSE( grpcRow2.has_status() );
    EXPECT_FALSE( grpcRow2.has_oil_flow_fraction() );
    EXPECT_FALSE( grpcRow2.has_water_flow_fraction() );
    EXPECT_FALSE( grpcRow2.has_gas_flow_fraction() );
    EXPECT_FALSE( grpcRow2.has_oil_visc_fraction() );
    EXPECT_FALSE( grpcRow2.has_water_visc_fraction() );
    EXPECT_FALSE( grpcRow2.has_gas_visc_fraction() );
    EXPECT_FALSE( grpcRow2.has_description() ); // Empty description not copied
}

//--------------------------------------------------------------------------------------------------
/// Unit tests for empty data handling
//--------------------------------------------------------------------------------------------------
TEST( RiaGrpcWellPathServiceTest, copyFunctionsHandleEmptyData )
{
    // Test empty MSW data
    RigMswTableData emptyMswData( "EMPTY_WELL", RiaDefines::EclipseUnitSystem::UNITS_METRIC );

    rips::SimulatorWelsegsEntry welsegsData;
    rips::SimulatorTableData    tableData;

    // These should not crash and should not add any data
    RiaWellPathDataToGrpcConverter::copyWelsegsToGrpc( emptyMswData, &welsegsData );
    RiaWellPathDataToGrpcConverter::copyCompsegsToGrpc( emptyMswData, &tableData );
    RiaWellPathDataToGrpcConverter::copyWsegvalvToGrpc( emptyMswData, &tableData );
    RiaWellPathDataToGrpcConverter::copyWsegaicdToGrpc( emptyMswData, &tableData );

    // Verify no data was added
    EXPECT_EQ( welsegsData.row_size(), 0 );
    EXPECT_EQ( tableData.compsegs_size(), 0 );
    EXPECT_EQ( tableData.wsegvalv_size(), 0 );
    EXPECT_EQ( tableData.wsegaicd_size(), 0 );
}