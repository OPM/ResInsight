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

#include "gtest/gtest.h"
#include <iostream>

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmDocument.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmPointer.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmReferenceHelper.h"

#include <QFile>

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
        CAF_PDM_InitObject( "InheritedDemoObj", "", "ToolTip InheritedDemoObj", "Whatsthis InheritedDemoObj" );

        CAF_PDM_InitFieldNoDefault( &m_texts, "Texts", "Some words", "", "", "" );
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

TEST( BaseTest, Delete )
{
    SimpleObj* s2 = new SimpleObj;
    delete s2;
}

//--------------------------------------------------------------------------------------------------
/// This is a testbed to try out different aspects, instead of having a main in a prototype program
/// To be disabled when everything gets more mature.
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, Start )
{
    DemoPdmObject* a = new DemoPdmObject;

    caf::PdmObjectHandle* demo = caf::PdmDefaultObjectFactory::instance()->create( "DemoPdmObject" );
    EXPECT_TRUE( demo != NULL );

    QString          xml;
    QXmlStreamWriter xmlStream( &xml );
    xmlStream.setAutoFormatting( true );

    SimpleObj*                 s2 = new SimpleObj;
    caf::PdmPointer<SimpleObj> sp;
    sp = s2;
    std::cout << sp.p() << std::endl;

    {
        SimpleObj s;
        s.m_dir        = 10000;
        sp             = &s;
        a->m_textField = "Hei og hå";
        //*s2 = s;
        a->m_simpleObjPtrField = s2;

        s.writeFields( xmlStream );
    }
    a->writeFields( xmlStream );

    caf::PdmObjectGroup og;
    og.objects.push_back( a );
    og.objects.push_back( new SimpleObj );
    og.writeFields( xmlStream );
    std::cout << sp.p() << std::endl,

        std::cout << xml.toStdString() << std::endl;
}

//--------------------------------------------------------------------------------------------------
/// Test of PdmField operations
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, NormalPdmField )
{
    class ObjectWithVectors : public caf::PdmObjectHandle
    {
    public:
        ObjectWithVectors()
        {
            this->addField( &field1, "field1" );
            this->addField( &field2, "field2" );
            this->addField( &field3, "field3" );
        }

        caf::PdmField<std::vector<double>> field1;
        caf::PdmField<std::vector<double>> field2;
        caf::PdmField<std::vector<double>> field3;
    };

    std::vector<double> testValue;
    testValue.push_back( 1.1 );
    testValue.push_back( 1.2 );
    testValue.push_back( 1.3 );

    std::vector<double> testValue2;
    testValue2.push_back( 2.1 );
    testValue2.push_back( 2.2 );
    testValue2.push_back( 2.3 );

    ObjectWithVectors myObj;

    // Constructors
    myObj.field2 = testValue;
    EXPECT_EQ( 1.3, myObj.field2.v()[2] );

    myObj.field3 = myObj.field2;
    EXPECT_EQ( 1.3, myObj.field3.v()[2] );
    EXPECT_EQ( size_t( 0 ), myObj.field1().size() );

    // Operators
    EXPECT_FALSE( myObj.field1 == myObj.field3 );
    myObj.field1 = myObj.field2;
    EXPECT_EQ( 1.3, myObj.field1()[2] );
    myObj.field1 = testValue2;
    EXPECT_EQ( 2.3, myObj.field1()[2] );
    myObj.field3 = myObj.field1;
    EXPECT_TRUE( myObj.field1 == myObj.field3 );
}

#if 0
//--------------------------------------------------------------------------------------------------
/// Test of PdmField of pointer operations
//--------------------------------------------------------------------------------------------------

TEST(BaseTest, PointerPdmField)
{
    SimpleObj* testValue = new SimpleObj;
    testValue->m_numbers.v().push_back(1.1);
    testValue->m_numbers.v().push_back(1.2);
    testValue->m_numbers.v().push_back(1.3);

    SimpleObj* testValue2 = new SimpleObj;
    testValue->m_numbers.v().push_back(2.1);
    testValue->m_numbers.v().push_back(2.2);
    testValue->m_numbers.v().push_back(2.3);

    // Constructors
    caf::PdmField<SimpleObj*> field2(testValue);
    EXPECT_EQ(testValue, field2.v());
    caf::PdmField<SimpleObj*> field3(field2);
    EXPECT_EQ(testValue, field3.v());
    caf::PdmField<SimpleObj*> field1;
    EXPECT_EQ((SimpleObj*)0, field1.v());

    // Operators
    EXPECT_FALSE(field1 == field3);
    field1 = field2;
    EXPECT_EQ(testValue, field1);
    field1 = testValue2;
    field3 = testValue2;
    EXPECT_EQ(testValue2, field1);
    EXPECT_TRUE(field1 == field3);
    delete testValue;
    delete testValue2;
    EXPECT_EQ((SimpleObj*)0, field1);
    EXPECT_EQ((SimpleObj*)0, field2);
    EXPECT_EQ((SimpleObj*)0, field3);
}
#endif

