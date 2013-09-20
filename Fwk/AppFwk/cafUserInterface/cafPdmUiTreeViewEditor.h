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


#pragma once

#include "cafPdmUiObjectEditorHandle.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <QWidget>
#include <QPointer>

class QGridLayout;
class QVBoxLayout;
class QTreeView;

namespace caf 
{
class PdmUiFieldEditorHandle;
class PdmUiItem;
class UiTreeModelPdm;
class PdmUiProxyEditorHandle;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiTreeViewEditorAttribute()
    {
    }

public:
};


class PdmUiTreeViewEditor : public PdmUiObjectEditorHandle
{
public:
    PdmUiTreeViewEditor();
    ~PdmUiTreeViewEditor();

    QTreeView*          treeView();

protected:
    virtual QWidget*    createWidget(QWidget* parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

private:
    void                addEditorRecursively(PdmObject* pdmObject, PdmUiEditorHandle* editorHandle);

    static void         childObjects(PdmObject* pdmObject, std::vector<PdmObject*>& children);

private:
    QPointer<QWidget>   m_mainWidget;
    QVBoxLayout*        m_layout;

    QTreeView*          m_treeView;
    UiTreeModelPdm*     m_treeModelPdm;

    // Forward update events to the tree view editor connected to Pdm root object using a proxy editor
    PdmUiProxyEditorHandle* m_proxyEditor;

    PdmUiTreeViewEditorAttribute m_editorAttributes;
};



} // end namespace caf
