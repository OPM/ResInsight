//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafPdmUiTreeOrdering.h"

#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiTableViewEditor.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QPushButton>

namespace caf
{

class PdmChildArrayFieldHandle;
class PdmObjectHandle;
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiTableRowEditor;
class PdmUiTreeOrdering;
class PdmUiTreeViewEditor;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class TableViewPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TableViewPushButton(caf::PdmUiFieldHandle* field, const QString& text, QWidget* parent = nullptr);

private slots:
    void slotPressed();

private:
    caf::PdmUiFieldHandle* m_fieldHandle;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewQModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PdmUiTableViewQModel(QWidget* parent);

    QItemSelection          modelIndexFromPdmObject(PdmObjectHandle* pdmObject);
    PdmFieldHandle*         getField(const QModelIndex &index) const;
    void                    setArrayFieldAndBuildEditors(PdmChildArrayFieldHandle* pdmObject, const QString& configName);
    PdmObjectHandle*        pdmObjectForRow(int row) const;

    // Qt overrides
    int                     rowCount( const QModelIndex &parent = QModelIndex( ) ) const override;
    int                     columnCount( const QModelIndex &parent = QModelIndex( ) ) const override;
    QVariant                data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant                headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    Qt::ItemFlags           flags(const QModelIndex &index) const override;
    bool                    setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void                    notifyDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    bool                    isRepresentingBoolean(const QModelIndex &index) const;

    void                    createPersistentPushButtonWidgets(QTableView* tableView);

private:
    int                     getFieldIndex(PdmFieldHandle* field) const;
    void                    recreateTableItemEditors();
    PdmUiFieldHandle*       getUiFieldHandle(const QModelIndex& index) const;

    friend class PdmUiTableViewDelegate;
    QWidget*                getEditorWidgetAndTransferOwnership(QWidget* parent, const QModelIndex &index);
    PdmUiFieldEditorHandle* getEditor(const QModelIndex &index);

    PdmChildArrayFieldHandle*   childArrayFieldHandle() const;

private:
    // Required to have a PdmPointer to the owner object. Used to guard access to a field inside this object
    PdmPointer<PdmObjectHandle>                 m_ownerObject;
    PdmChildArrayFieldHandle*                   m_pdmList;
    QString                                     m_currentConfigName;

    std::map<QString, PdmUiFieldEditorHandle*>  m_fieldEditors;
    std::vector<int>                            m_modelColumnIndexToFieldIndex;

    std::vector<PdmUiTableRowEditor*>          m_tableRowEditors;

    PdmUiTableViewPushButtonEditorAttribute     m_pushButtonEditorAttributes;
};


} // End of namespace caf
