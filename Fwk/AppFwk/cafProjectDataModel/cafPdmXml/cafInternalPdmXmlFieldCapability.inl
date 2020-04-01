
#include "cafAssert.h"
#include "cafInternalPdmFieldIoHelper.h"
#include "cafPdmObjectFactory.h"
#include "cafPdmXmlObjectHandle.h"

#include <QStringList>

#include <iostream>

namespace caf
{
//==================================================================================================
/// XML Implementation for PdmFieldXmlCap<> methods
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template < typename FieldType>
bool caf::PdmFieldXmlCap<FieldType>::isVectorField() const
{
    return is_vector<FieldType>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename FieldType >
 void  caf::PdmFieldXmlCap<FieldType>::readFieldData(QXmlStreamReader& xmlStream, 
                                          PdmObjectFactory* objectFactory)
 { 
     this->assertValid(); 
     typename FieldType::FieldDataType value; 
     PdmFieldReader<typename FieldType::FieldDataType>::readFieldData(value, xmlStream, objectFactory);  
     m_field->setValue(value); 
 }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename FieldType >
void caf::PdmFieldXmlCap<FieldType>::writeFieldData(QXmlStreamWriter& xmlStream) const
 { 
     this->assertValid(); 
     PdmFieldWriter<typename FieldType::FieldDataType>::writeFieldData(m_field->value(), xmlStream); 
 }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template < typename FieldType>
bool caf::PdmFieldXmlCap<FieldType>::resolveReferences()
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmPtrField<>
//==================================================================================================

 //--------------------------------------------------------------------------------------------------
 /// 
 //--------------------------------------------------------------------------------------------------

 template<typename DataType >
 void caf::PdmFieldXmlCap< caf::PdmPtrField<DataType*> >::readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory*)
 {
     this->assertValid(); 

     PdmFieldIOHelper::skipComments(xmlStream);
     if (!xmlStream.isCharacters()) return;

     QString dataString = xmlStream.text().toString();

     // Make stream point to end of element
     QXmlStreamReader::TokenType type = xmlStream.readNext();
     Q_UNUSED(type);
     PdmFieldIOHelper::skipCharactersAndComments(xmlStream);

     // This resolving can NOT be done here. 
     // It must be done when we know that the complete hierarchy is read and created, 
     // The object pointed to is not always read and created at this point in time. 
     // We rather need to do something like :
     // m_refenceString = dataString;
     // m_isResolved = false;
     // m_field->setRawPtr(NULL);
     //
     // and then we need a traversal of the object hierarchy to resolve all references before initAfterRead.

     m_isResolved = false;
     m_referenceString = dataString;
     m_field->setRawPtr(NULL);
}

 //--------------------------------------------------------------------------------------------------
 /// 
 //--------------------------------------------------------------------------------------------------

 template<typename DataType >
 void caf::PdmFieldXmlCap< caf::PdmPtrField<DataType*> >::writeFieldData(QXmlStreamWriter& xmlStream) const
 {
     this->assertValid(); 

     QString dataString;

     dataString = PdmReferenceHelper::referenceFromFieldToObject(m_field, m_field->m_fieldValue.rawPtr());

     xmlStream.writeCharacters(dataString);
 }


 //--------------------------------------------------------------------------------------------------
 /// 
 //--------------------------------------------------------------------------------------------------
 template < typename DataType>
 bool caf::PdmFieldXmlCap< PdmPtrField<DataType*> >::resolveReferences()
 {
     if (m_isResolved) return true;
     if (m_referenceString.isEmpty()) return true;

     PdmObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference(this->fieldHandle(), m_referenceString);
     m_field->setRawPtr(objHandle);
     m_isResolved = true;

     return objHandle != nullptr;
 }

 //--------------------------------------------------------------------------------------------------
 ///
 //--------------------------------------------------------------------------------------------------
 template<typename DataType>
 QString caf::PdmFieldXmlCap<PdmPtrField<DataType*>>::referenceString() const
 {
     return m_referenceString;
 }

 //==================================================================================================
/// XML Implementation for PdmPtrArrayField<>
//==================================================================================================

 //--------------------------------------------------------------------------------------------------
 /// 
 //--------------------------------------------------------------------------------------------------

