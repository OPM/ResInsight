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

#include <QHBoxLayout>
#include "cafPdmUiTreeViewEditor.h"
#include <QTreeView>

namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::PdmUiTreeView(QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_layout);

    m_treeViewEditor = new PdmUiTreeViewEditor();

    QWidget * widget = m_treeViewEditor->getOrCreateWidget(this);

    this->m_layout->insertWidget(0, widget);

    connect(treeView()->selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotOnSelectionChanged( const QItemSelection & , const QItemSelection & )));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::~PdmUiTreeView()
{
    if (m_treeViewEditor) delete m_treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setUiConfigurationName(QString uiConfigName)
{
    // Reset everything, and possibly create widgets etc afresh
    if (m_uiConfigName != uiConfigName)
    { 
        m_uiConfigName = uiConfigName;

        m_treeViewEditor->updateUi(m_uiConfigName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setPdmItem(caf::PdmUiItem* object)
{
    m_treeViewEditor->setPdmItemRoot(object);
    m_treeViewEditor->updateUi(m_uiConfigName);
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
void PdmUiTreeView::selectedObjects(std::vector<PdmUiItem*>& objects)
{
    m_treeViewEditor->selectedUiItems(objects);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::slotOnSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    emit selectionChanged();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::enableDefaultContextMenu(bool enable)
{
    m_treeViewEditor->enableDefaultContextMenu(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setCurrentSelectionToCurrentEditorSelection(bool enable)
{
    m_treeViewEditor->setCurrentSelectionToCurrentEditorSelection(enable);
}

} //End of namespace caf

