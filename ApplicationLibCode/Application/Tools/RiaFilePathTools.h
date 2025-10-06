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

#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <map>
#include <string>

//==================================================================================================
//
//==================================================================================================
namespace RiaFilePathTools
{
const QChar                 separator();
QString                     toInternalSeparator( const QString& path );
QString&                    appendSeparatorIfNo( QString& path );
QString                     relativePath( const QString& rootDir, const QString& dir );
bool                        equalPaths( const QString& path1, const QString& path2 );
QString                     canonicalPath( const QString& path );
std::pair<QString, QString> toFolderAndFileName( const QString& absFileName );
QString                     removeDuplicatePathSeparators( const QString& path );
QString                     rootSearchPathFromSearchFilter( const QString& searchFilter );
QString                     commonRootOfFileNames( const QStringList& filePaths );
std::string                 makeSuitableAsFileName( const std::string candidateName );
std::string                 normalizePath( std::string path );

QStringList splitPathIntoComponents( const QString& path, bool splitExtensionIntoSeparateEntry = false );

std::map<QString, QStringList> keyPathComponentsForEachFilePath( const QStringList& filePaths );

bool isFirstOlderThanSecond( const std::string& firstFileName, const std::string& secondFileName, int timeThresholdInSeconds );
}; // namespace RiaFilePathTools
