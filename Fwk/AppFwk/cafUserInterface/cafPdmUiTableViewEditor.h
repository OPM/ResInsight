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

#include "cafPdmDocument.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafSelectionManager.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>

class QItemSelection;
class QLabel;
class QMenu;
class QTableView;

namespace caf 
{

class PdmUiCheckBoxDelegate;
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiTableViewDelegate;
class PdmUiTableViewModel;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class PdmUiTableViewEditor : public PdmUiEditorHandle
{
    Q_OBJECT

public:
    PdmUiTableViewEditor();
    ~PdmUiTableViewEditor();

    void            enableDefaultContextMenu(bool enable);
    void            enableHeaderText(bool enable);
    void            setSelectionRole(SelectionManager::SelectionRole role);

    PdmObjectHandle* pdmObjectFromModelIndex(const QModelIndex& mi);

    void            setListField(PdmChildArrayFieldHandle* pdmListField);
    QWidget*        createWidget(QWidget* parent);

    QTableView*     tableView();

    void            handleModelSelectionChange();

protected:
    virtual void    configureAndUpdateUi(const QString& uiConfigName) override;

private:
    void            updateContextMenuSignals();
    void            selectedUiItems(const QModelIndexList& modelIndexList, std::vector<PdmUiItem*>& objects);
    bool            isSelectionRoleDefined() const;
    void            tableViewWidgetFocusChanged(QEvent* focusEvent);
    void            updateSelectionManagerFromTableSelection();

private slots:
    void            customMenuRequested(QPoint pos);
    void            slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous);
    void            slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    friend class FocusEventHandler;

    QPointer<QWidget>       m_mainWidget;
    QLayout*                m_layout;
    QLabel*                 m_tableHeading;
    QLabel*                 m_tableHeadingIcon;

    QTableView*             m_tableView;
    PdmUiTableViewModel*    m_tableModelPdm;

    PdmChildArrayFieldHandle* m_pdmListField;
    PdmUiTableViewDelegate* m_delegate;
    PdmUiCheckBoxDelegate*  m_checkboxDelegate;

    bool                    m_useDefaultContextMenu;
    SelectionManager::SelectionRole m_selectionRole;
};



} // end namespace caf
