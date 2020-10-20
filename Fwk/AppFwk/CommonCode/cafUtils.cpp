//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafUtils.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QLineEdit>

#include <QtOpenGL/QGLContext>

#include <QFileDialog>
#include <QMessageBox>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Utils::editToDouble( QLineEdit* lineEdit, double defaultVal )
{
    if ( !lineEdit ) return false;
    //    CVF_ASSERT(lineEdit);

    bool   ok  = false;
    double val = lineEdit->text().toDouble( &ok );
    if ( ok )
    {
        return val;
    }
    else
    {
        return defaultVal;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Utils::absoluteFileName( const QString& fileName )
{
    QFileInfo fi( fileName );
    return QDir::toNativeSeparators( fi.absoluteFilePath() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList Utils::getFilesInDirectory( const QString& dirPath, const QString& nameFilter, bool getAbsoluteFileNames )
{
    QStringList nameFilters;
    nameFilters << nameFilter;
    return getFilesInDirectory( dirPath, nameFilters, getAbsoluteFileNames );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList Utils::getFilesInDirectory( const QString& dirPath, const QStringList& nameFilters, bool getAbsoluteFileNames )
{
    QDir::SortFlags sortFlags = QDir::SortFlags( QDir::Name | QDir::IgnoreCase );

    // Only get files
    QDir::Filters typeFilter = QDir::Files;

    QDir dir( dirPath );
    dir.setFilter( typeFilter );
    dir.setNameFilters( nameFilters );
    dir.setSorting( sortFlags );

    QFileInfoList fileInfoList = dir.entryInfoList();

    QStringList retFileNames;

    QListIterator<QFileInfo> it( fileInfoList );
    while ( it.hasNext() )
    {
        QFileInfo fi    = it.next();
        QString   entry = getAbsoluteFileNames ? fi.absoluteFilePath() : fi.fileName();

        entry = QDir::toNativeSeparators( entry );
        retFileNames.push_back( entry );
    }

    return retFileNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Utils::constructFullFileName( const QString& folder, const QString& baseFileName, const QString& extension )
{
    QFileInfo fi( folder, baseFileName + extension );
    QString   fullFileName = fi.filePath();
    fullFileName           = QDir::toNativeSeparators( fullFileName );

    return fullFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Utils::makeValidFileBasename( const QString& fileBasenameCandidate )
{
    QString cleanBasename = fileBasenameCandidate.trimmed();
    cleanBasename.replace( ".", "_" );
    cleanBasename.replace( ",", "_" );
    cleanBasename.replace( ":", "_" );
    cleanBasename.replace( ";", "_" );
    cleanBasename.replace( " ", "_" );
    cleanBasename.replace( "/", "_" );
    cleanBasename.replace( "\\", "_" );
    cleanBasename.replace( "<", "_" );
    cleanBasename.replace( ">", "_" );
    cleanBasename.replace( "\"", "_" );
    cleanBasename.replace( "|", "_" );
    cleanBasename.replace( "?", "_" );
    cleanBasename.replace( "*", "_" );
    cleanBasename.replace( "\n", "_" );

    cleanBasename.replace( QRegExp( "_+" ), "_" );

    return cleanBasename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Utils::indentString( int numSpacesToIndent, const QString& str )
{
    QString indentString;
    indentString.fill( ' ', numSpacesToIndent );

    QStringList strList = str.split( "\n" );

    QString retStr = indentString + strList.join( "\n" + indentString );
    return retStr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Utils::getSaveDirectoryAndCheckOverwriteFiles( const QString& defaultDir, std::vector<QString> fileNames, QString* saveDir )
{
    bool overWriteFiles = false;
    ( *saveDir )        = QFileDialog::getExistingDirectory( nullptr, "Select save directory", defaultDir );

    std::vector<QString> filesToOverwrite;
    for ( QString fileName : fileNames )
    {
        QFileInfo fileInfo( ( *saveDir ) + "/" + fileName );
        if ( fileInfo.exists() )
        {
            filesToOverwrite.push_back( fileName );
        }
    }

    if ( filesToOverwrite.size() == 0 )
    {
        overWriteFiles = true;
        return overWriteFiles;
    }
    else if ( filesToOverwrite.size() > 0 )
    {
        QMessageBox msgBox;

        QString message = "The following files will be overwritten in the export:";
        for ( QString fileName : filesToOverwrite )
        {
            message += "\n" + ( *saveDir ) + "/" + fileName;
        }
        msgBox.setText( message );

        msgBox.setInformativeText( "Do you want to continue?" );
        msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
        msgBox.setDefaultButton( QMessageBox::Ok );
        int ret = msgBox.exec();

        switch ( ret )
        {
            case QMessageBox::Ok:
                overWriteFiles = true;
                break;
            case QMessageBox::Cancel:
                overWriteFiles = false;
                break;
            default:
                // should never be reached
                break;
        }
    }

    return overWriteFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Utils::fileExists( const QString& fileName )
{
    QFileInfo fi( fileName );

    // QFileInfo::exists returns true for both files and folders
    // Also check if the path points to a file

    if ( fi.exists() && fi.isFile() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Utils::fileExtension( const QString& fileName )
{
    QFileInfo fi( fileName );

    return fi.suffix();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Utils::isFolderWritable( const QString& folderName )
{
    // See platform issues here
    // http://doc.qt.io/qt-4.8/qfile.html#platform-specific-issues

    QFileInfo dir( folderName );

    return dir.isWritable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Utils::isStringMatch( const QString& filterString, const QString& value )
{
    if ( filterString.isEmpty() ) return true;
    if ( filterString.trimmed() == "*" )
    {
        if ( !value.isEmpty() )
            return true;
        else
            return false;
    }

    QRegExp searcher( filterString, Qt::CaseInsensitive, QRegExp::WildcardUnix );
    return searcher.exactMatch( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Utils::removeDirectoryAndFilesRecursively( const QString& dirName )
{
    bool result = true;
    QDir dir( dirName );

    if ( dir.exists() )
    {
        QFileInfoList fileInfoList =
            dir.entryInfoList( QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files,
                               QDir::DirsFirst );
        for ( const auto& fileInfo : fileInfoList )
        {
            if ( fileInfo.isDir() )
            {
                result = removeDirectoryAndFilesRecursively( fileInfo.absoluteFilePath() );
            }
            else
            {
                result = QFile::remove( fileInfo.absoluteFilePath() );
            }

            if ( !result )
            {
                return result;
            }
        }

        result = QDir().rmdir( dirName );
    }

    return result;
}

} // namespace caf
