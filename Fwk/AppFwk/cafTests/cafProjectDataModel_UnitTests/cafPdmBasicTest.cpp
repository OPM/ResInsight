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

#include <iostream>
#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmDocument.h"

#include "cafAppEnum.h"
#include <memory>
#include <QFile>


/// Demo objects to show the usage of the Pdm system


class SimpleObj: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    SimpleObj()
    {
        CAF_PDM_InitObject("Simple Object", "", "", "");

        CAF_PDM_InitField(&m_position,  "Position", 8765.2, "Position",     "", "", "");
        CAF_PDM_InitField(&m_dir,       "Dir",      123.56, "Direction",    "", "", "");
        CAF_PDM_InitField(&m_up,        "Up",       0.0,    "Up value",     "", "", "" );
        CAF_PDM_InitFieldNoDefault(&m_numbers, "Numbers", "Important Numbers", "", "", "");
    }

    /// Assignment and copying of PDM objects is not focus for the features. This is only a
    /// "would it work" test
    SimpleObj(const SimpleObj& other)
        : PdmObject()
    {
        CAF_PDM_InitField(&m_position,  "Position", 8765.2, "Position",     "", "", "");
        CAF_PDM_InitField(&m_dir,       "Dir",      123.56, "Direction",    "", "", "");
        CAF_PDM_InitField(&m_up,        "Up",       0.0,    "Up value",     "", "", "" );
        CAF_PDM_InitFieldNoDefault(&m_numbers, "Numbers", "Important Numbers", "", "", "");

        m_position = other.m_position;
        m_dir = other.m_dir;
        m_up = other.m_up;
        m_numbers = other.m_numbers;
    }

    ~SimpleObj() {}


    caf::PdmField<double>               m_position;
    caf::PdmField<double>               m_dir;
    caf::PdmField<double>               m_up;
    caf::PdmField<std::vector<double> > m_numbers;
};
CAF_PDM_SOURCE_INIT(SimpleObj, "SimpleObj");



class DemoPdmObject: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    DemoPdmObject() 
    {   
        CAF_PDM_InitObject("Demo Object", "", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", 
            "Enter a big number here", 
            "This is a place you can enter a big real value if you want" );

        CAF_PDM_InitField(&m_intField, "IntNumber", 0,  "Small Number","",
            "Enter some small number here",
            "This is a place you can enter a small integer value if you want");

        CAF_PDM_InitField(&m_textField, "TextField", QString("∆ÿ≈ Test text   end"),  "", "", "", "");
        CAF_PDM_InitFieldNoDefault(&m_simpleObjPtrField, "SimpleObjPtrField", "", "", "", "");
        CAF_PDM_InitFieldNoDefault(&m_simpleObjPtrField2, "SimpleObjPtrField2", "", "", "", "");
        m_simpleObjPtrField2 = new SimpleObj;
    }

    ~DemoPdmObject() 
    {
        delete m_simpleObjPtrField2();
    }

    // Fields
    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;

    caf::PdmField<SimpleObj*> m_simpleObjPtrField;
    caf::PdmField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_PDM_SOURCE_INIT(DemoPdmObject, "DemoPdmObject");


class InheritedDemoObj : public DemoPdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    enum TestEnumType
    {
        T1, T2, T3
    };
    
    InheritedDemoObj()
    {
        CAF_PDM_InitFieldNoDefault(&m_texts, "Texts", "Some words", "", "", "");
        CAF_PDM_InitFieldNoDefault(&m_testEnumField, "TestEnumValue",  "An Enum", "", "", "");
        CAF_PDM_InitFieldNoDefault(&m_simpleObjectsField, "SimpleObjects",  "A child object", "", "", "");

    }

    caf::PdmField<std::vector<QString> >        m_texts;
    caf::PdmField< caf::AppEnum<TestEnumType> > m_testEnumField;
    caf::PdmPointersField<SimpleObj*>           m_simpleObjectsField;

};
CAF_PDM_SOURCE_INIT(InheritedDemoObj, "InheritedDemoObj");


