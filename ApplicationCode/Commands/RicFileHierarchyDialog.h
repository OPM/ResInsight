/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "Rim3dOverlayInfoConfig.h"

#include "cafPdmPointer.h"

#include <QDialog>

class QLabel;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QMainWindow;
class RicFileHierarchyDialogResult;

//==================================================================================================
///  
//==================================================================================================
class RicFileHierarchyDialog : public QDialog
{
    Q_OBJECT

public:
    RicFileHierarchyDialog(QWidget* parent);
    ~RicFileHierarchyDialog();

    QStringList         files() const;
    QString             rootDir() const;

    static RicFileHierarchyDialogResult  getOpenFileNames(QWidget *parent = 0,
                                                         const QString &caption = QString(),
                                                         const QString &dir = QString(),
                                                         const QString &fileNameFilter = QString(),
                                                         const QStringList &fileExtensions = QStringList());

private:
    static QStringList  findMatchingFiles(const QString &rootDir,
                                          const QString& pathFilter,
                                          const QString &fileNameFilter,
                                          const QStringList &fileExtensions);

    static QStringList  findFilesRecursive(const QString &dir,
                                           const QString &fileNameFilter,
                                           const QStringList &fileExtensions);

    static QStringList  createNameFilterList(const QString &fileNameFilter,
                                             const QStringList &fileExtensions);

    static QStringList  filterByPathFilter(const QStringList& files, const QString& pathFilter);

private slots:
    void slotDialogOkClicked();
    void slotDialogCancelClicked();
    void slotBrowseButtonClicked();

private:
    QLabel*                             m_rootDirLabel;
    QLineEdit*                          m_rootDir;
    QPushButton*                        m_browseButton;

    QLabel*                             m_pathFilterLabel;
    QLineEdit*                          m_pathFilter;

    QLabel*                             m_fileFilterLabel;
    QLineEdit*                          m_fileFilter;

    QLabel*                             m_fileExtension;

    QDialogButtonBox*                   m_buttons;

    QStringList                         m_files;
};


//==================================================================================================
///  
//==================================================================================================
class RicFileHierarchyDialogResult
{
public:
    RicFileHierarchyDialogResult(bool ok, const QStringList& files, const QString& rootDir) :
        ok(ok), files(files), rootDir(rootDir) {}
    bool            ok;
    QStringList     files;
    QString         rootDir;
};