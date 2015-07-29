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

#include "cafPdmUiTreeViewEditor.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewModel.h"
#include "cafSelectionManager.h"

#include <QGridLayout>
#include <QMenu>
#include <QModelIndexList>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QWidget>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::PdmUiTreeViewEditor()
{
    m_useDefaultContextMenu = false;
    m_currentSelectionFollowsEditorSelection = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::~PdmUiTreeViewEditor()
{
    m_treeViewModel->setPdmItemRoot(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeViewEditor::createWidget(QWidget* parent)
{
    m_mainWidget = new QWidget(parent);
    m_layout     = new QVBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_mainWidget->setLayout(m_layout);

    m_treeViewModel = new caf::PdmUiTreeViewModel(this);
    m_treeView = new QTreeView(m_mainWidget);
    m_treeView->setModel(m_treeViewModel);

    connect(m_treeView->selectionModel(), SIGNAL(currentChanged( const QModelIndex & , const QModelIndex & )), SLOT(slotCurrentChanged( const QModelIndex& , const QModelIndex& )));
    
    m_layout->addWidget(m_treeView);

    updateContextMenuSignals();

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    // If we have a real object, get its editor attributes (Column headers for now)

    if (this->pdmItemRoot() && dynamic_cast<PdmUiObjectHandle*>(this->pdmItemRoot()))
    {
        dynamic_cast<PdmUiObjectHandle*>(this->pdmItemRoot())->objectEditorAttribute(uiConfigName, &m_editorAttributes);
    }

    m_treeViewModel->setColumnHeaders(m_editorAttributes.columnHeaders);
    m_treeViewModel->setUiConfigName(uiConfigName);
    m_treeViewModel->setPdmItemRoot(this->pdmItemRoot());

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTreeView* PdmUiTreeViewEditor::treeView()
{
    return m_treeView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectedUiItems(std::vector<PdmUiItem*>& objects)
{
    if (m_treeViewModel) { m_treeViewModel->selectedUiItems(objects);  }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::uiItemsFromModelIndexList(const QModelIndexList& modelIndexList, std::vector<PdmUiItem*>& objects)
{
    if (m_treeViewModel)
    {

        for (int i = 0; i < modelIndexList.size(); i++)
        {
            QModelIndex mi = modelIndexList.at(i);

            PdmUiTreeOrdering* treeOrdering = m_treeViewModel->treeItemFromIndex(mi);
            if (treeOrdering->activeItem())
            {
                objects.push_back(treeOrdering->activeItem());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateMySubTree(PdmUiItem* uiItem)
{
    if (m_treeViewModel) { m_treeViewModel->updateSubTree(uiItem);  }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableDefaultContextMenu(bool enable)
{
    m_useDefaultContextMenu = enable;

    updateContextMenuSignals();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setCurrentSelectionToCurrentEditorSelection(bool enable)
{
    m_currentSelectionFollowsEditorSelection = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateContextMenuSignals()
{
    if (!m_treeView) return;

    if (m_useDefaultContextMenu)
    {
        m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));
    }
    else
    {
        m_treeView->setContextMenuPolicy(Qt::DefaultContextMenu);
        disconnect(m_treeView, 0, this, 0);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::customMenuRequested(QPoint pos)
{
    SelectionManager::instance()->setActiveChildArrayFieldHandle(this->currentChildArrayFieldHandle());

    QMenu menu;
    caf::PdmUiCommandSystemProxy::instance()->populateMenuWithDefaultCommands("PdmUiTreeViewEditor", &menu);

    if (menu.actions().size() > 0)
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
        QPoint globalPos = m_treeView->viewport()->mapToGlobal(pos);

        menu.exec(globalPos);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (m_currentSelectionFollowsEditorSelection)
    {
        QModelIndexList list;
        list.append(current);

        std::vector<PdmUiItem*> items;
        uiItemsFromModelIndexList(list, items);

        SelectionManager::instance()->setSelectedItems(items, SelectionManager::CURRENT);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmChildArrayFieldHandle* PdmUiTreeViewEditor::currentChildArrayFieldHandle()
{
    PdmUiItem* currentSelectedItem = SelectionManager::instance()->selectedItem(SelectionManager::CURRENT);

    PdmUiFieldHandle* uiFieldHandle = dynamic_cast<PdmUiFieldHandle*>(currentSelectedItem);
    if (uiFieldHandle)
    {
        PdmFieldHandle* fieldHandle = uiFieldHandle->fieldHandle();

        if (dynamic_cast<PdmChildArrayFieldHandle*>(fieldHandle))
        {
            return dynamic_cast<PdmChildArrayFieldHandle*>(fieldHandle);
        }
    }

    PdmObjectHandle* pdmObject = dynamic_cast<caf::PdmObjectHandle*>(currentSelectedItem);
    if (pdmObject)
    {
        PdmChildArrayFieldHandle* parentChildArray = dynamic_cast<PdmChildArrayFieldHandle*>(pdmObject->parentField());

        if (parentChildArray)
        {
            return parentChildArray;
        }
    }

    return NULL;
}


} // end namespace caf
