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
#pragma once

#include "cafPdmUiLineEditor.h"
#include "cafPdmCoreVec3d.h"

#include "cvfAssert.h"
#include "cvfVector3.h"

#include <memory>
#include <vector>

class QPushButton;

namespace caf
{
class PickEventHandler;

//==================================================================================================
///
//==================================================================================================
class PdmUiVec3dEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiVec3dEditorAttribute() : startInPickingMode(false) {}

public:
    bool                              startInPickingMode;
    std::shared_ptr<PickEventHandler> pickEventHandler;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiVec3dEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiVec3dEditor() {}
    ~PdmUiVec3dEditor() override;

protected:
    QWidget* createEditorWidget(QWidget* parent) override;
    QWidget* createLabelWidget(QWidget* parent) override;
    void     configureAndUpdateUi(const QString& uiConfigName) override;
    QMargins calculateLabelContentMargins() const override;

protected slots:
    void slotEditingFinished();
    void pickButtonToggled(bool checked);
private:
    bool isMultipleFieldsWithSameKeywordSelected(PdmFieldHandle* editorField) const;
    PickEventHandler* pickEventHandler();
private:
    QPointer<QLineEdit>   m_lineEdit;
    QPointer<QPushButton> m_pickButton;
    QPointer<QLabel>      m_label;

    PdmUiVec3dEditorAttribute m_attribute;
};

} // end namespace caf


