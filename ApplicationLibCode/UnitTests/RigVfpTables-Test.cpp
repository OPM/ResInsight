/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "ProjectDataModel/VerticalFlowPerformance/RimVfpDefines.h"
#include "RigVfpTables.h"

#include "FileInterface/RifVfpInjTable.h"
#include "FileInterface/RifVfpProdTable.h"
#include "ProjectDataModel/RiaOpmParserTools.h"

#include "opm/input/eclipse/Units/UnitSystem.hpp"

#include "RiaTestDataDirectory.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, MatchingValues )
{
    std::vector<double> sourceValues   = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> valuesForMatch = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<int>    closestIndices = RigVfpTables::uniqueClosestIndices( sourceValues, valuesForMatch );
    for ( size_t i = 0; i < sourceValues.size(); i++ )
    {
        EXPECT_EQ( i, closestIndices[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, MoreDestinationValues )
{
    std::vector<double> sourceValues   = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> valuesForMatch = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    std::vector<int>    closestIndices = RigVfpTables::uniqueClosestIndices( sourceValues, valuesForMatch );
    for ( size_t i = 0; i < sourceValues.size(); i++ )
    {
        EXPECT_EQ( i, closestIndices[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, MatchInBetweenValues )
{
    std::vector<double> sourceValues   = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> valuesForMatch = { 1.0, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0 };
    std::vector<int>    closestIndices = RigVfpTables::uniqueClosestIndices( sourceValues, valuesForMatch );

    EXPECT_EQ( 5, (int)closestIndices.size() );

    EXPECT_EQ( 0, closestIndices[0] );
    EXPECT_EQ( 2, closestIndices[1] );
    EXPECT_EQ( 3, closestIndices[2] );
    EXPECT_EQ( 4, closestIndices[3] );
    EXPECT_EQ( 5, closestIndices[4] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, SingleValue )
{
    std::vector<double> sourceValues   = { 1.0, 2.0, 3.0 };
    std::vector<double> valuesForMatch = { 2.1 };
    std::vector<int>    closestIndices = RigVfpTables::uniqueClosestIndices( sourceValues, valuesForMatch );

    EXPECT_EQ( 3, (int)closestIndices.size() );

    EXPECT_EQ( -1, closestIndices[0] );
    EXPECT_EQ( 0, closestIndices[1] );
    EXPECT_EQ( -1, closestIndices[2] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, FieldUnitDisplayUnits )
{
    // Test field unit system
    Opm::UnitSystem fieldUnitSystem = Opm::UnitSystem::newFIELD();

    QString thpUnit = RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType::THP, fieldUnitSystem );
    EXPECT_EQ( "psia", thpUnit.toStdString() );

    QString flowRateUnit = RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType::FLOW_RATE, fieldUnitSystem );
    EXPECT_EQ( "stb/day", flowRateUnit.toStdString() );

    QString thpUnitBracket = RigVfpTables::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP, fieldUnitSystem );
    EXPECT_EQ( "[psia]", thpUnitBracket.toStdString() );

    QString flowRateUnitBracket = RigVfpTables::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::FLOW_RATE, fieldUnitSystem );
    EXPECT_EQ( "[stb/day]", flowRateUnitBracket.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, MetricUnitDisplayUnits )
{
    // Test metric unit system
    Opm::UnitSystem metricUnitSystem = Opm::UnitSystem::newMETRIC();

    QString thpUnit = RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType::THP, metricUnitSystem );
    EXPECT_EQ( "Bar", thpUnit.toStdString() );

    QString flowRateUnit = RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType::FLOW_RATE, metricUnitSystem );
    EXPECT_EQ( "Sm3/day", flowRateUnit.toStdString() );

    QString thpUnitBracket = RigVfpTables::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP, metricUnitSystem );
    EXPECT_EQ( "[Bar]", thpUnitBracket.toStdString() );

    QString flowRateUnitBracket = RigVfpTables::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::FLOW_RATE, metricUnitSystem );
    EXPECT_EQ( "[Sm3/day]", flowRateUnitBracket.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigVfpTables, ImportVfpDataFromFile )
{
    // Import VFP data using the new ResInsight VFP file readers
    std::string vfpFilePath = QString( "%1/RigSimulationInputTool/model5/include/flowl_b_vfp.ecl" ).arg( TEST_DATA_DIR ).toStdString();

    auto [unitSystem, prodTables, injTables] = RiaOpmParserTools::extractVfpTablesFromDataFile( vfpFilePath );

    // Verify we got data
    EXPECT_FALSE( prodTables.empty() );
    EXPECT_TRUE( injTables.empty() ); // This file contains only production tables

    // Check unit system is metric (based on VFPPROD header "METRIC")
    EXPECT_EQ( Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC, unitSystem.getType() );

    // Verify first production table
    const auto& table = prodTables[0];
    EXPECT_EQ( 4, table.tableNum() );
    EXPECT_DOUBLE_EQ( 114.60, table.datumDepth() );

    // Check table type indicators
    EXPECT_EQ( RifVfpProdTable::FLO_TYPE::FLO_LIQ, table.floType() );
    EXPECT_EQ( RifVfpProdTable::WFR_TYPE::WFR_WCT, table.wfrType() );
    EXPECT_EQ( RifVfpProdTable::GFR_TYPE::GFR_GOR, table.gfrType() );
    EXPECT_EQ( RifVfpProdTable::ALQ_TYPE::ALQ_GRAT, table.alqType() );

    // Verify axis data preserved from original file
    const auto& floAxis = table.floAxis();
    EXPECT_EQ( 19, floAxis.size() );
    EXPECT_DOUBLE_EQ( 20.0, floAxis[0] );
    EXPECT_DOUBLE_EQ( 30.0, floAxis[1] );
    EXPECT_DOUBLE_EQ( 11171.0, floAxis[floAxis.size() - 1] );

    const auto& thpAxis = table.thpAxis();
    EXPECT_EQ( 6, thpAxis.size() );
    EXPECT_DOUBLE_EQ( 2.00, thpAxis[0] );
    EXPECT_DOUBLE_EQ( 35.00, thpAxis[thpAxis.size() - 1] );

    const auto& wfrAxis = table.wfrAxis();
    EXPECT_EQ( 10, wfrAxis.size() );
    EXPECT_DOUBLE_EQ( 0.000, wfrAxis[0] );
    EXPECT_DOUBLE_EQ( 0.990, wfrAxis[wfrAxis.size() - 1] );

    const auto& gfrAxis = table.gfrAxis();
    EXPECT_EQ( 9, gfrAxis.size() );
    EXPECT_DOUBLE_EQ( 20.0, gfrAxis[0] );
    EXPECT_DOUBLE_EQ( 1000.0, gfrAxis[gfrAxis.size() - 1] );

    const auto& alqAxis = table.alqAxis();
    EXPECT_EQ( 6, alqAxis.size() );
    EXPECT_DOUBLE_EQ( 0.0, alqAxis[0] );
    EXPECT_DOUBLE_EQ( 300000.0, alqAxis[alqAxis.size() - 1] );

    // Test table dimensions
    auto shape = table.shape();
    EXPECT_EQ( 6, shape[0] ); // THP
    EXPECT_EQ( 10, shape[1] ); // WFR
    EXPECT_EQ( 9, shape[2] ); // GFR
    EXPECT_EQ( 6, shape[3] ); // ALQ
    EXPECT_EQ( 19, shape[4] ); // FLO

    // Test specific table value (first data point from file)
    // THP=1, WFR=1, GFR=1, ALQ=1 should give BHP=13.977
    EXPECT_DOUBLE_EQ( 13.977, table( 0, 0, 0, 0, 0 ) );

    // Test RigVfpTables functionality with imported data
    RigVfpTables vfpTables;
    vfpTables.setUnitSystem( unitSystem );
    vfpTables.addProductionTable( table );

    // Verify table numbers
    auto tableNumbers = vfpTables.productionTableNumbers();
    EXPECT_EQ( 1, tableNumbers.size() );
    EXPECT_EQ( 4, tableNumbers[0] );

    // Test unit display with imported data
    QString thpUnit = RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType::THP, vfpTables.unitSystem() );
    EXPECT_EQ( "Bar", thpUnit.toStdString() );

    QString flowRateUnit = RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType::FLOW_RATE, vfpTables.unitSystem() );
    EXPECT_EQ( "Sm3/day", flowRateUnit.toStdString() );
}
