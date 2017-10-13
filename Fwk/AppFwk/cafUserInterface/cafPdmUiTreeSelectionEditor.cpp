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
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiTreeSelectionQModel.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <algorithm>


//==================================================================================================
/// Helper class used to control height of size hint
//==================================================================================================
class QTreeViewHeightHint : public QTreeView
{
public:
    explicit QTreeViewHeightHint(QWidget *parent = 0)
        : m_heightHint(-1)
    {
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual QSize sizeHint() const override
    {
        QSize mySize = QTreeView::sizeHint();

        if (m_heightHint > 0)
        {
            mySize.setHeight(m_heightHint);
        }

        return mySize;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void setHeightHint(int heightHint)
    {
        m_heightHint = heightHint;
    }

private:
    int m_heightHint;
};


//==================================================================================================
/// 
//==================================================================================================
class FilterLeafNodesOnlyProxyModel : public QSortFilterProxyModel
{
public:
    FilterLeafNodesOnlyProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
    }

protected:
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

        if (sourceModel()->hasChildren(index))
        {
            // Always include node if node has children
            return true;
        }

        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
};


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
    : m_model(nullptr),
    m_proxyModel(nullptr)
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

    if (!m_model)
    {
        m_model = new caf::PdmUiTreeSelectionQModel(m_treeView);
        
        m_proxyModel = new FilterLeafNodesOnlyProxyModel;
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        
        m_treeView->setModel(m_proxyModel);

        connect(m_treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                this, SLOT(slotCurrentChanged(QModelIndex, QModelIndex)));
    }

    bool optionsOnly = true;
    QList<PdmOptionItemInfo> options = field()->valueOptions(&optionsOnly);

    bool itemCountHasChaged = false;
    if (m_model->optionItemCount() != options.size()) itemCountHasChaged = true;

    // TODO: If the count is different between incoming and current list of items,
    // use cafQTreeViewStateSerializer to restore collapsed state
    m_model->setOptions(this, options);

    if (itemCountHasChaged)
    {
        m_treeView->expandAll();
    }

    QVariant fieldValue = field()->uiValue();
    if (PdmUiTreeSelectionQModel::isSingleValueField(fieldValue))
    {
        m_textFilterLineEdit->hide();
        m_toggleAllCheckBox->hide();
    }
    else if (PdmUiTreeSelectionQModel::isMultipleValueField(fieldValue))
    {

        connect(m_treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

        caf::PdmUiObjectHandle* uiObject = uiObj(field()->fieldHandle()->ownerObject());
        if (uiObject)
        {
            uiObject->editorAttribute(field()->fieldHandle(), uiConfigName, &m_attributes);
        }

        if (m_attributes.singleSelectionMode)
        {
            m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
            m_treeView->setContextMenuPolicy(Qt::NoContextMenu);
        
            m_model->enableSingleSelectionMode(m_attributes.singleSelectionMode);

            connect(m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)));
        }
        else
        {
            m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
            m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        }

        if (!m_attributes.showTextFilter)
        {
            m_textFilterLineEdit->hide();
        }