//--------------------------------------------------------------------------------------------------
/// Test of PdmPointersField operations
//--------------------------------------------------------------------------------------------------
#if 0
TEST(BaseTest, PdmChildArrayField)
{
    std::vector<caf::PdmFieldHandle*> parentFields;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    SimpleObj* s1 = new SimpleObj;
    SimpleObj* s2 = new SimpleObj;
    SimpleObj* s3 = new SimpleObj;

    // empty() number 1
    EXPECT_TRUE(ihd1->m_simpleObjectsField.empty());
    EXPECT_EQ(size_t(0), ihd1->m_simpleObjectsField.size());

    // push_back()
    ihd1->m_simpleObjectsField.push_back(s1);
    ihd1->m_simpleObjectsField.push_back(s2);
    ihd1->m_simpleObjectsField.push_back(s3);

    s1->parentField(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // size()
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());

    // operator[]
    EXPECT_EQ(s2, ihd1->m_simpleObjectsField[1]);
    EXPECT_EQ(s3, ihd1->m_simpleObjectsField[2]);

    // childObjects
    std::vector<caf::PdmObjectHandle*> objects;
    ihd1->m_simpleObjectsField.childObjects(&objects);
    EXPECT_EQ(size_t(3), objects.size());

    // Operator ==, Operator =
    InheritedDemoObj* ihd2 = new InheritedDemoObj;
    EXPECT_FALSE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 
    ihd2->m_simpleObjectsField = ihd1->m_simpleObjectsField;
    EXPECT_TRUE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 

    s1->parentFields(parentFields);
    EXPECT_EQ(size_t(2),  parentFields.size());
    parentFields.clear();

    // set(), Operator=
    ihd2->m_simpleObjectsField.set(1, NULL);
    EXPECT_FALSE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 
    EXPECT_TRUE(NULL == ihd2->m_simpleObjectsField[1]);

    s2->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // removeAll(pointer)
    ihd2->m_simpleObjectsField.removeChildObject(NULL);
    EXPECT_EQ(size_t(2), ihd2->m_simpleObjectsField.size());
    EXPECT_EQ(s3, ihd2->m_simpleObjectsField[1]);
    EXPECT_EQ(s1, ihd2->m_simpleObjectsField[0]);

    // insert()
    ihd2->m_simpleObjectsField.insert(1, s2);
    EXPECT_TRUE(ihd2->m_simpleObjectsField == ihd1->m_simpleObjectsField); 

    s2->parentFields(parentFields);
    EXPECT_EQ(size_t(2),  parentFields.size());
    parentFields.clear();

    // erase (index)
    ihd2->m_simpleObjectsField.erase(1);
    EXPECT_EQ(size_t(2),  ihd2->m_simpleObjectsField.size());
    EXPECT_EQ(s3, ihd2->m_simpleObjectsField[1]);
    EXPECT_EQ(s1, ihd2->m_simpleObjectsField[0]);

    s2->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // clear()
    ihd2->m_simpleObjectsField.clear();
    EXPECT_EQ(size_t(0),  ihd2->m_simpleObjectsField.size());

    s1->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

 
}
#endif

void PrintTo( const QString& val, std::ostream* os )
{
    *os << val.toLatin1().data();
}

