//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiWidgetObjectEditorHandle.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>

class QTableView;

namespace caf
{
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmObjectCollection;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiListViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiListViewEditorAttribute() {}

public:
    QStringList fieldNames;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiListViewModelPdm : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit UiListViewModelPdm( QObject* parent );

    void setPdmData( PdmObjectCollection* objectGroup, const QString& configName );

    // Qt overrides
    int      rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    int      columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

private:
    void computeColumnCount();

private:
    PdmObjectCollection*         m_pdmObjectGroup;
    QString                      m_configName;
    PdmUiListViewEditorAttribute m_editorAttribute;
    int                          m_columnCount;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiListViewEditor : public PdmUiWidgetObjectEditorHandle
{
public:
    PdmUiListViewEditor();
    ~PdmUiListViewEditor() override;

protected:
    QWidget* createWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

private:
    QPointer<QTableView> m_tableView;
    UiListViewModelPdm*  m_tableModelPdm;
};

} // end namespace caf
