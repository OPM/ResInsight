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

#include "cafPdmUiTableView.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafSelectionManager.h"

#include "cafPdmUiCommandSystemProxy.h"
#include <QTableView>
#include <QVBoxLayout>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableView::PdmUiTableView( QWidget* parent, Qt::WindowFlags f )
    : QWidget( parent, f )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );

    setLayout( layout );

    m_listViewEditor = new PdmUiTableViewEditor();

    m_listViewEditor->createWidgets( this );

    {
        QWidget* widget = m_listViewEditor->labelWidget();
        layout->addWidget( widget );
    }

    {
        QWidget* widget = m_listViewEditor->editorWidget();
        layout->addWidget( widget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableView::~PdmUiTableView()
{
    if ( m_listViewEditor ) delete m_listViewEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableView::setChildArrayField( PdmChildArrayFieldHandle* childArrayField )
{
    CAF_ASSERT( m_listViewEditor );

    if ( childArrayField )
    {
        // Keep the possible custom context menu setting from the user of the table view.
        // setUIField will set it based on the PdmUIItem settings, but turning the custom menu off should not
        // be respected when using the field in a separate view.
        auto orgContextPolicy = m_listViewEditor->tableView()->contextMenuPolicy();

        m_listViewEditor->setUiField( childArrayField->uiCapability() );

        auto newContextPolicy = m_listViewEditor->tableView()->contextMenuPolicy();
        if ( newContextPolicy == Qt::DefaultContextMenu )
        {
            m_listViewEditor->tableView()->setContextMenuPolicy( orgContextPolicy );
        }
    }
    else
    {
        m_listViewEditor->setUiField( nullptr );
    }

    // SIG_CAF_HACK
    m_listViewEditor->updateUi( m_uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// SIG_CAF_HACK
void PdmUiTableView::setUiConfigurationName( QString uiConfigName )
{
    if ( uiConfigName != m_uiConfigName )
    {
        m_uiConfigName = uiConfigName;
        m_listViewEditor->updateUi( uiConfigName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTableView* PdmUiTableView::tableView()
{
    CAF_ASSERT( m_listViewEditor );

    return m_listViewEditor->tableView();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableView::enableHeaderText( bool enable )
{
    m_listViewEditor->enableHeaderText( enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableView::setTableSelectionLevel( int selectionLevel )
{
    m_listViewEditor->setTableSelectionLevel( selectionLevel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableView::setRowSelectionLevel( int selectionLevel )
{
    m_listViewEditor->setRowSelectionLevel( selectionLevel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmUiTableView::pdmObjectFromModelIndex( const QModelIndex& mi )
{
    return m_listViewEditor->pdmObjectFromModelIndex( mi );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableView::addActionsToMenu( QMenu* menu, PdmChildArrayFieldHandle* childArrayField )
{
    // This is function is required to execute before populating the menu
    // Several commands rely on the activeChildArrayFieldHandle in the selection manager
    SelectionManager::instance()->setActiveChildArrayFieldHandle( childArrayField );

    caf::PdmUiCommandSystemProxy::instance()->populateMenuWithDefaultCommands( "PdmUiTreeViewEditor", menu );
}

} // End of namespace caf
