//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafPdmUiTableRowEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTableViewQModel.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableRowEditor::PdmUiTableRowEditor(PdmUiTableViewQModel* model, caf::PdmObjectHandle* pdmObject, int row)
{
    m_model = model;
    m_row   = row;

    caf::PdmUiObjectHandle* uiObject = uiObj(pdmObject);
    this->bindToPdmItem(uiObject);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTableRowEditor::~PdmUiTableRowEditor() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTableRowEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    caf::PdmUiObjectHandle* uiObject = dynamic_cast<caf::PdmUiObjectHandle*>(this->pdmItem());
    if (uiObject)
    {
        // Call uiOrdering method, as this method is responsible for control of
        // object states like hidden/readOnly, etc...

        caf::PdmUiOrdering dummy;
        uiObject->uiOrdering(uiConfigName, dummy);
    }

    if (m_model)
    {
        QModelIndex miStart = m_model->index(m_row, 0);
        QModelIndex miEnd   = m_model->index(m_row, m_model->columnCount());

        m_model->notifyDataChanged(miStart, miEnd);
    }
}

} // end namespace caf
