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


#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <iostream>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "cafPdmObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmUiTreeOrdering.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// Reads all the fields into this PdmObject
/// Assumes xmlStream points to the start element token of the PdmObject for which to read fields.
/// ( and not first token of object content)
/// This makes attribute based field storage possible.
/// Leaves the xmlStream pointing to the EndElement of the PdmObject.
//--------------------------------------------------------------------------------------------------
void PdmObject::readFields (QXmlStreamReader& xmlStream )
{
   bool isObjectFinished = false;
   QXmlStreamReader::TokenType type;
   while(!isObjectFinished)
   {
       type = xmlStream.readNext();

       switch (type)
       {
       case QXmlStreamReader::StartElement:
           {
               QString name = xmlStream.name().toString();
               if (name == QString("SimpleObjPtrField"))
               {
                   int a;
                   a = 2 + 7;
               }
               PdmFieldHandle* currentField = findField(name);
               if (currentField)
               {
                   if (currentField->isIOReadable())
                   {
                       // readFieldData assumes that the xmlStream points to first token of field content.
                       // After reading, the xmlStream is supposed to point to the first token after the field content.
                       // (typically an "endElement")
                       QXmlStreamReader::TokenType tt;
                       tt = xmlStream.readNext();
                       currentField->readFieldData( xmlStream );
                   }
                   else
                   {
                       xmlStream.skipCurrentElement();
                   }
               }
               else
               {
                   std::cout << "Line "<< xmlStream.lineNumber() << ": Warning: Could not find a field with name " << name.toLatin1().data() << " in the current object : " << classKeyword().toLatin1().data() << std::endl;
                   xmlStream.skipCurrentElement();
               }
               break;
           }
           break;
       case QXmlStreamReader::EndElement:
           {
               // End of object.
               QString name = xmlStream.name().toString(); // For debugging
               isObjectFinished = true;
           }
           break;
       case QXmlStreamReader::EndDocument:
           {
               // End of object.
               isObjectFinished = true;
           }
           break;
       default:
           {
               // Just read on
               // Todo: Error handling   
           }
           break;
       }
   }

}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::writeFields(QXmlStreamWriter& xmlStream)
{
    std::vector<PdmFieldHandle*>::iterator it;
    for (it = m_fields.begin(); it != m_fields.end(); ++it)
    {
        PdmFieldHandle* field = *it;
        if (field->isIOWritable())
        {
            QString keyword = field->keyword();
            assert(PdmObject::isValidXmlElementName(keyword));

            xmlStream.writeStartElement("", keyword);
            field->writeFieldData(xmlStream);
            xmlStream.writeEndElement();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle*  PdmObject::findField(const QString& keyword)
{
    std::vector<PdmFieldHandle*>::iterator it;
    for (it = m_fields.begin(); it != m_fields.end(); it++)
    {
        PdmFieldHandle* obj = *it;
        if (obj->keyword() == keyword)
        {
            return obj;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObject::~PdmObject()
{
    // Set all guarded pointers pointing to this to NULL

    std::set<PdmObject**>::iterator it;
    for (it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end() ; ++it)
    {
        (**it) = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::fields(std::vector<PdmFieldHandle*>& fields) const
{
    fields = m_fields;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::addParentField(PdmFieldHandle* parentField)
{
    if (parentField != NULL) m_parentFields.insert(parentField);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::removeParentField(PdmFieldHandle* parentField)
{
   if (parentField != NULL) m_parentFields.erase(parentField);
}

//--------------------------------------------------------------------------------------------------
/// Appends pointers to all the PdmFields/PdmPointerFields containing a pointer to this object.
/// As the PdmPointersField can hold several pointers to the same object, the returned vector can 
/// contain multiple pointers to the same field. 
//--------------------------------------------------------------------------------------------------
void PdmObject::parentFields(std::vector<PdmFieldHandle*>& parentFields) const
{
    std::multiset<PdmFieldHandle*>::const_iterator it;

    for (it = m_parentFields.begin(); it != m_parentFields.end(); ++it)
    {
        parentFields.push_back(*it);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::parentObjects(std::vector<PdmObject*>& objects) const
{
    std::vector<caf::PdmFieldHandle*> parentFields;
    this->parentFields(parentFields);
    size_t i;
    for (i = 0; i < parentFields.size(); i++)
    {
        objects.push_back(parentFields[i]->ownerObject());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::removeFromParentFields()
{
    std::vector<caf::PdmFieldHandle*> parentFields;
    this->parentFields(parentFields);
    size_t i;
    for (i = 0; i < parentFields.size(); i++)
    {
        parentFields[i]->removeChildObject(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::addFieldNoDefault(PdmFieldHandle* field, const QString& keyword, PdmUiItemInfo * fieldDescription)
{
    field->setUiItemInfo(fieldDescription);
    field->setKeyword(keyword);
    field->setOwnerObject(this);

    assert(findField(keyword) == NULL);
    m_fields.push_back(field);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::uiOrdering(QString uiConfigName, PdmUiOrdering& uiOrdering) 
{
    this->defineUiOrdering(uiConfigName, uiOrdering);
    if (!uiOrdering.forgetRemainingFields())
    {
        // Add the remaining Fields To UiConfig

        for (size_t i = 0; i < m_fields.size(); ++i)
        {
            if (!uiOrdering.contains(m_fields[i]))
            {
                uiOrdering.add( m_fields[i]);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::editorAttribute(const PdmFieldHandle* field, QString uiConfigName, PdmUiEditorAttribute * attribute)
{
    this->defineEditorAttribute(field, uiConfigName, attribute);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::objectEditorAttribute(QString uiConfigName, PdmUiEditorAttribute* attribute)
{
    this->defineObjectEditorAttribute(uiConfigName, attribute);
}


//--------------------------------------------------------------------------------------------------
/// Check if a string is a valid Xml element name
//
/// http://www.w3schools.com/xml/xml_elements.asp
///
/// XML elements must follow these naming rules:
///   Names can contain letters, numbers, and other characters
///   Names cannot start with a number or punctuation character
///   Names cannot start with the letters xml (or XML, or Xml, etc)
///   Names cannot contain spaces
//--------------------------------------------------------------------------------------------------
bool PdmObject::isValidXmlElementName(const QString& name)
{
    if (name.size() > 0)
    {
        QChar firstChar = name[0];
        if (firstChar.isDigit() || firstChar == '.')
        {
            return false;
        }
    }

    if (name.size() >= 3)
    {
        if (name.left(3).compare("xml", Qt::CaseInsensitive) == 0)
        {
            return false;
        }
    }

    if (name.contains(' '))
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObject* PdmObject::deepCopy()
{
    QString encodedXml;
    
    {
        PdmObjectGroup typedObjectGroup;
        typedObjectGroup.addObject(this);

        QXmlStreamWriter xmlStream(&encodedXml);
        xmlStream.setAutoFormatting(true);

        typedObjectGroup.writeFields(xmlStream);
    }

    PdmObject* pdmCopy = NULL;
    {
        // Read back XML into object group, factory methods will be called that will create new objects
        PdmObjectGroup destinationObjectGroup;
        QXmlStreamReader xmlStream(encodedXml);
        destinationObjectGroup.readFields(xmlStream);

        if (destinationObjectGroup.objects.size() == 1)
        {   
            pdmCopy = destinationObjectGroup.objects[0];
        }
    }

    return pdmCopy;

    /*
   PdmObject* cloneRoot = PdmObjectFactory::instance()->create(this->classKeyword());



    QString encodedXml;
    {
        QXmlStreamWriter xmlStream(&encodedXml);
        xmlStream.setAutoFormatting(true);

        this->writeFields(xmlStream);
    }

    QXmlStreamReader xmlStream(encodedXml);
    cloneRoot->readFields(xmlStream);

    return cloneRoot;
    */
}

//--------------------------------------------------------------------------------------------------
/// This method is to be used to create a tree-representation of the object hierarchy starting at this
/// object. The caller is responsible to delete the returned PdmUiTreeOrdering
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* PdmObject::uiTreeOrdering(QString uiConfigName /*= ""*/)
{
    PdmUiTreeOrdering* uiTreeOrdering = new PdmUiTreeOrdering(NULL, -1, this);

    this->defineUiTreeOrdering(*uiTreeOrdering, uiConfigName);

    if (!uiTreeOrdering->forgetRemainingFields())
    {
        // Add the remaining Fields To UiConfig

        for (size_t fIdx = 0; fIdx < m_fields.size(); ++fIdx)
        {
            if (m_fields[fIdx]->hasChildObjects() && !uiTreeOrdering->containsField(m_fields[fIdx]))
            {
                if (m_fields[fIdx]->isUiHidden() && !m_fields[fIdx]->isUiChildrenHidden())
                {
                    std::vector<PdmObject*> children;
                    m_fields[fIdx]->childObjects(&children);

                    for (size_t cIdx = 0; cIdx < children.size(); cIdx++)
                    {
                        if (!uiTreeOrdering->containsObject(children[cIdx]))
                        {
                            uiTreeOrdering->add(children[cIdx]);
                        }
                    }
                }
                else if ( !m_fields[fIdx]->isUiHidden())
                {
                    uiTreeOrdering->add(m_fields[fIdx]);
                }
            }
        }
    }

    addUiTreeChildren(uiTreeOrdering, uiConfigName);
    return uiTreeOrdering;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObject::addUiTreeChildren(PdmUiTreeOrdering* root, QString uiConfigName /*= "" */)
{
    if (!root) return;

    if ( root->childCount() == 0) // This means that no one has tried to expand it.
    {
        if (!root->ignoreSubTree() && root->dataObject()) 
        {

            if (root->m_field && !root->m_field->isUiChildrenHidden(uiConfigName))
            {
                std::vector<PdmObject*> fieldsChildObjects;
                root->m_field->childObjects(&fieldsChildObjects);
                for (size_t cIdx = 0; cIdx < fieldsChildObjects.size(); ++cIdx)
                {
                    root->add(fieldsChildObjects[cIdx]);
                }
            }
            else
            {
                root->object()->defineUiTreeOrdering(*root, uiConfigName);
            }
        }
    }

    for (int cIdx = 0; cIdx < root->childCount(); ++cIdx)
    {
        PdmUiTreeOrdering* child = dynamic_cast<PdmUiTreeOrdering*>(root->child(cIdx));
        if (!child->ignoreSubTree())
        {
            addUiTreeChildren(child);
        }
    }
}


} //End of namespace caf
