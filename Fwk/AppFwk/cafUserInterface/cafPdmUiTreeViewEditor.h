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

#include "cafIconProvider.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiTreeEditorHandle.h"
#include "cafPdmUiTreeViewQModel.h"
#include "cafSignal.h"

#include <QAbstractItemModel>
#include <QColor>
#include <QItemSelectionModel>
#include <QPointer>
#include <QProxyStyle>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QWidget>

#include <memory>

class QGridLayout;
class QMenu;
class QTreeView;
class QVBoxLayout;

namespace caf
{
class PdmChildArrayFieldHandle;
class PdmUiDragDropInterface;
class PdmUiItem;
class PdmUiTreeViewEditor;
class PdmUiTreeViewQModel;
class PdmUiTreeViewWidget;
class PdmUiTreeViewItemDelegate;

class PdmUiTreeViewStyle : public QProxyStyle
{
public:
    void drawPrimitive( QStyle::PrimitiveElement element,
                        const QStyleOption*      option,
                        QPainter*                painter,
                        const QWidget*           widget ) const override;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewWidget : public QTreeView
{
public:
    explicit PdmUiTreeViewWidget( QWidget* parent = nullptr )
        : QTreeView( parent )
    {
        m_style = std::make_shared<PdmUiTreeViewStyle>();
        setStyle( m_style.get() );
    };

    ~PdmUiTreeViewWidget() override {};

    bool isTreeItemEditWidgetActive() const { return state() == QAbstractItemView::EditingState; }

protected:
    void dragMoveEvent( QDragMoveEvent* event ) override;
    void dragLeaveEvent( QDragLeaveEvent* event ) override;

    std::shared_ptr<PdmUiTreeViewStyle> m_style;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewEditor : public PdmUiTreeEditorHandle
{
    Q_OBJECT
public:
    PdmUiTreeViewEditor();
    ~PdmUiTreeViewEditor() override;

    void enableDefaultContextMenu( bool enable );
    void enableSelectionManagerUpdating( bool enable );

    void enableAppendOfClassNameToUiItemText( bool enable );
    bool isAppendOfClassNameToUiItemTextEnabled();

    QTreeView*       treeView();
    const QTreeView* treeView() const;
    bool             isTreeItemEditWidgetActive() const;

    void selectAsCurrentItem( const PdmUiItem* uiItem );
    void selectItems( std::vector<const PdmUiItem*> uiItems );
    void selectedUiItems( std::vector<PdmUiItem*>& objects );
    void setExpanded( const PdmUiItem* uiItem, bool doExpand ) const;

    PdmUiItem*         uiItemFromModelIndex( const QModelIndex& index ) const;
    PdmUiTreeOrdering* uiTreeOrderingFromModelIndex( const QModelIndex& index ) const;
    QModelIndex        findModelIndex( const PdmUiItem* object ) const;

    QWidget* createWidget( QWidget* parent ) override;

    void setDragDropInterface( PdmUiDragDropInterface* dragDropInterface );

    void setFilterString( QString filterStr );

signals:
    void selectionChanged();

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

    void updateMySubTree( PdmUiItem* uiItem, bool notifyEditors ) override;
    void updateContextMenuSignals();

private slots:
    void customMenuRequested( QPoint pos );
    void slotOnSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

private:
    PdmChildArrayFieldHandle* currentChildArrayFieldHandle();

    QModelIndex mapIndexIfNecessary( QModelIndex index ) const;

    bool updateSelectionManager();
    void updateItemDelegateForSubTree( const QModelIndex& subRootIndex = QModelIndex() );

    bool eventFilter( QObject* obj, QEvent* event ) override;

private:
    QPointer<QWidget>     m_mainWidget;
    QPointer<QVBoxLayout> m_layout;

    QPointer<PdmUiTreeViewWidget>       m_treeView;
    QPointer<PdmUiTreeViewQModel>       m_treeViewModel;
    QPointer<PdmUiTreeViewItemDelegate> m_delegate;
    QPointer<QSortFilterProxyModel>     m_filterModel;

    bool m_useDefaultContextMenu;
    bool m_updateSelectionManager;
    bool m_appendClassNameToUiItemText;
};

} // end namespace caf
