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

#include <QEvent>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QVerticalScrollArea::QVerticalScrollArea(QWidget* parent) :
    QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QVerticalScrollArea::eventFilter(QObject* object, QEvent* event)
{
    // This works because QScrollArea::setWidget installs an eventFilter on the widget
    if (object && object == widget() && event->type() == QEvent::Resize)
    {
        setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());
    }

    return QScrollArea::eventFilter(object, event);
}


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyView::PdmUiPropertyView(QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f)
{
    QVerticalScrollArea* scrollArea = new QVerticalScrollArea(this);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);

    m_placeholder = new QWidget();
    scrollArea->setWidget(m_placeholder);

    m_placeHolderLayout = new QVBoxLayout();
    m_placeHolderLayout->setContentsMargins(5,5,5,0);
    m_placeholder->setLayout(m_placeHolderLayout);

    QVBoxLayout* dummy = new QVBoxLayout(this);
    dummy->setContentsMargins(0,0,0,0);
    dummy->addWidget(scrollArea);

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
            PdmObjectHandle* object = m_currentObjectView->pdmObject();
            delete m_currentObjectView;
            m_currentObjectView = NULL;
            this->showProperties(object);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyView::showProperties( PdmObjectHandle* object)
{
    // Find specialized object view handle 

    // If the current ObjectView has the same type as the one to view, reuse, with Widget etc.

    bool rebuildWidget = false;

    if (!m_currentObjectView) rebuildWidget = true;
        
    if (m_currentObjectView && m_currentObjectView->pdmObject())
    {
        if (object)
        {
            PdmUiObjectHandle* uiObject1 = uiObj(m_currentObjectView->pdmObject());
            PdmUiObjectHandle* uiObject2 = uiObj(object);

            if (uiObject1 && uiObject2 && (uiObject1->uiEditorTypeName(m_uiConfigName) != uiObject2->uiEditorTypeName(m_uiConfigName)))
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
            this->m_placeHolderLayout->removeWidget(m_currentObjectView->widget());
            delete m_currentObjectView;
            m_currentObjectView = NULL;
        }

        if (!m_currentObjectView)
        {
            PdmUiDefaultObjectEditor* defaultEditor = new PdmUiDefaultObjectEditor();
            m_currentObjectView = defaultEditor;
        }

        // Create widget to handle this
        QWidget* propertyWidget = m_currentObjectView->getOrCreateWidget(m_placeholder);
        
        CAF_ASSERT(propertyWidget);

        this->m_placeHolderLayout->insertWidget(0, propertyWidget);

        // Add stretch to make sure the property widget is not stretched
        this->m_placeHolderLayout->insertStretch(-1, 1);
    }

    m_currentObjectView->setPdmObject(object);
    m_currentObjectView->updateUi(m_uiConfigName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmUiPropertyView::currentObject()
{
    if (!m_currentObjectView) return NULL;
    return m_currentObjectView->pdmObject();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize PdmUiPropertyView::sizeHint() const
{
    if (m_placeholder)
    {
        return m_placeholder->sizeHint();
    }

    return QWidget::sizeHint();
}

} //End of namespace caf

