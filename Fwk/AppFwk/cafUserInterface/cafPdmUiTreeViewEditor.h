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
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QWidget>

#include <memory>

class MySortFilterProxyModel;

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
        setStyle( new PdmUiTreeViewStyle );
    };
    ~PdmUiTreeViewWidget() override{};

    bool isTreeItemEditWidgetActive() const { return state() == QAbstractItemView::EditingState; }

protected:
    void dragMoveEvent( QDragMoveEvent* event ) override;
    void dragLeaveEvent( QDragLeaveEvent* event ) override;
};

class PdmUiTreeViewItemAttribute : public PdmUiEditorAttribute
{
public:
    struct Tag : public SignalEmitter
    {
        enum Position
        {
            IN_FRONT,
            AT_END
        };
        Tag()
            : text()
            , position( AT_END )
            , bgColor( Qt::red )
            , fgColor( Qt::white )
            , selectedOnly( false )
            , clicked( this )
        {
        }
        QString      text;
        IconProvider icon;
        Position     position;
        QColor       bgColor;
        QColor       fgColor;
        bool         selectedOnly;

        caf::Signal<size_t> clicked;

        static std::unique_ptr<Tag> create() { return std::unique_ptr<Tag>( new Tag ); }

    private:
        Tag( const Tag& rhs ) = default;
        Tag& operator         =( const Tag& rhs ) { return *this; }
    };

    std::vector<std::unique_ptr<Tag>> tags;
};

class PdmUiTreeViewItemDelegate : public QStyledItemDelegate
{
public:
    PdmUiTreeViewItemDelegate( PdmUiTreeViewEditor* parent, PdmUiTreeViewQModel* model );
    void clearTags( QModelIndex index );
    void clearAllTags();
    void addTag( QModelIndex index, std::unique_ptr<PdmUiTreeViewItemAttribute::Tag> tag );
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override;
    std::vector<const PdmUiTreeViewItemAttribute::Tag*> tags( QModelIndex index ) const;

protected:
    bool  editorEvent( QEvent*                     event,
                       QAbstractItemModel*         model,
                       const QStyleOptionViewItem& option,
                       const QModelIndex&          itemIndex ) override;
    bool  tagClicked( const QPoint&                           clickPos,
                      const QRect&                            itemRect,
                      const QModelIndex&                      itemIndex,
                      const PdmUiTreeViewItemAttribute::Tag** tag ) const;
    QRect tagRect( const QRect& itemRect, QModelIndex itemIndex, size_t tagIndex ) const;

private:
    PdmUiTreeViewEditor*                                                                 m_treeView;
    PdmUiTreeViewQModel*                                                                 m_model;
    std::map<QModelIndex, std::vector<std::unique_ptr<PdmUiTreeViewItemAttribute::Tag>>> m_tags;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiTreeViewEditorAttribute() { currentObject = nullptr; }

public:
    QStringList columnHeaders;

    /// This object is set as current item in the tree view in configureAndUpdateUi()
    caf::PdmObjectHandle* currentObject;
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

    PdmUiItem*  uiItemFromModelIndex( const QModelIndex& index ) const;
    QModelIndex findModelIndex( const PdmUiItem* object ) const;

    QWidget* createWidget( QWidget* parent ) override;

    void setDragDropInterface( PdmUiDragDropInterface* dragDropInterface );

signals:
    void selectionChanged();

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

    void updateMySubTree( PdmUiItem* uiItem ) override;
    void updateContextMenuSignals();

private slots:
    void customMenuRequested( QPoint pos );
    void slotOnSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );

private:
    PdmChildArrayFieldHandle* currentChildArrayFieldHandle();

    void updateSelectionManager();
    void updateItemDelegateForSubTree( const QModelIndex& modelIndex = QModelIndex() );

    bool eventFilter( QObject* obj, QEvent* event ) override;

private:
    QPointer<QWidget> m_mainWidget;
    QVBoxLayout*      m_layout;

    PdmUiTreeViewWidget*       m_treeView;
    PdmUiTreeViewQModel*       m_treeViewModel;
    PdmUiTreeViewItemDelegate* m_delegate;

    bool m_useDefaultContextMenu;
    bool m_updateSelectionManager;
    bool m_appendClassNameToUiItemText;
};

} // end namespace caf
