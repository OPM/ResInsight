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
