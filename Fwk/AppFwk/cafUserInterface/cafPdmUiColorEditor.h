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

#include <QString>
#include <QWidget>
#include <QPointer>
#include <QLineEdit>
#include <QLabel>


namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================
class PdmUiColorEditorAttribute : public PdmUiEditorAttribute
{
public:
    bool showAlpha;

public:
    PdmUiColorEditorAttribute()
    {
        showAlpha = false;
    }
};


//==================================================================================================
/// See cafPdmFieldCvfColor for conversion between cvf::Color3f and QColor
//==================================================================================================
class PdmUiColorEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiColorEditor(); 
    virtual ~PdmUiColorEditor() {} 

protected:
    virtual QWidget*    createEditorWidget(QWidget * parent);
    virtual QWidget*    createLabelWidget(QWidget * parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

protected slots:
    void                colorSelectionClicked();

private:
    void                setColor(const QColor& c);

private:
    QPointer<QLabel>    m_label;

    QColor              m_color;
    QPointer<QLabel>    m_colorPixmapLabel;
    QPointer<QLabel>    m_colorTextLabel;
  
    PdmUiColorEditorAttribute m_attributes;
};


} // end namespace caf
