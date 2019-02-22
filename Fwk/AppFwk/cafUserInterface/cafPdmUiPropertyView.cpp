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
#include "QTimer"



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

    m_defaultObjectEditor = nullptr;

    m_scrollToSelectedItemTimer = new QTimer(this);
    connect(m_scrollToSelectedItemTimer, SIGNAL(timeout()), this, SLOT(slotScrollToSelectedItemsInFieldEditors()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyView::~PdmUiPropertyView()
{
    if (m_defaultObjectEditor) delete m_defaultObjectEditor;
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

        if (m_defaultObjectEditor)
        {
            PdmObjectHandle* object = m_defaultObjectEditor->pdmObject();
            delete m_defaultObjectEditor;
            m_defaultObjectEditor = nullptr;
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

    if (!m_defaultObjectEditor) rebuildWidget = true;
        
    if (m_defaultObjectEditor && m_defaultObjectEditor->pdmObject())
    {
        if (object)
        {
            PdmUiObjectHandle* uiObject1 = uiObj(m_defaultObjectEditor->pdmObject());
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
        if (m_defaultObjectEditor)
        {
            this->m_placeHolderLayout->removeWidget(m_defaultObjectEditor->widget());
            delete m_defaultObjectEditor;
            m_defaultObjectEditor = nullptr;
        }

        if (!m_defaultObjectEditor)
        {
            m_defaultObjectEditor = new PdmUiDefaultObjectEditor();
        }

        // Create widget to handle this
        QWidget* propertyWidget = m_defaultObjectEditor->getOrCreateWidget(m_placeholder);
        
        CAF_ASSERT(propertyWidget);

        this->m_placeHolderLayout->insertWidget(0, propertyWidget);

        // Add stretch to make sure the property widget is not stretched
        this->m_placeHolderLayout->insertStretch(-1, 1);
    }

    m_defaultObjectEditor->setPdmObject(object);
    m_defaultObjectEditor->updateUi(m_uiConfigName);

    if (object)
    {
        if (!m_scrollToSelectedItemTimer->isActive())
        {
            m_scrollToSelectedItemTimer->setSingleShot(true);
            m_scrollToSelectedItemTimer->start(150);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyView::slotScrollToSelectedItemsInFieldEditors()
{
    if (m_defaultObjectEditor)
    {
        m_defaultObjectEditor->scrollToSelectedItemsInFieldEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmUiPropertyView::currentObject()
{
    if (!m_defaultObjectEditor) return nullptr;
    return m_defaultObjectEditor->pdmObject();
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

