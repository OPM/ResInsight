//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafPdmUiPickableLineEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPickEventHandler.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiPickableLineEditor::~PdmUiPickableLineEditor()
{
    if (m_attribute.pickEventHandler)
    {
        m_attribute.pickEventHandler->unregisterAsPickEventHandler();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiPickableLineEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    PdmUiLineEditor::configureAndUpdateUi(uiConfigName);

    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attribute);
    }

    if (m_attribute.pickEventHandler)
    {
        if (m_attribute.enablePicking)
        {
            m_attribute.pickEventHandler->registerAsPickEventHandler();
            m_lineEdit->setStyleSheet("QLineEdit {"
                                      "color: #000000;"
                                      "background-color: #FFDCFF;}");            
        }
        else
        {
            m_attribute.pickEventHandler->unregisterAsPickEventHandler();
            m_lineEdit->setStyleSheet("");
        }
    }

    m_lineEdit->setToolTip(uiField()->uiToolTip(uiConfigName));
}

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire build. The
// variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same line number.
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiPickableLineEditor);
