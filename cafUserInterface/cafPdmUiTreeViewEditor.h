//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cafPdmUiObjectEditorHandle.h"

#include <QWidget>
#include <QPointer>

class QGridLayout;
class QTreeView;

namespace caf 
{
class PdmUiFieldEditorHandle;
class PdmUiItem;
class UiTreeModelPdm;
class PdmUiProxyEditorHandle;


class PdmUiTreeViewEditor : public PdmUiObjectEditorHandle
{
public:
    PdmUiTreeViewEditor();
    ~PdmUiTreeViewEditor();

protected:
    virtual QWidget*    createWidget(QWidget* parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

private:
    void                addEditorRecursively(PdmObject* pdmObject, PdmUiEditorHandle* editorHandle);

    static void         childObjects(PdmObject* pdmObject, std::vector<PdmObject*>& children);

private:
    QPointer<QWidget>   m_mainWidget;
    QGridLayout*        m_layout;

    QTreeView*          m_treeView;
    UiTreeModelPdm*     m_treeModelPdm;

    // Forward update events to the tree view editor connected to Pdm root object using a proxy editor
    PdmUiProxyEditorHandle* m_proxyEditor;
};



} // end namespace caf
