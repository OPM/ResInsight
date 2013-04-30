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
#include <QLabel>
#include <QWidget>
#include <QPointer>
#include <QSpinBox>
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
    int m_minimum;
    int m_maximum;
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
    virtual QWidget*    createLabelWidget(QWidget * parent);

protected slots:
    void                slotSliderValueChanged(int position);
    void                slotSpinBoxValueChanged(int position);

private:
    void                updateSliderPosition();
    void                writeValueToField();

private:
    QPointer<QSpinBox>  m_spinBox;
    QPointer<QSlider>   m_slider;
    QPointer<QLabel>    m_label;
  
    PdmUiSliderEditorAttribute m_attributes;
};


} // end namespace caf
