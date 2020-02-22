//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUiObjectEditorHandle.h"

#include "cafFactory.h"

#include <QWidget>
#include <QPointer>

// PdmUiObjectEditorHandle -<| PdmUiWidgetObjectEditorHandle --<| PdmUiFormLayoutObjectEditor 
//                         -<| PdmUi3dObjectEditorHandle
namespace caf
{

//==================================================================================================
/// Macros helping in development of PDM UI 3d editors
//==================================================================================================

/// CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT assists the factory used when creating editors
/// Place this in the header file inside the class definition of your PdmUiEditor

#define CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT \
public: \
    static QString uiEditorTypeName()

/// CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT  implements editorTypeName() and registers the field editor in the field editor factory
/// Place this in the cpp file, preferably above the constructor

#define CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT(EditorClassName) \
    QString EditorClassName::uiEditorTypeName() { return #EditorClassName; } \
    CAF_FACTORY_REGISTER(caf::PdmUi3dObjectEditorHandle, EditorClassName, QString, EditorClassName::uiEditorTypeName())

//==================================================================================================
/// Abstract class for 3D editors editing complete PdmObjects
//==================================================================================================

class PdmUi3dObjectEditorHandle : public caf::PdmUiObjectEditorHandle
{
public:
    PdmUi3dObjectEditorHandle();
    ~PdmUi3dObjectEditorHandle() override;

    void setViewer(QWidget* ownerViewer, bool isInComparisonView);

protected:
    QWidget* ownerViewer() const { return m_ownerViewer;}
    bool     isInComparisonView() const { return m_isInComparisonView; }

private:
    QPointer<QWidget>  m_ownerViewer;
    bool               m_isInComparisonView;
};

}