        if (m_attributes.singleSelectionMode || !m_attributes.showToggleAllCheckbox)
        {
            m_toggleAllCheckBox->hide();
        }
        else
        {
            if (options.size() == 0)
            {
                m_toggleAllCheckBox->setChecked(false);
            }
            else
            {
                QModelIndexList indices = allVisibleSourceModelIndices();
                if (indices.size() > 0)
                {
                    bool allItemsChecked = true;
                    for (auto mi : indices)
                    {
                        if (m_model->data(mi, Qt::CheckStateRole).toBool() == false)
                        {
                            allItemsChecked = false;
                        }
                    }

                    m_toggleAllCheckBox->setChecked(allItemsChecked);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createEditorWidget(QWidget* parent)
{
    QFrame* frame = new QFrame(parent);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(layout);

    {
        QHBoxLayout* headerLayout = new QHBoxLayout;
        headerLayout->setContentsMargins(0, 0, 0, 0);
        layout->addLayout(headerLayout);

        PdmUiTreeSelectionEditorAttribute attrib;

        m_toggleAllCheckBox = new QCheckBox();
        headerLayout->addWidget(m_toggleAllCheckBox);

        connect(m_toggleAllCheckBox, SIGNAL(clicked(bool)), this, SLOT(slotToggleAll()));

        m_textFilterLineEdit = new QLineEdit();
        headerLayout->addWidget(m_textFilterLineEdit);

        connect(m_textFilterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotTextFilterChanged()));
    }

    QTreeViewHeightHint* treeViewHeightHint = new QTreeViewHeightHint(parent);
    treeViewHeightHint->setHeightHint(2000);
    treeViewHeightHint->setHeaderHidden(true);
   
    m_treeView = treeViewHeightHint;

    layout->addWidget(treeViewHeightHint);

    return frame;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::customMenuRequested(const QPoint& pos)
{
    QMenu menu;

    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes(); 

    bool onlyHeadersInSelection = true;
    for (auto mi : selectedIndexes)
    {
        QVariant v = m_proxyModel->data(mi, PdmUiTreeSelectionQModel::headingRole());
        if (v.toBool() == false)
        {
            onlyHeadersInSelection = false;
        }
    }

    if (onlyHeadersInSelection)
    {
        {
            QAction* act = new QAction("Sub Items On", this);
            connect(act, SIGNAL(triggered()), SLOT(slotSetSubItemsOn()));

            menu.addAction(act);
        }

        {
            QAction* act = new QAction("Sub Items Off", this);
            connect(act, SIGNAL(triggered()), SLOT(slotSetSubItemsOff()));

            menu.addAction(act);
        }
    }
    else if (selectedIndexes.size() > 0)
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
    if (!m_proxyModel) return;

    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
    for (auto mi : selectedIndexes)
    {
        m_proxyModel->setData(mi, true, Qt::CheckStateRole);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSelectedOff()
{
    if (!m_proxyModel) return;

    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
    for (auto mi : selectedIndexes)
    {
        m_proxyModel->setData(mi, false, Qt::CheckStateRole);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSubItemsOn()
{
    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
    for (auto mi : selectedIndexes)
    {
        for (int i = 0; i < m_proxyModel->rowCount(mi); i++)
        {
            QModelIndex childIndex = m_proxyModel->index(i, 0, mi);
            m_proxyModel->setData(childIndex, true, Qt::CheckStateRole);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotSetSubItemsOff()
{
    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
    for (auto mi : selectedIndexes)
    {
        for (int i = 0; i < m_proxyModel->rowCount(mi); i++)
        {
            QModelIndex childIndex = m_proxyModel->index(i, 0, mi);
            m_proxyModel->setData(childIndex, false, Qt::CheckStateRole);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotToggleAll()
{
    if (m_toggleAllCheckBox->isChecked())
    {
        checkAllItems();
    }
    else
    {
        unCheckAllItems();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotTextFilterChanged()
{
    QString searchString = m_textFilterLineEdit->text();
    searchString += "*";

    m_proxyModel->setFilterWildcard(searchString);

    updateUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (m_attributes.singleSelectionMode)
    {
        m_proxyModel->setData(current, true, Qt::CheckStateRole);
    }

    if (m_attributes.fieldToReceiveCurrentItemValue)
    {
        PdmUiFieldHandle* uiFieldHandle = m_attributes.fieldToReceiveCurrentItemValue->uiCapability();
        if (uiFieldHandle)
        {
            QVariant v = m_proxyModel->data(current, PdmUiTreeSelectionQModel::optionItemValueRole());

            PdmUiCommandSystemProxy::instance()->setUiValueToField(uiFieldHandle, v);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::slotClicked(const QModelIndex& current)
{
    m_treeView->setCurrentIndex(current);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::checkAllItems()
{
    QModelIndexList indices = allVisibleSourceModelIndices();
    
    m_model->setCheckedStateForItems(indices, true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::unCheckAllItems()
{
    QModelIndexList indices = allVisibleSourceModelIndices();

    m_model->setCheckedStateForItems(indices, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndexList PdmUiTreeSelectionEditor::allVisibleSourceModelIndices() const
{
    QModelIndexList indices;

    recursiveAppendVisibleSourceModelIndices(QModelIndex(), &indices);

    return indices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeSelectionEditor::recursiveAppendVisibleSourceModelIndices(const QModelIndex& parent, QModelIndexList* sourceModelIndices) const
{
    for (int row = 0; row < m_proxyModel->rowCount(parent); row++)
    {
        QModelIndex mi = m_proxyModel->index(row, 0, parent);
        if (mi.isValid())
        {
            QVariant v = m_proxyModel->data(mi, PdmUiTreeSelectionQModel::headingRole());
            if (v.toBool() == false)
            {
                sourceModelIndices->push_back(m_proxyModel->mapToSource(mi));
            }

            recursiveAppendVisibleSourceModelIndices(mi, sourceModelIndices);
        }
    }
}



} // end namespace caf
