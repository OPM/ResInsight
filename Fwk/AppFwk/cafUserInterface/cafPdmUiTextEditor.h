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
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QWidget>

class QGridLayout;

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiTextEditorAttribute : public PdmUiEditorAttribute
{
public:
    enum WrapMode
    {
        NoWrap,
        WordWrap,
        ManualWrap,
        WrapAnywhere,
        WrapAtWordBoundaryOrAnywhere
    };

    PdmUiTextEditorAttribute()
    {
        textMode       = PLAIN;
        showSaveButton = false;
        wrapMode       = WrapAtWordBoundaryOrAnywhere;
        heightHint     = 2000;
    }

    enum TextMode
    {
        PLAIN,
        HTML
    };

public:
    TextMode textMode;
    bool     showSaveButton;
    WrapMode wrapMode;
    QFont    font;
    int      heightHint;
};

//==================================================================================================
/// Override focus out event and emit editinghFinished to avoid multiple field changed events
//==================================================================================================
class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit( QWidget* parent = nullptr );

    QSize sizeHint() const override;
    void  setHeightHint( int heightHint );

protected:
    void focusOutEvent( QFocusEvent* e ) override;

signals:
    void editingFinished();

private:
    int m_heightHint;
};

//==================================================================================================
/// An editor to show (and possibly edit?) formatted larger portions of text
//==================================================================================================
class PdmUiTextEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTextEditor() { m_textMode = PdmUiTextEditorAttribute::PLAIN; }
    ~PdmUiTextEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    bool     isMultiRowEditor() const override;

protected slots:
    void slotSetValueToField();

private:
    QTextOption::WrapMode toQTextOptionWrapMode( PdmUiTextEditorAttribute::WrapMode wrapMode );

private:
    QPointer<TextEdit>        m_textEdit;
    QPointer<QPushButton>     m_saveButton;
    QPointer<QShortenedLabel> m_label;

    PdmUiTextEditorAttribute::TextMode m_textMode;
};

} // end namespace caf
