#include "gtest/gtest.h"

#include "RiaEnsembleNameTools.h"
#include "RiaFilePathTools.h"

#include <iostream>

std::ostream& operator<<( std::ostream& out, const QString& text )
{
    out << text.toStdString();
    return out;
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, rootSearchPathFromSearchFilter )
{
    {
        QString testPath( "" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "" ), resultRootPath );
    }

    {
        QString testPath( "D:/" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "D:/" ), resultRootPath );
    }
    {
        QString testPath( "D:/A" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "D:/A" ), resultRootPath );
    }

    {
        QString testPath( "D:/A/B[cd]/E" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "D:/A" ), resultRootPath );
    }
    {
        QString testPath( "/A/B[cd]/E" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "/A" ), resultRootPath );
    }
    {
        QString testPath( "/A/B?/E" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "/A" ), resultRootPath );
    }
    {
        QString testPath( "//A/B/E*" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B" ), resultRootPath );
    }
    {
        QString testPath( "//A/B/E" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B/E" ), resultRootPath );
    }
    {
        QString testPath( "//A/B/E/" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B/E/" ), resultRootPath );
    }

    {
        QString testPath( "//A/B[[]/E/" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B[[]/E/" ), resultRootPath );
    }
    {
        QString testPath( "//A/B[]]/E/" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B[]]/E/" ), resultRootPath );
    }
    {
        QString testPath( "//A/B[*]/E/" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B[*]/E/" ), resultRootPath );
    }
    {
        QString testPath( "//A/B[?]/E/" );
        QString resultRootPath = RiaFilePathTools::rootSearchPathFromSearchFilter( testPath );
        EXPECT_EQ( QString( "//A/B[?]/E/" ), resultRootPath );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, removeDuplicatePathSeparators )
{
    {
        QString testPath( "//myshare/folder-a/folder-b/" );
        QString resultRootPath = RiaFilePathTools::removeDuplicatePathSeparators( testPath );
        EXPECT_STRCASEEQ( testPath.toLatin1(), resultRootPath.toLatin1() );
    }

    {
        QString testPath( "//myshare/folder-a//folder-b/" );
        QString expectedPath( "//myshare/folder-a/folder-b/" );
        QString resultRootPath = RiaFilePathTools::removeDuplicatePathSeparators( testPath );
        EXPECT_STRCASEEQ( expectedPath.toLatin1(), resultRootPath.toLatin1() );
    }

    {
        QString testPath( R"(\\myshare\folder-a\folder-b\)" );
        QString resultRootPath = RiaFilePathTools::removeDuplicatePathSeparators( testPath );
        EXPECT_STRCASEEQ( testPath.toLatin1(), resultRootPath.toLatin1() );
    }

    {
        QString testPath( R"(\\myshare\folder-a\\folder-b\\)" );
        QString expectedPath( R"(\\myshare\folder-a\folder-b\)" );
        QString resultRootPath = RiaFilePathTools::removeDuplicatePathSeparators( testPath );
        EXPECT_STRCASEEQ( expectedPath.toLatin1(), resultRootPath.toLatin1() );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, splitIntoComponets )
{
    {
        QString testPath( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );

        auto words = RiaFilePathTools::splitPathIntoComponents( testPath );

        EXPECT_EQ( 8, words.size() );

        EXPECT_EQ( QString( "models" ), words[0] );
        EXPECT_EQ( QString( "from_equinor_sftp" ), words[1] );
        EXPECT_EQ( QString( "drogon3d_ahm" ), words[2] );
        EXPECT_EQ( QString( "realization-0" ), words[3] );
        EXPECT_EQ( QString( "iter-3" ), words[4] );
        EXPECT_EQ( QString( "eclipse" ), words[5] );
        EXPECT_EQ( QString( "model" ), words[6] );
        EXPECT_EQ( QString( "DROGON-0.SMSPEC" ), words[7] );
    }

    {
        QString testPath( "/home/builder/models/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );

        auto words = RiaFilePathTools::splitPathIntoComponents( testPath );

        EXPECT_EQ( 8, words.size() );

        EXPECT_EQ( QString( "home" ), words[0] );
        EXPECT_EQ( QString( "builder" ), words[1] );
        EXPECT_EQ( QString( "models" ), words[2] );
        EXPECT_EQ( QString( "realization-0" ), words[3] );
        EXPECT_EQ( QString( "iter-3" ), words[4] );
        EXPECT_EQ( QString( "eclipse" ), words[5] );
        EXPECT_EQ( QString( "model" ), words[6] );
        EXPECT_EQ( QString( "DROGON-0.SMSPEC" ), words[7] );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, EnsembleName )
{
    {
        QString     testPath1( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );
        QString     testPath2( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-1/iter-3/eclipse/model/DROGON-1.SMSPEC" );
        QStringList allPaths = { testPath1, testPath2 };

        auto ensembleName =
            RiaEnsembleNameTools::findSuitableEnsembleName( allPaths, RiaEnsembleNameTools::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE );

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
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, keyPathComponentsForEachFilePath )
{
    {
        QString     testPath0( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );
        QString     testPath1( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-1/iter-3/eclipse/model/DROGON-1.SMSPEC" );
        QStringList allPaths = { testPath0, testPath1 };

        auto keyComponents = RiaFilePathTools::keyPathComponentsForEachFilePath( allPaths );

        auto test0 = keyComponents[testPath0];
        EXPECT_EQ( QString( "realization-0" ), test0.front() );

        auto test1 = keyComponents[testPath1];
        EXPECT_EQ( QString( "realization-1" ), test1.front() );
    }
}
