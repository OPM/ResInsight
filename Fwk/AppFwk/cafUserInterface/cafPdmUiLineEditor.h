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
#include <QLineEdit>
#include <QPointer>
#include <QString>
#include <QValidator>
#include <QWidget>

class QGridLayout;
class QCompleter;
class QStringListModel;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiLineEditorAttribute()
    {
        avoidSendingEnterEventToParentWidget = false;
        completerCaseSensitivity             = Qt::CaseInsensitive;
        completerFilterMode                  = Qt::MatchContains;
        maximumWidth                         = -1;
        selectAllOnFocusEvent                = false;
        placeholderText                      = "";
    }

public:
    bool                 avoidSendingEnterEventToParentWidget;
    QPointer<QValidator> validator;

    // Completer setup
    Qt::CaseSensitivity completerCaseSensitivity;
    Qt::MatchFlags      completerFilterMode;
    int                 maximumWidth;
    bool                selectAllOnFocusEvent;
    QString             placeholderText;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditorAttributeUiDisplayString : public PdmUiEditorAttribute
{
public:
    PdmUiLineEditorAttributeUiDisplayString() {}

public:
    QString m_displayString;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    PdmUiLineEdit( QWidget* parent );
    void setAvoidSendingEnterEventToParentWidget( bool avoidSendingEnter );

protected:
    void keyPressEvent( QKeyEvent* event ) override;

private:
    bool m_avoidSendingEnterEvent;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiLineEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiLineEditor()
        : m_ignoreCompleterActivated( false )
    {
    }
    ~PdmUiLineEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    QMargins calculateLabelContentMargins() const override;

    virtual bool eventFilter( QObject* watched, QEvent* event ) override;

protected slots:
    void slotEditingFinished();
    void slotCompleterActivated( const QModelIndex& index );

private:
    bool isMultipleFieldsWithSameKeywordSelected( PdmFieldHandle* editorField ) const;

protected:
    QPointer<PdmUiLineEdit>   m_lineEdit;
    QPointer<QShortenedLabel> m_label;

    QPointer<QCompleter>       m_completer;
    QPointer<QStringListModel> m_completerTextList;
    QList<PdmOptionItemInfo>   m_optionCache;
    bool                       m_ignoreCompleterActivated;

    int findIndexToOption( const QString& uiText );
};

} // end namespace caf
