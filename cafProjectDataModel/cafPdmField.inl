//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cafPdmObject.h"
#include <vector>

namespace caf
{

//==================================================================================================
/// Implementation of PdmField<T> methods
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmField<DataType>::setValueFromUi(const QVariant& uiValue)
{
    QVariant oldValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);

    if (m_optionEntryCache.size())
    {
        // Check if we got an index into the option list
        if (uiValue.type() == QVariant::UInt)
        {
            assert(uiValue.toUInt() < static_cast<unsigned int>(m_optionEntryCache.size()));
            PdmFieldTypeSpecialization<DataType>::setFromVariant(m_optionEntryCache[uiValue.toUInt()].value, m_fieldValue); 
        }
        else
        {
            m_optionEntryCache.clear();
        }
    }
    else
    {
        PdmFieldTypeSpecialization<DataType>::setFromVariant(uiValue, m_fieldValue);
    }

    QVariant newValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);

    // Call changed methods if field value has changed
    if (newValue != oldValue)
    {
        assert(m_ownerObject != NULL);
        m_ownerObject->fieldChangedByUi(this, oldValue, newValue);
    }
} 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
QList<PdmOptionItemInfo> caf::PdmField<DataType>::valueOptions(bool* useOptionsOnly)
{
    if (m_ownerObject)
    {
        m_optionEntryCache = m_ownerObject->calculateValueOptions(this, useOptionsOnly);
        if (m_optionEntryCache.size())
        {
            // Find this field value in the list if present
            QVariant convertedFieldValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);
            unsigned int index;
            bool foundFieldValue = PdmOptionItemInfo::findValue(m_optionEntryCache, convertedFieldValue, &index);

            // If not found, we have to add it to the list, to be able to show it

            if (!foundFieldValue)
            {
                m_optionEntryCache.push_front(PdmOptionItemInfo(convertedFieldValue.toString(), convertedFieldValue, true, QIcon()));
            }

            if (m_optionEntryCache.size()) return m_optionEntryCache;
        }
    }

    return PdmFieldTypeSpecialization<DataType>::valueOptions(useOptionsOnly, m_fieldValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
QVariant caf::PdmField<DataType>::uiValue() const
{
    if (m_optionEntryCache.size())
    {
        QVariant convertedFieldValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);
        unsigned int index;
        bool foundFieldValue = PdmOptionItemInfo::findValue(m_optionEntryCache, convertedFieldValue, &index);
        assert(foundFieldValue);
        return QVariant(index);
    }

    return PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);
}

//==================================================================================================
/// Implementation of PdmField<T*> methods
/// (Partial specialization for pointers)
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