 template<typename DataType >
 void caf::PdmFieldXmlCap< caf::PdmPtrArrayField<DataType*> >::readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory*)
 {
     this->assertValid();

     PdmFieldIOHelper::skipComments(xmlStream);
     if(!xmlStream.isCharacters()) return;

     QString dataString = xmlStream.text().toString();

     // Make stream point to end of element
     QXmlStreamReader::TokenType type = xmlStream.readNext();
     Q_UNUSED(type);
     PdmFieldIOHelper::skipCharactersAndComments(xmlStream);

     // This resolving can NOT be done here. 
     // It must be done when we know that the complete hierarchy is read and created, 
     // and then we need a traversal of the object hierarchy to resolve all references before initAfterRead.

     m_isResolved = false;
     m_referenceString = dataString;
     m_field->clear();
 }

 //--------------------------------------------------------------------------------------------------
 /// 
 //--------------------------------------------------------------------------------------------------

 template<typename DataType >
 void caf::PdmFieldXmlCap< caf::PdmPtrArrayField<DataType*> >::writeFieldData(QXmlStreamWriter& xmlStream) const
 {
     this->assertValid();

     QString dataString;
     size_t pointerCount = m_field->m_pointers.size();
     for (size_t i = 0; i < pointerCount; ++i)
     {
         dataString += PdmReferenceHelper::referenceFromFieldToObject(m_field, m_field->m_pointers[i].rawPtr());
         if (!dataString.isEmpty() && i < pointerCount-1) dataString += " | \n\t";
     }
     xmlStream.writeCharacters(dataString);
 }


 //--------------------------------------------------------------------------------------------------
 /// 
 //--------------------------------------------------------------------------------------------------
 template < typename DataType>
 bool caf::PdmFieldXmlCap< PdmPtrArrayField<DataType*> >::resolveReferences()
 {
     if(m_isResolved) return true;
     if(m_referenceString.isEmpty()) return true;
     m_field->clear();

     bool foundValidObjectFromString = true;
     QStringList tokens = m_referenceString.split('|');
     for(int i = 0; i < tokens.size(); ++i)
     {
         PdmObjectHandle* objHandle = PdmReferenceHelper::objectFromFieldReference(this->fieldHandle(), tokens[i]);
         if (!tokens[i].isEmpty() && !objHandle)
         {
             foundValidObjectFromString = false;
         }

         m_field->m_pointers.push_back(NULL);
         m_field->m_pointers.back().setRawPtr(objHandle);
     }

     m_isResolved = true;

     return foundValidObjectFromString;
 }
 
 //--------------------------------------------------------------------------------------------------
 ///
 //--------------------------------------------------------------------------------------------------
 template < typename DataType>
 bool caf::PdmFieldXmlCap< caf::PdmPtrArrayField<DataType*> >::isVectorField() const
 {
     return true;
 }

//==================================================================================================
/// XML Implementation for PdmChildField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

