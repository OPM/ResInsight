/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026  Equinor ASA
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

#include "Well/RigWellTargetMappingTools.h"

#include "RiaResultNames.h"
#include "RigActiveCellInfo.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNncConnection.h"

#include "cvfStructGrid.h"

#include <limits>
#include <vector>

using FaceType    = cvf::StructGridInterface::FaceType;
using VolumeType  = RigWellTargetMapping::VolumeType;
using VolumesType = RigWellTargetMapping::VolumesType;

//==================================================================================================
// getValueForFace
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, GetValueForFace_PosI_ReturnsXAtIndex )
{
    std::vector<double> x = { 1.0, 2.0, 3.0 };
    std::vector<double> y = { 10.0, 20.0, 30.0 };
    std::vector<double> z = { 100.0, 200.0, 300.0 };

    EXPECT_DOUBLE_EQ( 2.0, RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::POS_I, ActiveCellIndex( 1 ) ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_NegI_ReturnsXAtIndex )
{
    std::vector<double> x = { 1.0, 2.0, 3.0 };
    std::vector<double> y = { 10.0, 20.0, 30.0 };
    std::vector<double> z = { 100.0, 200.0, 300.0 };

    EXPECT_DOUBLE_EQ( 1.0, RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::NEG_I, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_PosJ_ReturnsYAtIndex )
{
    std::vector<double> x = { 1.0, 2.0, 3.0 };
    std::vector<double> y = { 10.0, 20.0, 30.0 };
    std::vector<double> z = { 100.0, 200.0, 300.0 };

    EXPECT_DOUBLE_EQ( 30.0, RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::POS_J, ActiveCellIndex( 2 ) ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_NegJ_ReturnsYAtIndex )
{
    std::vector<double> x = { 1.0, 2.0, 3.0 };
    std::vector<double> y = { 10.0, 20.0, 30.0 };
    std::vector<double> z = { 100.0, 200.0, 300.0 };

    EXPECT_DOUBLE_EQ( 20.0, RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::NEG_J, ActiveCellIndex( 1 ) ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_PosK_ReturnsZAtIndex )
{
    std::vector<double> x = { 1.0, 2.0, 3.0 };
    std::vector<double> y = { 10.0, 20.0, 30.0 };
    std::vector<double> z = { 100.0, 200.0, 300.0 };

    EXPECT_DOUBLE_EQ( 100.0, RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::POS_K, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_NegK_ReturnsZAtIndex )
{
    std::vector<double> x = { 1.0, 2.0, 3.0 };
    std::vector<double> y = { 10.0, 20.0, 30.0 };
    std::vector<double> z = { 100.0, 200.0, 300.0 };

    EXPECT_DOUBLE_EQ( 300.0, RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::NEG_K, ActiveCellIndex( 2 ) ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_NoFace_ReturnsInfinity )
{
    std::vector<double> x = { 1.0 };
    std::vector<double> y = { 2.0 };
    std::vector<double> z = { 3.0 };

    double result = RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::NO_FACE, ActiveCellIndex( 0 ) );

    EXPECT_TRUE( std::isinf( result ) );
}

TEST( RigWellTargetMappingToolsTest, GetValueForFace_PosI_NegI_ReturnSameXValue )
{
    // Both I-directions should return from the same x vector
    std::vector<double> x = { 5.0 };
    std::vector<double> y = { 50.0 };
    std::vector<double> z = { 500.0 };

    double posI = RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::POS_I, ActiveCellIndex( 0 ) );
    double negI = RigWellTargetMappingTools::getValueForFace( x, y, z, FaceType::NEG_I, ActiveCellIndex( 0 ) );

    EXPECT_DOUBLE_EQ( posI, negI );
}

//==================================================================================================
// getTransmissibilityValueForFace
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_PosI_UsesCellIndex )
{
    std::vector<double> x = { 5.0, 15.0, 25.0 };
    std::vector<double> y = { 6.0, 16.0, 26.0 };
    std::vector<double> z = { 7.0, 17.0, 27.0 };

    double result =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::POS_I, ActiveCellIndex( 0 ), ActiveCellIndex( 2 ) );

    EXPECT_DOUBLE_EQ( 5.0, result ); // x[0]
}

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_NegI_UsesNeighborIndex )
{
    std::vector<double> x = { 5.0, 15.0, 25.0 };
    std::vector<double> y = { 6.0, 16.0, 26.0 };
    std::vector<double> z = { 7.0, 17.0, 27.0 };

    double result =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::NEG_I, ActiveCellIndex( 0 ), ActiveCellIndex( 2 ) );

    EXPECT_DOUBLE_EQ( 25.0, result ); // x[2] (neighbor)
}

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_PosJ_UsesCellIndex )
{
    std::vector<double> x = { 5.0, 15.0 };
    std::vector<double> y = { 6.0, 16.0 };
    std::vector<double> z = { 7.0, 17.0 };

    double result =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::POS_J, ActiveCellIndex( 0 ), ActiveCellIndex( 1 ) );

    EXPECT_DOUBLE_EQ( 6.0, result ); // y[0]
}

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_NegJ_UsesNeighborIndex )
{
    std::vector<double> x = { 5.0, 15.0 };
    std::vector<double> y = { 6.0, 16.0 };
    std::vector<double> z = { 7.0, 17.0 };

    double result =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::NEG_J, ActiveCellIndex( 0 ), ActiveCellIndex( 1 ) );

    EXPECT_DOUBLE_EQ( 16.0, result ); // y[1] (neighbor)
}

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_PosK_UsesCellIndex )
{
    std::vector<double> x = { 5.0, 15.0 };
    std::vector<double> y = { 6.0, 16.0 };
    std::vector<double> z = { 7.0, 17.0 };

    double result =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::POS_K, ActiveCellIndex( 0 ), ActiveCellIndex( 1 ) );

    EXPECT_DOUBLE_EQ( 7.0, result ); // z[0]
}

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_NegK_UsesNeighborIndex )
{
    std::vector<double> x = { 5.0, 15.0 };
    std::vector<double> y = { 6.0, 16.0 };
    std::vector<double> z = { 7.0, 17.0 };

    double result =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::NEG_K, ActiveCellIndex( 0 ), ActiveCellIndex( 1 ) );

    EXPECT_DOUBLE_EQ( 17.0, result ); // z[1] (neighbor)
}

