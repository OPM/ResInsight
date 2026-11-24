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

#include "RiaFileSearchTools.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Creates a symbolic link to a folder. Returns false if a folder link is not supported, for example
/// on Windows where QFile::link creates a shortcut file
//--------------------------------------------------------------------------------------------------
bool createFolderLink( const QString& targetFolder, const QString& linkPath )
{
    if ( !QFile::link( targetFolder, linkPath ) ) return false;

    QFileInfo linkInfo( linkPath );
    return linkInfo.isSymLink() && linkInfo.isDir();
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaFileSearchTools, SymlinkToParentFolderIsNotSearchedRecursively )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // root/eclipse/model/loop -> root/eclipse
    QDir rootDir( tempDir.path() );
    ASSERT_TRUE( rootDir.mkpath( "eclipse/model" ) );

    const QString eclipseFolder = rootDir.absoluteFilePath( "eclipse" );
    const QString modelFolder   = rootDir.absoluteFilePath( "eclipse/model" );
    if ( !createFolderLink( eclipseFolder, modelFolder + "/loop" ) )
    {
        GTEST_SKIP() << "Symbolic links to folders are not supported";
    }

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( eclipseFolder, "*", matchingFolders );

    EXPECT_EQ( 2, matchingFolders.size() );
    EXPECT_TRUE( matchingFolders.contains( eclipseFolder ) );
    EXPECT_TRUE( matchingFolders.contains( modelFolder ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaFileSearchTools, SymlinkToFolderOutsideSearchPathIsFollowed )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // root/search/linked -> root/outside, and root/outside/sub
    QDir rootDir( tempDir.path() );
    ASSERT_TRUE( rootDir.mkpath( "search" ) );
    ASSERT_TRUE( rootDir.mkpath( "outside/sub" ) );

    const QString searchFolder = rootDir.absoluteFilePath( "search" );
    if ( !createFolderLink( rootDir.absoluteFilePath( "outside" ), searchFolder + "/linked" ) )
    {
        GTEST_SKIP() << "Symbolic links to folders are not supported";
    }

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( searchFolder, "*", matchingFolders );

    EXPECT_EQ( 3, matchingFolders.size() );
    EXPECT_TRUE( matchingFolders.contains( searchFolder ) );
    EXPECT_TRUE( matchingFolders.contains( searchFolder + "/linked" ) );
    EXPECT_TRUE( matchingFolders.contains( searchFolder + "/linked/sub" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaFileSearchTools, SiblingSymlinksToSameOutsideFolderAreBothFollowed )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // root/search/linkA -> root/outside, root/search/linkB -> root/outside, and root/outside/sub
    QDir rootDir( tempDir.path() );
    ASSERT_TRUE( rootDir.mkpath( "search" ) );
    ASSERT_TRUE( rootDir.mkpath( "outside/sub" ) );

    const QString searchFolder  = rootDir.absoluteFilePath( "search" );
    const QString outsideFolder = rootDir.absoluteFilePath( "outside" );
    if ( !createFolderLink( outsideFolder, searchFolder + "/linkA" ) || !createFolderLink( outsideFolder, searchFolder + "/linkB" ) )
    {
        GTEST_SKIP() << "Symbolic links to folders are not supported";
    }

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( searchFolder, "*", matchingFolders );

    EXPECT_EQ( 5, matchingFolders.size() );
    EXPECT_TRUE( matchingFolders.contains( searchFolder ) );
    EXPECT_TRUE( matchingFolders.contains( searchFolder + "/linkA" ) );
    EXPECT_TRUE( matchingFolders.contains( searchFolder + "/linkA/sub" ) );
    EXPECT_TRUE( matchingFolders.contains( searchFolder + "/linkB" ) );
    EXPECT_TRUE( matchingFolders.contains( searchFolder + "/linkB/sub" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaFileSearchTools, SymlinkToGrandparentFolderIsNotSearchedRecursively )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // root/eclipse/model/include/loop -> root/eclipse
    QDir rootDir( tempDir.path() );
    ASSERT_TRUE( rootDir.mkpath( "eclipse/model/include" ) );

    const QString eclipseFolder = rootDir.absoluteFilePath( "eclipse" );
    const QString modelFolder   = rootDir.absoluteFilePath( "eclipse/model" );
    const QString includeFolder = rootDir.absoluteFilePath( "eclipse/model/include" );
    if ( !createFolderLink( eclipseFolder, includeFolder + "/loop" ) )
    {
        GTEST_SKIP() << "Symbolic links to folders are not supported";
    }

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( eclipseFolder, "*", matchingFolders );

    EXPECT_EQ( 3, matchingFolders.size() );
    EXPECT_TRUE( matchingFolders.contains( eclipseFolder ) );
    EXPECT_TRUE( matchingFolders.contains( modelFolder ) );
    EXPECT_TRUE( matchingFolders.contains( includeFolder ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaFileSearchTools, SymlinksBetweenSiblingFoldersAreNotSearchedRecursively )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // root/search/a/toB -> root/search/b, root/search/b/toA -> root/search/a
    QDir rootDir( tempDir.path() );
    ASSERT_TRUE( rootDir.mkpath( "search/a" ) );
    ASSERT_TRUE( rootDir.mkpath( "search/b" ) );

    const QString searchFolder = rootDir.absoluteFilePath( "search" );
    const QString aFolder      = searchFolder + "/a";
    const QString bFolder      = searchFolder + "/b";
    if ( !createFolderLink( bFolder, aFolder + "/toB" ) || !createFolderLink( aFolder, bFolder + "/toA" ) )
    {
        GTEST_SKIP() << "Symbolic links to folders are not supported";
    }

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( searchFolder, "*", matchingFolders );

    // Each link is followed once, and the link back to a folder already on the search path is skipped
    EXPECT_EQ( 5, matchingFolders.size() );
    EXPECT_TRUE( matchingFolders.contains( searchFolder ) );
    EXPECT_TRUE( matchingFolders.contains( aFolder ) );
    EXPECT_TRUE( matchingFolders.contains( aFolder + "/toB" ) );
    EXPECT_TRUE( matchingFolders.contains( bFolder ) );
    EXPECT_TRUE( matchingFolders.contains( bFolder + "/toA" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaFileSearchTools, SymlinkToParentFolderWithPathFilter )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    // root/realization-0/eclipse/model/loop -> root/realization-0/eclipse
    QDir rootDir( tempDir.path() );
    ASSERT_TRUE( rootDir.mkpath( "realization-0/eclipse/model" ) );

    const QString eclipseFolder = rootDir.absoluteFilePath( "realization-0/eclipse" );
    const QString modelFolder   = rootDir.absoluteFilePath( "realization-0/eclipse/model" );
    if ( !createFolderLink( eclipseFolder, modelFolder + "/loop" ) )
    {
        GTEST_SKIP() << "Symbolic links to folders are not supported";
    }

    QStringList matchingFolders;
    RiaFileSearchTools::findMatchingFoldersRecursively( rootDir.absolutePath(), "realization-*/eclipse/model/*", matchingFolders );

    ASSERT_EQ( 1, matchingFolders.size() );
    EXPECT_EQ( modelFolder, matchingFolders.front() );
}
