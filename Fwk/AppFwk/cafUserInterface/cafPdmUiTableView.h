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
#include "cafPdmUiFieldEditorHandle.h"
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    void    registerPushButtonTextForFieldKeyword(const QString& keyword, const QString& text);

    bool    showPushButtonForFieldKeyword(const QString& keyword) const;
    QString pushButtonText(const QString& keyword) const;

private:
    std::map<QString, QString> m_fieldKeywordAndPushButtonText;
};

//==================================================================================================
/// 
//==================================================================================================
class PdmUiTableView : public QWidget, public DataModelObserver
{
    Q_OBJECT
public:
    PdmUiTableView(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~PdmUiTableView();

    PdmObjectHandle*  pdmObjectFromModelIndex(const QModelIndex& mi);

    // SIG_CAF_HACK
    void        setUiConfigurationName(QString uiConfigName);

    void        setUiFieldHandle(PdmUiFieldHandle* uiFieldHandle);

    void        enableDefaultContextMenu(bool enable);
    void        enableHeaderText(bool enable);
    void        setSelectionRole(SelectionManager::SelectionRole role);

    QTableView* tableView();

    void handleModelNotification(caf::PdmObjectHandle* itemThatChanged) override;
    void handleModelSelectionChange() override;

private:
    PdmUiTableViewEditor*   m_listViewEditor;
    QString                 m_uiConfigName;
};



} // End of namespace caf