//--------------------------------------------------------------------------------------------------
/// Tests the roundtrip: Create, write, read, write and checks that the first and second file are identical
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ReadWrite )
{
    QString xmlDocumentContentWithErrors;

    {
        MyPdmDocument xmlDoc;

        // Create objects
        DemoPdmObject*    d1  = new DemoPdmObject;
        DemoPdmObject*    d2  = new DemoPdmObject;
        InheritedDemoObj* id1 = new InheritedDemoObj;
        InheritedDemoObj* id2 = new InheritedDemoObj;

        SimpleObj* s1 = new SimpleObj;
        SimpleObj  s2;

        s1->m_numbers.v().push_back( 1.7 );

        // set some values
        s2.m_numbers.v().push_back( 2.4 );
        s2.m_numbers.v().push_back( 2.5 );
        s2.m_numbers.v().push_back( 2.6 );
        s2.m_numbers.v().push_back( 2.7 );

        id1->m_texts.v().push_back( "Hei" );
        id1->m_texts.v().push_back( "og" );
        id1->m_texts.v().push_back( "Hå test with whitespace" );

        d2->m_simpleObjPtrField  = &s2;
        d2->m_simpleObjPtrField2 = s1;

        id1->m_simpleObjectsField.push_back( new SimpleObj );
        id1->m_simpleObjectsField[0]->m_numbers.v().push_back( 3.0 );
        id1->m_simpleObjectsField.push_back( new SimpleObj );
        id1->m_simpleObjectsField[1]->m_numbers.v().push_back( 3.1 );
        id1->m_simpleObjectsField[1]->m_numbers.v().push_back( 3.11 );
        id1->m_simpleObjectsField[1]->m_numbers.v().push_back( 3.12 );
        id1->m_simpleObjectsField[1]->m_numbers.v().push_back( 3.13 );
        id1->m_simpleObjectsField.push_back( new SimpleObj );
        id1->m_simpleObjectsField[2]->m_numbers.v().push_back( 3.2 );
        id1->m_simpleObjectsField.push_back( new SimpleObj );
        id1->m_simpleObjectsField[3]->m_numbers.v().push_back( 3.3 );

        // Add to document

        xmlDoc.objects.push_back( d1 );
        xmlDoc.objects.push_back( d2 );
        xmlDoc.objects.push_back( new SimpleObj );
        xmlDoc.objects.push_back( id1 );
        xmlDoc.objects.push_back( id2 );

        // Write file
        xmlDoc.fileName = "PdmTestFil.xml";
        xmlDoc.writeFile();

        caf::PdmObjectGroup pog;
        for ( size_t i = 0; i < xmlDoc.objects.size(); i++ )
        {
            pog.addObject( xmlDoc.objects[i] );
        }

        {
            std::vector<caf::PdmPointer<DemoPdmObject>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 4 ), demoObjs.size() );
        }
        {
            std::vector<caf::PdmPointer<InheritedDemoObj>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 2 ), demoObjs.size() );
        }
        {
            std::vector<caf::PdmPointer<SimpleObj>> demoObjs;
            pog.objectsByType( &demoObjs );
            EXPECT_EQ( size_t( 1 ), demoObjs.size() );
        }

        d2->m_simpleObjPtrField = NULL;
        xmlDoc.objects.deleteAllChildObjects();
    }

    {
        MyPdmDocument xmlDoc;

        // Read file
        xmlDoc.fileName = "PdmTestFil.xml";
        xmlDoc.readFile();

        caf::PdmObjectGroup pog;
        for ( size_t i = 0; i < xmlDoc.objects.size(); i++ )
        {
            pog.addObject( xmlDoc.objects[i] );
        }

        // Test sample of that writing actually took place

        std::vector<caf::PdmPointer<InheritedDemoObj>> ihDObjs;
        pog.objectsByType( &ihDObjs );
        EXPECT_EQ( size_t( 2 ), ihDObjs.size() );
        ASSERT_EQ( size_t( 4 ), ihDObjs[0]->m_simpleObjectsField.size() );
        ASSERT_EQ( size_t( 4 ), ihDObjs[0]->m_simpleObjectsField[1]->m_numbers().size() );
        EXPECT_EQ( 3.13, ihDObjs[0]->m_simpleObjectsField[1]->m_numbers()[3] );

        EXPECT_EQ( QString( "ÆØÅ Test text   end" ), ihDObjs[0]->m_textField() );

        // Write file
        QFile xmlFile( "PdmTestFil2.xml" );
        xmlFile.open( QIODevice::WriteOnly | QIODevice::Text );
        xmlDoc.writeFile( &xmlFile );
        xmlFile.close();
    }

    // Check that the files are identical
    {
        QFile f1( "PdmTestFil.xml" );
        QFile f2( "PdmTestFil2.xml" );
        f1.open( QIODevice::ReadOnly | QIODevice::Text );
        f2.open( QIODevice::ReadOnly | QIODevice::Text );
        QByteArray ba1   = f1.readAll();
        QByteArray ba2   = f2.readAll();
        bool       equal = ba1 == ba2;
        EXPECT_TRUE( equal );

        // Then test how errors are handled
        {
            int pos            = 0;
            int occurenceCount = 0;
            while ( occurenceCount < 1 )
            {
                pos = ba1.indexOf( "<SimpleObj>", pos + 1 );
                occurenceCount++;
            }
            ba1.insert( pos + 1, "Error" );
        }

        {
            int pos            = 0;
            int occurenceCount = 0;
            while ( occurenceCount < 1 )
            {
                pos = ba1.indexOf( "</SimpleObj>", pos + 1 );
                occurenceCount++;
            }
            ba1.insert( pos + 2, "Error" );
        }

        {
            int pos            = 0;
            int occurenceCount = 0;
            while ( occurenceCount < 6 ) // Second position in a pointersfield
            {
                pos = ba1.indexOf( "<SimpleObj>", pos + 1 );
                occurenceCount++;
            }
            ba1.insert( pos + 1, "Error" );
        }

        {
            int pos            = 0;
            int occurenceCount = 0;
            while ( occurenceCount < 6 ) // Second position in a pointersfield
            {
                pos = ba1.indexOf( "</SimpleObj>", pos + 1 );
                occurenceCount++;
            }
            ba1.insert( pos + 2, "Error" );
        }
        {
            int pos = ba1.indexOf( "<BigNumber>" );
            ba1.insert( pos + 1, "Error" );
            pos = ba1.indexOf( "</BigNumber>" );
            ba1.insert( pos + 2, "Error" );
        }
        {
            int pos            = 0;
            int occurenceCount = 0;
            while ( occurenceCount < 4 )
            {
                pos = ba1.indexOf( "<Numbers>", pos + 1 );
                occurenceCount++;
            }
            ba1.insert( pos + 1, "Error" );
        }

        {
            int pos            = 0;
            int occurenceCount = 0;
            while ( occurenceCount < 4 )
            {
                pos = ba1.indexOf( "</Numbers>", pos + 1 );
                occurenceCount++;
            }
            ba1.insert( pos + 2, "Error" );
        }

        // Write the edited document

        QFile f3( "PdmTestFilWithError.xml" );
        f3.open( QIODevice::WriteOnly | QIODevice::Text );
        f3.write( ba1 );
        f3.close();

        // Read the document containing errors

        MyPdmDocument xmlErrorDoc;
        xmlErrorDoc.fileName = "PdmTestFilWithError.xml";
        xmlErrorDoc.readFile();

        caf::PdmObjectGroup pog;
        for ( size_t i = 0; i < xmlErrorDoc.objects.size(); i++ )
        {
            pog.addObject( xmlErrorDoc.objects[i] );
        }

        // Check the pointersfield
        std::vector<caf::PdmPointer<InheritedDemoObj>> ihDObjs;
        pog.objectsByType( &ihDObjs );

        EXPECT_EQ( size_t( 2 ), ihDObjs.size() );
        ASSERT_EQ( size_t( 3 ), ihDObjs[0]->m_simpleObjectsField.size() );

        // check single pointer field
        std::vector<caf::PdmPointer<DemoPdmObject>> demoObjs;
        pog.objectsByType( &demoObjs );

        EXPECT_EQ( size_t( 4 ), demoObjs.size() );
        EXPECT_TRUE( demoObjs[0]->m_simpleObjPtrField == NULL );
        EXPECT_TRUE( demoObjs[0]->m_simpleObjPtrField2 != NULL );

        // check single pointer field
        std::vector<caf::PdmPointer<SimpleObj>> simpleObjs;
        pog.objectsByType( &simpleObjs );
        EXPECT_EQ( size_t( 1 ), simpleObjs.size() );
        EXPECT_EQ( size_t( 0 ), simpleObjs[0]->m_numbers().size() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of PdmPointer
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmPointer )
{
    caf::PdmDocument* d = new caf::PdmDocument;

    {
        caf::PdmPointer<caf::PdmDocument> p;
        EXPECT_TRUE( p == NULL );
    }

    {
        caf::PdmPointer<caf::PdmDocument> p( d );
        caf::PdmPointer<caf::PdmDocument> p2( p );

        EXPECT_TRUE( p == d && p2 == d );
        EXPECT_TRUE( p.p() == d );
        p = 0;
        EXPECT_TRUE( p == NULL );
        EXPECT_TRUE( p.isNull() );
        EXPECT_TRUE( p2 == d );
        p = p2;
        EXPECT_TRUE( p == d );
        delete d;
        EXPECT_TRUE( p.isNull() && p2.isNull() );
    }

    caf::PdmPointer<DemoPdmObject> p3( new DemoPdmObject() );

    delete p3;
}

//--------------------------------------------------------------------------------------------------
/// Tests the PdmFactory
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmObjectFactory )
{
    {
        SimpleObj* s = dynamic_cast<SimpleObj*>( caf::PdmDefaultObjectFactory::instance()->create( "SimpleObj" ) );
        EXPECT_TRUE( s != NULL );
    }
    {
        DemoPdmObject* s =
            dynamic_cast<DemoPdmObject*>( caf::PdmDefaultObjectFactory::instance()->create( "DemoPdmObject" ) );
        EXPECT_TRUE( s != NULL );
        delete s;
    }
    {
        InheritedDemoObj* s =
            dynamic_cast<InheritedDemoObj*>( caf::PdmDefaultObjectFactory::instance()->create( "InheritedDemoObj" ) );
        EXPECT_TRUE( s != NULL );
    }

    {
        caf::PdmDocument* s =
            dynamic_cast<caf::PdmDocument*>( caf::PdmDefaultObjectFactory::instance()->create( "PdmDocument" ) );
        EXPECT_TRUE( s != NULL );
    }

    {
        caf::PdmObjectGroup* s =
            dynamic_cast<caf::PdmObjectGroup*>( caf::PdmDefaultObjectFactory::instance()->create( "PdmObjectGroup" ) );
        EXPECT_TRUE( s != NULL );
    }
}

