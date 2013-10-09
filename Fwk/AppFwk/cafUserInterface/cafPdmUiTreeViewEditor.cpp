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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmUiEditorHandle.h"
#include "cafUiTreeModelPdm.h"

#include <QWidget>
#include <QGridLayout>
#include <QTreeView>
#include <QSortFilterProxyModel>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class MySortFilterProxyModel : public QSortFilterProxyModel
{
public:
    MySortFilterProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {

    }

    void notifyModelChanged()
    {
        QModelIndex startModelIdx = index(0,0);
        QModelIndex endModelIdx = index(rowCount(startModelIdx), 0);

        emit dataChanged(startModelIdx, endModelIdx);
    }
};



namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::PdmUiTreeViewEditor()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::~PdmUiTreeViewEditor()
{
    if (m_proxyEditor)
    {
        delete m_proxyEditor;
        m_proxyEditor = NULL;
    }
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

    m_treeModelPdm = new caf::UiTreeModelPdm(m_mainWidget);
    m_treeView = new QTreeView(m_mainWidget);

    m_proxyTreeModelPdm = new MySortFilterProxyModel(m_mainWidget);
    m_proxyTreeModelPdm->setSourceModel(m_treeModelPdm);
    m_treeView->setModel(m_proxyTreeModelPdm);

    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(1, Qt::AscendingOrder);

    
    m_layout->addWidget(m_treeView);

    m_proxyEditor = new PdmUiProxyEditorHandle(this);
    
    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    if (this->pdmObject())
    {
        this->pdmObject()->objectEditorAttribute(uiConfigName, &m_editorAttributes);
    }

    if (!m_treeModelPdm->treeItemRoot() || m_treeModelPdm->treeItemRoot()->dataObject() != this->pdmObject())
    {
        caf::PdmUiTreeItem* treeItemRoot = caf::UiTreeItemBuilderPdm::buildViewItems(NULL, -1, this->pdmObject());
        m_treeModelPdm->setTreeItemRoot(treeItemRoot);
    }

    // Update tree model, and set the proxy editor for all Pdm child objects
    if (m_treeModelPdm->treeItemRoot() && m_treeModelPdm->treeItemRoot()->dataObject())
    {
        m_treeModelPdm->updateUiSubTree(m_treeModelPdm->treeItemRoot()->dataObject());

        std::vector<PdmObject*> children;
        childObjects(m_treeModelPdm->treeItemRoot()->dataObject(), children);

        size_t cIdx;
        for (cIdx = 0; cIdx < children.size(); ++cIdx)
        {
            addEditorRecursively(children[cIdx], m_proxyEditor);
        }
    }

    m_treeModelPdm->setColumnHeaders(m_editorAttributes.columnHeaders);

    // Notify all connected views that the complete model is updated
    m_treeModelPdm->notifyModelChanged();
    m_proxyTreeModelPdm->notifyModelChanged();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::addEditorRecursively(PdmObject* pdmObject, PdmUiEditorHandle* editorHandle)
{
    if (!pdmObject) return;

    std::vector<PdmObject*> children;
    childObjects(pdmObject, children);

    size_t cIdx;
    for (cIdx = 0; cIdx < children.size(); ++cIdx)
    {
        addEditorRecursively(children[cIdx], editorHandle);
    }

    pdmObject->addFieldEditor(editorHandle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::childObjects(PdmObject* pdmObject, std::vector<PdmObject*>& children)
{
    if (!pdmObject) return;

    std::vector<PdmFieldHandle*> fields;
    pdmObject->fields(fields);

    size_t fIdx;
    for (fIdx = 0; fIdx < fields.size(); ++fIdx)
    {
        if (fields[fIdx]) fields[fIdx]->childObjects(&children);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTreeView* PdmUiTreeViewEditor::treeView()
{
    return m_treeView;
}



} // end namespace caf
