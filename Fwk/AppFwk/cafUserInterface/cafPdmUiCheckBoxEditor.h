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

#include <QWidget>
#include <QPointer>
#include <QCheckBox>
#include <QLabel>

namespace caf 
{

//==================================================================================================
/// The default editor for several PdmFields.
//==================================================================================================

class PdmUiCheckBoxEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiCheckBoxEditorAttribute()
    {
        m_useNativeCheckBoxLabel = false;
    }

public:
    bool m_useNativeCheckBoxLabel;
};


class PdmUiCheckBoxEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiCheckBoxEditor()          {} 
    ~PdmUiCheckBoxEditor() override {} 

protected:
    QWidget*    createEditorWidget(QWidget * parent) override;
    QWidget*    createLabelWidget(QWidget * parent) override;
    void        configureAndUpdateUi(const QString& uiConfigName) override;

protected slots:
    void                slotClicked(bool checked);

private:
    QPointer<QCheckBox>            m_checkBox;
    QPointer<QShortenedLabel> m_label;
};


} // end namespace caf
