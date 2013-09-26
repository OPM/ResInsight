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
#include "cafPdmUiObjectEditorHandle.h"
#include <map>
#include <QString>
#include <QWidget>
#include <QPointer>
#include <QGroupBox>

class QGridLayout;

namespace caf 
{
class PdmUiFieldEditorHandle;
class PdmUiItem;

//==================================================================================================
/// The default editor for PdmObjects. Manages the field editors in a gridlayout vertically
//==================================================================================================

class PdmUiDefaultObjectEditor : public PdmUiObjectEditorHandle
{
public:
    PdmUiDefaultObjectEditor();
    ~PdmUiDefaultObjectEditor();

protected:
    virtual QWidget* createWidget(QWidget* parent);
    virtual void     configureAndUpdateUi(const QString& uiConfigName);

private:
    void recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName );

    std::map<QString, PdmUiFieldEditorHandle*>  m_fieldViews; 
    std::map<QString, QPointer<QGroupBox> >     m_groupBoxes;
    std::map<QString, QPointer<QGroupBox> >     m_newGroupBoxes; ///< used temporarily to store the new(complete) set of group boxes

    QPointer<QWidget>                           m_mainWidget;
    QGridLayout*                                m_layout;
};



} // end namespace caf