TEST( RigWellTargetMappingToolsTest, GetTransmissibilityValueForFace_SameIndex_SameValueForAllDirections )
{
    // When cell and neighbor share the same result index, direction does not matter
    std::vector<double> x = { 9.0 };
    std::vector<double> y = { 19.0 };
    std::vector<double> z = { 29.0 };

    double posI =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::POS_I, ActiveCellIndex( 0 ), ActiveCellIndex( 0 ) );
    double negI =
        RigWellTargetMappingTools::getTransmissibilityValueForFace( x, y, z, FaceType::NEG_I, ActiveCellIndex( 0 ), ActiveCellIndex( 0 ) );

    EXPECT_DOUBLE_EQ( posI, negI );
}

//==================================================================================================
// getOilVectorName
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, GetOilVectorName_ReservoirVolumes )
{
    EXPECT_EQ( QString( "RFIPOIL" ), RigWellTargetMappingTools::getOilVectorName( VolumesType::RESERVOIR_VOLUMES ) );
}

TEST( RigWellTargetMappingToolsTest, GetOilVectorName_SurfaceVolumesSfip )
{
    EXPECT_EQ( QString( "SFIPOIL" ), RigWellTargetMappingTools::getOilVectorName( VolumesType::SURFACE_VOLUMES_SFIP ) );
}

TEST( RigWellTargetMappingToolsTest, GetOilVectorName_SurfaceVolumesFip )
{
    EXPECT_EQ( QString( "FIPOIL" ), RigWellTargetMappingTools::getOilVectorName( VolumesType::SURFACE_VOLUMES_FIP ) );
}

TEST( RigWellTargetMappingToolsTest, GetOilVectorName_ReservoirVolumesComputed )
{
    EXPECT_EQ( RiaResultNames::riPorvSoil(), RigWellTargetMappingTools::getOilVectorName( VolumesType::RESERVOIR_VOLUMES_COMPUTED ) );
}

