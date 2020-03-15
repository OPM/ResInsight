
#include "gtest/gtest.h"

#include "cafAppEnum.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafPdmXmlObjectHandleMacros.h"
#include "cafPdmDataValueField.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"

#include <QXmlStreamWriter>


class DemoPdmObject : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:
   enum TestEnumType
    {
        T1, T2, T3
    };

    DemoPdmObject() : PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_proxyDoubleField, "BigNumber");
        m_proxyDoubleField.registerSetMethod(this, &DemoPdmObject::setDoubleMember);
        m_proxyDoubleField.registerGetMethod(this, &DemoPdmObject::doubleMember);

        CAF_PDM_XML_InitField(&m_proxyEnumField, "AppEnum");
        m_proxyEnumField.registerSetMethod(this, &DemoPdmObject::setEnumMember);
        m_proxyEnumField.registerGetMethod(this, &DemoPdmObject::enumMember);
        m_enumMember = T1;
    }

    ~DemoPdmObject() 
    {
    }

    // Fields

    caf::PdmProxyValueField<double>  m_proxyDoubleField;
    caf::PdmProxyValueField<caf::AppEnum<TestEnumType> >  m_proxyEnumField;

private:
    void setDoubleMember(const double& d) { m_doubleMember = d; std::cout << "setDoubleMember" << std::endl; }
    double doubleMember() const { std::cout << "doubleMember" << std::endl; return m_doubleMember; }
    
    void setEnumMember(const caf::AppEnum<TestEnumType>& val) { m_enumMember = val; }
    caf::AppEnum<TestEnumType> enumMember() const { return m_enumMember; }

    double m_doubleMember;
    TestEnumType m_enumMember;
};

CAF_PDM_XML_SOURCE_INIT(DemoPdmObject, "DemoPdmObject");



namespace caf
{

template<>
void AppEnum<DemoPdmObject::TestEnumType>::setUp()
{
    addItem(DemoPdmObject::T1,           "T1",         "An A letter");
    addItem(DemoPdmObject::T2,           "T2",         "A B letter");
    addItem(DemoPdmObject::T3,           "T3",         "A B letter");
    setDefault(DemoPdmObject::T1);
}

}

TEST(BaseTest, Delete)
{
     DemoPdmObject* s2 = new DemoPdmObject;
     delete s2;
}

#if 0 
//--------------------------------------------------------------------------------------------------
/// Read/write Xml using PdmObjectGroup
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, Start)
{
    QString serializedString;
    {
        DemoPdmObject* a = new DemoPdmObject;

        a->m_proxyDoubleField.setValue(2.5);
        a->m_proxyEnumField.setValue(DemoPdmObject::T3);

        ASSERT_DOUBLE_EQ(2.5, a->m_proxyDoubleField.value());

        caf::PdmObjectGroup objGroup;
        objGroup.addObject(a);

        QXmlStreamWriter xmlStream(&serializedString);
        xmlStream.setAutoFormatting(true);
        objGroup.writeFields(xmlStream, NULL);

        std::cout << serializedString.toStdString() << std::endl;

        delete a;
    }

    /*
        <PdmObjects>
          <DemoPdmObject>
            <BigNumber>2.5</BigNumber>
            <TestEnumValue>T3</TestEnumValue>
          </DemoPdmObject>
        </PdmObjects>
    */

    {
        caf::PdmObjectGroup destinationObjectGroup;
        QXmlStreamReader xmlStream(serializedString);
        destinationObjectGroup.readFields(xmlStream, caf::PdmDefaultObjectFactory::instance(), NULL);

        DemoPdmObject* a = dynamic_cast<DemoPdmObject*>(destinationObjectGroup.objects[0]);

        ASSERT_DOUBLE_EQ(2.5, a->m_proxyDoubleField.value());
        ASSERT_EQ(DemoPdmObject::T3, a->m_proxyEnumField());

    }
}
#endif
//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a QString
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, FieldWrite)
{
    QString serializedString;
    {
        DemoPdmObject* a = new DemoPdmObject;

        a->m_proxyDoubleField.setValue(2.5);
        ASSERT_DOUBLE_EQ(2.5, a->m_proxyDoubleField.value());

        serializedString = a->writeObjectToXmlString();

        std::cout << serializedString.toStdString() << std::endl;

        delete a;
    }

    /*
    <DemoPdmObject>
        <BigNumber>2.5</BigNumber>
        <TestEnumValue>T3</TestEnumValue>
    </DemoPdmObject>
    */

    {
        DemoPdmObject* a = new DemoPdmObject;

        a->readObjectFromXmlString(serializedString,  caf::PdmDefaultObjectFactory::instance());

    }

}

