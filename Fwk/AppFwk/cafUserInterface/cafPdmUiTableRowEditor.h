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

#pragma once

#include "cafPdmUiEditorHandle.h"

#include <QPointer>

namespace caf
{
class PdmUiTableViewQModel;
class PdmObjectHandle;

//--------------------------------------------------------------------------------------------------
/// This class is used to update a row in the model when a field value changes
//--------------------------------------------------------------------------------------------------
class PdmUiTableRowEditor : public PdmUiEditorHandle
{
public:
    PdmUiTableRowEditor(PdmUiTableViewQModel* model, caf::PdmObjectHandle* pdmObject, int row);
    ~PdmUiTableRowEditor() override;

protected:
    void configureAndUpdateUi(const QString& uiConfigName) override;

private:
    QPointer<PdmUiTableViewQModel> m_model;
    int                            m_row;
};

} // End of namespace caf
