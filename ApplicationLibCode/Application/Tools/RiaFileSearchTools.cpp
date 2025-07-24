/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaFileSearchTools.h"

#include "RiaFilePathTools.h"

#include <QDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaFileSearchTools::findFilesInFolders( const QStringList&                           folders,
                                                    const QStringList&                           fileFilters,
                                                    const std::function<bool( const QString& )>& callback )
{
    QStringList allFiles;

    for ( const auto& folder : folders )
    {
        QDir        qdir( folder );
        QStringList files = qdir.entryList( fileFilters, QDir::Files );

        if ( callback && !callback( qdir.absolutePath() ) )
        {
            return allFiles;
        }

        for ( const auto& file : files )
        {
            QString absFilePath = qdir.absoluteFilePath( file );
            allFiles.append( absFilePath );
        }
    }
    return allFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileSearchTools::findMatchingFoldersRecursively( const QString&                               currentDir,
                                                         const QString&                               folderFilter,
                                                         QStringList&                                 matchingFolders,
                                                         const std::function<bool( const QString& )>& callback )
{
    if ( callback && !callback( currentDir ) )
    {
        return;
    }

    QStringList pathFilterPartList;
    if ( folderFilter.isEmpty() )
    {
        pathFilterPartList.push_back( "*" );
    }
    else
    {
        pathFilterPartList = folderFilter.split( RiaFilePathTools::separator() );
    }

    QDir        qdir( currentDir, pathFilterPartList[0], QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot );
    QStringList subDirs = qdir.entryList();

    if ( pathFilterPartList.size() == 1 && pathFilterPartList[0] == "*" )
    {
        matchingFolders.push_back( currentDir );
    }

    for ( const QString& subDir : subDirs )
    {
        QString fullPath = qdir.absoluteFilePath( subDir );
        QString nextPathFilter;

        if ( pathFilterPartList.size() == 1 && pathFilterPartList[0] == "*" )
        {
            nextPathFilter = "*";
        }
        else
        {
            auto nextFilterList = pathFilterPartList;
            nextFilterList.removeFirst();
            nextPathFilter = nextFilterList.join( RiaFilePathTools::separator() );
        }

        findMatchingFoldersRecursively( fullPath, nextPathFilter, matchingFolders, callback );
    }
}
