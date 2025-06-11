/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaEnsembleNameTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaEnsembleNameToolsTests, commonBaseName )
{
    QStringList fileNames = { "/base/realization-0/iter-0/stimplan/output/B-H3_StimPlanModel_01_job-01/mainfrac/data_vs_time.csv",
                              "/base/realization-0/iter-0/stimplan/output/B-H3_StimPlanModel_02_job-02/mainfrac/data_vs_time.csv" };

    ASSERT_EQ( "data_vs_time", RiaEnsembleNameTools::findCommonBaseName( fileNames ).toStdString() );
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, EnsembleName )
{
    {
        QString     testPath1( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );
        QString     testPath2( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-1/iter-3/eclipse/model/DROGON-1.SMSPEC" );
        QStringList allPaths = { testPath1, testPath2 };

        auto ensembleName = RiaEnsembleNameTools::findSuitableEnsembleName( allPaths, RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE );

        EXPECT_EQ( QString( "iter-3" ), ensembleName );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, RealizationName )
{
    {
        QString     testPath0( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );
        QString     testPath1( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-1/iter-3/eclipse/model/DROGON-1.SMSPEC" );
        QStringList allPaths = { testPath0, testPath1 };

        QString fileName = "DROGON-0.SMSPEC";

        auto name = RiaEnsembleNameTools::uniqueShortName( testPath1, allPaths, fileName );

        EXPECT_EQ( QString( "real-1" ), name );
    }

    {
        QString     testPath0( "/drogon3d_ahm/run-0/DROGON-0.SMSPEC" );
        QString     testPath1( "/drogon3d_ahm/run-1/DROGON-1.SMSPEC" );
        QStringList allPaths = { testPath0, testPath1 };

        QString fileName = "DROGON-0.SMSPEC";

        auto name = RiaEnsembleNameTools::uniqueShortName( testPath1, allPaths, fileName );

        EXPECT_EQ( QString( "run-1" ), name );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, EnsembleGroupingMultipleEnsembles )
{
    {
        std::vector<std::string> filepaths = { "f:/Models/scratch/project_a/realization-0/iter-0/eclipse/model/PROJECT-0.SMSPEC",
                                               "f:/Models/scratch/project_a/realization-1/iter-0/eclipse/model/PROJECT-1.SMSPEC",
                                               "f:/Models/scratch/project_a/realization-22/iter-0/eclipse/model/PROJECT-22.SMSPEC",
                                               "f:/Models/scratch/project_b/realization-0/iter-0/eclipse/model/PROJECT-0.SMSPEC",
                                               "f:/Models/scratch/project_b/realization-1/iter-0/eclipse/model/PROJECT-1.SMSPEC",
                                               "f:/Models/scratch/project_b/realization-2/iter-0/eclipse/model/PROJECT-2.SMSPEC",
                                               "f:/Models/scratch/project_c/realization-0/iter-0/eclipse/model/PROJECT-0.SMSPEC",
                                               "f:/Models/scratch/project_c/realization-1/iter-0/eclipse/model/PROJECT-1.SMSPEC",
                                               "f:/Models/scratch/project_c/realization-2/iter-0/eclipse/model/PROJECT-2.SMSPEC",
                                               "f:/Models/scratch/project_c/realization-0/iter-1/eclipse/model/PROJECT-0.SMSPEC",
                                               "f:/Models/scratch/project_c/realization-1\\iter-1/eclipse/model\\PROJECT-1.SMSPEC",
                                               "f:/Models/scratch/project_c/realization-\\iter-1/eclipse/model\\PROJECT-2.SMSPEC" };

        auto grouping = RiaEnsembleNameTools::groupFilePathsFmu( filepaths );

        std::set<std::string> keys1 = { "project_a", "project_b", "project_c" };
        std::set<std::string> keys2 = { "iter-0", "iter-1" };

        EXPECT_EQ( 4, grouping.size() );
        for ( const auto& [keys, paths] : grouping )
        {
            const auto& [key1, key2] = keys;
            EXPECT_TRUE( keys1.find( key1 ) != keys1.end() );
            EXPECT_TRUE( keys2.find( key2 ) != keys2.end() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, EnsembleGroupingEverest )
{
    {
        std::vector<std::string> filepaths = {
            "f:/Models/scratch/my_case/batch_0/geo_realization_0/simulation_0/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_0/geo_realization_1/simulation_1/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_0/geo_realization_2/simulation_2/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_0/geo_realization_2/simulation_2/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_1/geo_realization_0/simulation_0/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_1/geo_realization_1/simulation_1/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_1/geo_realization_2/simulation_2/eclipse/model/PROJECT-0.SMSPEC",
            "f:/Models/scratch/my_case/batch_1/geo_realization_2/simulation_2/eclipse/model/PROJECT-0.SMSPEC",
        };

        auto grouping = RiaEnsembleNameTools::groupFilePathsEverest( filepaths );

        std::set<std::string> keys1 = { "my_case" };
        std::set<std::string> keys2 = { "batch_0", "batch_1" };

        for ( const auto& [keys, paths] : grouping )
        {
            const auto& [key1, key2] = keys;
            EXPECT_TRUE( keys1.find( key1 ) != keys1.end() );
            EXPECT_TRUE( keys2.find( key2 ) != keys2.end() );
        }
    }
}
