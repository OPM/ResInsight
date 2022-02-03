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

#include "cafPdmUiTreeView.h"

#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiDragDropInterface.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewEditor.h"
#include "cafQTreeViewStateSerializer.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::PdmUiTreeView( QWidget* parent, Qt::WindowFlags f )
    : QWidget( parent, f )
{
    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( m_layout );

    QHBoxLayout* searchLayout = new QHBoxLayout();

    m_searchBox = new QLineEdit( this );
    searchLayout->addWidget( m_searchBox );

    m_clearSearchButton = new QPushButton( "X" );
    m_clearSearchButton->setMaximumSize( 30, 30 );
    searchLayout->addWidget( m_clearSearchButton );

    m_layout->addLayout( searchLayout );

    m_treeViewEditor    = new PdmUiTreeViewEditor();
    QWidget* treewidget = m_treeViewEditor->getOrCreateWidget( this );

    m_layout->addWidget( treewidget );

    connect( m_treeViewEditor, SIGNAL( selectionChanged() ), SLOT( slotOnSelectionChanged() ) );
    connect( m_clearSearchButton, SIGNAL( clicked() ), SLOT( slotOnClearSearchBox() ) );
    connect( m_searchBox, SIGNAL( textChanged( QString ) ), SLOT( onSlotSearchTextChanged() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::~PdmUiTreeView()
{
    if ( m_treeViewEditor ) delete m_treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setUiConfigurationName( QString uiConfigName )
{
    // Reset everything, and possibly create widgets etc afresh
    if ( m_uiConfigName != uiConfigName )
    {
        m_uiConfigName = uiConfigName;

        m_treeViewEditor->updateUi( m_uiConfigName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setPdmItem( caf::PdmUiItem* object )
{
    m_treeViewEditor->setPdmItemRoot( object );
    m_treeViewEditor->updateUi( m_uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTreeView* PdmUiTreeView::treeView()
{
    return m_treeViewEditor->treeView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeView::isTreeItemEditWidgetActive() const
{
    return m_treeViewEditor->isTreeItemEditWidgetActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::selectedUiItems( std::vector<PdmUiItem*>& objects )
{
    m_treeViewEditor->selectedUiItems( objects );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::slotOnSelectionChanged()
{
    emit selectionChanged();

    std::vector<PdmUiItem*> objects;
    m_treeViewEditor->selectedUiItems( objects );
    PdmObjectHandle* objHandle = nullptr;

    if ( objects.size() )
    {
        PdmUiObjectHandle* uiObjH = dynamic_cast<PdmUiObjectHandle*>( objects[0] );
        if ( uiObjH )
        {
            objHandle = uiObjH->objectHandle();
        }
    }

    emit selectedObjectChanged( objHandle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::slotOnClearSearchBox()
{
    m_searchBox->setText( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::onSlotSearchTextChanged()
{
    QString searchText = m_searchBox->text().trimmed();
    m_treeViewEditor->setFilterString( searchText );
    if ( searchText.isEmpty() )
    {
        if ( !m_treeStateString.isEmpty() )
        {
            m_treeViewEditor->treeView()->collapseAll();
            QTreeViewStateSerializer::applyTreeViewStateFromString( m_treeViewEditor->treeView(), m_treeStateString );
        }
        return;
    }
    else if ( m_treeStateString.isEmpty() )
    {
        QTreeViewStateSerializer::storeTreeViewStateToString( m_treeViewEditor->treeView(), m_treeStateString );
    }
    m_treeViewEditor->treeView()->expandAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::enableDefaultContextMenu( bool enable )
{
    m_treeViewEditor->enableDefaultContextMenu( enable );
}

//--------------------------------------------------------------------------------------------------
/// Enables or disables automatic updating of the SelectionManager selection state based on
/// the selections in this tree view
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::enableSelectionManagerUpdating( bool enable )
{
    m_treeViewEditor->enableSelectionManagerUpdating( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::selectAsCurrentItem( const PdmUiItem* uiItem )
{
    m_treeViewEditor->selectAsCurrentItem( uiItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::selectItems( const std::vector<const PdmUiItem*>& uiItems )
{
    m_treeViewEditor->selectItems( uiItems );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setExpanded( const PdmUiItem* uiItem, bool doExpand ) const
{
    m_treeViewEditor->setExpanded( uiItem, doExpand );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmUiTreeView::uiItemFromModelIndex( const QModelIndex& index ) const
{
    return m_treeViewEditor->uiItemFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmUiTreeView::uiTreeOrderingFromModelIndex( const QModelIndex& index ) const
{
    return m_treeViewEditor->uiTreeOrderingFromModelIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeView::findModelIndex( const PdmUiItem* object ) const
{
    return m_treeViewEditor->findModelIndex( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::updateSubTree( const QModelIndex& index )
{
    auto uiItem = uiItemFromModelIndex( index );
    if ( uiItem )
    {
        m_treeViewEditor->updateSubTree( uiItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setDragDropInterface( PdmUiDragDropInterface* dragDropInterface )
{
    m_treeViewEditor->setDragDropInterface( dragDropInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::enableAppendOfClassNameToUiItemText( bool enable )
{
    m_treeViewEditor->enableAppendOfClassNameToUiItemText( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::treeVisibilityChanged( bool visible )
{
    if ( visible ) slotOnSelectionChanged();
}

} // End of namespace caf
