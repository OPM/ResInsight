//##################################################################################################
//
//   Custom Visualization Core library
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


#include "cafPdmUiTreeSelectionEditor.h"

#include "cafAssert.h"
#include "cafPdmObject.h"
#include "cafPdmUiTreeSelectionQModel.h"

#include <QLabel>
#include <QMenu>
#include <QTreeView>


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiTreeSelectionEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeSelectionEditor::PdmUiTreeSelectionEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeSelectionEditor::~PdmUiTreeSelectionEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    // Label
    CAF_ASSERT(!m_label.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    // Tree view

    bool optionsOnly = true;
    QList<PdmOptionItemInfo> options = field()->valueOptions(&optionsOnly);

    if (!m_treeView->model())
    {
        caf::PdmUiTreeSelectionQModel* model = new caf::PdmUiTreeSelectionQModel(m_treeView);
        m_treeView->setModel(model);
        connect(model, SIGNAL(signalSelectionStateForIndexHasChanged(int, bool)), this, SLOT(slotSetSelectionStateForIndex(int, bool)));
    }

    caf::PdmUiTreeSelectionQModel* treeSelectionQModel = dynamic_cast<caf::PdmUiTreeSelectionQModel*>(m_treeView->model());
    if (treeSelectionQModel)
    {
        bool itemCountHasChaged = false;
        if (treeSelectionQModel->optionItemCount() != options.size()) itemCountHasChaged = true;

        // TODO: If the count is different between incoming and current list of items,
        // use cafQTreeViewStateSerializer to restore collapsed state
        treeSelectionQModel->setOptions(this, options);

        if (itemCountHasChaged)
        {
            m_treeView->expandAll();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSelectionStateForIndex(int index, bool setSelected)
{
    std::vector<int> indices;
    indices.push_back(index);

    setSelectionStateForIndices(indices, setSelected);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::customMenuRequested(const QPoint& pos)
{
    QMenu menu;

    {
        std::vector<int> items = selectedCheckableItems();
        if (items.size() > 0)
        {
            {
                QAction* act = new QAction("Set Selected On", this);
                connect(act, SIGNAL(triggered()), SLOT(slotSetSelectedOn()));

                menu.addAction(act);
            }

            {
                QAction* act = new QAction("Set Selected Off", this);
                connect(act, SIGNAL(triggered()), SLOT(slotSetSelectedOff()));

                menu.addAction(act);
            }
        }
    }

    {
        std::vector<int> items = selectedHeaderItems();
        if (items.size() > 0)
        {
            {
                QAction* act = new QAction("Set Sub Items On", this);
                connect(act, SIGNAL(triggered()), SLOT(slotSetSubItemsOn()));

                menu.addAction(act);
            }

            {
                QAction* act = new QAction("Set Sub Items Off", this);
                connect(act, SIGNAL(triggered()), SLOT(slotSetSubItemsOff()));

                menu.addAction(act);
            }
        }
    }

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
void PdmUiTreeSelectionEditor::slotSetSelectedOn()
{
    std::vector<int> items = selectedCheckableItems();
    if (items.size() > 0)
    {
        setSelectionStateForIndices(items, true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSelectedOff()
{
    std::vector<int> items = selectedCheckableItems();
    if (items.size() > 0)
    {
        setSelectionStateForIndices(items, false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSubItemsOn()
{
    caf::PdmUiTreeSelectionQModel* treeSelectionQModel = dynamic_cast<caf::PdmUiTreeSelectionQModel*>(m_treeView->model());

    std::vector<int> items = selectedHeaderItems();
    for (auto i : items)
    {
        std::vector<int> children = treeSelectionQModel->allSubItemIndices(i);

        setSelectionStateForIndices(children, true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSubItemsOff()
{
    caf::PdmUiTreeSelectionQModel* treeSelectionQModel = dynamic_cast<caf::PdmUiTreeSelectionQModel*>(m_treeView->model());

    std::vector<int> items = selectedHeaderItems();
    for (auto i : items)
    {
        std::vector<int> children = treeSelectionQModel->allSubItemIndices(i);

        setSelectionStateForIndices(children, false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> PdmUiTreeSelectionEditor::selectedCheckableItems() const
{
    std::vector<int> items;

    caf::PdmUiTreeSelectionQModel* treeSelectionQModel = dynamic_cast<caf::PdmUiTreeSelectionQModel*>(m_treeView->model());
    if (treeSelectionQModel)
    {
        QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();

        for (auto mi : selectedIndexes)
        {
            auto optionItem = treeSelectionQModel->optionItem(mi);
            if (!optionItem->isHeading())
            {
                items.push_back(treeSelectionQModel->optionItemIndex(mi));
            }
        }
    }

    return items;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<int> PdmUiTreeSelectionEditor::selectedHeaderItems() const
{
    std::vector<int> items;

    caf::PdmUiTreeSelectionQModel* treeSelectionQModel = dynamic_cast<caf::PdmUiTreeSelectionQModel*>(m_treeView->model());
    if (treeSelectionQModel)
    {
        QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();

        for (auto mi : selectedIndexes)
        {
            auto optionItem = treeSelectionQModel->optionItem(mi);
            if (optionItem->isHeading())
            {
                items.push_back(treeSelectionQModel->optionItemIndex(mi));
            }
        }
    }

    return items;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::setSelectionStateForIndices(const std::vector<int>& indices, bool setSelected)
{
    std::vector<unsigned int> selectedIndices;
    {
        QVariant fieldValue = field()->uiValue();
        QList<QVariant> fieldValueSelection = fieldValue.toList();

        for (auto v : fieldValueSelection)
        {
            selectedIndices.push_back(v.toUInt());
        }
    }

    for (auto index : indices)
    {
        unsigned int unsignedIndex = static_cast<unsigned int>(index);

        if (setSelected)
        {
            bool isIndexPresent = false;
            for (auto indexInField : selectedIndices)
            {
                if (indexInField == unsignedIndex)
                {
                    isIndexPresent = true;
                }
            }

            if (!isIndexPresent)
            {
                selectedIndices.push_back(unsignedIndex);
            }
        }
        else
        {
            selectedIndices.erase(std::remove(selectedIndices.begin(), selectedIndices.end(), unsignedIndex), selectedIndices.end());
        }
    }

    QList<QVariant> fieldValueSelection;
    for (auto v : selectedIndices)
    {
        fieldValueSelection.push_back(QVariant(v));
    }

    this->setValueToField(fieldValueSelection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createEditorWidget(QWidget * parent)
{
    m_treeView = new QTreeView(parent);

    m_treeView->setHeaderHidden(true);
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

    return m_treeView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}


} // end namespace caf