//==================================================================================================
// getGasVectorName
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, GetGasVectorName_ReservoirVolumes )
{
    EXPECT_EQ( QString( "RFIPGAS" ), RigWellTargetMappingTools::getGasVectorName( VolumesType::RESERVOIR_VOLUMES ) );
}

TEST( RigWellTargetMappingToolsTest, GetGasVectorName_SurfaceVolumesSfip )
{
    EXPECT_EQ( QString( "SFIPGAS" ), RigWellTargetMappingTools::getGasVectorName( VolumesType::SURFACE_VOLUMES_SFIP ) );
}

TEST( RigWellTargetMappingToolsTest, GetGasVectorName_SurfaceVolumesFip )
{
    EXPECT_EQ( QString( "FIPGAS" ), RigWellTargetMappingTools::getGasVectorName( VolumesType::SURFACE_VOLUMES_FIP ) );
}

TEST( RigWellTargetMappingToolsTest, GetGasVectorName_ReservoirVolumesComputed )
{
    EXPECT_EQ( RiaResultNames::riPorvSgas(), RigWellTargetMappingTools::getGasVectorName( VolumesType::RESERVOIR_VOLUMES_COMPUTED ) );
}

//==================================================================================================
// isSaturationSufficient
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_OilAboveThreshold_ReturnsTrue )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.3 };
    data.saturationGas = { 0.0 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.1;

    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_OilBelowThreshold_ReturnsFalse )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.1 };
    data.saturationGas = { 0.0 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.1;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 1 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_OilExactlyAtThreshold_ReturnsTrue )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.2 };
    data.saturationGas = { 0.0 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.1;

    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_GasAboveThreshold_ReturnsTrue )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.0 };
    data.saturationGas = { 0.5 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::GAS, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_GasBelowThreshold_ReturnsFalse )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.0 };
    data.saturationGas = { 0.1 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::GAS, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_GasExactlyAtThreshold_ReturnsTrue )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.0 };
    data.saturationGas = { 0.3 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::GAS, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_Hydrocarbon_OilSufficient_ReturnsTrue )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.4 };
    data.saturationGas = { 0.05 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::HYDROCARBON, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_Hydrocarbon_GasSufficient_ReturnsTrue )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.05 };
    data.saturationGas = { 0.5 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::HYDROCARBON, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_Hydrocarbon_NeitherSufficient_ReturnsFalse )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.05 };
    data.saturationGas = { 0.05 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::HYDROCARBON, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_OilType_HighGasIsIgnored_ReturnsFalse )
{
    // VolumeType::OIL only checks oil saturation — high gas must not rescue it
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.05 };
    data.saturationGas = { 0.9 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_GasType_HighOilIsIgnored_ReturnsFalse )
{
    // VolumeType::GAS only checks gas saturation — high oil must not rescue it
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.9 };
    data.saturationGas = { 0.05 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.3;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::GAS, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_ZeroSaturation_ReturnsFalse )
{
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.0 };
    data.saturationGas = { 0.0 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.01;
    limits.saturationGas = 0.01;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 0 ) ) );
    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::GAS, data, limits, ActiveCellIndex( 0 ) ) );
    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::HYDROCARBON, data, limits, ActiveCellIndex( 0 ) ) );
}

TEST( RigWellTargetMappingToolsTest, IsSaturationSufficient_SecondIndex_UsesCorrectElement )
{
    // Verify the index selects the right element from the vectors
    RigWellTargetMappingTools::DataContainer data;
    data.saturationOil = { 0.05, 0.5 }; // index 0 is below limit, index 1 is above
    data.saturationGas = { 0.05, 0.05 };

    RigWellTargetMapping::ClusteringLimits limits{};
    limits.saturationOil = 0.2;
    limits.saturationGas = 0.2;

    EXPECT_FALSE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 0 ) ) );
    EXPECT_TRUE( RigWellTargetMappingTools::isSaturationSufficient( VolumeType::OIL, data, limits, ActiveCellIndex( 1 ) ) );
}