class InheritedDemoObj : public DemoPdmObject
{
    CAF_PDM_XML_HEADER_INIT;
public:
    
    InheritedDemoObj()
    {
        CAF_PDM_XML_InitField(&m_texts, "Texts");
        CAF_PDM_XML_InitField(&m_childArrayField, "DemoPdmObjectects");
    }

    ~InheritedDemoObj()
    {
        m_childArrayField.deleteAllChildObjects();
    }

    caf::PdmDataValueField<QString >   m_texts;
    caf::PdmChildArrayField<DemoPdmObject*>         m_childArrayField;

};
CAF_PDM_XML_SOURCE_INIT(InheritedDemoObj, "InheritedDemoObj");


class SimpleObj : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:
    SimpleObj() : PdmObjectHandle(), PdmXmlObjectHandle(this, false),
        m_doubleMember(0.0)
    {
        CAF_PDM_XML_InitField(&m_position,  "Position");
        CAF_PDM_XML_InitField(&m_dir,       "Dir");
        CAF_PDM_XML_InitField(&m_up,        "Up");
   
        CAF_PDM_XML_InitField(&m_singleFilePath,    "m_singleFilePath");
        CAF_PDM_XML_InitField(&m_multipleFilePath,  "m_multipleFilePath");

        CAF_PDM_XML_InitField(&m_proxyDouble,  "m_proxyDouble");
        m_proxyDouble.registerSetMethod(this, &SimpleObj::setDoubleMember);
        m_proxyDouble.registerGetMethod(this, &SimpleObj::doubleMember);
    }

    caf::PdmDataValueField<double>               m_position;
    caf::PdmDataValueField<double>               m_dir;
    caf::PdmDataValueField<int>                  m_up;
    caf::PdmProxyValueField<double>          m_proxyDouble;

    caf::PdmDataValueField<caf::FilePath>               m_singleFilePath;
    caf::PdmDataValueField<std::vector<caf::FilePath>>  m_multipleFilePath;


    void setDoubleMember(const double& d) { m_doubleMember = d; std::cout << "setDoubleMember" << std::endl; }
    double doubleMember() const { std::cout << "doubleMember" << std::endl; return m_doubleMember; }

    double m_doubleMember;


};
CAF_PDM_XML_SOURCE_INIT(SimpleObj, "SimpleObj");


class ReferenceDemoPdmObject : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:

    ReferenceDemoPdmObject() : PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_pointersField, "SimpleObjPtrField");
        CAF_PDM_XML_InitField(&m_simpleObjPtrField2, "SimpleObjPtrField2");
    }

    ~ReferenceDemoPdmObject()
    {
        delete m_pointersField();
        m_simpleObjPtrField2.deleteAllChildObjects();
    }

    // Fields
    caf::PdmChildField<PdmObjectHandle*> m_pointersField;
    caf::PdmChildArrayField<SimpleObj*> m_simpleObjPtrField2;
};

