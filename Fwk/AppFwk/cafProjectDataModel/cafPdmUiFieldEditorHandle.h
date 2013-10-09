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
#include <vector>
#include <QString>

#include "cafPdmUiItem.h"
#include <QPointer>
#include <QWidget>

#include "cafFactory.h"

#include "cafPdmUiEditorHandle.h"
#include <typeinfo>

// Taken from gtest.h
//
// Due to C++ preprocessor weirdness, we need double indirection to
// concatenate two tokens when one of them is __LINE__.  Writing
//
//   foo ## __LINE__
//
// will result in the token foo__LINE__, instead of foo followed by
// the current line number.  For more details, see
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.6
#define PDM_FIELD_EDITOR_STRING_CONCATENATE(foo, bar) PDM_FIELD_EDITOR_STRING_CONCATENATE_IMPL_(foo, bar)
#define PDM_FIELD_EDITOR_STRING_CONCATENATE_IMPL_(foo, bar) foo ## bar

namespace caf 
{

//==================================================================================================
/// Macros helping in development of PDM UI editors
//==================================================================================================

/// Create a QString based on a typename
#define qStringTypeName(TypeName) QString(typeid(TypeName).name())
    
/// CAF_PDM_UI_EDITOR_HEADER_INIT assists the factory used when creating editors
/// Place this in the header file inside the class definition of your PdmUiEditor

#define CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT \
public: \
    static QString uiEditorTypeName()

    /// CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT implements editorTypeName() and registers the field editor in the field editor factory
    /// Place this in the cpp file, preferably above the constructor

#define CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(EditorClassName) \
    QString EditorClassName::uiEditorTypeName() { return #EditorClassName; } \
    static bool PDM_FIELD_EDITOR_STRING_CONCATENATE(pdm_field_editor_registrate_, __LINE__) = caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance()->registerCreator<EditorClassName>(EditorClassName::uiEditorTypeName())

#define CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR(EditorClassName, TypeName) \
    static bool PDM_FIELD_EDITOR_STRING_CONCATENATE(pdm_field_register_default_editor_, __LINE__) = caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance()->registerCreator<EditorClassName>(qStringTypeName(caf::PdmField<TypeName>))

class PdmUiGroup;
class PdmFieldHandle;

//==================================================================================================
/// Abstract class to handle editors of PdmFields
//==================================================================================================

class PdmUiFieldEditorHandle : public PdmUiEditorHandle
{
public:

    PdmUiFieldEditorHandle();
    ~PdmUiFieldEditorHandle();

    PdmFieldHandle*     field(); 
    void                setField(PdmFieldHandle * field);

    void                createWidgets(QWidget * parent);
    QWidget*            combinedWidget()                        { return m_combinedWidget; }
    QWidget*            editorWidget()                          { return m_editorWidget; }
    QWidget*            labelWidget()                           { return m_labelWidget; }

protected: // Virtual interface to override
    /// Implement one of these, or both editor and label. The widgets will be used in the parent layout according to 
    /// being "Label" Editor" or a single combined widget. 

    virtual QWidget*    createCombinedWidget(QWidget * parent) { return NULL; }
    virtual QWidget*    createEditorWidget(QWidget * parent)   { return NULL; }
    virtual QWidget*    createLabelWidget(QWidget * parent)    { return NULL; }

    void                setValueToField(const QVariant& value);

private:
    QPointer<QWidget>   m_combinedWidget;
    QPointer<QWidget>   m_editorWidget;
    QPointer<QWidget>   m_labelWidget;
};

//==================================================================================================
/// Abstract base class to handle special editor dependent attributes
//==================================================================================================

class PdmUiEditorAttribute
{
public:
    PdmUiEditorAttribute() {}
    virtual ~PdmUiEditorAttribute() {} 
};


} // End of namespace caf

