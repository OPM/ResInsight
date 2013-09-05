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

#include "cafPdmUiTreeView.h"

#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"

#include <QHBoxLayout>
#include "cafPdmUiTreeViewEditor.h"


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::PdmUiTreeView(QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f)
{
    m_layout = new QVBoxLayout(this);
    m_layout->insertStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    setLayout(m_layout);

    m_treeViewEditor = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::~PdmUiTreeView()
{
    if (m_treeViewEditor) delete m_treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setUiConfigurationName(QString uiConfigName)
{
    // Reset everything, and possibly create widgets etc afresh
    if (m_uiConfigName != uiConfigName)
    { 
        m_uiConfigName = uiConfigName;

        if (m_treeViewEditor)
        {
            PdmObject* object = m_treeViewEditor->pdmObject();
            delete m_treeViewEditor;
            m_treeViewEditor = NULL;
            this->showTree(object);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::showTree(caf::PdmObject* object)
{
    // Find specialized object view handle 

    // If the current ObjectView has the same type as the one to view, reuse, with Widget etc.

    bool rebuildWidget = false;

    if (!m_treeViewEditor) rebuildWidget = true;
        
    if (m_treeViewEditor && m_treeViewEditor->pdmObject())
    {
        if (object)
        {
            if (m_treeViewEditor->pdmObject()->uiEditorTypeName(m_uiConfigName) != object->uiEditorTypeName(m_uiConfigName))
            {
                rebuildWidget = true;
            }
        }
    }

    if (rebuildWidget)
    {
        // Remove Widget from layout
        if (m_treeViewEditor)
        {
            layout()->removeWidget(m_treeViewEditor->widget());
            delete m_treeViewEditor;
            m_treeViewEditor = NULL;
        }

        //m_currentObjectView = PdmObjViewFactory::instance()->create(object->editorType(m_uiConfigName));
        if (!m_treeViewEditor)
        {
            m_treeViewEditor = new PdmUiTreeViewEditor();
        }

        // Create widget to handle this
        QWidget * widget = NULL;
        widget = m_treeViewEditor->getOrCreateWidget(this);

        assert(widget);

        this->m_layout->insertWidget(0, widget);

        this->m_layout->setStretchFactor(widget, 10);

    }

    m_treeViewEditor->setPdmObject(object);
    m_treeViewEditor->updateUi(m_uiConfigName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* PdmUiTreeView::currentObject()
{
    if (!m_treeViewEditor) return NULL;
    return m_treeViewEditor->pdmObject();
}


} //End of namespace caf

