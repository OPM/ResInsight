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

#pragma once

#include "cafPdmUiFieldEditorHandle.h"

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QWidget>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiCheckBoxEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiCheckBoxEditorAttribute() { m_useNativeCheckBoxLabel = false; }

public:
    bool m_useNativeCheckBoxLabel;
};

//==================================================================================================
//
// Checkbox editor used to display default Qt checkbox
// On Windows, the default behavior is like this
//
//  "some text as label" [x]
//
//==================================================================================================
class PdmUiCheckBoxEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiCheckBoxEditor() {}
    ~PdmUiCheckBoxEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotClicked( bool checked );

    virtual PdmUiCheckBoxEditorAttribute defaultAttributes() const;

private:
    QPointer<QCheckBox>       m_checkBox;
    QPointer<QShortenedLabel> m_label;
};

//==================================================================================================
//
// Check box editor used to display native checkbox
// On Windows, the default behavior to show the checkbox to the left of the label text
//
//  [x] "some text as label"
//
//==================================================================================================
class PdmUiNativeCheckBoxEditor : public PdmUiCheckBoxEditor
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiNativeCheckBoxEditor()           = default;
    ~PdmUiNativeCheckBoxEditor() override = default;

    static void configureFieldForEditor( caf::PdmFieldHandle* fieldHandle );

protected:
    PdmUiCheckBoxEditorAttribute defaultAttributes() const override;
};

} // end namespace caf
