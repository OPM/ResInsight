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
#include <QLineEdit>
#include <QGroupBox>
#include <QSlider>


namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================

class PdmUiSliderEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiSliderEditorAttribute()
    {
        m_minimum = 0;
        m_maximum = 10;
    }
public:
    int     m_minimum;
    int     m_maximum;
};


class PdmUiSliderEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiSliderEditor()          {} 
    virtual ~PdmUiSliderEditor() {} 

protected:
    virtual void        configureAndUpdateUi(const QString& uiConfigName);
    virtual QWidget*    createEditorWidget(QWidget * parent);

protected slots:
    void                slotEditingFinished();
    void                slotSliderValueChanged(int position);

private:
    void                updateSliderPosition();

private:
    QPointer<QGroupBox> m_groupBox;
    QPointer<QLineEdit> m_lineEdit;
    QPointer<QSlider>   m_slider;
  
    PdmUiSliderEditorAttribute m_attributes;
};


} // end namespace caf
