//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUiObjectEditorHandle.h"

#include <QPointer>
#include <QWidget>

namespace caf 
{

//==================================================================================================
/// Abstract class to handle widget based editors for complete PdmObjects
//==================================================================================================

class PdmUiWidgetObjectEditorHandle : public PdmUiObjectEditorHandle
{
public:
    PdmUiWidgetObjectEditorHandle();
    ~PdmUiWidgetObjectEditorHandle() override;
   
    QWidget*                getOrCreateWidget(QWidget* parent);
    QWidget*                widget() const;
 
protected:
    /// Supposed to create the top level widget of the editor with a suitable QLayout 
    virtual QWidget*        createWidget(QWidget* parent) = 0;

private:
    QPointer<QWidget>       m_widget;
};



} // End of namespace caf

