#include "gtest/gtest.h"

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

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, removeFileExtension )
{
    // A folder name can contain a dot. Only a file extension is removed, the folder name is left untouched.
    const QString pathWithExtension = "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX).ESMRY";
    EXPECT_EQ( QString( "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX)" ),
               RiaFilePathTools::removeFileExtension( pathWithExtension ) );

    // A path without a file extension must be returned unmodified, see https://github.com/OPM/ResInsight/issues/14470
    const QString pathWithoutExtension = "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX)";
    EXPECT_EQ( pathWithoutExtension, RiaFilePathTools::removeFileExtension( pathWithoutExtension ) );

#ifdef WIN32
    // A backslash is only recognized as a path separator on Windows
    const QString windowsPath = "d:\\scratch\\drogon.2024\\realization-0\\DROGON-0";
    EXPECT_EQ( windowsPath, RiaFilePathTools::removeFileExtension( windowsPath ) );
#endif

    // No folder part
    EXPECT_EQ( QString( "DROGON-0" ), RiaFilePathTools::removeFileExtension( "DROGON-0.SMSPEC" ) );

    // Empty path
    EXPECT_EQ( QString( "" ), RiaFilePathTools::removeFileExtension( "" ) );
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, replaceSubFolderInPath )
{
    {
        std::string testPath0( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/iter-3/eclipse/model/DROGON-0.SMSPEC" );
        std::string testPath1( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-1/iter-3/eclipse/model/DROGON-1.SMSPEC" );

        std::string expPath0( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-0/wp-0/eclipse/model/DROGON-0.SMSPEC" );
        std::string expPath1( "e:/models/from_equinor_sftp/drogon3d_ahm/realization-1/wp-0/eclipse/model/DROGON-1.SMSPEC" );

        auto newPath0 = RiaFilePathTools::replaceSubFolderInPath( testPath0, "iter-3", "wp-0" );
        auto newPath1 = RiaFilePathTools::replaceSubFolderInPath( testPath1, "iter-3", "wp-0" );

        EXPECT_EQ( expPath0, newPath0 );
        EXPECT_EQ( expPath1, newPath1 );
    }
}

//--------------------------------------------------------------------------------------------------
TEST( RiaFilePathTools, replaceExtension )
{
    // A folder name can contain a dot. Only a file extension is removed, the folder name is left untouched.
    const std::string pathWithExtension = "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX).ESMRY";
    EXPECT_EQ( std::string( "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX).DATA" ),
               RiaFilePathTools::replaceFileExtension( pathWithExtension, "DATA" ) );

    // A path without a file extension must be returned unmodified, see https://github.com/OPM/ResInsight/issues/14470
    const std::string pathWithoutExtension = "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX).DATA";
    EXPECT_EQ( pathWithoutExtension, RiaFilePathTools::replaceFileExtension( pathWithoutExtension, "DATA" ) );

#ifdef WIN32
    // A backslash is only recognized as a path separator on Windows
    const std::string windowsPath = "d:\\scratch\\drogon.2024\\realization-0\\DROGON-0.DATA";
    EXPECT_EQ( windowsPath, RiaFilePathTools::replaceFileExtension( windowsPath, "DATA" ) );
#endif

    // No folder part
    EXPECT_EQ( std::string( "DROGON-0.DATA" ), RiaFilePathTools::replaceFileExtension( "DROGON-0.SMSPEC", "DATA" ) );

    // Empty path
    EXPECT_EQ( std::string( "" ), RiaFilePathTools::replaceFileExtension( "", "DATA" ) );
}