template<typename DataType >
void caf::PdmField<DataType*>::readFieldData(QXmlStreamReader& xmlStream)
{
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
    if (!xmlStream.isStartElement()) return; // Todo: Error handling

    QString className = xmlStream.name().toString();

    if (m_fieldValue.isNull())
    {
        m_fieldValue.setRawPtr(caf::PdmObjectFactory::instance()->create(className));
        if (m_fieldValue.notNull())
        {
            m_fieldValue.rawPtr()->addParentField(this);
        }
    }

    if (m_fieldValue.isNull()) return; // Warning: Unknown className read

    if (xmlStream.name() != m_fieldValue.rawPtr()->classKeyword()) return; // Error: Field contains different class type than on file

    m_fieldValue.rawPtr()->readFields(xmlStream);

    // Make stream point to end of element
    QXmlStreamReader::TokenType type;
    type = xmlStream.readNext();
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

template<typename DataType >
void caf::PdmField<DataType*>::writeFieldData(QXmlStreamWriter& xmlStream)
{
    if (m_fieldValue == NULL) return;

    QString className = m_fieldValue.rawPtr()->classKeyword(); 

    xmlStream.writeStartElement("", className);
    m_fieldValue.rawPtr()->writeFields(xmlStream);
    xmlStream.writeEndElement();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmField<DataType*>::childObjects(std::vector<PdmObject*>* objects)
{
    assert (objects);
    PdmObject* obj = m_fieldValue.rawPtr();
    if (obj)
    {
        objects->push_back(obj);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmField<DataType*>::PdmField(const PdmField& other)
{
    if (m_fieldValue) m_fieldValue.rawPtr()->removeParentField(this);
    m_fieldValue = other.m_fieldValue;
    if (m_fieldValue != NULL) m_fieldValue.rawPtr()->addParentField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmField<DataType*>::PdmField(const DataTypePtr& fieldValue)
{
    if (m_fieldValue) m_fieldValue->removeParentField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addParentField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmField<DataType*>::~PdmField()
{
    if (!m_fieldValue.isNull()) m_fieldValue.rawPtr()->removeParentField(this);
    m_fieldValue = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmField<DataType*>& PdmField<DataType*>::operator=(const PdmField& other)
{
    if (m_fieldValue) m_fieldValue->removeParentField(this);
    m_fieldValue = other.m_fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addParentField(this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmField<DataType*>& PdmField<DataType*>::operator=(const DataTypePtr & fieldValue)
{
    if (m_fieldValue) m_fieldValue->removeParentField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addParentField(this);
    return *this;
}


//==================================================================================================
/// Implementation of PdmPointersField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
PdmPointersField<DataType*>::PdmPointersField(const PdmPointersField& other)
{
    this->removeThisAsParentField();
    m_pointers = other.m_pointers;
    this->addThisAsParentField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
PdmPointersField<DataType*>::~PdmPointersField()
{
    this->removeThisAsParentField();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
caf::PdmPointersField<DataType*>& PdmPointersField<DataType*>::operator=(const PdmPointersField& other)
{
    this->removeThisAsParentField();
    m_pointers = other.m_pointers;
    this->addThisAsParentField();

    return *this;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
DataType* PdmPointersField<DataType*>::operator[](size_t index) const
{
    return m_pointers[index];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::push_back(DataType* pointer)
{
    m_pointers.push_back(pointer);
    if (pointer) pointer->addParentField(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::set(size_t index, DataType* pointer)
{
    if (m_pointers[index]) m_pointers[index]->removeParentField(this);
    m_pointers[index] = pointer;
    if (m_pointers[index]) pointer->addParentField(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::insert(size_t indexAfter, DataType* pointer)
{
    m_pointers.insert(m_pointers.begin()+indexAfter, pointer);

    if (pointer) pointer->addParentField(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::clear()
{
    
    this->removeThisAsParentField();
    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::deleteChildren()
{
    size_t index;
    for (index = 0; index < m_pointers.size(); ++index)
    {
        delete(m_pointers[index]);
    }

    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::erase(size_t index)
{
    if (m_pointers[index]) m_pointers[index]->removeParentField(this);
    m_pointers.erase(m_pointers.begin() + index);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::removeAll(DataType* pointer)
{
    size_t index;
    std::vector< PdmPointer<DataType> > tempPointers;
    tempPointers = m_pointers;
    m_pointers.clear();
    for (index = 0; index < tempPointers.size(); ++index)
    {
        if (tempPointers[index] != pointer)
        {
            m_pointers.push_back(tempPointers[index]);
        }
        else
        {
            if (tempPointers[index]) tempPointers[index]->removeParentField(this);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
 void PdmPointersField<DataType*>::writeFieldData( QXmlStreamWriter& xmlStream)
{
    typename std::vector< PdmPointer<DataType> >::iterator it;
    for (it = m_pointers.begin(); it != m_pointers.end(); ++it)
    {
        if (*it == NULL) continue;

        QString className = (*it)->classKeyword();

        xmlStream.writeStartElement("", className);
        (*it)->writeFields(xmlStream);
        xmlStream.writeEndElement();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
 template<typename DataType>
 void PdmPointersField<DataType*>::readFieldData(QXmlStreamReader& xmlStream)
{
    DataType * currentObject = NULL;
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
    while (xmlStream.isStartElement())
    {
        QString className = xmlStream.name().toString();

        PdmObject * obj = PdmObjectFactory::instance()->create(className);

        if (obj == NULL)
        {
            // Warning: Unknown className read
            // Skip to corresponding end element
            xmlStream.skipCurrentElement();
            PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
            continue; 
        }

        currentObject = dynamic_cast<DataType *> (obj);

        if (currentObject == NULL)
        {
            // Warning: Inconsistency in factory !! Assert ?
            // Skip to corresponding end element
            xmlStream.skipCurrentElement();
            PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
            continue; 
        }

        currentObject->readFields(xmlStream);
        m_pointers.push_back(currentObject);

        // Skip comments and for some reason: Characters. The last bit should not be correct, 
        // but Qt reports a character token between EndElement and StartElement

        QXmlStreamReader::TokenType type;
        type = xmlStream.readNext();
        PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::childObjects(std::vector<PdmObject*>* objects)
{
    if (!objects) return;
    size_t i;
    for (i = 0; i < m_pointers.size(); ++i)
    {
        objects->push_back(m_pointers[i]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::removeThisAsParentField()
{
    typename std::vector< PdmPointer< DataType > >::iterator it;
    for (it = m_pointers.begin(); it != m_pointers.end(); ++it)
    {
        if (!it->isNull()) 
        {
            (*it)->removeParentField(this);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType>
void PdmPointersField<DataType*>::addThisAsParentField()
{
    typename std::vector< PdmPointer< DataType > >::iterator it;
    for (it = m_pointers.begin(); it != m_pointers.end(); ++it)
    {
        if (!it->isNull()) 
        {
            (*it)->addParentField(this);
        }
    }
}

} //End of namespace caf
