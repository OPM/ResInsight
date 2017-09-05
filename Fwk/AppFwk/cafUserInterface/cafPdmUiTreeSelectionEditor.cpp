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

#include <QTreeView>
#include <QLabel>

// #include <QAbstractItemModel>
// #include <QLabel>
// #include <QTreeView>




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
    bool optionsOnly = true;
    QList<PdmOptionItemInfo> options = field()->valueOptions(&optionsOnly);

    caf::PdmUiTreeSelectionQModel* model = new caf::PdmUiTreeSelectionQModel(m_treeView);
    m_treeView->setModel(model);

    // TODO: Wire up signal from model to editor

    model->setOptions(this, options);

     m_treeView->expandAll();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeSelectionEditor::createEditorWidget(QWidget * parent)
{
    m_treeView = new QTreeView(parent);

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