namespace caf
{

template<>
void AppEnum<InheritedDemoObj::TestEnumType>::setUp()
{
    addItem(InheritedDemoObj::T1,           "T1",         "An A letter");
    addItem(InheritedDemoObj::T2,           "T2",         "A B letter");
    addItem(InheritedDemoObj::T3,           "T3",         "A B letter");
    setDefault(InheritedDemoObj::T1);
}

}

//--------------------------------------------------------------------------------------------------
/// This is a testbed to try out different aspects, instead of having a main in a prototype program
/// To be disabled when everything gets more mature.
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, Start)
{
  DemoPdmObject* a = new DemoPdmObject;

  caf::PdmObject* demo = caf::PdmObjectFactory::instance()->create("DemoPdmObject");
  EXPECT_TRUE(demo != NULL);

  QString xml;
  QXmlStreamWriter xmlStream(&xml);
  xmlStream.setAutoFormatting(true);

  SimpleObj* s2 = new SimpleObj;
  caf::PdmPointer<SimpleObj> sp;
  sp = s2;
  std::cout << sp.p() << std::endl;

  {
      SimpleObj s;
      s.m_dir = 10000;
      sp = &s;
      a->m_textField = "Hei og hÂ";
      *s2 = s;
      a->m_simpleObjPtrField = s2;

      s.writeFields(xmlStream);
  }
  a->writeFields(xmlStream);
  caf::PdmObjectGroup og;
  og.objects.push_back(a);
  og.objects.push_back(s2);
  og.writeFields(xmlStream);
  std::cout << sp.p() << std::endl,

  std::cout << xml.toStdString() << std::endl;


}

//--------------------------------------------------------------------------------------------------
/// Test of PdmField operations
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, NormalPdmField)
{
    std::vector<double> testValue;
    testValue.push_back(1.1);
    testValue.push_back(1.2);
    testValue.push_back(1.3);

    std::vector<double> testValue2;
    testValue2.push_back(2.1);
    testValue2.push_back(2.2);
    testValue2.push_back(2.3);

    // Constructors
    caf::PdmField<std::vector<double> > field2(testValue);
    EXPECT_EQ(1.3, field2.v()[2]);
    caf::PdmField<std::vector<double> > field3(field2);
    EXPECT_EQ(1.3, field3.v()[2]);
    caf::PdmField<std::vector<double> > field1;
    EXPECT_EQ(size_t(0), field1().size());

    // Operators
    EXPECT_FALSE(field1 == field3);
    field1 = field2;
    EXPECT_EQ(1.3, field1()[2]);
    field1 = testValue2;
    EXPECT_EQ(2.3, field1()[2]);
    field3 = field1;
    EXPECT_TRUE(field1 == field3);
}

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

//--------------------------------------------------------------------------------------------------
/// Test of PdmPointersField operations
//--------------------------------------------------------------------------------------------------

TEST(BaseTest, PdmPointersField)
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

    s1->parentFields(parentFields);
    EXPECT_EQ(size_t(1),  parentFields.size());
    parentFields.clear();

    // size()
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());

    // operator[]
    EXPECT_EQ(s2, ihd1->m_simpleObjectsField[1]);
    EXPECT_EQ(s3, ihd1->m_simpleObjectsField[2]);

    // childObjects
    std::vector<caf::PdmObject*> objects;
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
template <>
inline void GTestStreamToHelper<QString>(std::ostream* os, const QString& val) {
    *os << val.toLatin1().data();
}

