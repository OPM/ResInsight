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
#include <QDir>
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
