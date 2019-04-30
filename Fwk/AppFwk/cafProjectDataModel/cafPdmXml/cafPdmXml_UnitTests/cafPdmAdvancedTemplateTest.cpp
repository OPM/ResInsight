
#include "gtest/gtest.h"


#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafPdmXmlObjectHandleMacros.h"
#include "cafFilePath.h"

#include <QXmlStreamWriter>





class ItemPdmObject : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:

    ItemPdmObject() :  PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_name, "Name");
    }

    explicit ItemPdmObject(QString name) :  PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_name, "Name");
        m_name = name;
    }

    ~ItemPdmObject() 
    {
    }

    // Fields
    caf::PdmDataValueField<QString>  m_name;
};
CAF_PDM_XML_SOURCE_INIT(ItemPdmObject, "ItemPdmObject");

class DemoPdmObjectA;

class ContainerPdmObject : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:

    ContainerPdmObject() :  PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   
        CAF_PDM_XML_InitField(&m_items, "Items");
        CAF_PDM_XML_InitField(&m_containers, "Containers");
        CAF_PDM_XML_InitField(&m_demoObjs, "DemoObjs");

    }

    ~ContainerPdmObject() 
    {
    }

    // Fields
    caf::PdmChildArrayField<ItemPdmObject*>  m_items;
    caf::PdmChildArrayField<ContainerPdmObject*>  m_containers;
    caf::PdmChildArrayField<DemoPdmObjectA*>  m_demoObjs;

};
CAF_PDM_XML_SOURCE_INIT(ContainerPdmObject, "ContainerPdmObject");

class DemoPdmObjectA : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;
public:
   enum TestEnumType
    {
        T1, T2, T3
    };

    DemoPdmObjectA() : PdmObjectHandle(), PdmXmlObjectHandle(this, false) 
    {   

        CAF_PDM_XML_InitField(&m_doubleField, "BigNumber");
        m_doubleField.registerSetMethod(this, &DemoPdmObjectA::setDoubleMember);
        m_doubleField.registerGetMethod(this, &DemoPdmObjectA::doubleMember);

        CAF_PDM_XML_InitField(&m_pointerToItem, "TestPointerToItem");
        CAF_PDM_XML_InitField(&m_pointerToDemoObj, "TestPointerToDemo");

    }

    ~DemoPdmObjectA() 
    {
    }

    // Fields
    caf::PdmProxyValueField<double>  m_doubleField;
    caf::PdmPtrField< caf::PdmObjectHandle* > m_pointerToItem;
    caf::PdmPtrField< caf::PdmObjectHandle* > m_pointerToDemoObj;

    caf::PdmDataValueField< caf::FilePath > m_singleFilePath;


    void setDoubleMember(const double& d) { m_doubleMember = d; std::cout << "setDoubleMember" << std::endl; }
    double doubleMember() const { std::cout << "doubleMember" << std::endl; return m_doubleMember; }
    double m_doubleMember;
};

CAF_PDM_XML_SOURCE_INIT(DemoPdmObjectA, "DemoPdmObjectA");

namespace caf
{

template<>
void AppEnum<DemoPdmObjectA::TestEnumType>::setUp()
{
    addItem(DemoPdmObjectA::T1,           "T1",         "An A letter");
    addItem(DemoPdmObjectA::T2,           "T2",         "A B letter");
    addItem(DemoPdmObjectA::T3,           "T3",         "A B letter");
    setDefault(DemoPdmObjectA::T1);
}

}


//--------------------------------------------------------------------------------------------------
/// Read/write fields to a valid Xml document encoded in a QString
//--------------------------------------------------------------------------------------------------
TEST(AdvancedObjectTest, FieldWrite)
{
    ContainerPdmObject* root = new ContainerPdmObject;
    ContainerPdmObject* container = new ContainerPdmObject;
    ContainerPdmObject* sibling = new ContainerPdmObject;
    root->m_containers.push_back(container);
    root->m_containers.push_back(sibling);

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name = "Obj A";

        container->m_items.push_back(item);
    }
    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name = "Obj B";

        container->m_items.push_back(item);
    }

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name = "Obj C";

        container->m_items.push_back(item);
    }


    // Test with empty ptr field
    {
        QString serializedString;
        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back(a);
            serializedString = a->writeObjectToXmlString();
            delete a;
        }


        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back(a);

            a->readObjectFromXmlString(serializedString, caf::PdmDefaultObjectFactory::instance());

            ASSERT_TRUE(a->m_pointerToItem() == NULL);
        }
    }

    {
        QString serializedString;
        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back(a);

            a->m_pointerToItem = container->m_items[1];

            serializedString = a->writeObjectToXmlString();
             std::cout << serializedString.toStdString() << std::endl;
            delete a;
        }


        {
            DemoPdmObjectA* a = new DemoPdmObjectA;
            sibling->m_demoObjs.push_back(a);

            a->readObjectFromXmlString(serializedString, caf::PdmDefaultObjectFactory::instance());
            a->xmlCapability()->resolveReferencesRecursively();

            ASSERT_TRUE(a->m_pointerToItem() == container->m_items[1]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(AdvancedObjectTest, CopyOfObjects)
{
    ContainerPdmObject* root = new ContainerPdmObject;
    ContainerPdmObject* container = new ContainerPdmObject;
    ContainerPdmObject* sibling = new ContainerPdmObject;
    root->m_containers.push_back(container);
    root->m_containers.push_back(sibling);

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name = "Obj A";

        container->m_items.push_back(item);
    }
    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name = "Obj B";

        container->m_items.push_back(item);
    }

    {
        ItemPdmObject* item = new ItemPdmObject();
        item->m_name = "Obj C";

        container->m_items.push_back(item);
    
        {
            {
                DemoPdmObjectA* a = new DemoPdmObjectA;
                sibling->m_demoObjs.push_back(a);

                a->m_pointerToItem = container->m_items[1];

                {
                    auto* objCopy = dynamic_cast<DemoPdmObjectA*>(a->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
                    std::vector<caf::PdmFieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively(&fieldWithFailingResolve);
                    ASSERT_FALSE(fieldWithFailingResolve.empty());
                    delete objCopy;
                }


                {
                    auto* objCopy = dynamic_cast<DemoPdmObjectA*>(a->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
                    
                    sibling->m_demoObjs.push_back(objCopy);

                    std::vector<caf::PdmFieldHandle*> fieldWithFailingResolve;
                    objCopy->resolveReferencesRecursively(&fieldWithFailingResolve);
                    ASSERT_TRUE(fieldWithFailingResolve.empty());
                    delete objCopy;
                }
            }
        }
    }
}