//--------------------------------------------------------------------------------------------------
/// Tests the roundtrip: Create, write, read, write and checks that the first and second file are identical
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, ReadWrite)
{
    QString xmlDocumentContentWithErrors;

    {
        caf::PdmDocument xmlDoc;

        // Create objects
        DemoPdmObject* d1 = new DemoPdmObject;
        DemoPdmObject* d2 = new DemoPdmObject;
        InheritedDemoObj* id1 = new InheritedDemoObj;
        InheritedDemoObj* id2 = new InheritedDemoObj;

        SimpleObj* s1 = new SimpleObj;
        SimpleObj s2;

        s1->m_numbers.v().push_back(1.7);

        // set some values
        s2.m_numbers.v().push_back(2.4); 
        s2.m_numbers.v().push_back(2.5); 
        s2.m_numbers.v().push_back(2.6); 
        s2.m_numbers.v().push_back(2.7); 

        id1->m_texts.v().push_back("Hei");
        id1->m_texts.v().push_back("og");
        id1->m_texts.v().push_back("HÂ test with whitespace");

        d2->m_simpleObjPtrField = &s2;
        d2->m_simpleObjPtrField2 = s1;

        id1->m_simpleObjectsField.push_back(s1);
        id1->m_simpleObjectsField.push_back(&s2);
        id1->m_simpleObjectsField.push_back(&s2);
        id1->m_simpleObjectsField.push_back(&s2);

        // Add to document

        xmlDoc.addObject(d1);
        xmlDoc.addObject(d2);
        xmlDoc.addObject(s1);
        xmlDoc.addObject(id1);
        xmlDoc.addObject(id2);
        
        // Write file
        xmlDoc.fileName = "PdmTestFil.xml";
        xmlDoc.writeFile();



        {
            std::vector<caf::PdmPointer<DemoPdmObject> > demoObjs;
            xmlDoc.objectsByType(&demoObjs);
            EXPECT_EQ(size_t(4), demoObjs.size());
        }
        {
            std::vector<caf::PdmPointer<InheritedDemoObj> > demoObjs;
            xmlDoc.objectsByType(&demoObjs);
            EXPECT_EQ(size_t(2), demoObjs.size());
        }
        {
            std::vector<caf::PdmPointer<SimpleObj> > demoObjs;
            xmlDoc.objectsByType(&demoObjs);
            EXPECT_EQ(size_t(1), demoObjs.size());
        }

        xmlDoc.deleteObjects();
        EXPECT_EQ(size_t(0), xmlDoc.objects().size());
    }

    {
        caf::PdmDocument xmlDoc;

        // Read file
        xmlDoc.fileName = "PdmTestFil.xml";
        xmlDoc.readFile();

        // Test sample of that writing actually took place 

        std::vector<caf::PdmPointer<InheritedDemoObj> > ihDObjs;
        xmlDoc.objectsByType(&ihDObjs);
        EXPECT_EQ(size_t(2),ihDObjs.size() );
        ASSERT_EQ(size_t(4), ihDObjs[0]->m_simpleObjectsField.size());
        ASSERT_EQ(size_t(4), ihDObjs[0]->m_simpleObjectsField[1]->m_numbers().size());
        EXPECT_EQ(2.7, ihDObjs[0]->m_simpleObjectsField[1]->m_numbers()[3]);

        EXPECT_EQ(QString("∆ÿ≈ Test text   end"), ihDObjs[0]->m_textField());

        // Write file
        QFile xmlFile("PdmTestFil2.xml");
        xmlFile.open(QIODevice::WriteOnly);
        xmlDoc.writeFile(&xmlFile);
        xmlFile.close();
    }

        // Check that the files are identical
    {
        
        QFile f1("PdmTestFil.xml");
        QFile f2("PdmTestFil2.xml");
        f1.open(QIODevice::ReadOnly);
        f2.open(QIODevice::ReadOnly);
        QByteArray ba1 = f1.readAll();
        QByteArray ba2 = f2.readAll();
        bool equal = ba1 == ba2;
        EXPECT_TRUE(equal);

        // Then test how errors are handled
        {
            int pos = 0;
            int occurenceCount = 0;
            while (occurenceCount < 1)
            {
                pos = ba1.indexOf("<SimpleObj>", pos+1);
                occurenceCount++;
            }
            ba1.insert(pos+1, "Error");
        }

        {
            int pos = 0;
            int occurenceCount = 0;
            while (occurenceCount < 1)
            {
                pos = ba1.indexOf("</SimpleObj>", pos +1);
                occurenceCount++;
            }
            ba1.insert(pos+2, "Error");
        }

        {
            int pos = 0;
            int occurenceCount = 0;
            while (occurenceCount < 6) // Second position in a pointersfield
            {
                pos = ba1.indexOf("<SimpleObj>", pos +1);
                occurenceCount++;
            }
            ba1.insert(pos+1, "Error");
        }

        {
            int pos = 0;
            int occurenceCount = 0;
            while (occurenceCount < 6) // Second position in a pointersfield
            {
                pos = ba1.indexOf("</SimpleObj>", pos +1);
                occurenceCount++;
            }
            ba1.insert(pos+2, "Error");
        }
        {
            int pos = ba1.indexOf("<BigNumber>");
            ba1.insert(pos+1, "Error");
            pos = ba1.indexOf("</BigNumber>");
            ba1.insert(pos+2, "Error");
        }
        {
            int pos = 0;
            int occurenceCount = 0;
            while (occurenceCount < 4)
            {
                pos = ba1.indexOf("<Numbers>", pos +1);
                occurenceCount++;
            }
            ba1.insert(pos+1, "Error");
        }

        {
            int pos = 0;
            int occurenceCount = 0;
            while (occurenceCount < 4)
            {
                pos = ba1.indexOf("</Numbers>", pos +1);
                occurenceCount++;
            }
            ba1.insert(pos+2, "Error");
        }

        // Write the edited document

        QFile f3("PdmTestFilWithError.xml");
        f3.open(QIODevice::WriteOnly);
        f3.write(ba1);
        f3.close();

        // Read the document containing errors
        caf::PdmDocument xmlErrorDoc;
        xmlErrorDoc.fileName = "PdmTestFilWithError.xml";
        xmlErrorDoc.readFile();

        // Check the pointersfield
        std::vector<caf::PdmPointer<InheritedDemoObj> > ihDObjs;
        xmlErrorDoc.objectsByType(&ihDObjs);

        EXPECT_EQ(size_t(2), ihDObjs.size() );
        ASSERT_EQ(size_t(3), ihDObjs[0]->m_simpleObjectsField.size());

        // check single pointer field
        std::vector<caf::PdmPointer<DemoPdmObject> > demoObjs;
        xmlErrorDoc.objectsByType(&demoObjs);

        EXPECT_EQ(size_t(4), demoObjs.size() );
        EXPECT_TRUE(demoObjs[0]->m_simpleObjPtrField == NULL );
        EXPECT_TRUE(demoObjs[0]->m_simpleObjPtrField2 != NULL );

        // check single pointer field
        std::vector<caf::PdmPointer<SimpleObj> > simpleObjs;
        xmlErrorDoc.objectsByType(&simpleObjs);
        EXPECT_EQ(size_t(1), simpleObjs.size() );
        EXPECT_EQ(size_t(0), simpleObjs[0]->m_numbers().size());

    }   
}

