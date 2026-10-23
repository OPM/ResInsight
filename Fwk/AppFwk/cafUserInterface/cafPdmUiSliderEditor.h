//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of the GNU General Public License or
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

#include "cafPdmUiFieldLabelEditorHandle.h"

#include <QGroupBox>
#include <QLabel>
#include <QPointer>
#include <QSlider>
#include <QSpinBox>
#include <QString>
#include <QWidget>

namespace caf
{
class PdmUiSliderEditor : public PdmUiFieldLabelEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiSliderEditor() {}
    ~PdmUiSliderEditor() override {}

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        static inline const QString MINIMUM       = QStringLiteral( "minimum" );
        static inline const QString MAXIMUM       = QStringLiteral( "maximum" );
        static inline const QString SHOW_SPIN_BOX = QStringLiteral( "showSpinBox" );
        static inline const QString STEP          = QStringLiteral( "step" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::MINIMUM,
                                                                   Keys::MAXIMUM,
                                                                   Keys::SHOW_SPIN_BOX,
                                                                   Keys::STEP };

    // Clearing restores the editor defaults, use when the range is unknown to avoid keeping a stale range
    static void setRange( PdmUiItem* uiItem, int minimum, int maximum );
    static void clearRange( PdmUiItem* uiItem );

protected:
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    QWidget* createEditorWidget( QWidget* parent ) override;

protected slots:
    void slotSliderValueChanged( int position );
    void slotSpinBoxValueChanged( int position );

private:
    void updateSliderPosition();
    void writeValueToField();

private:
    QPointer<QSpinBox> m_spinBox;
    QPointer<QSlider>  m_slider;

    int  m_minimum     = 0;
    int  m_maximum     = 10;
    bool m_showSpinBox = true;
    int  m_step        = 1;
};

} // end namespace caf
