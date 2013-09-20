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
#include <QString>
#include <QWidget>

class QVBoxLayout;

namespace caf
{

class PdmObject;
class PdmUiObjectEditorHandle;

//==================================================================================================
/// 
//==================================================================================================

class PdmUiPropertyView : public QWidget
{
    Q_OBJECT
public:
    PdmUiPropertyView(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~PdmUiPropertyView();

    void setUiConfigurationName(QString uiConfigName);
    caf::PdmObject* currentObject();
public slots:
    void showProperties(caf::PdmObject* object);

private:
    PdmUiObjectEditorHandle*    m_currentObjectView; 
    QString                     m_uiConfigName;
    QVBoxLayout*                m_layout;
};



} // End of namespace caf