//==================================================================================================
// assignClusterIdToCells
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, AssignClusterIdToCells_AllActiveCells_SetsCorrectIds )
{
    RigActiveCellInfo activeCellInfo;
    activeCellInfo.setReservoirCellCount( 3 );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 0 ), ActiveCellIndex( 0 ) );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 1 ), ActiveCellIndex( 1 ) );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 2 ), ActiveCellIndex( 2 ) );

    std::vector<int>                clusters( 3, 0 );
    std::vector<ReservoirCellIndex> cells = { ReservoirCellIndex( 0 ), ReservoirCellIndex( 1 ), ReservoirCellIndex( 2 ) };

    RigWellTargetMappingTools::assignClusterIdToCells( activeCellInfo, cells, clusters, 5 );

    EXPECT_EQ( 5, clusters[0] );
    EXPECT_EQ( 5, clusters[1] );
    EXPECT_EQ( 5, clusters[2] );
}

TEST( RigWellTargetMappingToolsTest, AssignClusterIdToCells_InactiveCell_IsSkipped )
{
    RigActiveCellInfo activeCellInfo;
    activeCellInfo.setReservoirCellCount( 3 );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 0 ), ActiveCellIndex( 0 ) );
    // reservoir cells 1 and 2 have no result index (inactive)

    std::vector<int>                clusters( 1, 0 );
    std::vector<ReservoirCellIndex> cells = { ReservoirCellIndex( 0 ), ReservoirCellIndex( 1 ), ReservoirCellIndex( 2 ) };

    RigWellTargetMappingTools::assignClusterIdToCells( activeCellInfo, cells, clusters, 7 );

    EXPECT_EQ( 7, clusters[0] ); // cell 0 is active → updated
}

TEST( RigWellTargetMappingToolsTest, AssignClusterIdToCells_EmptyCells_ClustersUnchanged )
{
    RigActiveCellInfo activeCellInfo;
    activeCellInfo.setReservoirCellCount( 2 );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 0 ), ActiveCellIndex( 0 ) );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 1 ), ActiveCellIndex( 1 ) );

    std::vector<int>                clusters( 2, 99 );
    std::vector<ReservoirCellIndex> cells;

    RigWellTargetMappingTools::assignClusterIdToCells( activeCellInfo, cells, clusters, 1 );

    EXPECT_EQ( 99, clusters[0] );
    EXPECT_EQ( 99, clusters[1] );
}

TEST( RigWellTargetMappingToolsTest, AssignClusterIdToCells_OverwritesExistingClusterId )
{
    RigActiveCellInfo activeCellInfo;
    activeCellInfo.setReservoirCellCount( 2 );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 0 ), ActiveCellIndex( 0 ) );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 1 ), ActiveCellIndex( 1 ) );

    std::vector<int>                clusters = { 3, 3 };
    std::vector<ReservoirCellIndex> cells    = { ReservoirCellIndex( 0 ), ReservoirCellIndex( 1 ) };

    RigWellTargetMappingTools::assignClusterIdToCells( activeCellInfo, cells, clusters, 9 );

    EXPECT_EQ( 9, clusters[0] );
    EXPECT_EQ( 9, clusters[1] );
}

TEST( RigWellTargetMappingToolsTest, AssignClusterIdToCells_MixedActiveInactive_OnlyActiveUpdated )
{
    RigActiveCellInfo activeCellInfo;
    activeCellInfo.setReservoirCellCount( 4 );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 0 ), ActiveCellIndex( 0 ) );
    // cell 1 inactive
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 2 ), ActiveCellIndex( 1 ) );
    // cell 3 inactive

    std::vector<int> clusters( 2, 0 );
    std::vector<ReservoirCellIndex> cells = { ReservoirCellIndex( 0 ), ReservoirCellIndex( 1 ), ReservoirCellIndex( 2 ), ReservoirCellIndex( 3 ) };

    RigWellTargetMappingTools::assignClusterIdToCells( activeCellInfo, cells, clusters, 4 );

    EXPECT_EQ( 4, clusters[0] ); // result index 0 → updated
    EXPECT_EQ( 4, clusters[1] ); // result index 1 → updated
}

