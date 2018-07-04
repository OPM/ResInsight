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

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiTableViewEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTableViewEditor::PdmUiTableViewEditor()
{
    m_tableView = nullptr;
    m_tableHeading = nullptr;
    m_tableModelPdm = nullptr;
    m_tableHeadingIcon = nullptr;
    m_delegate = nullptr;
    m_previousFieldHandle = nullptr;

    m_useDefaultContextMenu = false;

    m_checkboxDelegate = new PdmUiCheckBoxDelegate(this);

    m_selectionRole = SelectionManager::CURRENT;
    m_isBlockingSelectionManagerChanged = false;
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
QWidget* PdmUiTableViewEditor::createEditorWidget(QWidget* parent)
{
    m_tableModelPdm = new PdmUiTableViewQModel(parent);

    m_delegate = new PdmUiTableViewDelegate(this, m_tableModelPdm);

    m_tableView = new QTableView(parent);
    m_tableView->setShowGrid(true);
    m_tableView->setModel(m_tableModelPdm);

    connect(m_tableView->selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotSelectionChanged( const QItemSelection & , const QItemSelection & )));

    return m_tableView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTableViewEditor::createLabelWidget(QWidget * parent)
{
    if (m_tableHeading.isNull())
    {
        m_tableHeading = new QLabel(parent);
    }

    if (m_tableHeadingIcon.isNull())
    {
        m_tableHeadingIcon = new QLabel(parent);
    }

    QHBoxLayout* layoutForIconLabel = new QHBoxLayout();
    layoutForIconLabel->setMargin(0);
    layoutForIconLabel->addWidget(m_tableHeadingIcon);
    layoutForIconLabel->addSpacing(3);
    layoutForIconLabel->addWidget(m_tableHeading);
    layoutForIconLabel->addStretch();
    
    QWidget* widget = new QWidget(parent);
    widget->setLayout(layoutForIconLabel);

    return widget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    if (!m_tableModelPdm) return;

    auto childArrayFH = childArrayFieldHandle();

    if ( childArrayFH && childArrayFH->ownerObject() && childArrayFH->ownerObject()->uiCapability() )
    {
        PdmUiTableViewEditorAttribute editorAttrib;
        childArrayFH->ownerObject()->uiCapability()->editorAttribute(childArrayFH, uiConfigName, &editorAttrib);
        this->setSelectionRole(editorAttrib.selectionRole);
        this->enableHeaderText(editorAttrib.enableHeaderText);
    }

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
        if ( childArrayFH->uiCapability()->uiIcon(uiConfigName).isNull() )
        {
            m_tableHeadingIcon->setText(childArrayFH->uiCapability()->uiName(uiConfigName) + QString(" (%1)").arg(childArrayFH->size()));
            m_tableHeading->setText("");
        }
        else
        {
            m_tableHeadingIcon->setPixmap(childArrayFH->uiCapability()->uiIcon(uiConfigName).pixmap(16, 16));
            m_tableHeading->setText(childArrayFH->uiCapability()->uiName(uiConfigName) + QString(" (%1)").arg(childArrayFH->size()));
        }
        m_tableModelPdm->createPersistentPushButtonWidgets(m_tableView);
    }
    else
    {
        m_tableHeading->setText("");
        m_tableHeadingIcon->setPixmap(QPixmap());
    }

    if (m_previousFieldHandle != childArrayFH)
    {
        m_tableView->resizeColumnsToContents();
        m_tableView->resizeRowsToContents();
        m_previousFieldHandle = childArrayFH;
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
QTableView* PdmUiTableViewEditor::tableView()
{
    return m_tableView;
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
void PdmUiTableViewEditor::setSelectionRole(SelectionManager::SelectionRole role)
{
    m_selectionRole = role;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewEditor::onSelectionManagerSelectionChanged()
{
    if (m_isBlockingSelectionManagerChanged) return;

    if (isSelectionRoleDefined())
    {
        std::vector<PdmUiItem*> items;
        SelectionManager::instance()->selectedItems(items, m_selectionRole);

        QItemSelection totalSelection;
        for (auto item: items)
        {
            PdmObject* pdmObj = dynamic_cast<PdmObject*>(item);
            QItemSelection itemSelection = m_tableModelPdm->modelIndexFromPdmObject(pdmObj);
            totalSelection.merge(itemSelection, QItemSelectionModel::Select);
        }
 
        m_tableView->selectionModel()->select(totalSelection, QItemSelectionModel::SelectCurrent);
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
void PdmUiTableViewEditor::updateSelectionManagerFromTableSelection()
{
    if (isSelectionRoleDefined())
    {
        std::set<PdmUiItem*> selectedRowObjects;
        QModelIndexList modelIndexList = m_tableView->selectionModel()->selectedIndexes();
        for (const QModelIndex& mi : modelIndexList)
        {
            PdmObjectHandle* obj = m_tableModelPdm->pdmObjectForRow(mi.row());

            if (obj && obj->uiCapability())
            {
                selectedRowObjects.insert(obj->uiCapability());
            }
        }

        std::vector<PdmUiItem*> items { selectedRowObjects.begin(), selectedRowObjects.end() };

        m_isBlockingSelectionManagerChanged = true;
        SelectionManager::instance()->setSelectedItems(items, m_selectionRole);
        m_isBlockingSelectionManagerChanged = false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmChildArrayFieldHandle* PdmUiTableViewEditor::childArrayFieldHandle()
{
    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = nullptr;
    if (this->uiField())
    {
        childArrayFieldHandle = dynamic_cast<PdmChildArrayFieldHandle*>(this->uiField()->fieldHandle());
    }

    return childArrayFieldHandle;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTableViewPushButtonEditorAttribute::registerPushButtonTextForFieldKeyword(const QString& keyword, const QString& text)
{
    m_fieldKeywordAndPushButtonText[keyword] = text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTableViewPushButtonEditorAttribute::showPushButtonForFieldKeyword(const QString& keyword) const
{
    if (m_fieldKeywordAndPushButtonText.count(keyword) > 0) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString PdmUiTableViewPushButtonEditorAttribute::pushButtonText(const QString& keyword) const
{
    if (showPushButtonForFieldKeyword(keyword))
    {
        return m_fieldKeywordAndPushButtonText.at(keyword);
    }

    return "";
}

} // end namespace caf


