/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

class RimProject;

namespace caf {
    class PdmUiTableView;
}

class QWidget;
class QLineEdit;

class RiuExportMultipleSnapshotsWidget : public QDialog
{
    Q_OBJECT
public:
    RiuExportMultipleSnapshotsWidget(QWidget* parent, RimProject* project);
    ~RiuExportMultipleSnapshotsWidget();

    void addSnapshotItemFromActiveView();
    void addEmptySnapshotItems(size_t itemCount);

private slots:
    void customMenuRequested(QPoint pos);
    void addSnapshotItem();
    void deleteAllSnapshotItems();
    void exportSnapshots();

    void folderSelectionClicked();

private:
    RimProject*          m_rimProject;
    caf::PdmUiTableView* m_pdmTableView;
    QLineEdit*           m_exportFolderLineEdit;
};
