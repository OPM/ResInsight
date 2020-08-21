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

#include <QFileDialog>

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
    return QFileDialog::getSaveFileName( parent, caption, dir, filter, selectedFilter, QFileDialog::DontUseNativeDialog );
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
#ifdef WIN32
    return QFileDialog::getExistingDirectory( parent, caption, dir );
#else
    auto options = QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog;
    return QFileDialog::getExistingDirectory( parent, caption, dir, options );
#endif
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
    return getOpenFileName( parent, caption, dir, filter, selectedFilter );
#else
    auto options = QFileDialog::DontUseNativeDialog;
    return getOpenFileName( parent, caption, dir, filter, selectedFilter, options );
#endif
}