TEST( RigWellTargetMappingToolsTest, AssignClusterIdToCells_SingleCell_SetsId )
{
    RigActiveCellInfo activeCellInfo;
    activeCellInfo.setReservoirCellCount( 1 );
    activeCellInfo.setCellResultIndex( ReservoirCellIndex( 0 ), ActiveCellIndex( 0 ) );

    std::vector<int>                clusters( 1, 0 );
    std::vector<ReservoirCellIndex> cells = { ReservoirCellIndex( 0 ) };

    RigWellTargetMappingTools::assignClusterIdToCells( activeCellInfo, cells, clusters, 42 );

    EXPECT_EQ( 42, clusters[0] );
}

//==================================================================================================
// nncConnectionCellAndResult
//==================================================================================================

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_NoConnections_ReturnsEmpty )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 0 ), mainGrid.p() );

    EXPECT_TRUE( result.empty() );
}

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_MatchingCell_ReturnsConnection )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)3, (size_t)7, FaceType::POS_I ) );

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 3 ), mainGrid.p() );

    ASSERT_EQ( 1u, result.size() );
    auto& [cellInfo, nncIdx] = result.front();
    EXPECT_EQ( 7u, cellInfo.first.value() );
    EXPECT_EQ( FaceType::POS_I, cellInfo.second );
    EXPECT_EQ( 0u, nncIdx );
}

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_NonMatchingCell_ReturnsEmpty )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)5, (size_t)8, FaceType::NEG_J ) );

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 0 ), mainGrid.p() );

    EXPECT_TRUE( result.empty() );
}

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_CellIsC2NotC1_NotReturned )
{
    // The function only matches on c1GlobIdx, not c2GlobIdx.
    // RigConnection always stores the smaller index as c1, so we need c1 < 3 to ensure cell 3 is stored as c2.
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)1, (size_t)3, FaceType::POS_J ) );

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 3 ), mainGrid.p() );

    EXPECT_TRUE( result.empty() );
}

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_MultipleConnections_ReturnsOnlyMatching )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)2, (size_t)10, FaceType::POS_I ) );
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)4, (size_t)11, FaceType::POS_J ) ); // different source
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)2, (size_t)12, FaceType::NEG_K ) );

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 2 ), mainGrid.p() );

    ASSERT_EQ( 2u, result.size() );

    auto it = result.begin();
    EXPECT_EQ( 10u, it->first.first.value() );
    EXPECT_EQ( 0u, it->second ); // position 0 in connections array
    ++it;
    EXPECT_EQ( 12u, it->first.first.value() );
    EXPECT_EQ( 2u, it->second ); // position 2 in connections array
}

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_NncIndexReflectsPositionInArray )
{
    // NNC index must be the position in the global connections array, not the position in the result list.
    // RigConnection always stores the smaller index as c1, so cell 7 must be smaller than its neighbor to be c1.
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)1, (size_t)9, FaceType::POS_I ) ); // idx 0 — no match (c1=1)
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)2, (size_t)9, FaceType::POS_J ) ); // idx 1 — no match (c1=2)
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)7, (size_t)11, FaceType::NEG_I ) ); // idx 2 — match (c1=7)
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)7, (size_t)12, FaceType::NEG_J ) ); // idx 3 — match (c1=7)

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 7 ), mainGrid.p() );

    ASSERT_EQ( 2u, result.size() );

    auto it = result.begin();
    EXPECT_EQ( 2u, it->second ); // global array index 2
    ++it;
    EXPECT_EQ( 3u, it->second ); // global array index 3
}

TEST( RigWellTargetMappingToolsTest, NncConnectionCellAndResult_StoredFaceTypeIsPreserved )
{
    cvf::ref<RigMainGrid> mainGrid = new RigMainGrid;
    mainGrid->nncData()->allConnections().push_back( RigConnection( (size_t)1, (size_t)2, FaceType::NEG_K ) );

    auto result = RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex( 1 ), mainGrid.p() );

    ASSERT_EQ( 1u, result.size() );
    EXPECT_EQ( FaceType::NEG_K, result.front().first.second );
}
