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

#include "RiuMultiCaseImportDialog.h"
#include "ui_RiuMultiCaseImportDialog.h"
#include <QFileSystemModel>
#include <QFileDialog>
#include <QStringListModel>
#include <QFileIconProvider>

class FileListModel: public QStringListModel
{
public:
    FileListModel(QObject *parent = 0) : m_isItemsEditable(false), QStringListModel(parent) 
    {
    }

    virtual Qt::ItemFlags flags (const QModelIndex& index) const
    {
        if (m_isItemsEditable)
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        else
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    virtual QVariant data ( const QModelIndex & index, int role ) const
    {
        if (role == Qt::DecorationRole)
        {
            QFileInfo fileInfo(stringList()[index.row()]);
            QFileIconProvider iconProv;
            return QVariant(iconProv.icon(fileInfo));
        }
        else
        {
            return QStringListModel::data(index, role);
        }
    }


    void setItemsEditable(bool isEditable)
    {
        m_isItemsEditable = isEditable;
    }

private:
    bool m_isItemsEditable;
};



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMultiCaseImportDialog::RiuMultiCaseImportDialog(QWidget *parent /*= 0*/)
    : QDialog(parent)
{
    ui = new Ui::RiuMultiCaseImportDialog;
    ui->setupUi(this);
    
    m_searchFolders = new FileListModel(this);
    ui->m_searchFolderList->setModel(m_searchFolders);

    m_eclipseGridFiles = new FileListModel(this);
    ui->m_eclipseCasesList->setModel(m_eclipseGridFiles);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMultiCaseImportDialog::~RiuMultiCaseImportDialog()
{
    delete ui;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMultiCaseImportDialog::on_m_addSearchFolderButton_clicked()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(this, "Select an Eclipse case search folder" );
    QStringList folderNames = m_searchFolders->stringList();
    if (!folderNames.contains(selectedFolder))
    {
        folderNames.push_back(selectedFolder);
        m_searchFolders->setStringList(folderNames);
        updateGridFileList();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMultiCaseImportDialog::on_m_removeSearchFolderButton_clicked()
{
    QModelIndexList selection = ui->m_searchFolderList->selectionModel()->selectedIndexes();
    for (int i = 0; i < selection.size(); ++i)
    {
        ui->m_searchFolderList->model()->removeRow(selection[i].row(), selection[i].parent());
    }

    if (selection.size())
    {
        updateGridFileList();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMultiCaseImportDialog::updateGridFileList()
{

    QStringList folderNames =  m_searchFolders->stringList();
    QStringList gridFileNames;

    for (int i = 0; i < folderNames.size(); i++)
    {
        QString folderName = folderNames[i];

        appendEGRIDFilesRecursively(folderName, gridFileNames);
    } 

    m_eclipseGridFiles->setStringList(gridFileNames);
}


void RiuMultiCaseImportDialog::appendEGRIDFilesRecursively(const QString& folderName, QStringList& gridFileNames)
{
    {
        QDir baseDir(folderName);
        baseDir.setFilter(QDir::Files);

        QStringList nameFilters;
        nameFilters << "*.egrid" << ".EGRID";
        baseDir.setNameFilters(nameFilters);

        QStringList fileNames = baseDir.entryList();

        for (int i = 0; i < fileNames.size(); ++i)
        {
            QString fileName = fileNames[i];

            QString absoluteFolderName = baseDir.absoluteFilePath(fileName);

            gridFileNames.append(absoluteFolderName);
        }
    }


    {
        QDir baseDir(folderName);
        baseDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

        QStringList subFolders = baseDir.entryList();

        for (int i = 0; i < subFolders.size(); ++i)
        {
            QString folderName = subFolders[i];

            QString absoluteFolderName = baseDir.absoluteFilePath(folderName);
            appendEGRIDFilesRecursively(absoluteFolderName, gridFileNames);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RiuMultiCaseImportDialog::eclipseCaseFileNames() const
{
    return m_eclipseGridFiles->stringList();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMultiCaseImportDialog::on_m_removeEclipseCaseButton_clicked()
{
    QModelIndexList selection = ui->m_eclipseCasesList->selectionModel()->selectedIndexes();
    for (int i = 0; i < selection.size(); ++i)
    {
        ui->m_eclipseCasesList->model()->removeRow(selection[i].row(), selection[i].parent());
    }
}
