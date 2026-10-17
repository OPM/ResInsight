/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RiuFileDialogTools.h"

#include <QDir>
#include <QFileDialog>
#include <QSettings>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFileDialogTools::getSaveFileName( QWidget*       parent /*= nullptr*/,
                                             const QString& caption /*= QString()*/,
                                             const QString& dir /*= QString()*/,
                                             const QString& filter /*= QString()*/,
                                             QString*       selectedFilter /*= nullptr */ )
{
#ifdef WIN32
    return QFileDialog::getSaveFileName( parent, caption, dir, filter, selectedFilter );
#else
    auto options = QFileDialog::DontUseNativeDialog;
    return QFileDialog::getSaveFileName( parent, caption, dir, filter, selectedFilter, options );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiuFileDialogTools::getOpenFileNames( QWidget*       parent /*= nullptr*/,
                                                  const QString& caption /*= QString()*/,
                                                  const QString& dir /*= QString()*/,
                                                  const QString& filter /*= QString()*/,
                                                  QString*       selectedFilter /*= nullptr */ )
{
#ifdef WIN32
    return QFileDialog::getOpenFileNames( parent, caption, dir, filter, selectedFilter );
#else
    auto options = QFileDialog::DontUseNativeDialog;
    return QFileDialog::getOpenFileNames( parent, caption, dir, filter, selectedFilter, options );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFileDialogTools::getExistingDirectory( QWidget*       parent /*= nullptr*/,
                                                  const QString& caption /*= QString()*/,
                                                  const QString& dir /*= QString() */ )
{
    const QString registryKey      = "RiuFileDialogTools/RecentFolders";
    const int     maxRecentFolders = 10;

    QSettings   settings;
    QStringList recentFolders;
    for ( const QString& folder : settings.value( registryKey ).toStringList() )
    {
        if ( QDir( folder ).exists() ) recentFolders.push_back( folder );
    }

    // The native dialog does not support a history, so the Qt dialog is used to show recently used
    // folders in the "Look in" dropdown
    QFileDialog dialog( parent, caption, dir );
    dialog.setFileMode( QFileDialog::Directory );
    dialog.setOptions( QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog );

    // The dialog lists the history with the last entry on top
    QStringList history( recentFolders.rbegin(), recentFolders.rend() );
    dialog.setHistory( history );

    if ( dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty() ) return {};

    const QString selectedFolder = QDir::cleanPath( dialog.selectedFiles().front() );

    recentFolders.removeAll( selectedFolder );
    recentFolders.prepend( selectedFolder );
    while ( recentFolders.size() > maxRecentFolders )
    {
        recentFolders.removeLast();
    }
    settings.setValue( registryKey, recentFolders );

    return selectedFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuFileDialogTools::getOpenFileName( QWidget*       parent /*= nullptr*/,
                                             const QString& caption /*= QString()*/,
                                             const QString& dir /*= QString()*/,
                                             const QString& filter /*= QString()*/,
                                             QString*       selectedFilter /*= nullptr */ )
{
#ifdef WIN32
    return QFileDialog::getOpenFileName( parent, caption, dir, filter, selectedFilter );
#else
    auto options = QFileDialog::DontUseNativeDialog;
    return QFileDialog::getOpenFileName( parent, caption, dir, filter, selectedFilter, options );
#endif
}
