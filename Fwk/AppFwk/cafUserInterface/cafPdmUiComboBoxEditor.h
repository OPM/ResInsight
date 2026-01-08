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

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QString>
#include <QToolButton>
#include <QWidget>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiComboBoxEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiComboBoxEditorAttribute()
    {
        adjustWidthToContents      = false;
        showPreviousAndNextButtons = false;
        minimumContentsLength      = 8;
        maximumMenuContentsLength  = 40;
        enableEditableContent      = false;
        enableAutoComplete         = true;
        minimumWidth               = -1;
        iconSize                   = QSize( 14, 14 );
        notifyWhenTextIsEdited     = false;
    }

public:
    bool adjustWidthToContents;
    bool showPreviousAndNextButtons;
    int  minimumContentsLength; // The length of string to adjust to if adjustWidthToContents = false.
                               // Set to <= 0 to ignore and use AdjustToContentsOnFirstShow instead
    int     maximumMenuContentsLength;
    bool    enableEditableContent;
    bool    enableAutoComplete;
    int     minimumWidth;
    QString placeholderText;
    QString nextButtonText;
    QString prevButtonText;

    QSize iconSize;
    QIcon nextIcon;
    QIcon previousIcon;

    bool notifyWhenTextIsEdited;
};

//==================================================================================================
///
//==================================================================================================
class PdmUiComboBoxEditor : public PdmUiFieldLabelEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiComboBoxEditor();
    ~PdmUiComboBoxEditor() override {}

    // Attribute key constants for compile-time safety and discoverability
    struct Keys
    {
        static inline const QString ADJUST_WIDTH_TO_CONTENTS       = QStringLiteral( "adjustWidthToContents" );
        static inline const QString SHOW_PREVIOUS_AND_NEXT_BUTTONS = QStringLiteral( "showPreviousAndNextButtons" );
        static inline const QString MINIMUM_CONTENTS_LENGTH        = QStringLiteral( "minimumContentsLength" );
        static inline const QString MAXIMUM_MENU_CONTENTS_LENGTH   = QStringLiteral( "maximumMenuContentsLength" );
        static inline const QString ENABLE_EDITABLE_CONTENT        = QStringLiteral( "enableEditableContent" );
        static inline const QString ENABLE_AUTO_COMPLETE           = QStringLiteral( "enableAutoComplete" );
        static inline const QString ICON_SIZE                      = QStringLiteral( "iconSize" );
        static inline const QString MINIMUM_WIDTH                  = QStringLiteral( "minimumWidth" );
        static inline const QString PLACEHOLDER_TEXT               = QStringLiteral( "placeholderText" );
        static inline const QString NEXT_BUTTON_TEXT               = QStringLiteral( "nextButtonText" );
        static inline const QString PREV_BUTTON_TEXT               = QStringLiteral( "prevButtonText" );
        static inline const QString NEXT_ICON                      = QStringLiteral( "nextIcon" );
        static inline const QString PREVIOUS_ICON                  = QStringLiteral( "previousIcon" );
        static inline const QString NOTIFY_WHEN_TEXT_IS_EDITED     = QStringLiteral( "notifyWhenTextIsEdited" );
    };

    // Set of all supported attributes for validation
    inline static const std::set<QString> SUPPORTED_ATTRIBUTES = { Keys::ADJUST_WIDTH_TO_CONTENTS,
                                                                   Keys::SHOW_PREVIOUS_AND_NEXT_BUTTONS,
                                                                   Keys::MINIMUM_CONTENTS_LENGTH,
                                                                   Keys::MAXIMUM_MENU_CONTENTS_LENGTH,
                                                                   Keys::ENABLE_EDITABLE_CONTENT,
                                                                   Keys::ENABLE_AUTO_COMPLETE,
                                                                   Keys::ICON_SIZE,
                                                                   Keys::MINIMUM_WIDTH,
                                                                   Keys::PLACEHOLDER_TEXT,
                                                                   Keys::NEXT_BUTTON_TEXT,
                                                                   Keys::PREV_BUTTON_TEXT,
                                                                   Keys::NEXT_ICON,
                                                                   Keys::PREVIOUS_ICON,
                                                                   Keys::NOTIFY_WHEN_TEXT_IS_EDITED };

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    QMargins calculateLabelContentMargins() const override;

protected slots:
    void slotIndexActivated( int index );
    void slotEditTextChanged( const QString& );

    void slotSetCurrentTextToField();
    void slotNextButtonPressed();
    void slotPreviousButtonPressed();
    void slotApplyAutoValue();

private:
    QPointer<QComboBox> m_comboBox;

    QPointer<QToolButton> m_previousItemButton;
    QPointer<QToolButton> m_nextItemButton;
    QPointer<QToolButton> m_autoValueToolButton;
    QPointer<QHBoxLayout> m_layout;
    QPointer<QWidget>     m_placeholder;

    PdmUiComboBoxEditorAttribute m_attributes;

    QString m_interactiveEditText;
    int     m_interactiveEditCursorPosition;
};

//--------------------------------------------------------------------------------------------------
// Special class used to prevent a combo box to steal focus when scrolling
// the QScrollArea using the mouse wheel
//
// Based on
// http://stackoverflow.com/questions/5821802/qspinbox-inside-a-qscrollarea-how-to-prevent-spin-box-from-stealing-focus-when
//--------------------------------------------------------------------------------------------------
class CustomQComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomQComboBox( QWidget* parent = nullptr );

    void wheelEvent( QWheelEvent* e ) override;

protected:
    void focusInEvent( QFocusEvent* e ) override;
    void focusOutEvent( QFocusEvent* e ) override;

signals:
    void signalFocusOutEvent();
};

} // end namespace caf
