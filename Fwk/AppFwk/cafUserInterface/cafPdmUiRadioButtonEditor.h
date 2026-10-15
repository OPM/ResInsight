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

#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPointer>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWidget>

namespace caf
{

//==================================================================================================
///
//==================================================================================================
class PdmUiRadioButtonEditor : public PdmUiFieldLabelEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiRadioButtonEditor();
    ~PdmUiRadioButtonEditor() override;

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        // Qt::Orientation. Vertical (default) or horizontal layout of radio buttons
        static inline const QString ORIENTATION = QStringLiteral( "orientation" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::ORIENTATION };

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotRadioButtonToggled( bool checked );

private:
    void updateRadioButtons();
    void clearRadioButtons();

private:
    QPointer<QWidget>      m_containerWidget;
    QPointer<QGroupBox>    m_groupBox;
    QPointer<QVBoxLayout>  m_verticalLayout;
    QPointer<QHBoxLayout>  m_horizontalLayout;
    QPointer<QButtonGroup> m_buttonGroup;

    QList<QPointer<QRadioButton>> m_radioButtons;
    Qt::Orientation               m_orientation = Qt::Vertical;
};

} // end namespace caf