CAF_PDM_XML_SOURCE_INIT(ReferenceDemoPdmObject, "ReferenceDemoPdmObject");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, PdmReferenceHelper)
{
    DemoPdmObject* s1 = new DemoPdmObject;
    DemoPdmObject* s2 = new DemoPdmObject;
    DemoPdmObject* s3 = new DemoPdmObject;

    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    ihd1->m_childArrayField.push_back(new DemoPdmObject);

    ihd1->m_childArrayField.push_back(s1);
    ihd1->m_childArrayField.push_back(s2);
    ihd1->m_childArrayField.push_back(s3);

    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToObject(ihd1, s3);
        QString expectedString = ihd1->m_childArrayField.keyword() + " 3";
        EXPECT_STREQ(refString.toLatin1(), expectedString.toLatin1());

        caf::PdmObjectHandle* fromRef = caf::PdmReferenceHelper::objectFromReference(ihd1, refString);
        EXPECT_TRUE(fromRef == s3);
    }

    ReferenceDemoPdmObject* objA = new ReferenceDemoPdmObject;
    objA->m_pointersField = ihd1;

    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToObject(objA, s3);

        caf::PdmObjectHandle* fromRef = caf::PdmReferenceHelper::objectFromReference(objA, refString);
        EXPECT_TRUE(fromRef == s3);
    }


    // Test reference to field
    {
        QString refString = caf::PdmReferenceHelper::referenceFromRootToField(objA, &(ihd1->m_childArrayField));

        caf::PdmFieldHandle* fromRef = caf::PdmReferenceHelper::fieldFromReference(objA, refString);
        EXPECT_TRUE(fromRef == &(ihd1->m_childArrayField));
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, ChildArrayFieldSerializing)
{
    DemoPdmObject* s1 = new DemoPdmObject;
    s1->m_proxyDoubleField.setValue(10);

    DemoPdmObject* s2 = new DemoPdmObject;
    s2->m_proxyDoubleField.setValue(20);

    DemoPdmObject* s3 = new DemoPdmObject;
    s3->m_proxyDoubleField.setValue(30);

    InheritedDemoObj* ihd1 = new InheritedDemoObj;
    ihd1->m_childArrayField.push_back(s1);
    ihd1->m_childArrayField.push_back(s2);
    ihd1->m_childArrayField.push_back(s3);

    QString serializedString;
    {
        serializedString = ihd1->writeObjectToXmlString();

        std::cout << serializedString.toStdString() << std::endl;

        delete ihd1;
    }

    {
        InheritedDemoObj* ihd1 = new InheritedDemoObj;
        ASSERT_EQ(0, ihd1->m_childArrayField.size());

        QXmlStreamReader xmlStream(serializedString);

        ihd1->readObjectFromXmlString(serializedString, caf::PdmDefaultObjectFactory::instance());

        ASSERT_DOUBLE_EQ(10, ihd1->m_childArrayField[0]->m_proxyDoubleField.value());
        ASSERT_DOUBLE_EQ(20, ihd1->m_childArrayField[1]->m_proxyDoubleField.value());
        ASSERT_DOUBLE_EQ(30, ihd1->m_childArrayField[2]->m_proxyDoubleField.value());
    }
}

//--------------------------------------------------------------------------------------------------
/// Testing that the QXmlStreamReader actually can not just read a list of fields.
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, QXMLStreamTest)
{
    QString xmlText = 
     //"<DemoPdmObject>"
         "<BigNumber>2.5</BigNumber>"
         "<TestEnumValue>T3</TestEnumValue>"
          "<TestEnumValue2>T3</TestEnumValue2>"
     //"</DemoPdmObject>"
     ;

    QXmlStreamReader inputStream(xmlText);

    QXmlStreamReader::TokenType tt;
    while (!inputStream.atEnd())
    {
        tt = inputStream.readNext();
        std::cout << inputStream.name().toString().toStdString() << std::endl;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, FilePathSerializing)
{
    SimpleObj* s1 = new SimpleObj;

    QString newVal = "path with space";
    s1->m_multipleFilePath.v().push_back(newVal);
    s1->m_multipleFilePath.v().push_back(newVal);

    s1->m_singleFilePath = newVal;

    QString serializedString = s1->writeObjectToXmlString();

    {
        SimpleObj* ihd1 = new SimpleObj;

        QXmlStreamReader xmlStream(serializedString);

        ihd1->readObjectFromXmlString(serializedString, caf::PdmDefaultObjectFactory::instance());

        EXPECT_EQ(2, ihd1->m_multipleFilePath.v().size());
        EXPECT_EQ(newVal.toStdString(), ihd1->m_singleFilePath().path().toStdString());
    
        delete ihd1;
    }

    delete s1;
}

// Type deduction is different on other platforms than Windows
#ifdef WIN32
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(BaseTest, TestDataType)
{
    SimpleObj* s1 = new SimpleObj;

    {
        auto dataTypeNameDouble = s1->m_position.xmlCapability()->dataTypeName();
        EXPECT_EQ("double", dataTypeNameDouble.toStdString());
    }
    
    {
        auto dataTypeNameDouble = s1->m_proxyDouble.xmlCapability()->dataTypeName();
        EXPECT_EQ("double", dataTypeNameDouble.toStdString());
    }

    {
        auto dataTypeNameDouble = s1->m_up.xmlCapability()->dataTypeName();
        EXPECT_EQ("int", dataTypeNameDouble.toStdString());
    }

    {
        auto dataTypeNameDouble = s1->m_singleFilePath.xmlCapability()->dataTypeName();
        EXPECT_EQ("class caf::FilePath", dataTypeNameDouble.toStdString());
    }

    {
        InheritedDemoObj* obj = new InheritedDemoObj;
        auto dataTypeNameDouble = obj->m_texts.xmlCapability()->dataTypeName();
        EXPECT_EQ("class QString", dataTypeNameDouble.toStdString());
    }

    delete s1;
}
#endif

