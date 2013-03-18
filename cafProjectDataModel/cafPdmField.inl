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
#include "cafPdmUiFieldEditorHandle.h"

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

    // Check whether we are handling selections of values or actual values
    if (m_optionEntryCache.size())
    {
       // This has an option based GUI, the uiValue is only indexes into the m_optionEntryCache
        if (uiValue.type() == QVariant::UInt)
        {
            assert(uiValue.toUInt() < static_cast<unsigned int>(m_optionEntryCache.size()));
            PdmFieldTypeSpecialization<DataType>::setFromVariant(m_optionEntryCache[uiValue.toUInt()].value, m_fieldValue); 
        }
        else if (uiValue.type() == QVariant::List)
        {
            QList<QVariant> selectedIndexes = uiValue.toList();
            QList<QVariant> valuesToSetInField;

            if (selectedIndexes.isEmpty())
            {
                PdmFieldTypeSpecialization<DataType>::setFromVariant(valuesToSetInField, m_fieldValue);
            }
            else
            {
                if (selectedIndexes.front().type() == QVariant::UInt)
                {
                    for (int i = 0; i < selectedIndexes.size(); ++i)
                    {
                        unsigned int opIdx = selectedIndexes[i].toUInt();
                        if (opIdx < static_cast<unsigned int>(m_optionEntryCache.size()))
                        {
                            valuesToSetInField.push_back(m_optionEntryCache[opIdx].value);
                        }
                    }

                    PdmFieldTypeSpecialization<DataType>::setFromVariant(valuesToSetInField, m_fieldValue);
                }
                else
                {
                    // We are not getting indexes as expected from the UI. For now assert, to catch this condition
                    // but it should possibly be handled as setting the values explicitly. The code for that is below the assert
                    assert(false);
                    PdmFieldTypeSpecialization<DataType>::setFromVariant(uiValue, m_fieldValue);
                    m_optionEntryCache.clear();
                }
            }

        }
        else 
        { 
            // We are not getting indexes as expected from the UI. For now assert, to catch this condition
            // but it should possibly be handled as setting the values explicitly. The code for that is below the assert
            assert(false);
            PdmFieldTypeSpecialization<DataType>::setFromVariant(uiValue, m_fieldValue);
            m_optionEntryCache.clear();
        }
    }
    else
    {   // Not an option based GUI, the uiValue is a real field value
        PdmFieldTypeSpecialization<DataType>::setFromVariant(uiValue, m_fieldValue);
    }

    QVariant newValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);

    // Call changed methods if field value has changed
    if (newValue != oldValue)
    {
        assert(m_ownerObject != NULL);
        m_ownerObject->fieldChangedByUi(this, oldValue, newValue);

        // This assumes that all field editors are updated by an instance of PdmUiObjectEditorHandle
        m_ownerObject->updateConnectedEditors();
    }
} 

//--------------------------------------------------------------------------------------------------
/// Returns the option values that is to be displayed in the UI for this field. 
/// This method calls the virtual PdmObject::calculateValueOptions to get the list provided from the 
/// application, then possibly adds the current field value(s) to the list, to 
/// make sure the actual values are shown
//--------------------------------------------------------------------------------------------------
template<typename DataType >
QList<PdmOptionItemInfo> caf::PdmField<DataType>::valueOptions(bool* useOptionsOnly)
{
    // First check if the owner PdmObject has a value options specification. 
    // if it has, use it.
    if (m_ownerObject)
    {
        m_optionEntryCache = m_ownerObject->calculateValueOptions(this, useOptionsOnly);
        if (m_optionEntryCache.size())
        {
            // Make sure the options contain the field values, event though they not necessarily 
            // is supplied as possible options by the application. This is a convenience making sure
            // the actual data in the pdmObject is shown correctly in the UI, and to prevent accidental 
            // changes of the field values

            // Find the field value(s) in the list if present

            QVariant convertedFieldValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);

            std::vector<unsigned int> foundIndexes;
            bool foundAllFieldValues = PdmOptionItemInfo::findValues(m_optionEntryCache, convertedFieldValue, foundIndexes);
            
            // If not all are found, we have to add the missing to the list, to be able to show it

            if (!foundAllFieldValues)
            {
                if (convertedFieldValue.type() != QVariant::List)  // Single value field
                {
                    m_optionEntryCache.push_front(PdmOptionItemInfo(convertedFieldValue.toString(), convertedFieldValue, true, QIcon()));
                }
                else // The field value is a list of values 
                {
                    QList<QVariant> valuesSelectedInField = convertedFieldValue.toList();
                    for (int i= 0 ; i < valuesSelectedInField.size(); ++i)
                    {
                        bool isFound = false;
                        for (unsigned int opIdx = 0; opIdx < static_cast<unsigned int>(m_optionEntryCache.size()); ++opIdx)
                        {
                            if (valuesSelectedInField[i] == m_optionEntryCache[opIdx].value) isFound = true;
                        }

                        if (!isFound)
                        {
                            m_optionEntryCache.push_front(PdmOptionItemInfo(valuesSelectedInField[i].toString(), valuesSelectedInField[i], true, QIcon()));
                        }
                    }
                }
            }

            return m_optionEntryCache;
        }
    }

    // If we have no options, use the options defined by the type. Normally only caf::AppEnum type

    return PdmFieldTypeSpecialization<DataType>::valueOptions(useOptionsOnly, m_fieldValue);
}

//--------------------------------------------------------------------------------------------------
/// Extracts a QVariant representation of the data in the field to be used in the UI. 
/// Note that for fields with a none empty valueOptions list the returned QVariant contains the 
/// indexes to the selected options rather than the actual values, if they can be found.
/// If we cant find them, the method asserts, forcing the valueOptions to always contain the field values
//--------------------------------------------------------------------------------------------------
template<typename DataType >
QVariant caf::PdmField<DataType>::uiValue() const
{
    if (m_optionEntryCache.size())
    {
        QVariant convertedFieldValue = PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);
        std::vector<unsigned int> indexes;
        PdmOptionItemInfo::findValues(m_optionEntryCache, convertedFieldValue, indexes);
        if (convertedFieldValue.type() == QVariant::List)
        {
            if (indexes.size() == static_cast<size_t>(convertedFieldValue.toList().size()))
            {
                QList<QVariant> returnList;
                for(size_t i = 0; i < indexes.size(); ++i)
                {
                    returnList.push_back(QVariant(indexes[i]));
                }
                return QVariant(returnList);
            }
            assert(false); // Did not find all the field values among the options available.
        }
        else
        {
            if (indexes.size() == 1) return QVariant(indexes.front());
        }

        assert(false);
        return convertedFieldValue;
    }
    else
    {
        return PdmFieldTypeSpecialization<DataType>::convert(m_fieldValue);
    }
    
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
    : PdmFieldHandle()
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
void PdmPointersField<DataType*>::insert(size_t indexAfter, const std::vector<PdmPointer<DataType> >& objects)
{
    m_pointers.insert(m_pointers.begin()+indexAfter, objects.begin(), objects.end());

    typename std::vector< PdmPointer< DataType > >::iterator it;
    for (it = m_pointers.begin()+indexAfter; it != m_pointers.end(); ++it)
    {
        if (!it->isNull()) 
        {
            (*it)->addParentField(this);
        }
    }
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
void PdmPointersField<DataType*>::deleteAllChildObjects()
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
void PdmPointersField<DataType*>::removeChildObject(PdmObject* object)
{
    DataType* pointer = dynamic_cast<DataType*>(object);

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
        this->push_back(currentObject);

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
