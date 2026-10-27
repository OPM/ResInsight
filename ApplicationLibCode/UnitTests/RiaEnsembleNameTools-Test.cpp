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

#include <QStandardItemModel>
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
// Some FMU-like data sets skip the "iter-N" folder level and place the case files directly inside
// "realization-N". Ensemble grouping mode NONE should still create a single best-effort ensemble
// containing all the files, regardless of the folder structure used.
//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, EnsembleGroupingNoneFlatRealizationStructure )
{
    QStringList fileNames = { "f:/scratch/grid_ensemble/realization-1/SIMPLE_R1.EGRID",
                              "f:/scratch/grid_ensemble/realization-2/SIMPLE_R2.EGRID",
                              "f:/scratch/grid_ensemble/realization-3/SIMPLE_R3.EGRID" };

    auto grouping = RiaEnsembleNameTools::groupFilesByEnsembleName( fileNames, RiaDefines::EnsembleGroupingMode::NONE );

    ASSERT_EQ( 1u, grouping.size() );

    const auto& [groupName, groupedFileNames] = *grouping.begin();
    EXPECT_FALSE( groupName.isEmpty() );
    EXPECT_EQ( 3, groupedFileNames.size() );
    for ( const auto& fileName : fileNames )
    {
        EXPECT_TRUE( groupedFileNames.contains( fileName ) );
    }
}

//--------------------------------------------------------------------------------------------------
// groupFilesByEnsembleName with an empty input list must not create a bogus ensemble entry.
//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, EnsembleGroupingNoneEmptyInput )
{
    QStringList fileNames;

    auto grouping = RiaEnsembleNameTools::groupFilesByEnsembleName( fileNames, RiaDefines::EnsembleGroupingMode::NONE );

    EXPECT_TRUE( grouping.empty() );
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

namespace
{
QStandardItem* addEnsemble( QStandardItemModel& model, const QStringList& paths )
{
    auto* ensembleItem = new QStandardItem( "ensemble" );
    ensembleItem->setCheckable( true );
    ensembleItem->setCheckState( Qt::Checked );
    model.invisibleRootItem()->appendRow( ensembleItem );

    for ( const auto& path : paths )
    {
        auto* child = new QStandardItem( path );
        child->setCheckable( true );
        child->setCheckState( Qt::Checked );
        ensembleItem->appendRow( child );
    }
    return ensembleItem;
}

const auto pathFromText = []( QStandardItem* item ) { return item->text(); };
} // namespace

//--------------------------------------------------------------------------------------------------
TEST( RiaEnsembleNameToolsTests, stripTrailingRealizationNumberOnlyStripsMatchingRealization )
{
    EXPECT_EQ( "WELL", RiaEnsembleNameTools::stripTrailingRealizationNumber( "WELL_001", "/r/realization-1/WELL_001.LAS" ).toStdString() );
    EXPECT_EQ( "WELL-3", RiaEnsembleNameTools::stripTrailingRealizationNumber( "WELL-3", "/r/realization-2/WELL-3.LAS" ).toStdString() );
    EXPECT_EQ( "WELL_A", RiaEnsembleNameTools::stripTrailingRealizationNumber( "WELL_A1", "/r/WELL_A1.LAS" ).toStdString() );
    EXPECT_EQ( "WELL", RiaEnsembleNameTools::stripTrailingRealizationNumber( "WELL-3", "/r/myrealization-12/WELL-3.LAS" ).toStdString() );
    EXPECT_EQ( "1", RiaEnsembleNameTools::stripTrailingRealizationNumber( "1", "/r/realization-1/1.LAS" ).toStdString() );
}

//--------------------------------------------------------------------------------------------------
TEST( RiaEnsembleNameToolsTests, uncheckFilenameOutliersUnchecksMinority )
{
    QStandardItemModel model;
    auto*              ensemble =
        addEnsemble( model,
                     { "realization-0/WELL_0.LAS", "realization-1/WELL_1.LAS", "realization-2/WELL_2.LAS", "realization-3/OTHER_3.LAS" } );

    auto warnings = RiaEnsembleNameTools::uncheckFilenameOutliers( model, pathFromText, "files", true );

    EXPECT_EQ( 1, warnings.size() );
    EXPECT_EQ( Qt::Checked, ensemble->child( 0 )->checkState() );
    EXPECT_EQ( Qt::Checked, ensemble->child( 1 )->checkState() );
    EXPECT_EQ( Qt::Checked, ensemble->child( 2 )->checkState() );
    EXPECT_EQ( Qt::Unchecked, ensemble->child( 3 )->checkState() );
}

//--------------------------------------------------------------------------------------------------
TEST( RiaEnsembleNameToolsTests, uncheckFilenameOutliersKeepsDistinctNamesWithDifferentTrailingNumbers )
{
    QStandardItemModel model;
    auto*              ensemble = addEnsemble( model, { "realization-1/OP_1.LAS", "realization-1/OP_2.LAS", "realization-1/OP_3.LAS" } );

    auto warnings = RiaEnsembleNameTools::uncheckFilenameOutliers( model, pathFromText, "files", true );

    EXPECT_TRUE( warnings.isEmpty() );
    for ( int i = 0; i < ensemble->rowCount(); i++ )
    {
        EXPECT_EQ( Qt::Checked, ensemble->child( i )->checkState() );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaEnsembleNameToolsTests, uncheckFilenameOutliersIgnoresUncheckedItemsAndTies )
{
    {
        // The unchecked WELL files must not outvote the checked OTHER files
        QStandardItemModel model;
        auto*              ensemble = addEnsemble( model,
                                                   { "realization-0/WELL_0.LAS",
                                                     "realization-1/WELL_1.LAS",
                                                     "realization-2/OTHER_2.LAS",
                                                     "realization-3/OTHER_3.LAS",
                                                     "realization-4/OTHER_4.LAS" } );
        ensemble->child( 0 )->setCheckState( Qt::Unchecked );
        ensemble->child( 1 )->setCheckState( Qt::Unchecked );
        ensemble->child( 4 )->setCheckState( Qt::Unchecked );

        auto warnings = RiaEnsembleNameTools::uncheckFilenameOutliers( model, pathFromText, "files", true );

        // Only OTHER files remain checked, so there is nothing to do
        EXPECT_TRUE( warnings.isEmpty() );
        EXPECT_EQ( Qt::Checked, ensemble->child( 2 )->checkState() );
        EXPECT_EQ( Qt::Checked, ensemble->child( 3 )->checkState() );
    }

    {
        QStandardItemModel model;
        auto*              ensemble = addEnsemble( model, { "realization-0/A_0.LAS", "realization-1/B_1.LAS" } );

        auto warnings = RiaEnsembleNameTools::uncheckFilenameOutliers( model, pathFromText, "files", true );

        EXPECT_TRUE( warnings.isEmpty() );
        EXPECT_EQ( Qt::Checked, ensemble->child( 0 )->checkState() );
        EXPECT_EQ( Qt::Checked, ensemble->child( 1 )->checkState() );
    }
}