template<typename DataType >
void caf::PdmFieldXmlCap< caf::PdmChildField<DataType*> >::readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory)
{
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
    if (!xmlStream.isStartElement())
    {
        return; // This happens when the field is "shortcut" empty (written like: <ElementName/>) 
    }

    QString className = xmlStream.name().toString();
    PdmObjectHandle* obj = NULL;

    // Create an object if needed 
    if (m_field->value() == NULL)
    {
        CAF_ASSERT(objectFactory);
        obj = objectFactory->create(className);

        if (obj == NULL)
        {
            std::cout << "Line " << xmlStream.lineNumber() << ": Warning: Unknown object type with class name: " << className.toLatin1().data() << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            xmlStream.skipCurrentElement(); // Skip to the endelement of the object we was supposed to read
            xmlStream.skipCurrentElement(); // Skip to the endelement of this field
            return;
        }
        else
        {
            PdmXmlObjectHandle* xmlObject = xmlObj(obj);
            if (!xmlObject || !xmlObject->matchesClassKeyword(className))
            {
                CAF_ASSERT(false); // Inconsistency in the factory. It creates objects of wrong type from the ClassKeyword

                xmlStream.skipCurrentElement(); // Skip to the endelement of the object we was supposed to read
                xmlStream.skipCurrentElement(); // Skip to the endelement of this field

                return;
            }

            m_field->m_fieldValue.setRawPtr(obj);
            obj->setAsParentField(m_field);
        }
    }
    else
    {
        obj = m_field->m_fieldValue.rawPtr();
    }

    PdmXmlObjectHandle* xmlObject = xmlObj(obj);
    if (!xmlObject || !xmlObject->matchesClassKeyword(className))
    {
        // Error: Field contains different class type than on file
        std::cout << "Line " << xmlStream.lineNumber() << ": Warning: Unknown object type with class name: " << className.toLatin1().data() << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;
        std::cout << "                     Expected class name: " << xmlObject->classKeyword().toLatin1().data() << std::endl;

        xmlStream.skipCurrentElement(); // Skip to the endelement of the object we was supposed to read
        xmlStream.skipCurrentElement(); // Skip to the endelement of this field

        return;
    }

    // Everything seems ok, so read the contents of the object:

    xmlObject->readFields(xmlStream, objectFactory, false);

    // Make stream point to endElement of this field

    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED(type);
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

template<typename DataType >
void caf::PdmFieldXmlCap< caf::PdmChildField<DataType*> >::writeFieldData(QXmlStreamWriter& xmlStream) const
{
    if (m_field->m_fieldValue.rawPtr() == NULL) return;

    PdmXmlObjectHandle* xmlObject = xmlObj(m_field->m_fieldValue.rawPtr());
    if (xmlObject)
    {
        QString className = xmlObject->classKeyword();

        xmlStream.writeStartElement("", className);
        xmlObject->writeFields(xmlStream);
        xmlStream.writeEndElement();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template < typename DataType>
bool caf::PdmFieldXmlCap<caf::PdmChildField<DataType *>>::resolveReferences()
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmChildArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void caf::PdmFieldXmlCap< caf::PdmChildArrayField<DataType*> >::writeFieldData(QXmlStreamWriter& xmlStream) const
{
    typename std::vector< PdmPointer<DataType> >::iterator it;
    for (it = m_field->m_pointers.begin(); it != m_field->m_pointers.end(); ++it)
    {
        if (it->rawPtr() == NULL) continue;

        PdmXmlObjectHandle* xmlObject = xmlObj(it->rawPtr());
        if (xmlObject)
        {
            QString className = xmlObject->classKeyword();

            xmlStream.writeStartElement("", className);
            xmlObject->writeFields(xmlStream);
            xmlStream.writeEndElement();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void caf::PdmFieldXmlCap< caf::PdmChildArrayField<DataType*> >::readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory)
{
    m_field->deleteAllChildObjects();
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
    while (xmlStream.isStartElement())
    {
        QString className = xmlStream.name().toString();

        CAF_ASSERT(objectFactory);
        PdmObjectHandle * obj = objectFactory->create(className);

        if (obj == NULL)
        {
            // Warning: Unknown className read
            // Skip to corresponding end element

            std::cout << "Line " << xmlStream.lineNumber() << ": Warning: Unknown object type with class name: " << className.toLatin1().data() << " found while reading the field : " << m_field->keyword().toLatin1().data() << std::endl;

            // Skip to EndElement of the object
            xmlStream.skipCurrentElement();

            // Jump off the end element, and head for next start element (or the final EndElement of the field)
            QXmlStreamReader::TokenType type = xmlStream.readNext();
            Q_UNUSED(type);
            PdmFieldIOHelper::skipCharactersAndComments(xmlStream);

            continue;
        }

        PdmXmlObjectHandle* xmlObject = xmlObj(obj);
        if (!xmlObject || !xmlObject->matchesClassKeyword(className))
        {
            CAF_ASSERT(false); // There is an inconsistency in the factory. It creates objects of type not matching the ClassKeyword

            // Skip to EndElement of the object
            xmlStream.skipCurrentElement();

            // Jump off the end element, and head for next start element (or the final EndElement of the field)
            QXmlStreamReader::TokenType type = xmlStream.readNext();
            Q_UNUSED(type);
            PdmFieldIOHelper::skipCharactersAndComments(xmlStream);

            continue;
        }

        xmlObject->readFields(xmlStream, objectFactory, false);

        m_field->m_pointers.push_back(PdmPointer<DataType>());
        m_field->m_pointers.back().setRawPtr(obj);
        obj->setAsParentField(m_field);

        // Jump off the end element, and head for next start element (or the final EndElement of the field)
        // Qt reports a character token between EndElements and StartElements so skip it

        QXmlStreamReader::TokenType type = xmlStream.readNext();
        Q_UNUSED(type);
        PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template < typename DataType>
bool caf::PdmFieldXmlCap<caf::PdmChildArrayField<DataType *>>::resolveReferences()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template < typename DataType>
bool caf::PdmFieldXmlCap< caf::PdmChildArrayField<DataType*> >::isVectorField() const
{
    return true;
}

//==================================================================================================
/// XML Implementation for PdmFieldXmlCap<std::vector<DataType>> methods
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template < typename DataType>
bool caf::PdmFieldXmlCap<caf::PdmField<std::vector<DataType>>>::isVectorField() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
 void  caf::PdmFieldXmlCap<caf::PdmField<std::vector<DataType>>>::readFieldData(QXmlStreamReader& xmlStream, 
                                          PdmObjectFactory* objectFactory)
 { 
     this->assertValid(); 
     typename FieldType::FieldDataType value; 
     PdmFieldReader<typename FieldType::FieldDataType>::readFieldData(value, xmlStream, objectFactory);  
     m_field->setValue(value); 
 }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmFieldXmlCap<caf::PdmField<std::vector<DataType>>>::writeFieldData(QXmlStreamWriter& xmlStream) const
 { 
     this->assertValid(); 
     PdmFieldWriter<typename FieldType::FieldDataType>::writeFieldData(m_field->value(), xmlStream); 
 }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template < typename DataType>
bool caf::PdmFieldXmlCap<caf::PdmField<std::vector<DataType>>>::resolveReferences()
{
    return true;
}

} // End namespace caf
