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
#include "cafSelectionChangedReceiver.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>

class QItemSelection;
class QLabel;
class QTableView;

namespace caf 
{

class PdmUiCheckBoxDelegate;
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiTableViewDelegate;
class PdmUiTableViewQModel;
class PdmChildArrayFieldHandle;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewPushButtonEditorAttribute : public PdmUiEditorAttribute
{
public:
    void    registerPushButtonTextForFieldKeyword(const QString& keyword, const QString& text);

    bool    showPushButtonForFieldKeyword(const QString& keyword) const;
    QString pushButtonText(const QString& keyword) const;

private:
    std::map<QString, QString> m_fieldKeywordAndPushButtonText;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiTableViewEditorAttribute()
        : selectionLevel(1)
        , enableHeaderText(true)
    {
    }

    int     selectionLevel;
    bool    enableHeaderText;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class PdmUiTableViewEditor : public PdmUiFieldEditorHandle, public SelectionChangedReceiver
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTableViewEditor();
    ~PdmUiTableViewEditor();

    void            enableHeaderText(bool enable);
    void            setSelectionLevel(int selectionLevel);

    PdmObjectHandle* pdmObjectFromModelIndex(const QModelIndex& mi);
    QTableView*     tableView();

protected:
    QWidget*        createEditorWidget(QWidget * parent) override;
    QWidget*        createLabelWidget(QWidget * parent) override;
    virtual void    configureAndUpdateUi(const QString& uiConfigName) override;

    virtual void    onSelectionManagerSelectionChanged(int selectionLevel) override;

private:
    void            selectedUiItems(const QModelIndexList& modelIndexList, std::vector<PdmUiItem*>& objects);
    bool            isSelectionRoleDefined() const;
    void            updateSelectionManagerFromTableSelection();

    PdmChildArrayFieldHandle* childArrayFieldHandle();

private slots:
    void            slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    friend class FocusEventHandler;

    QPointer<QLabel>        m_tableHeading;
    QPointer<QLabel>        m_tableHeadingIcon;

    QTableView*             m_tableView;
    PdmUiTableViewQModel*   m_tableModelPdm;

    PdmUiTableViewDelegate* m_delegate;
    PdmUiCheckBoxDelegate*  m_checkboxDelegate;

    bool                    m_useDefaultContextMenu;
    int                     m_selectionLevel;
    bool                    m_isBlockingSelectionManagerChanged;

    caf::PdmChildArrayFieldHandle* m_previousFieldHandle;
};



} // end namespace caf
