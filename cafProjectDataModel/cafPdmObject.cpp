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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <iostream>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// Reads all the fields into this PdmObject
/// Assumes xmlStream points to the start element token of the containing object.
/// ( and not first token of object content)
/// This makes attribute based field storage possible.
/// Leaves the xmlStream pointing to the EndElement corresponding to the start element.
//--------------------------------------------------------------------------------------------------
void PdmObject::readFields (QXmlStreamReader& xmlStream )
{
    if (!xmlStream.isStartElement())
    {
        // Error
        return ;
    }
/*  
   Attributes will not be used ...  

   QXmlStreamAttributes attribs = xmlStream.attributes();
   int i;
   for (i = 0; i < attribs.size(); ++i)
   {
       QString name = attribs[i].name().toString();

       PdmFieldBase* field = findField(name);

       if (field)
       {
           //field->readFieldData(attribs[i].value().toString());
       }
   }
   */

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
               PdmFieldHandle* currentField = findField(name);
               if (currentField)
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
                   std::cout << "Warning: Could not find a field with name " << name.toLatin1().data() << " in the current object : " << classKeyword().toLatin1().data() << std::endl;
                   xmlStream.skipCurrentElement();
               }
               break;
           }
           break;
       case QXmlStreamReader::EndElement:
           {
               // End of object.
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
        PdmFieldHandle* obj = *it;
        QString keyword = obj->keyword();
        xmlStream.writeStartElement("", keyword);
        obj->writeFieldData(xmlStream);
        xmlStream.writeEndElement();
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

void PdmObject::addFieldNoDefault(PdmFieldHandle* field, const QString& keyword, PdmUiItemInfo * fieldDescription)
{
    field->setUiItemInfo(fieldDescription);
    field->setKeyword(keyword);
    field->setOwnerObject(this);

    assert(findField(keyword) == NULL);
    m_fields.push_back(field);
}

} //End of namespace caf
