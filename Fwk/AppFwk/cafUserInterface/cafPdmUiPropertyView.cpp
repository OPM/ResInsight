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


#include "cafPdmUiPropertyView.h"

#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"

#include <QHBoxLayout>


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyView::PdmUiPropertyView(QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f)
{
    m_layout = new QVBoxLayout(this);
    m_layout->insertStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    setLayout(m_layout);

    m_currentObjectView = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyView::~PdmUiPropertyView()
{
    if (m_currentObjectView) delete m_currentObjectView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyView::setUiConfigurationName(QString uiConfigName)
{
    // Reset everything, and possibly create widgets etc afresh
    if (m_uiConfigName != uiConfigName)
    { 
        m_uiConfigName = uiConfigName;

        if (m_currentObjectView)
        {
            PdmObject* object = m_currentObjectView->pdmObject();
            delete m_currentObjectView;
            m_currentObjectView = NULL;
            this->showProperties(object);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyView::showProperties(caf::PdmObject* object)
{
    // Find specialized object view handle 

    // If the current ObjectView has the same type as the one to view, reuse, with Widget etc.

    bool rebuildWidget = false;

    if (!m_currentObjectView) rebuildWidget = true;
        
    if (m_currentObjectView && m_currentObjectView->pdmObject())
    {
        if (object)
        {
            if (m_currentObjectView->pdmObject()->uiEditorTypeName(m_uiConfigName) != object->uiEditorTypeName(m_uiConfigName))
            {
                rebuildWidget = true;
            }
        }
    }

    if (rebuildWidget)
    {
        // Remove Widget from layout
        if (m_currentObjectView)
        {
            layout()->removeWidget(m_currentObjectView->widget());
            delete m_currentObjectView;
            m_currentObjectView = NULL;
        }

        //m_currentObjectView = PdmObjViewFactory::instance()->create(object->editorType(m_uiConfigName));
        if (!m_currentObjectView)
        {
            PdmUiDefaultObjectEditor* defaultEditor = new PdmUiDefaultObjectEditor();
            m_currentObjectView = defaultEditor;
        }

        // Create widget to handle this
        QWidget * page = NULL;
        page = m_currentObjectView->getOrCreateWidget(this);

        assert(page);

        this->m_layout->insertWidget(0, page);
    }

    m_currentObjectView->setPdmObject(object);
    m_currentObjectView->updateUi(m_uiConfigName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* PdmUiPropertyView::currentObject()
{
    if (!m_currentObjectView) return NULL;
    return m_currentObjectView->pdmObject();
}


} //End of namespace caf

