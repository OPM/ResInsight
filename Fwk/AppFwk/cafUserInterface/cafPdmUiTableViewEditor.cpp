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


#include "cafPdmUiTableViewEditor.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiCheckBoxDelegate.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTableViewDelegate.h"
#include "cafPdmUiTableViewQModel.h"

#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QTableView>
#include <QWidget>



namespace caf
{

class FocusEventHandler : public QObject
{
public:
    explicit FocusEventHandler(PdmUiTableViewEditor* tableViewEditor)
        : QObject(tableViewEditor)
    {
        m_tableViewEditor = tableViewEditor;
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override 
    {
        if (event->type() == QEvent::FocusIn ||
            event->type() == QEvent::FocusOut)
        {
            m_tableViewEditor->tableViewWidgetFocusChanged(event);
        }

        // standard event processing
        return QObject::eventFilter(obj, event);
    }


private:
    PdmUiTableViewEditor* m_tableViewEditor;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTableViewEditor::PdmUiTableViewEditor()
{
    m_layout = nullptr;;
    m_tableView = nullptr;
    m_tableHeading = nullptr;
    m_tableModelPdm = nullptr;
    m_tableHeadingIcon = nullptr;
    m_delegate = nullptr;

    m_useDefaultContextMenu = false;

    m_checkboxDelegate = new PdmUiCheckBoxDelegate(this);

    m_selectionRole = SelectionManager::CURRENT;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTableViewEditor::~PdmUiTableViewEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTableViewEditor::createWidget(QWidget* parent)
{
    m_mainWidget = new QWidget(parent);

    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_mainWidget->setLayout(m_layout);

    m_tableModelPdm = new PdmUiTableViewQModel(m_mainWidget);

    m_delegate = new PdmUiTableViewDelegate(this, m_tableModelPdm);

    m_tableView = new QTableView(m_mainWidget);
    m_tableView->setShowGrid(true);
    m_tableView->setModel(m_tableModelPdm);

    connect(m_tableView->selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotSelectionChanged( const QItemSelection & , const QItemSelection & )));
    connect(m_tableView->selectionModel(), SIGNAL(currentChanged( const QModelIndex & , const QModelIndex & )), SLOT(slotCurrentChanged( const QModelIndex& , const QModelIndex& )));

    FocusEventHandler* tableViewWidgetFocusEventHandler = new FocusEventHandler(this);
    m_tableView->installEventFilter(tableViewWidgetFocusEventHandler);

    updateContextMenuSignals();

    QHBoxLayout* layoutForIconLabel = new QHBoxLayout();
    m_tableHeading = new QLabel(m_mainWidget);
    m_tableHeadingIcon = new QLabel(m_mainWidget);
    layoutForIconLabel->addWidget(m_tableHeadingIcon);
    layoutForIconLabel->addSpacing(5);
    layoutForIconLabel->addWidget(m_tableHeading);
    layoutForIconLabel->addStretch();

    m_layout->addItem(layoutForIconLabel);

    m_layout->addWidget(m_tableView);

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    auto childArrayFH = childArrayFieldHandle();
    m_tableModelPdm->setPdmData(childArrayFH, uiConfigName);

    if (m_tableModelPdm->rowCount() > 0)
    {
        for (int i = 0; i < m_tableModelPdm->columnCount(); i++)
        {
            if (m_tableModelPdm->isRepresentingBoolean(m_tableModelPdm->index(0, i)))
            {
                m_tableView->setItemDelegateForColumn(i, m_checkboxDelegate);
            }
            else
            {
                m_tableView->setItemDelegateForColumn(i, m_delegate);
            }
        }
    }

    if (childArrayFH && childArrayFH->uiCapability())
    {
        QString text = "";
        m_tableHeadingIcon->setPixmap(childArrayFH->uiCapability()->uiIcon(uiConfigName).pixmap(16, 16));
        m_tableHeading->setText(childArrayFH->uiCapability()->uiName(uiConfigName) + QString(" (%1)").arg(childArrayFH->size()));

        m_tableModelPdm->createPersistentPushButtonWidgets(m_tableView);
    }
    else
    {
        m_tableHeading->setText("");
        m_tableHeadingIcon->setPixmap(QPixmap());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::selectedUiItems(const QModelIndexList& modelIndexList, std::vector<PdmUiItem*>& objects)
{
    for (const QModelIndex& mi : modelIndexList)
    {
        int row = mi.row();

        caf::PdmUiObjectHandle* uiObject = uiObj(m_tableModelPdm->pdmObjectForRow(row));
        if (uiObject)
        {
            objects.push_back(uiObject);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::customMenuRequested(QPoint pos)
{
    // This is function is required to execute before populating the menu
    // Several commands rely on the activeChildArrayFieldHandle in the selection manager
    auto childArrayFH = childArrayFieldHandle();
    SelectionManager::instance()->setActiveChildArrayFieldHandle(childArrayFH);

    QMenu menu;
    caf::PdmUiCommandSystemProxy::instance()->populateMenuWithDefaultCommands("PdmUiTreeViewEditor", &menu);

    if (!menu.actions().empty())
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
        QPoint globalPos = m_tableView->viewport()->mapToGlobal(pos);

        menu.exec(globalPos);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTableView* PdmUiTableViewEditor::tableView()
{
    return m_tableView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::enableDefaultContextMenu(bool enable)
{
    m_useDefaultContextMenu = enable;

    updateContextMenuSignals();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::enableHeaderText(bool enable)
{
    m_tableHeading->setVisible(enable);
    m_tableHeadingIcon->setVisible(enable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::updateContextMenuSignals()
{
    if (!m_tableView) return;

    if (m_useDefaultContextMenu)
    {
        m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_tableView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));
    }
    else
    {
        m_tableView->setContextMenuPolicy(Qt::DefaultContextMenu);
        disconnect(m_tableView, nullptr, this, nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::setSelectionRole(SelectionManager::SelectionRole role)
{
    m_selectionRole = role;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if (isSelectionRoleDefined())
    {
        std::vector<PdmUiItem*> items;
        QModelIndexList list;
        list.append(current);
        selectedUiItems(list, items);

        SelectionManager::instance()->setSelectedItems(items, m_selectionRole);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::handleModelSelectionChange()
{
    if (isSelectionRoleDefined())
    {
        std::vector<PdmUiItem*> items;
        SelectionManager::instance()->selectedItems(items, m_selectionRole);

        // TODO: Handle multiple selection
        if (items.size() == 1)
        {
            PdmObject* pdmObj = dynamic_cast<PdmObject*>(items[0]);
            QItemSelection itemSelection = m_tableModelPdm->modelIndexFromPdmObject(pdmObj);
            if (!itemSelection.empty())
            {
                m_tableView->selectionModel()->select(itemSelection, QItemSelectionModel::SelectCurrent);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// NOTE: If no selection role is defined, the selection manager is not changed, the selection in the 
/// editor is local to the editor
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    updateSelectionManagerFromTableSelection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewEditor::isSelectionRoleDefined() const
{
    return m_selectionRole != SelectionManager::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmUiTableViewEditor::pdmObjectFromModelIndex(const QModelIndex& mi)
{
    if (mi.isValid())
    {
        return m_tableModelPdm->pdmObjectForRow(mi.row());
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::tableViewWidgetFocusChanged(QEvent* focusEvent)
{
    if (m_delegate->isEditorOpen())
    {
        // The table view emits focus out when a table cell editor is active
        // Do not update the selection when this state occurs
        return;
    }

    if (isSelectionRoleDefined())
    {
        if (focusEvent->type() == QEvent::FocusIn)
        {
            updateSelectionManagerFromTableSelection();
        }
        else if (focusEvent->type() == QEvent::FocusOut)
        {
            // Clearing the selection here causes the Menu to not display all items
            // Not sure how this can be handled correctly
            // SelectionManager::instance()->clear(m_selectionRole);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::updateSelectionManagerFromTableSelection()
{
    if (isSelectionRoleDefined())
    {
        std::vector<PdmUiItem*> items;

        QModelIndexList modelIndexList = m_tableView->selectionModel()->selectedIndexes();
        for (const QModelIndex& mi : modelIndexList)
        {
            PdmFieldHandle* pdmFieldHandle = m_tableModelPdm->getField(mi);

            if (pdmFieldHandle && pdmFieldHandle->uiCapability())
            {
                items.push_back(pdmFieldHandle->uiCapability());
            }
        }

        if (items.size() > 1)
        {
            // Selection of a single row is handled by slotCurrentChanged()
            // Multiple selection of fields is handled here
            SelectionManager::instance()->setSelectedItems(items, m_selectionRole);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* PdmUiTableViewEditor::childArrayFieldHandle()
{
    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = nullptr;
    if (this->field())
    {
        childArrayFieldHandle = dynamic_cast<PdmChildArrayFieldHandle*>(this->field()->fieldHandle());
    }

    return childArrayFieldHandle;
}

} // end namespace caf


