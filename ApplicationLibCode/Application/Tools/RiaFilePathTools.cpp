/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaFilePathTools.h"
#include "RiaTextStringTools.h"

#include "cafAssert.h"

#include <QDir>

#include <filesystem>
#include <memory>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QChar RiaFilePathTools::separator()
{
    return '/';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFilePathTools::toInternalSeparator( const QString& path )
{
    QString currNativeSep = QDir::separator();

    if ( currNativeSep == "/" )
    {
        // On Linux like system -> Do not convert separators
        return path;
    }

    // On other systems (i.e. Windows) -> Convert to internal separator (/)
    QString output = path;
    return output.replace( QString( "\\" ), separator() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString& RiaFilePathTools::appendSeparatorIfNo( QString& path )
{
    if ( !path.endsWith( separator() ) )
    {
        path.append( separator() );
    }
    return path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFilePathTools::relativePath( const QString& rootDir, const QString& dir )
{
    if ( dir.startsWith( rootDir ) )
    {
        QString relPath = dir;
        relPath.remove( 0, rootDir.size() );

        if ( relPath.startsWith( separator() ) ) relPath.remove( 0, 1 );
        return appendSeparatorIfNo( relPath );
    }
    else
    {
        return dir;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaFilePathTools::equalPaths( const QString& path1, const QString& path2 )
{
    QString p1 = path1;
    QString p2 = path2;
    appendSeparatorIfNo( p1 );
    appendSeparatorIfNo( p2 );
    return p1 == p2;
}

//--------------------------------------------------------------------------------------------------
/// Own canonicalPath method since the QDir::canonicalPath seems to not work
//--------------------------------------------------------------------------------------------------
QString RiaFilePathTools::canonicalPath( const QString& path )
{
    return QDir( path ).absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RiaFilePathTools::toFolderAndFileName( const QString& absFileName )
{
    auto absFN   = toInternalSeparator( absFileName );
    int  lastSep = absFN.lastIndexOf( separator() );
    if ( lastSep > 0 )
    {
        return std::make_pair( absFN.left( lastSep ), absFN.mid( lastSep + 1 ) );
    }
    else
    {
        return std::make_pair( "", absFN );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFilePathTools::removeDuplicatePathSeparators( const QString& path )
{
    QString correctedPath = path;
    QString prefix;

    QString doubleBackslash = R"(\\)";
    if ( correctedPath.size() > 2 )
    {
        QString prefixCandidate = correctedPath.left( 2 );
        if ( prefixCandidate == doubleBackslash || prefixCandidate == "//" )
        {
            prefix = prefixCandidate;

            correctedPath = correctedPath.right( correctedPath.size() - 2 );
        }
    }

    correctedPath.replace( QString( "%1%1" ).arg( separator() ), separator() );
    correctedPath.replace( doubleBackslash, R"(\)" );

    correctedPath = prefix + correctedPath;

    return correctedPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFilePathTools::rootSearchPathFromSearchFilter( const QString& searchFilter )
{
    QStringList pathPartList = searchFilter.split( separator() );

    QStringList::iterator pathPartIt = pathPartList.begin();

    for ( ; pathPartIt != pathPartList.end(); ++pathPartIt )
    {
        QString pathPart = *pathPartIt;

        // Remove allowed escaping of wildcards

        pathPart.replace( "[[]", "" );
        pathPart.replace( "[]]", "" );
        pathPart.replace( "[?]", "" );
        pathPart.replace( "[*]", "" );

        if ( pathPart.contains( "*" ) ) break;
        if ( pathPart.contains( "?" ) ) break;
        if ( pathPart.contains( "[" ) ) break;
    }

    pathPartList.erase( pathPartIt, pathPartList.end() );

    return pathPartList.join( separator() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaFilePathTools::commonRootOfFileNames( const QStringList& fileList )
{
    QStringList fileNameList;
    for ( auto filePath : fileList )
    {
        QFileInfo fileInfo( filePath );
        QString   fileNameWithoutExt = fileInfo.baseName();
        fileNameList.push_back( fileNameWithoutExt );
    }
    QString root = RiaTextStringTools::commonRoot( fileNameList );
    return root;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaFilePathTools::splitPathIntoComponents( const QString& inputPath, bool splitExtensionIntoSeparateEntry )
{
    auto path = QDir::cleanPath( inputPath );

    auto indexOfLastSeparator = path.lastIndexOf( separator() );
    auto indexOfDrive         = path.indexOf( ':' );

    QString pathWithoutDrive = path.mid( indexOfDrive + 1, indexOfLastSeparator - ( indexOfDrive + 1 ) );

    QStringList components = RiaTextStringTools::splitSkipEmptyParts( pathWithoutDrive, separator() );

    QFileInfo fileInfo( path );

    if ( splitExtensionIntoSeparateEntry )
    {
        QString extension = fileInfo.completeSuffix();
        path              = path.replace( QString( ".%1" ).arg( extension ), "" );
        components.push_back( extension );
        components.push_back( fileInfo.baseName() );
    }
    else
    {
        components.push_back( fileInfo.fileName() );
    }

    return components;
}

struct PathNode
{
    QString                              name;
    PathNode*                            parent;
    std::list<std::unique_ptr<PathNode>> children;
    QString                              fileName;

    PathNode( const QString& name, PathNode* parent )
        : name( name )
        , parent( parent )
    {
    }
};

void addToPathTree( PathNode* node, QStringList pathComponents, const QString& fileName )
{
    CAF_ASSERT( node );
    if ( !pathComponents.empty() )
    {
        QString pathComponent = pathComponents.front();
        pathComponents.pop_front();

        for ( auto it = node->children.begin(); it != node->children.end(); ++it )
        {
            if ( it->get()->name == pathComponent )
            {
                addToPathTree( it->get(), pathComponents, fileName );
                return;
            }
        }

        node->children.push_back( std::unique_ptr<PathNode>( new PathNode( pathComponent, node ) ) );
        addToPathTree( node->children.back().get(), pathComponents, fileName );
    }
    else
    {
        // Reached leaf, just set file name
        node->fileName = fileName;
    }
}

void trimTree( PathNode* node )
{
    if ( node->children.size() == 1u )
    {
        // Unnecessary level. Remove it.
        std::unique_ptr<PathNode> singleChildNode = std::move( node->children.front() );
        node->children.clear();
        node->children.swap( singleChildNode->children );
        node->fileName = singleChildNode->fileName;

        // Re-parent children
        for ( auto it = node->children.begin(); it != node->children.end(); ++it )
        {
            it->get()->parent = node;
        }
        trimTree( node );
    }
    else
    {
        for ( auto it = node->children.begin(); it != node->children.end(); ++it )
        {
            trimTree( it->get() );
        }
    }
}

void extractLeafNodes( PathNode* node, std::list<PathNode*>* leafNodes )
{
    if ( node->children.empty() )
    {
        leafNodes->push_back( node );
    }
    else
    {
        for ( auto it = node->children.begin(); it != node->children.end(); ++it )
        {
            extractLeafNodes( it->get(), leafNodes );
        }
    }
}

void pathToNode( PathNode* node, QStringList* path )
{
    CAF_ASSERT( path );

    if ( node != nullptr )
    {
        if ( !node->name.isEmpty() ) path->push_front( node->name );
        pathToNode( node->parent, path );
    }
}

//--------------------------------------------------------------------------------------------------
/// Takes a list of file paths and returns a map with the key components that separate the path
/// from the others.
//--------------------------------------------------------------------------------------------------
std::map<QString, QStringList> RiaFilePathTools::keyPathComponentsForEachFilePath( const QStringList& filePaths )
{
    std::map<QString, QStringList> allComponents;

    for ( auto fileName : filePaths )
    {
        QStringList pathComponentsForFile = splitPathIntoComponents( fileName, true );
        allComponents[fileName]           = pathComponentsForFile;
    }

    auto topNode = std::unique_ptr<PathNode>( new PathNode( "", nullptr ) );

    for ( auto keyComponentsPair : allComponents )
    {
        addToPathTree( topNode.get(), keyComponentsPair.second, keyComponentsPair.first );
    }

    trimTree( topNode.get() );
    std::list<PathNode*> leafNodes;
    extractLeafNodes( topNode.get(), &leafNodes );

    std::map<QString, QStringList> keyComponents;
    for ( PathNode* node : leafNodes )
    {
        QStringList path;
        pathToNode( node, &path );
        keyComponents[node->fileName] = path;
    }

    return keyComponents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaFilePathTools::isFirstOlderThanSecond( const std::string& firstFileName, const std::string& secondFileName )
{
    if ( !std::filesystem::exists( firstFileName ) || !std::filesystem::exists( secondFileName ) ) return false;

    auto timeFirstFile  = std::filesystem::last_write_time( firstFileName );
    auto timeSecondFile = std::filesystem::last_write_time( secondFileName );

    return ( timeFirstFile < timeSecondFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaFilePathTools::makeSuitableAsFileName( const std::string candidateName )
{
    if ( candidateName.empty() ) return "noname";

    QString tmp = QString::fromStdString( candidateName );

    tmp.replace( ' ', '_' );
    tmp.replace( '/', '_' );
    tmp.replace( '\\', '_' );
    tmp.replace( ':', '_' );
    tmp.replace( '&', '_' );
    tmp.replace( '|', '_' );

    return tmp.toStdString();
}
