/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include <QDialog>

namespace Ui {
    class RiuMultiCaseImportDialog;
};

class FileListModel;

//==================================================================================================
//
// 
//
//==================================================================================================

class RiuMultiCaseImportDialog: public QDialog
{
    Q_OBJECT

public:
    explicit RiuMultiCaseImportDialog(QWidget *parent = 0);
    virtual ~RiuMultiCaseImportDialog();

    QStringList eclipseCaseFileNames() const; 

protected slots:
    void on_m_addSearchFolderButton_clicked();
    void on_m_removeSearchFolderButton_clicked();
    void on_m_removeEclipseCaseButton_clicked();
private:
    void updateGridFileList();
    static void appendEGRIDFilesRecursively(const QString& folderName, QStringList& gridFileNames);
    Ui::RiuMultiCaseImportDialog* ui;

    FileListModel *m_searchFolders;
    FileListModel *m_eclipseGridFiles;
};
