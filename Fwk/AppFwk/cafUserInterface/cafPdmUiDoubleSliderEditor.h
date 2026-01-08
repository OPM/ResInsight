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

#include "cafPdmUiFieldLabelEditorHandle.h"
#include "cafPdmUiSliderTools.h"

#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QSlider>
#include <QString>

class QWidget;

namespace caf
{

//==================================================================================================
///
//==================================================================================================
class PdmUiDoubleSliderEditor : public PdmUiFieldLabelEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiDoubleSliderEditor();
    ~PdmUiDoubleSliderEditor() override;

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        static inline const QString MINIMUM           = QStringLiteral( "minimum" );
        static inline const QString MAXIMUM           = QStringLiteral( "maximum" );
        static inline const QString DECIMALS          = QStringLiteral( "decimals" );
        static inline const QString SLIDER_TICK_COUNT = QStringLiteral( "sliderTickCount" );
        static inline const QString DELAY_SLIDER_UPDATE_UNTIL_RELEASE =
            QStringLiteral( "delaySliderUpdateUntilRelease" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::MINIMUM,
                                                                   Keys::MAXIMUM,
                                                                   Keys::DECIMALS,
                                                                   Keys::SLIDER_TICK_COUNT,
                                                                   Keys::DELAY_SLIDER_UPDATE_UNTIL_RELEASE };

protected:
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    QWidget* createEditorWidget( QWidget* parent ) override;

protected slots:
    void slotEditingFinished();
    void slotSliderValueChanged( int value );
    void slotSliderReleased();

private:
    void writeValueToField( double value );

private:
    QPointer<QLineEdit> m_lineEdit;
    QPointer<QSlider>   m_slider;
    double              m_sliderValue;

    PdmUiDoubleSliderEditorAttribute m_attributes;
};

} // end namespace caf
