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
#include <QToolButton>
#include <QWidget>

class QGridLayout;

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmUiFilePathEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiFilePathEditorAttribute()
    {
        m_selectSaveFileName           = false;
        m_fileSelectionFilter          = "All files (*.*)";
        m_defaultPath                  = QString();
        m_selectDirectory              = false;
        m_appendUiSelectedFolderToText = false;
        m_multipleItemSeparator        = ';';
    }

public:
    bool    m_selectSaveFileName;
    QString m_fileSelectionFilter;

    QString m_defaultPath;
    bool    m_selectDirectory;
    bool    m_appendUiSelectedFolderToText;
    QChar   m_multipleItemSeparator;
};

//==================================================================================================
///
//==================================================================================================
class PdmUiFilePathEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiFilePathEditor() {}
    ~PdmUiFilePathEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotEditingFinished();
    void fileSelectionClicked();

private:
    QPointer<QLineEdit>       m_lineEdit;
    QPointer<QShortenedLabel> m_label;
    QPointer<QToolButton>     m_button;

    PdmUiFilePathEditorAttribute m_attributes;
};

} // end namespace caf
