//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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


#pragma once

#include "cafNotificationCenter.h"
#include "cafSelectionManager.h"

#include <QModelIndex>
#include <QString>
#include <QWidget>

class QTableView;

namespace caf
{

class PdmObjectHandle;
class PdmUiTableViewEditor;
class PdmChildArrayFieldHandle;

//==================================================================================================
/// 
//==================================================================================================
class PdmUiTableView : public QWidget, public DataModelObserver
{
    Q_OBJECT
public:
    PdmUiTableView(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~PdmUiTableView();

    PdmObjectHandle*  pdmObjectFromModelIndex(const QModelIndex& mi);

    // SIG_CAF_HACK
    void        setUiConfigurationName(QString uiConfigName);

    void        setListField(PdmChildArrayFieldHandle* object);

    void        enableDefaultContextMenu(bool enable);
    void        enableHeaderText(bool enable);
    void        setSelectionRole(SelectionManager::SelectionRole role);

    QTableView* tableView();

    virtual void handleModelNotification(caf::PdmObjectHandle* itemThatChanged);
    virtual void handleModelSelectionChange();

private:
    PdmUiTableViewEditor*   m_listViewEditor;
    QString                 m_uiConfigName;
};



} // End of namespace caf

