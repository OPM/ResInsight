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

#include "cafPdmUiFieldEditorHandle.h"

#include <QLabel>
#include <QPointer>
#include <QString>
#include <QWidget>

#include <functional>

class QGridLayout;

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiLabelEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiLabelEditorAttribute()
        : m_useWordWrap( false )
        , m_useSingleWidgetInsteadOfLabelAndEditorWidget( false )
    {
    }

public:
    bool                                  m_useWordWrap;
    bool                                  m_useSingleWidgetInsteadOfLabelAndEditorWidget;
    QString                               m_linkText;
    std::function<void( const QString& )> m_linkActivatedCallback;
};

//==================================================================================================
/// An editor to show (and possibly edit?) formatted larger portions of text
//==================================================================================================
class PdmUiLabelEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiLabelEditor();
    ~PdmUiLabelEditor() override;

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        static inline const QString USE_WORD_WRAP = QStringLiteral( "useWordWrap" );
        static inline const QString USE_SINGLE_WIDGET_INSTEAD_OF_LABEL_AND_EDITOR_WIDGET =
            QStringLiteral( "useSingleWidgetInsteadOfLabelAndEditorWidget" );
        static inline const QString LINK_TEXT               = QStringLiteral( "linkText" );
        static inline const QString LINK_ACTIVATED_CALLBACK = QStringLiteral( "linkActivatedCallback" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::USE_WORD_WRAP,
                                                                   Keys::USE_SINGLE_WIDGET_INSTEAD_OF_LABEL_AND_EDITOR_WIDGET,
                                                                   Keys::LINK_TEXT,
                                                                   Keys::LINK_ACTIVATED_CALLBACK };

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

    QWidget* createCombinedWidget( QWidget* parent ) override;

private:
    QPointer<QLabel> m_label;
};

} // end namespace caf
