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

#include "RiaApplication.h"

#include "RiuTools.h"

#include "ui_RiuMultiCaseImportDialog.h"

#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QStringListModel>

class FileListModel: public QStringListModel
{
public:
    explicit FileListModel(QObject *parent = 0) : m_isItemsEditable(false), QStringListModel(parent) 
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

    void removeFileNames(const QModelIndexList& indexes)
    {
        for (int i = indexes.size() - 1; i >= 0; i--)
        {
            removeRow(indexes[i].row(), indexes[i].parent());
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
    : QDialog(parent, RiuTools::defaultDialogFlags())
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
    QString selectedFolder = QFileDialog::getExistingDirectory(this, "Select an Eclipse case search folder", RiaApplication::instance()->lastUsedDialogDirectory("MULTICASEIMPORT"));
    QStringList folderNames = m_searchFolders->stringList();

    if (!folderNames.contains(selectedFolder))
    {
        folderNames.push_back(selectedFolder);
        m_searchFolders->setStringList(folderNames);
        updateGridFileList();
    }

    RiaApplication::instance()->setLastUsedDialogDirectory("MULTICASEIMPORT", selectedFolder);

    QPushButton* okButton = ui->m_dialogButtons->button(QDialogButtonBox::Ok);
    if (okButton)
    {
        okButton->setFocus(Qt::OtherFocusReason);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMultiCaseImportDialog::on_m_removeSearchFolderButton_clicked()
{
    QModelIndexList selection = ui->m_searchFolderList->selectionModel()->selectedIndexes();
    QStringList folderNames = m_searchFolders->stringList();

    QStringList searchFoldersToRemove;

    for (int i = 0; i < selection.size(); ++i)
    {
        searchFoldersToRemove.push_back(folderNames[selection[i].row()]);
    }

    for (int i = 0; i < searchFoldersToRemove.size(); ++i)
    {
        folderNames.removeOne(searchFoldersToRemove[i]);
    }

    m_searchFolders->setStringList(folderNames);

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

    // Filter the search folders to remove subfolders of existing roots'

    QStringList searchFoldersToRemove;

    for (int i = 0; i < folderNames.size(); ++i)
        for (int j = 0; j < folderNames.size(); ++j)
        {
            if ( i != j)
            {
                if (folderNames[i].startsWith(folderNames[j]))
                {
                    // Remove folderNames[i]
                    searchFoldersToRemove.push_back(folderNames[i]);
                }
            }
        }

    // Remove the subfolders when adding a root

    for (int i = 0; i < searchFoldersToRemove.size(); ++i)
    {
        folderNames.removeOne(searchFoldersToRemove[i]);
    }

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
    if (selection.size())
    {
        FileListModel* dataModel = static_cast<FileListModel*>(ui->m_eclipseCasesList->model());
        Q_ASSERT(dataModel);

        dataModel->removeFileNames(selection);
    }
}
