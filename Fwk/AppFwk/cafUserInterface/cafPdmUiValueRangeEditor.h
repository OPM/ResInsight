//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2023 Ceetron Solutions AS
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
#include "cafPdmUiSliderTools.h"

class QLineEdit;

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiValueRangeEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiValueRangeEditor();
    ~PdmUiValueRangeEditor() override;

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

private slots:
    void slotMinEditingFinished();
    void slotMaxEditingFinished();
    void slotMinSliderValueChanged( int value );
    void slotMaxSliderValueChanged( int value );
    void slotMinSliderReleased();
    void slotMaxSliderReleased();

private:
    void clampAndWriteValues( double valueMin, double valueMax, bool isMinChanged );
    void writeValues( double valueMin, double valueMax );

private:
    QPointer<QLineEdit> m_lineEditMin;
    QPointer<QSlider>   m_sliderMin;
    QPointer<QLineEdit> m_lineEditMax;
    QPointer<QSlider>   m_sliderMax;

    QPointer<QLabel> m_label;
    double           m_sliderValueMin;
    double           m_sliderValueMax;

    PdmUiDoubleSliderEditorAttribute m_attributes;
};

} // end namespace caf
