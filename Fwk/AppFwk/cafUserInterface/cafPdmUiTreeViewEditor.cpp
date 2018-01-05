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
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiDragDropInterface.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewModel.h"
#include "cafSelectionManager.h"

#include <QDragMoveEvent>
#include <QEvent>
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
class PdmUiTreeViewWidget : public QTreeView
{
public:
    explicit PdmUiTreeViewWidget(QWidget* parent = 0) : QTreeView(parent) {};
    virtual ~PdmUiTreeViewWidget() {};

    bool isTreeItemEditWidgetActive() const
    {
        return state() == QAbstractItemView::EditingState;
    }

protected:
    virtual void dragMoveEvent(QDragMoveEvent* event)
    {
        caf::PdmUiTreeViewModel* treeViewModel = dynamic_cast<caf::PdmUiTreeViewModel*>(model());
        if (treeViewModel && treeViewModel->dragDropInterface())
        {
            treeViewModel->dragDropInterface()->onProposedDropActionUpdated(event->proposedAction());
        }

        QTreeView::dragMoveEvent(event);
    }

    virtual void dragLeaveEvent(QDragLeaveEvent* event)
    {
        caf::PdmUiTreeViewModel* treeViewModel = dynamic_cast<caf::PdmUiTreeViewModel*>(model());
        if (treeViewModel && treeViewModel->dragDropInterface())
        {
            treeViewModel->dragDropInterface()->onDragCanceled();
        }

        QTreeView::dragLeaveEvent(event);
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::PdmUiTreeViewEditor()
{
    m_useDefaultContextMenu = false;
    m_updateSelectionManager = false;
    m_appendClassNameToUiItemText = false;
    m_layout = nullptr;
    m_treeView = nullptr;
    m_treeViewModel = nullptr;
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
    m_treeView = new PdmUiTreeViewWidget(m_mainWidget);
    m_treeView->setModel(m_treeViewModel);
    m_treeView->installEventFilter(this);

    connect(treeView()->selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotOnSelectionChanged( const QItemSelection & , const QItemSelection & )));

    m_layout->addWidget(m_treeView);

    updateContextMenuSignals();

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    PdmUiTreeViewEditorAttribute editorAttributes;

    {
        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>(this->pdmItemRoot());
        if (uiObjectHandle)
        {
            uiObjectHandle->objectEditorAttribute(uiConfigName, &editorAttributes);
        }
    }

    m_treeViewModel->setColumnHeaders(editorAttributes.columnHeaders);
    m_treeViewModel->setUiConfigName(uiConfigName);
    m_treeViewModel->setPdmItemRoot(this->pdmItemRoot());

    if (editorAttributes.currentObject)
    {
        PdmUiObjectHandle* uiObjectHandle = editorAttributes.currentObject->uiCapability();
        if (uiObjectHandle)
        {
            selectAsCurrentItem(uiObjectHandle);
        }
    }
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
bool PdmUiTreeViewEditor::isTreeItemEditWidgetActive() const
{
    return m_treeView->isTreeItemEditWidgetActive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectedUiItems(std::vector<PdmUiItem*>& objects)
{
    if (!this->treeView()) return;

    QModelIndexList idxList = this->treeView()->selectionModel()->selectedIndexes();

    for (int i = 0; i < idxList.size(); i++)
    {
        caf::PdmUiItem* item = this->m_treeViewModel->uiItemFromModelIndex(idxList[i]);
        if (item)
        {
            objects.push_back(item);
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
void PdmUiTreeViewEditor::enableSelectionManagerUpdating(bool enable)
{
    m_updateSelectionManager = enable;
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
    // This seems a bit strange. Why ?
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectAsCurrentItem(const PdmUiItem* uiItem)
{
    QModelIndex index = m_treeViewModel->findModelIndex(uiItem);
    m_treeView->setCurrentIndex(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::slotOnSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    this->updateSelectionManager();

    emit selectionChanged();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setExpanded(const PdmUiItem* uiItem, bool doExpand) const
{
    QModelIndex index = m_treeViewModel->findModelIndex(uiItem);
    m_treeView->setExpanded(index, doExpand);
    m_treeView->scrollTo(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmUiTreeViewEditor::uiItemFromModelIndex(const QModelIndex& index) const
{
    return m_treeViewModel->uiItemFromModelIndex(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewEditor::findModelIndex(const PdmUiItem* object) const
{
    return m_treeViewModel->findModelIndex(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setDragDropInterface(PdmUiDragDropInterface* dragDropInterface)
{
    m_treeViewModel->setDragDropInterface(dragDropInterface);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        this->updateSelectionManager();
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateSelectionManager()
{
    if (m_updateSelectionManager)
    {
        std::vector<PdmUiItem*> items;
        this->selectedUiItems(items);

        SelectionManager::instance()->setSelectedItems(items);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableAppendOfClassNameToUiItemText(bool enable)
{
    m_appendClassNameToUiItemText = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::isAppendOfClassNameToUiItemTextEnabled()
{
    return m_appendClassNameToUiItemText;
}


} // end namespace caf
