//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiFieldEditorHandle.h"

#include <vector>

#include <QString>

class QToolBar;
class QMainWindow;

namespace caf
{
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmFieldHandle;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiToolBarEditor : public PdmUiEditorHandle
{
public:
    PdmUiToolBarEditor(const QString& title, QMainWindow* mainWindow);
    ~PdmUiToolBarEditor() override;

    bool isEditorDataValid(const std::vector<caf::PdmFieldHandle*>& fields) const;
    void setFields(std::vector<caf::PdmFieldHandle*>& fields);
    void clear();

    void    setFocusWidgetFromKeyword(const QString& fieldKeyword);
    QString keywordForFocusWidget();
    
    void show();
    void hide();

    static QString uiEditorConfigName();

private:
    void configureAndUpdateUi(const QString& uiConfigName) override;
    
    static QWidget* focusWidget(PdmUiFieldEditorHandle* uiFieldEditorHandle);

private:
    QPointer<QToolBar> m_toolbar;

    std::vector<caf::PdmFieldHandle*>          m_fields;
    std::map<QString, PdmUiFieldEditorHandle*> m_fieldViews;

    QList<QAction*> m_actions;
};

} // end namespace caf