//--------------------------------------------------------------------------------------------------
/// Validate Xml keywords
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, ValidXmlKeywords )
{
    EXPECT_TRUE( caf::PdmXmlObjectHandle::isValidXmlElementName( "Valid_name" ) );

    EXPECT_FALSE( caf::PdmXmlObjectHandle::isValidXmlElementName( "2Valid_name" ) );
    EXPECT_FALSE( caf::PdmXmlObjectHandle::isValidXmlElementName( ".Valid_name" ) );
    EXPECT_FALSE( caf::PdmXmlObjectHandle::isValidXmlElementName( "xml_Valid_name" ) );
    EXPECT_FALSE( caf::PdmXmlObjectHandle::isValidXmlElementName( "Valid_name_with_space " ) );
}

TEST( BaseTest, PdmPointersFieldInsertVector )
{
    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    SimpleObj* s1 = new SimpleObj;
    SimpleObj* s2 = new SimpleObj;
    SimpleObj* s3 = new SimpleObj;

    caf::PdmObjectGroup pdmGroup;
    pdmGroup.addObject( s1 );
    pdmGroup.addObject( s2 );
    pdmGroup.addObject( s3 );

    std::vector<caf::PdmPointer<SimpleObj>> typedObjects;
    pdmGroup.objectsByType( &typedObjects );
    EXPECT_EQ( size_t( 3 ), typedObjects.size() );

    std::vector<caf::PdmPointer<SimpleObj>> objs;
    objs.push_back( new SimpleObj );
    objs.push_back( new SimpleObj );
    objs.push_back( new SimpleObj );

    ihd1->m_simpleObjectsField.insert( ihd1->m_simpleObjectsField.size(), objs );
    EXPECT_EQ( size_t( 3 ), ihd1->m_simpleObjectsField.size() );

    delete ihd1;
}

