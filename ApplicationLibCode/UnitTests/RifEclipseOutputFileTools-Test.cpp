/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RifEclipseOutputFileTools.h"
#include "RifEclipseReportKeywords.h"

#include "RiaDefines.h"

#include "RigActiveCellInfo.h"

//--------------------------------------------------------------------------------------------------
/// A grid with no active matrix cells must not cause division by zero
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseOutputFileTools, ValidKeywordsNoActiveMatrixCells )
{
    std::vector<RifEclipseKeywordValueCount> keywordItemCounts;
    keywordItemCounts.emplace_back( "PRESSURE", 50, RifEclipseKeywordValueCount::KeywordDataType::FLOAT );

    RigActiveCellInfo matrixActiveCellInfo;
    matrixActiveCellInfo.setReservoirCellCount( 100 );

    RigActiveCellInfo fractureActiveCellInfo;
    fractureActiveCellInfo.setReservoirCellCount( 100 );
    fractureActiveCellInfo.setGridCount( 1 );
    fractureActiveCellInfo.setGridActiveCellCounts( 0, 50 );
    fractureActiveCellInfo.computeDerivedData();

    EXPECT_EQ( size_t( 0 ), matrixActiveCellInfo.reservoirActiveCellCount() );

    auto matrixKeywords = RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                                    &matrixActiveCellInfo,
                                                                                    &fractureActiveCellInfo,
                                                                                    RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                                    1 );
    EXPECT_TRUE( matrixKeywords.empty() );

    auto fractureKeywords = RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                                      &matrixActiveCellInfo,
                                                                                      &fractureActiveCellInfo,
                                                                                      RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                                      1 );
    EXPECT_EQ( size_t( 1 ), fractureKeywords.size() );
}

//--------------------------------------------------------------------------------------------------
/// Empty active cell info for both porosity models must not cause division by zero
//--------------------------------------------------------------------------------------------------
TEST( RifEclipseOutputFileTools, ValidKeywordsNoActiveCells )
{
    std::vector<RifEclipseKeywordValueCount> keywordItemCounts;
    keywordItemCounts.emplace_back( "PRESSURE", 100, RifEclipseKeywordValueCount::KeywordDataType::FLOAT );

    RigActiveCellInfo matrixActiveCellInfo;
    RigActiveCellInfo fractureActiveCellInfo;

    EXPECT_TRUE( RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                           &matrixActiveCellInfo,
                                                                           &fractureActiveCellInfo,
                                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                           1 )
                     .empty() );

    EXPECT_TRUE( RifEclipseOutputFileTools::validKeywordsForPorosityModel( keywordItemCounts,
                                                                           &matrixActiveCellInfo,
                                                                           &fractureActiveCellInfo,
                                                                           RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                           1 )
                     .empty() );
}
