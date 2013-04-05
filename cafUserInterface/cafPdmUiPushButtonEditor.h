//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once
#include "cafPdmUiFieldEditorHandle.h"

#include <QWidget>
#include <QPointer>
#include <QPushButton>
#include <QLabel>

namespace caf 
{

//==================================================================================================
/// The default editor for several PdmFields.
//==================================================================================================

class PdmUiPushButtonEditorAttribute : public PdmUiEditorAttribute
{
public:
    QIcon m_buttonIcon;
};


class PdmUiPushButtonEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiPushButtonEditor()          {} 
    virtual ~PdmUiPushButtonEditor() {} 

protected:
    virtual QWidget*    createEditorWidget(QWidget * parent);
    virtual QWidget*    createLabelWidget(QWidget * parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

protected slots:
    void                slotClicked(bool checked);

private:
    QPointer<QPushButton> m_pushButton;
    QPointer<QLabel>    m_label;
};


} // end namespace caf