//--------------------------------------------------------------------------------------------------
/// Tests the features of PdmPointer
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, PdmPointer)
{
    caf::PdmDocument * d = new caf::PdmDocument;

    {
        caf::PdmPointer<caf::PdmDocument> p;
        EXPECT_TRUE(p == NULL);
    }

    {
        caf::PdmPointer<caf::PdmDocument> p(d);
        caf::PdmPointer<caf::PdmDocument> p2(p);

        EXPECT_TRUE(p == d && p2 == d);
        EXPECT_TRUE(p.p() == d);
        EXPECT_TRUE((*p).uiName() == (*d).uiName());
        EXPECT_TRUE(p->uiName() == "File");
        p = 0;
        EXPECT_TRUE(p == NULL);
        EXPECT_TRUE(p.isNull());
        EXPECT_TRUE(p2 == d);
        p = p2;
        EXPECT_TRUE(p == d );
        delete d;
        EXPECT_TRUE(p.isNull() && p2.isNull());
    }

    caf::PdmPointer<DemoPdmObject> p3(new DemoPdmObject());
    
    delete p3;

}



//--------------------------------------------------------------------------------------------------
/// Tests the PdmFactory
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, PdmObjectFactory)
{
    {
        SimpleObj* s = NULL;
        s = dynamic_cast<SimpleObj*> (caf::PdmObjectFactory::instance()->create("SimpleObj"));
        EXPECT_TRUE(s != NULL);
    }
    {
        DemoPdmObject* s = NULL;
        s = dynamic_cast<DemoPdmObject*> (caf::PdmObjectFactory::instance()->create("DemoPdmObject"));
        EXPECT_TRUE(s != NULL);
        delete s;
    }
    {
        InheritedDemoObj* s = NULL;
        s = dynamic_cast<InheritedDemoObj*> (caf::PdmObjectFactory::instance()->create("InheritedDemoObj"));
        EXPECT_TRUE(s != NULL);
    }

    {
        caf::PdmDocument* s = NULL;
        s = dynamic_cast<caf::PdmDocument*> (caf::PdmObjectFactory::instance()->create("PdmDocument"));
        EXPECT_TRUE(s != NULL);
    }

    {
        caf::PdmObjectGroup* s = NULL;
        s = dynamic_cast<caf::PdmObjectGroup*> (caf::PdmObjectFactory::instance()->create("PdmObjectGroup"));
        EXPECT_TRUE(s != NULL);
    }

}