TEST( BaseTest, PdmObjectGroupCopyOfTypedObjects )
{
    SimpleObj* s1  = new SimpleObj;
    s1->m_position = 1000;
    s1->m_numbers.v().push_back( 10 );

    SimpleObj* s2  = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3  = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    caf::PdmObjectGroup og;
    og.objects.push_back( s1 );
    og.objects.push_back( s2 );
    og.objects.push_back( s3 );
    og.objects.push_back( ihd1 );

    std::vector<caf::PdmPointer<SimpleObj>> simpleObjList;
    og.createCopyByType( &simpleObjList, caf::PdmDefaultObjectFactory::instance() );
    EXPECT_EQ( size_t( 3 ), simpleObjList.size() );
    EXPECT_EQ( 1000, simpleObjList[0]->m_position );
    EXPECT_EQ( size_t( 1 ), simpleObjList[0]->m_numbers.v().size() );
    EXPECT_EQ( 10, simpleObjList[0]->m_numbers.v()[0] );

    EXPECT_EQ( 2000, simpleObjList[1]->m_position );
    EXPECT_EQ( 3000, simpleObjList[2]->m_position );

    std::vector<caf::PdmPointer<InheritedDemoObj>> inheritObjList;
    og.createCopyByType( &inheritObjList, caf::PdmDefaultObjectFactory::instance() );
    EXPECT_EQ( size_t( 1 ), inheritObjList.size() );

    og.deleteObjects();
    EXPECT_EQ( size_t( 3 ), simpleObjList.size() );
    EXPECT_EQ( size_t( 1 ), inheritObjList.size() );
}

