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

#include "cafPdmUiTreeViewEditor.h"
#include <QHBoxLayout>
#include <QTreeView>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::PdmUiTreeView( QWidget* parent, Qt::WindowFlags f )
    : QWidget( parent, f )
{
    m_layout = new QVBoxLayout( this );
    m_layout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( m_layout );

    m_treeViewEditor = new PdmUiTreeViewEditor();

    QWidget* widget = m_treeViewEditor->getOrCreateWidget( this );

    this->m_layout->insertWidget( 0, widget );

    connect( m_treeViewEditor, SIGNAL( selectionChanged() ), SLOT( slotOnSelectionChanged() ) );
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
QModelIndex PdmUiTreeView::findModelIndex( const PdmUiItem* object ) const
{
    return m_treeViewEditor->findModelIndex( object );
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

} // End of namespace caf