//--------------------------------------------------------------------------------------------------
/// Validate Xml keywords
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, ValidXmlKeywords)
{
    EXPECT_TRUE(caf::PdmObject::isValidXmlElementName("Valid_name"));

    EXPECT_FALSE(caf::PdmObject::isValidXmlElementName("2Valid_name"));
    EXPECT_FALSE(caf::PdmObject::isValidXmlElementName(".Valid_name"));
    EXPECT_FALSE(caf::PdmObject::isValidXmlElementName("xml_Valid_name"));
    EXPECT_FALSE(caf::PdmObject::isValidXmlElementName("Valid_name_with_space "));
}

TEST(BaseTest, PdmPointersFieldInsertVector)
{
    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    SimpleObj* s1 = new SimpleObj;
    SimpleObj* s2 = new SimpleObj;
    SimpleObj* s3 = new SimpleObj;

    caf::PdmObjectGroup pdmGroup;
    pdmGroup.addObject(s1);
    pdmGroup.addObject(s2);
    pdmGroup.addObject(s3);

    std::vector<caf::PdmPointer<SimpleObj> > typedObjects;
    pdmGroup.objectsByType(&typedObjects);

    ihd1->m_simpleObjectsField.insert(ihd1->m_simpleObjectsField.size(), typedObjects);
    EXPECT_EQ(size_t(3), ihd1->m_simpleObjectsField.size());

    delete ihd1;
}

TEST(BaseTest, PdmObjectGroupCopyOfTypedObjects)
{
    SimpleObj* s1 = new SimpleObj;
    s1->m_position = 1000;
    s1->m_numbers.v().push_back(10);

    SimpleObj* s2 = new SimpleObj;
    s2->m_position = 2000;

    SimpleObj* s3 = new SimpleObj;
    s3->m_position = 3000;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;

    caf::PdmObjectGroup og;
    og.objects.push_back(s1);
    og.objects.push_back(s2);
    og.objects.push_back(s3);
    og.objects.push_back(ihd1);

    std::vector<caf::PdmPointer<SimpleObj> > simpleObjList;
    og.createCopyByType(&simpleObjList);
    EXPECT_EQ(size_t(3), simpleObjList.size());

    std::vector<caf::PdmPointer<InheritedDemoObj> > inheritObjList;
    og.createCopyByType(&inheritObjList);
    EXPECT_EQ(size_t(1), inheritObjList.size());

    og.deleteObjects();
    EXPECT_EQ(size_t(3), simpleObjList.size());
    EXPECT_EQ(size_t(1), inheritObjList.size());
}
