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

#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmCodeGenerator.h"
#include "cafPdmDocument.h"
#include "cafPdmField.h"
#include "cafPdmFieldIOScriptability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmPointer.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmReferenceHelper.h"

#include <QFile>

#include "cafPdmObjectScriptability.h"
#include <memory>

/// Demo objects to show the usage of the Pdm system

class SimpleObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    SimpleObj()
        : PdmObject()
        , m_doubleMember( 0.0 )
    {
        CAF_PDM_InitObject( "SimpleObj", "", "Tooltip SimpleObj", "WhatsThis SimpleObj" );

        CAF_PDM_InitField( &m_position, "Position", 8765.2, "Position", "", "Tooltip", "WhatsThis" );
        CAF_PDM_InitField( &m_dir, "Dir", 123.56, "Direction", "", "Tooltip", "WhatsThis" );
        CAF_PDM_InitField( &m_up, "Up", 0.0, "Up value", "", "Tooltip", "WhatsThis" );
        CAF_PDM_InitFieldNoDefault( &m_numbers, "Numbers", "Important Numbers", "", "Tooltip", "WhatsThis" );
#if 1
        m_proxyDouble.registerSetMethod( this, &SimpleObj::setDoubleMember );
        m_proxyDouble.registerGetMethod( this, &SimpleObj::doubleMember );
        AddUiCapabilityToField( &m_proxyDouble );
        AddXmlCapabilityToField( &m_proxyDouble );
        CAF_PDM_InitFieldNoDefault( &m_proxyDouble, "ProxyDouble", "ProxyDouble", "", "", "" );
#endif
    }

    /// Assignment and copying of PDM objects is not focus for the features. This is only a
    /// "would it work" test
    SimpleObj( const SimpleObj& other )
        : PdmObject()
    {
        CAF_PDM_InitField( &m_position, "Position", 8765.2, "Position", "", "", "WhatsThis" );
        CAF_PDM_InitField( &m_dir, "Dir", 123.56, "Direction", "", "", "WhatsThis" );
        CAF_PDM_InitField( &m_up, "Up", 0.0, "Up value", "", "", "WhatsThis" );

        CAF_PDM_InitFieldNoDefault( &m_numbers, "Numbers", "Important Numbers", "", "", "WhatsThis" );

        m_position     = other.m_position;
        m_dir          = other.m_dir;
        m_up           = other.m_up;
        m_numbers      = other.m_numbers;
        m_doubleMember = other.m_doubleMember;
    }

    ~SimpleObj() {}

    caf::PdmField<double>              m_position;
    caf::PdmField<double>              m_dir;
    caf::PdmField<double>              m_up;
    caf::PdmField<std::vector<double>> m_numbers;
    caf::PdmProxyValueField<double>    m_proxyDouble;

    void setDoubleMember( const double& d )
    {
        m_doubleMember = d;
        std::cout << "setDoubleMember" << std::endl;
    }
    double doubleMember() const
    {
        std::cout << "doubleMember" << std::endl;
        return m_doubleMember;
    }

    double m_doubleMember;
};
CAF_PDM_SOURCE_INIT( SimpleObj, "SimpleObj" );

class DemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObject()
    {
        CAF_PDM_InitObject( "DemoPdmObject", "", "Tooltip DemoPdmObject", "WhatsThis DemoPdmObject" );

        CAF_PDM_InitField( &m_doubleField,
                           "BigNumber",
                           0.0,
                           "",
                           "",
                           "Enter a big number here",
                           "This is a place you can enter a big real value if you want" );

        CAF_PDM_InitField( &m_intField,
                           "IntNumber",
                           0,
                           "",
                           "",
                           "Enter some small number here",
                           "This is a place you can enter a small integer value if you want" );

        CAF_PDM_InitField( &m_textField, "TextField", QString( "ÆØÅ Test text   end" ), "TextField", "", "Tooltip", "WhatsThis" );
        CAF_PDM_InitFieldNoDefault( &m_simpleObjPtrField, "SimpleObjPtrField", "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
        CAF_PDM_InitFieldNoDefault( &m_simpleObjPtrField2, "SimpleObjPtrField2", "SimpleObjPtrField2", "", "Tooltip", "WhatsThis" );
        m_simpleObjPtrField2 = new SimpleObj;
    }

    ~DemoPdmObject()
    {
        delete m_simpleObjPtrField();
        delete m_simpleObjPtrField2();
    }

    // Fields
    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;

    caf::PdmChildField<SimpleObj*> m_simpleObjPtrField;
    caf::PdmChildField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_PDM_SOURCE_INIT( DemoPdmObject, "DemoPdmObject" );

class InheritedDemoObj : public DemoPdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    InheritedDemoObj()
    {
        CAF_PDM_InitScriptableObjectWithNameAndComment( "InheritedDemoObj",
                                                        "",
                                                        "ToolTip InheritedDemoObj",
                                                        "Whatsthis InheritedDemoObj",
                                                        "ScriptClassName_InheritedDemoObj",
                                                        "Script comment test" );

        CAF_PDM_InitScriptableFieldWithIONoDefault( &m_texts, "Texts", "Some words", "", "", "" );
        CAF_PDM_InitFieldNoDefault( &m_testEnumField, "TestEnumValue", "An Enum", "", "", "" );
        CAF_PDM_InitFieldNoDefault( &m_simpleObjectsField,
                                    "SimpleObjects",
                                    "SimpleObjectsField",
                                    "",
                                    "ToolTip SimpleObjectsField",
                                    "Whatsthis SimpleObjectsField" );
    }

    ~InheritedDemoObj() { m_simpleObjectsField.deleteAllChildObjects(); }

    caf::PdmField<std::vector<QString>>       m_texts;
    caf::PdmField<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::PdmChildArrayField<SimpleObj*>       m_simpleObjectsField;
};
CAF_PDM_SOURCE_INIT( InheritedDemoObj, "InheritedDemoObj" );

class MyPdmDocument : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    MyPdmDocument()
    {
        CAF_PDM_InitObject( "PdmObjectCollection", "", "", "" );
        CAF_PDM_InitFieldNoDefault( &objects, "PdmObjects", "", "", "", "" )
    }

    ~MyPdmDocument() { objects.deleteAllChildObjects(); }

    caf::PdmChildArrayField<PdmObjectHandle*> objects;
};
CAF_PDM_SOURCE_INIT( MyPdmDocument, "MyPdmDocument" );

namespace caf
{
template <>
void AppEnum<InheritedDemoObj::TestEnumType>::setUp()
{
    addItem( InheritedDemoObj::T1, "T1", "An A letter" );
    addItem( InheritedDemoObj::T2, "T2", "A B letter" );
    addItem( InheritedDemoObj::T3, "T3", "A B letter" );
    setDefault( InheritedDemoObj::T1 );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmScriptingTest, BasicUse )
{
    std::unique_ptr<MyPdmDocument> document( new MyPdmDocument );

    std::string fileExt = "py";

    std::unique_ptr<caf::PdmCodeGenerator> generator( caf::PdmCodeGeneratorFactory::instance()->create( fileExt ) );
    auto generatedText = generator->generate( caf::PdmDefaultObjectFactory::instance() );
}