//--------------------------------------------------------------------------------------------------
/// PdmChildArrayFieldHandle
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmChildArrayFieldHandle )
{
    //     virtual size_t      size() const = 0;
    //     virtual bool        empty() const = 0;
    //     virtual void        clear() = 0;
    //     virtual PdmObject*  createAppendObject(int indexAfter) = 0;
    //     virtual void        erase(size_t index) = 0;
    //     virtual void        deleteAllChildObjects() = 0;
    //
    //     virtual PdmObject*  at(size_t index) = 0;
    //
    //     bool                hasSameFieldCountForAllObjects();

    SimpleObj* s1  = new SimpleObj;
    s1->m_position = 1000;
    s1->m_numbers.v().push_back( 10 );

    SimpleObj* s2  = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3  = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj*              ihd1      = new InheritedDemoObj;
    caf::PdmChildArrayFieldHandle* listField = &( ihd1->m_simpleObjectsField );

    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( new SimpleObj );
    EXPECT_EQ( 1u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    ihd1->m_simpleObjectsField.push_back( s1 );
    ihd1->m_simpleObjectsField.push_back( s2 );
    ihd1->m_simpleObjectsField.push_back( s3 );

    EXPECT_EQ( 4u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->erase( 0 );
    EXPECT_EQ( 3u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_FALSE( listField->empty() );

    listField->deleteAllChildObjects();
    EXPECT_EQ( 0u, listField->size() );
    EXPECT_TRUE( listField->hasSameFieldCountForAllObjects() );
    EXPECT_TRUE( listField->empty() );
}

class ReferenceDemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    ReferenceDemoPdmObject()
    {
        CAF_PDM_InitObject( "ReferenceDemoPdmObject", "", "Tooltip DemoPdmObject", "WhatsThis DemoPdmObject" );

        CAF_PDM_InitFieldNoDefault( &m_pointersField, "SimpleObjPtrField", "SimpleObjPtrField", "", "Tooltip", "WhatsThis" );
        CAF_PDM_InitFieldNoDefault( &m_simpleObjPtrField2, "SimpleObjPtrField2", "SimpleObjPtrField2", "", "Tooltip", "WhatsThis" );
    }

    // Fields
    caf::PdmChildField<PdmObjectHandle*> m_pointersField;
    caf::PdmChildArrayField<SimpleObj*>  m_simpleObjPtrField2;
};

CAF_PDM_SOURCE_INIT( ReferenceDemoPdmObject, "ReferenceDemoPdmObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( BaseTest, PdmReferenceHelper )
{
    SimpleObj* s1  = new SimpleObj;
    s1->m_position = 1000;
    s1->m_numbers.v().push_back( 10 );

    SimpleObj* s2  = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3  = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    ihd1->m_simpleObjectsField.push_back( new SimpleObj );

    ihd1->m_simpleObjectsField.push_back( s1 );
    ihd1->m_simpleObjectsField.push_back( s2 );
    ihd1->m_simpleObjectsField.push_back( s3 );

    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToObject( NULL, s3 );
        EXPECT_TRUE( refString.isEmpty() );

        refString              = caf::PdmReferenceHelper::referenceFromRootToObject( ihd1, s3 );
        QString expectedString = ihd1->m_simpleObjectsField.keyword() + " 3";
        EXPECT_STREQ( refString.toLatin1(), expectedString.toLatin1() );

        caf::PdmObjectHandle* fromRef = caf::PdmReferenceHelper::objectFromReference( ihd1, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    ReferenceDemoPdmObject* objA = new ReferenceDemoPdmObject;
    objA->m_pointersField        = ihd1;

    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToObject( objA, s3 );

        caf::PdmObjectHandle* fromRef = caf::PdmReferenceHelper::objectFromReference( objA, refString );
        EXPECT_TRUE( fromRef == s3 );
    }

    // Test reference to field
    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToField( objA, &( ihd1->m_simpleObjectsField ) );

        caf::PdmFieldHandle* fromRef = caf::PdmReferenceHelper::fieldFromReference( objA, refString );
        EXPECT_TRUE( fromRef == &( ihd1->m_simpleObjectsField ) );
    }
}
