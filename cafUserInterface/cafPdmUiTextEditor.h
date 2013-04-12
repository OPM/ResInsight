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
#include <QString>
#include <QWidget>
#include <QPointer>
#include <QTextEdit>
#include <QLabel>

class QGridLayout;

namespace caf 
{

//==================================================================================================
/// An editor to show (and possibly edit?) formatted larger portions of text
//==================================================================================================

class PdmUiTextEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiTextEditorAttribute()
    {
        textMode = PLAIN;
    }

    enum TextMode 
    {
        PLAIN, 
        HTML
    };

public:
    TextMode textMode;
};


class PdmUiTextEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTextEditor()          { m_textMode = PdmUiTextEditorAttribute::PLAIN; } 
    virtual ~PdmUiTextEditor() {} 

protected:
    virtual QWidget*    createEditorWidget(QWidget * parent);
    virtual QWidget*    createLabelWidget(QWidget * parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);

protected slots:
    void                slotTextChanged();

private:
    QPointer<QTextEdit> m_textEdit;
    QPointer<QLabel>    m_label;

    PdmUiTextEditorAttribute::TextMode m_textMode; 
};


} // end namespace caf
