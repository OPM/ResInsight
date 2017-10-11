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
#include "cafPdmUiTableView.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QPushButton>

namespace caf
{

class PdmChildArrayFieldHandle;
class PdmObjectHandle;
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiTableItemEditor;
class PdmUiTreeOrdering;
class PdmUiTreeViewEditor;


class TableViewPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TableViewPushButton(caf::PdmUiFieldHandle* field, const QString& text, QWidget* parent = 0);

private slots:
    void slotPressed();

private:
    caf::PdmUiFieldHandle* m_fieldHandle;
};


//==================================================================================================
//
//
//==================================================================================================
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PdmUiTableViewModel(QWidget* parent);

    QItemSelection          modelIndexFromPdmObject(PdmObjectHandle* pdmObject);
    PdmFieldHandle*         getField(const QModelIndex &index) const;
    void                    setPdmData(PdmChildArrayFieldHandle* pdmObject, const QString& configName);
    PdmObjectHandle*        pdmObjectForRow(int row) const;

    // Qt overrides
    virtual int             rowCount( const QModelIndex &parent = QModelIndex( ) ) const;
    virtual int             columnCount( const QModelIndex &parent = QModelIndex( ) ) const;
    virtual QVariant        data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant        headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags   flags(const QModelIndex &index) const;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void                    notifyDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    bool                    isRepresentingBoolean(const QModelIndex &index) const;

    void                    createPersistentPushButtonWidgets(QTableView* tableView);

private:
    int                     getFieldIndex(PdmFieldHandle* field) const;
    void                    recreateTableItemEditors();

    friend class PdmUiTableViewDelegate;
    QWidget*                getEditorWidgetAndTransferOwnership(QWidget* parent, const QModelIndex &index);
    PdmUiFieldEditorHandle* getEditor(const QModelIndex &index);

private:
    PdmChildArrayFieldHandle*                   m_pdmList;
    QString                                     m_currentConfigName;

    std::map<QString, PdmUiFieldEditorHandle*>  m_fieldEditors;
    std::vector<int>                            m_modelColumnIndexToFieldIndex;

    std::vector<PdmUiTableItemEditor*>          m_tableItemEditors;

    PdmUiTableViewEditorAttribute               m_attributes;
};


} // End of namespace caf
