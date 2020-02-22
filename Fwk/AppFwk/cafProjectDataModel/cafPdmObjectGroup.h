#pragma once

#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"

#include <QXmlStreamWriter>

namespace caf 
{

class PdmReferenceHelper;

//==================================================================================================
/// The PdmObjectGroup serves as a container of unknown PdmObjects
//==================================================================================================
class PdmObjectGroup : public PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    PdmObjectGroup();
    ~PdmObjectGroup() override;

    std::vector<PdmObjectHandle*> objects;

    void                         deleteObjects();
    void                         addObject(PdmObjectHandle * obj);

    template <typename T>
    void objectsByType(std::vector<PdmPointer<T> >* typedObjects ) const
    {
        if (!typedObjects) return;
        size_t it;
        for (it = 0; it != objects.size(); ++it)
        {
            T* obj = dynamic_cast<T*>(objects[it]);
            if (obj) typedObjects->push_back(obj);
        }
    }

    template <typename T>
    void createCopyByType(std::vector<PdmPointer<T> >* copyOfTypedObjects, PdmObjectFactory* objectFactory) const;
};


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectGroup::createCopyByType(std::vector<PdmPointer<T> >* copyOfTypedObjects, PdmObjectFactory* objectFactory) const
{
    std::vector<PdmPointer<T> > sourceTypedObjects;
    objectsByType(&sourceTypedObjects);

    for (size_t i = 0; i < sourceTypedObjects.size(); i++)
    {
        QString xml = xmlObj(sourceTypedObjects[i])->writeObjectToXmlString();

        PdmObjectHandle* objectCopy = PdmXmlObjectHandle::readUnknownObjectFromXmlString(xml, PdmDefaultObjectFactory::instance(), true);

        T* typedObject = dynamic_cast<T*>(objectCopy);
        CAF_ASSERT(typedObject);

        copyOfTypedObjects->push_back(typedObject);
    }
}


//==================================================================================================
/// The PdmObjectCollection serves as a container of unknown PdmObjects stored in a PdmChildArrayField
//==================================================================================================
class PdmObjectCollection : public PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    PdmObjectCollection();
    ~PdmObjectCollection() override;

    caf::PdmChildArrayField<PdmObjectHandle*> objects;
};


} // End of namespace caf


