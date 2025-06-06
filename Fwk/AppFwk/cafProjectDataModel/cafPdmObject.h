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

#include "cafPdmObjectHandle.h"

class QXmlStreamReader;
class QXmlStreamWriter;

#include "cafInternalPdmUiFieldCapability.h"
#include "cafPdmObjectCapability.h"
#include "cafPdmUiObjectHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafPdmXmlObjectHandleMacros.h"

namespace caf
{
class PdmFieldHandle;
template <class FieldDataType>
class PdmField;
class PdmUiEditorAttribute;
class PdmUiTreeOrdering;
class PdmObjectCapability;

#define CAF_PDM_HEADER_INIT CAF_PDM_XML_HEADER_INIT
#define CAF_PDM_SOURCE_INIT CAF_PDM_XML_SOURCE_INIT
#define CAF_PDM_ABSTRACT_SOURCE_INIT CAF_PDM_XML_ABSTRACT_SOURCE_INIT

/// InitObject sets up the user interface related information for the object
/// Placed in the constructor of your PdmObject
/// Note that classKeyword() is not virtual in the constructor of the PdmObject
/// This is expected and fine.

#define CAF_PDM_InitObject( uiName, ... )                                                              \
    {                                                                                                  \
        const std::vector<QString> arguments = { __VA_ARGS__ };                                        \
        QString                    iconResourceName;                                                   \
        QString                    toolTip;                                                            \
        QString                    whatsThis;                                                          \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                     \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                            \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                          \
                                                                                                       \
        this->isInheritedFromPdmUiObject();                                                            \
        this->isInheritedFromPdmXmlSerializable();                                                     \
        this->registerClassKeyword( classKeyword() );                                                  \
                                                                                                       \
        static caf::PdmUiItemInfo objDescr( uiName, QString( iconResourceName ), toolTip, whatsThis ); \
        this->setUiItemInfo( &objDescr );                                                              \
    }

/// InitField sets the file keyword for the field,
/// adds the field to the internal data structure in the PdmObject,
/// sets the default value for the field,
/// and sets up the static user interface related information for the field
/// Note that classKeyword() is not virtual in the constructor of the PdmObject
/// This is expected and fine.

#define CAF_PDM_InitField( field, keyword, default, uiName, ... )                                                                       \
    {                                                                                                                                   \
        const std::vector<QString> arguments = { __VA_ARGS__ };                                                                         \
        QString                    iconResourceName;                                                                                    \
        QString                    toolTip;                                                                                             \
        QString                    whatsThis;                                                                                           \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                                                      \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                                                             \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                                                           \
        CAF_PDM_VERIFY_XML_KEYWORD( keyword )                                                                                           \
                                                                                                                                        \
        static bool chekingThePresenceOfHeaderAndSourceInitMacros =                                                                     \
            Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
        Q_UNUSED( chekingThePresenceOfHeaderAndSourceInitMacros )                                                                       \
        this->isInheritedFromPdmUiObject();                                                                                             \
        this->isInheritedFromPdmXmlSerializable();                                                                                      \
                                                                                                                                        \
        addXmlCapabilityToField( field );                                                                                               \
        addUiCapabilityToField( field );                                                                                                \
        registerClassWithField( classKeyword(), field );                                                                                \
                                                                                                                                        \
        static caf::PdmUiItemInfo objDescr( uiName, QString( iconResourceName ), toolTip, whatsThis, keyword );                         \
        addFieldUi( field, keyword, default, &objDescr );                                                                               \
    }

/// InitFieldNoDefault does the same as InitField but omits the default value.
/// Note that classKeyword() is not virtual in the constructor of the PdmObject
/// This is expected and fine.

#define CAF_PDM_InitFieldNoDefault( field, keyword, uiName, ... )                                                                       \
    {                                                                                                                                   \
        const std::vector<QString> arguments = { __VA_ARGS__ };                                                                         \
        QString                    iconResourceName;                                                                                    \
        QString                    toolTip;                                                                                             \
        QString                    whatsThis;                                                                                           \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                                                      \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                                                             \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                                                           \
        CAF_PDM_VERIFY_XML_KEYWORD( keyword )                                                                                           \
                                                                                                                                        \
        static bool checkingThePresenceOfHeaderAndSourceInitMacros =                                                                    \
            Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
        Q_UNUSED( checkingThePresenceOfHeaderAndSourceInitMacros )                                                                      \
        this->isInheritedFromPdmUiObject();                                                                                             \
        this->isInheritedFromPdmXmlSerializable();                                                                                      \
                                                                                                                                        \
        addXmlCapabilityToField( field );                                                                                               \
        addUiCapabilityToField( field );                                                                                                \
        registerClassWithField( classKeyword(), field );                                                                                \
                                                                                                                                        \
        static caf::PdmUiItemInfo objDescr( uiName, QString( iconResourceName ), toolTip, whatsThis, keyword );                         \
        addFieldUiNoDefault( field, keyword, &objDescr );                                                                               \
    }

} // End of namespace caf

namespace caf
{
class PdmObject : public PdmObjectHandle, public PdmXmlObjectHandle, public PdmUiObjectHandle
{
public:
    CAF_PDM_HEADER_INIT;

    PdmObject();
    ~PdmObject() override {}

    /// Returns _this_ if _this_ has requested class keyword
    /// Traverses parents recursively and returns first parent of the requested
    /// type.
    void firstAncestorOrThisFromClassKeyword( const QString& classKeyword, PdmObject*& ancestor ) const;

    /// Traverses all children recursively to find objects of the requested
    /// class keyword. This object is also
    /// included if it has the requested class keyword
    void descendantsIncludingThisFromClassKeyword( const QString& classKeyword, std::vector<PdmObject*>& descendants ) const;

    /// Gets all children matching class keyword. Not recursive.
    void childrenFromClassKeyword( const QString& classKeyword, std::vector<PdmObject*>& children ) const;

    /// Adds field to the internal data structure and sets the file keyword and Ui information
    /// Consider this method private. Please use the CAF_PDM_InitField() macro instead
    template <typename FieldDataType>
    void addFieldUi( PdmField<FieldDataType>* field,
                     const QString&           keyword,
                     const FieldDataType&     defaultValue,
                     PdmUiItemInfo*           fieldDescription )
    {
        addFieldUiNoDefault( field, keyword, fieldDescription );
        field->setDefaultValue( defaultValue );
        *field = defaultValue;
    }

    /// Template specialization for AppEnum to make it possible to use an enum value as the default value.
    ///
    /// In the header file:
    ///   caf::PdmField<caf::AppEnum<MyEnumType>> m_enum2Field;
    /// In the source file:
    ///   CAF_PDM_InitField( &m_enum2Field, "m_enum2Field", MyEnumType::T6, "Subset using setEnumSubset()" );
    ///
    template <typename FieldDataType>
    void addFieldUi( PdmField<AppEnum<FieldDataType>>* field,
                     const QString&                    keyword,
                     const FieldDataType&              defaultValue,
                     PdmUiItemInfo*                    fieldDescription )
    {
        auto defaultAppEnum = AppEnum<FieldDataType>( defaultValue );
        addFieldUi( field, keyword, defaultAppEnum, fieldDescription );
    }

    /// Does the same as the above method, but omits the default value.
    /// Consider this method private. Please use the CAF_PDM_InitFieldNoDefault() macro instead.
    void addFieldUiNoDefault( PdmFieldHandle* field, const QString& keyword, PdmUiItemInfo* fieldDescription );

protected:
    PdmObjectHandle* doCopyObject() const override;
};

} // End of namespace caf
