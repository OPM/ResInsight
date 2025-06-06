//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
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

#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QWidget>
class QHBoxLayout;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiActionPushButtonEditorAttribute : public PdmUiEditorAttribute
{
public:
    // This is unused for now, but relevant customization would be:
    // * Size requirement to make several buttons same size
    //   possibly in the form of number of "X" characters
    // * Horizontal Alignment options controlling how the button will place itself in the
    //   assigned area: Fill, Left, Right Center  default is Right
    //
    // * Optional label/icon to use in front of button
};

//--------------------------------------------------------------------------------------------------
/// Button using the uiIcon or (if not present) the uiName of the field as button text.
/// Aligns to the left by default, and uses only the minimum needed place.
/// Expects to work with a PdmField<bool>
//--------------------------------------------------------------------------------------------------
class PdmUiActionPushButtonEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiActionPushButtonEditor() {}
    ~PdmUiActionPushButtonEditor() override {}

    static void configureEditorForField( PdmFieldHandle* fieldHandle );

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotClicked( bool checked );

private:
    QPointer<QPushButton> m_pushButton;
    QPointer<QHBoxLayout> m_buttonLayout;
};

} // end namespace caf
