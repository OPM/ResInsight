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

#include "cafPdmUiFieldEditorHandle.h"
#include "cafSelectionChangedReceiver.h"

#include <QAbstractItemModel>
#include <QBoxLayout>
#include <QPointer>
#include <QTableView>
#include <QWidget>

class QItemSelection;
class QLabel;

namespace caf
{
class PdmObjectHandle;
class PdmUiCheckBoxDelegate;
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiTableViewDelegate;
class PdmUiTableViewQModel;
class PdmChildArrayFieldHandle;

class TableView : public QTableView
{
public:
    explicit TableView( QWidget* parent = nullptr );

    QSize sizeHint() const override;
    void  setHeightHint( int heightHint );

private:
    int m_heightHint;
};

class PdmUiTableViewEditor : public PdmUiFieldEditorHandle, public SelectionChangedReceiver
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTableViewEditor();
    ~PdmUiTableViewEditor() override;

    enum ResizePolicy
    {
        NO_AUTOMATIC_RESIZE,
        RESIZE_TO_FIT_CONTENT,
        RESIZE_TO_FILL_CONTAINER
    };

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        static inline const QString TABLE_SELECTION_LEVEL        = QStringLiteral( "tableSelectionLevel" );
        static inline const QString ROW_SELECTION_LEVEL          = QStringLiteral( "rowSelectionLevel" );
        static inline const QString ENABLE_HEADER_TEXT           = QStringLiteral( "enableHeaderText" );
        static inline const QString MINIMUM_HEIGHT               = QStringLiteral( "minimumHeight" );
        static inline const QString HEIGHT_HINT                  = QStringLiteral( "heightHint" );
        static inline const QString ALWAYS_ENFORCE_RESIZE_POLICY = QStringLiteral( "alwaysEnforceResizePolicy" );
        static inline const QString RESIZE_POLICY                = QStringLiteral( "resizePolicy" );
        static inline const QString COLUMN_WIDTHS                = QStringLiteral( "columnWidths" );
        static inline const QString BASE_COLOR                   = QStringLiteral( "baseColor" );
        static inline const QString ENABLE_DROP_TARGET           = QStringLiteral( "enableDropTarget" );
        static inline const QString EDIT_ON_SINGLE_CLICK         = QStringLiteral( "editOnSingleClick" );
    };

    // Attribute keys set on the fields of the objects shown as rows in the table
    struct CellKeys
    {
        // QString. Show a push button with this text in the cells of the field
        static inline const QString PUSH_BUTTON_TEXT = QStringLiteral( "tableViewPushButtonText" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::TABLE_SELECTION_LEVEL,
                                                                   Keys::ROW_SELECTION_LEVEL,
                                                                   Keys::ENABLE_HEADER_TEXT,
                                                                   Keys::MINIMUM_HEIGHT,
                                                                   Keys::HEIGHT_HINT,
                                                                   Keys::ALWAYS_ENFORCE_RESIZE_POLICY,
                                                                   Keys::RESIZE_POLICY,
                                                                   Keys::COLUMN_WIDTHS,
                                                                   Keys::BASE_COLOR,
                                                                   Keys::ENABLE_DROP_TARGET,
                                                                   Keys::EDIT_ON_SINGLE_CLICK };

    void enableHeaderText( bool enable );
    void setTableSelectionLevel( int selectionLevel );
    void setRowSelectionLevel( int selectionLevel );

    PdmObjectHandle* pdmObjectFromModelIndex( const QModelIndex& mi );
    QTableView*      tableView();

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;
    bool isMultiRowEditor() const override;

private:
    void selectedUiItems( const QModelIndexList& modelIndexList, std::vector<PdmUiItem*>& objects );
    bool isSelectionRoleDefined() const;
    void updateSelectionManagerFromTableSelection();

    bool                      eventFilter( QObject* obj, QEvent* event ) override;
    PdmChildArrayFieldHandle* childArrayFieldHandle();

private slots:
    void slotSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

private:
    friend class FocusEventHandler;

    QPointer<QLabel> m_tableHeading;
    QPointer<QLabel> m_tableHeadingIcon;

    TableView*            m_tableView;
    PdmUiTableViewQModel* m_tableModelPdm;

    PdmUiTableViewDelegate* m_delegate;
    PdmUiCheckBoxDelegate*  m_checkboxDelegate;

    bool m_useDefaultContextMenu;
    int  m_tableSelectionLevel;
    int  m_rowSelectionLevel;
    bool m_isBlockingSelectionManagerChanged;
    bool m_isUpdatingSelectionQModel;

    caf::PdmChildArrayFieldHandle* m_previousFieldHandle;
};

} // end namespace caf
