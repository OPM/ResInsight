
#include "gtest/gtest.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafPdmXmlObjectHandleMacros.h"
#include "cafPdmDataValueField.h"
#include "cafPdmPtrArrayField.h"


class MyItemPdmObject : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:

    MyItemPdmObject() :  PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_name, "Name");
    }

    explicit MyItemPdmObject(QString name) :  PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_name, "Name");
        m_name = name;
    }

    ~MyItemPdmObject() 
    {
    }

    // Fields
    caf::PdmDataValueField<QString>  m_name;
};
CAF_PDM_XML_SOURCE_INIT(MyItemPdmObject, "MyItemPdmObject");

class MyContainerPdmObject : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:

    MyContainerPdmObject() :  PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_items, "Items");
        CAF_PDM_XML_InitField(&m_containers, "Containers");

    }


    ~MyContainerPdmObject() 
    {
    }


    // Fields
    caf::PdmChildArrayField<MyItemPdmObject*>   m_items;
    caf::PdmPtrArrayField<MyItemPdmObject*>     m_containers;

};
CAF_PDM_XML_SOURCE_INIT(MyContainerPdmObject, "MyContainerPdmObject");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PtrArrayBaseTest, PtrArraySerializeTest)
{
    
    MyContainerPdmObject* objA = new MyContainerPdmObject;

    MyItemPdmObject* s1 = new MyItemPdmObject;
    MyItemPdmObject* s2 = new MyItemPdmObject;
    MyItemPdmObject* s3 = new MyItemPdmObject;

    objA->m_items.push_back(s1);
    objA->m_items.push_back(s2);
    objA->m_items.push_back(s3);

    objA->m_containers.push_back(s1);
    objA->m_containers.push_back(s2);
    objA->m_containers.push_back(s3);

    //delete s2;

    QString serializedString;
    {
        serializedString = objA->writeObjectToXmlString();

        std::cout << serializedString.toStdString() << std::endl;

    }

    {
        MyContainerPdmObject* ihd1 = new MyContainerPdmObject;

        QXmlStreamReader xmlStream(serializedString);

        ihd1->readObjectFromXmlString(serializedString, caf::PdmDefaultObjectFactory::instance());
        ihd1->resolveReferencesRecursively();

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PtrArrayBaseTest, DeleteObjectPtrArraySerializeTest)
{

    MyContainerPdmObject* objA = new MyContainerPdmObject;

    MyItemPdmObject* s1 = new MyItemPdmObject;
    MyItemPdmObject* s2 = new MyItemPdmObject;
    MyItemPdmObject* s3 = new MyItemPdmObject;

    objA->m_items.push_back(s1);
    objA->m_items.push_back(s2);
    objA->m_items.push_back(s3);

    objA->m_containers.push_back(s1);
    objA->m_containers.push_back(s2);
    objA->m_containers.push_back(s3);

    delete s2;

    QString serializedString;
    {
        serializedString = objA->writeObjectToXmlString();

        std::cout << serializedString.toStdString() << std::endl;

    }

    {
        MyContainerPdmObject* ihd1 = new MyContainerPdmObject;

        QXmlStreamReader xmlStream(serializedString);

        ihd1->readObjectFromXmlString(serializedString, caf::PdmDefaultObjectFactory::instance());
        ihd1->resolveReferencesRecursively();

        EXPECT_TRUE(ihd1->m_containers.at(0) != nullptr);
        EXPECT_TRUE(ihd1->m_containers.at(1) == nullptr); // Deleted
        EXPECT_TRUE(ihd1->m_containers.at(2) == nullptr); // Pointing to item at index 2, does not longer exist

        EXPECT_TRUE(ihd1->m_items.size() == size_t(2));
        EXPECT_TRUE(ihd1->m_items[0] != nullptr);
        EXPECT_TRUE(ihd1->m_items[1] != nullptr);
    }
}

