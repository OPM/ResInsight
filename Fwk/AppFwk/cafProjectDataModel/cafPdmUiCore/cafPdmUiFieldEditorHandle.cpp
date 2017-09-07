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


#include "cafPdmUiFieldEditorHandle.h"

#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiCommandSystemProxy.h"

#include <QLabel>
#include <QVariant>

namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldEditorHandle::PdmUiFieldEditorHandle()
{
    m_combinedWidget = QPointer<QWidget>();
    m_editorWidget = QPointer<QWidget>();
    m_labelWidget = QPointer<QWidget>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldEditorHandle::~PdmUiFieldEditorHandle()
{
    if (!m_combinedWidget.isNull()) m_combinedWidget->deleteLater(); 
    if (!m_editorWidget.isNull())   m_editorWidget->deleteLater();
    if (!m_labelWidget.isNull())    m_labelWidget->deleteLater();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFieldEditorHandle::setField(PdmUiFieldHandle * field)
{
    this->bindToPdmItem(field);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle* PdmUiFieldEditorHandle::field()
{
    return dynamic_cast<PdmUiFieldHandle*>(pdmItem());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFieldEditorHandle::createWidgets(QWidget * parent)
{
    if (m_combinedWidget.isNull()) m_combinedWidget = createCombinedWidget(parent);
    if (m_editorWidget.isNull()) m_editorWidget = createEditorWidget(parent);
    if (m_labelWidget.isNull()) m_labelWidget = createLabelWidget(parent);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFieldEditorHandle::setValueToField(const QVariant& newUiValue)
{
    PdmUiCommandSystemProxy::instance()->setUiValueToField(field(), newUiValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiFieldEditorHandle::updateLabelFromField(QLabel* label, const QString& uiConfigName) const
{
    CAF_ASSERT(label);
    
    const PdmUiFieldHandle* fieldHandle = dynamic_cast<const PdmUiFieldHandle*>(pdmItem());
    if (fieldHandle)
    {
        QIcon ic = fieldHandle->uiIcon(uiConfigName);
        if (!ic.isNull())
        {
            label->setPixmap(ic.pixmap(ic.actualSize(QSize(64, 64))));
        }
        else
        {
            QString uiName = fieldHandle->uiName(uiConfigName);
            label->setText(uiName);
        }

        label->setEnabled(!fieldHandle->isUiReadOnly(uiConfigName));
        label->setToolTip(fieldHandle->uiToolTip(uiConfigName));
    }
}

} //End of namespace caf

