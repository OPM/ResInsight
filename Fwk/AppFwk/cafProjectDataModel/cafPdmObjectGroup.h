#pragma once

#include "cafPdmObject.h"

#include <QXmlStreamWriter>

namespace caf 
{

class PdmReferenceHelper;

//==================================================================================================
/// The PdmObjectGroup serves as a container of unknown PdmObjects

/// It is not clear whether it really has a reusable value on its own.
//==================================================================================================
class PdmObjectGroup : public PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    PdmObjectGroup();
    ~PdmObjectGroup();

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

    QString encodedXml;
    {
        // Write original objects to XML file
        QXmlStreamWriter xmlStream(&encodedXml);
        xmlStream.setAutoFormatting(true);

        xmlStream.writeStartElement("", "PdmObjects");
        for (size_t i = 0; i < sourceTypedObjects.size(); i++)
        {
            PdmXmlObjectHandle* xmlObjHandle = sourceTypedObjects[i]->capability<PdmXmlObjectHandle>();
            assert(xmlObjHandle);

            QString className = xmlObjHandle->classKeyword();

            xmlStream.writeStartElement("", className);
            xmlObjHandle->writeFields(xmlStream);
            xmlStream.writeEndElement();
        }
        xmlStream.writeEndElement();
    }

    // Read back XML into object group, factory methods will be called that will create new objects
    PdmObjectGroup destinationObjectGroup;
    QXmlStreamReader xmlStream(encodedXml);

    PdmXmlObjectHandle* xmlObjHandle = destinationObjectGroup.capability<PdmXmlObjectHandle>();
    assert(xmlObjHandle);
    xmlObjHandle->readFields(xmlStream, objectFactory);

    for (size_t it = 0; it < destinationObjectGroup.objects.size(); it++)
    {
        T* obj = dynamic_cast<T*>(destinationObjectGroup.objects[it]);
        if (obj) copyOfTypedObjects->push_back(obj);
    }
}



} // End of namespace caf


