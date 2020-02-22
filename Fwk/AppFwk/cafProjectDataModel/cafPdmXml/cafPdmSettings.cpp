//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015 Ceetron Solutions AS
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


#include "cafPdmSettings.h"

#include "cafPdmField.h"
#include "cafPdmXmlObjectHandle.h"


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmSettings::readFieldsFromApplicationStore(caf::PdmObjectHandle* object, const QString context)
{
    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(), 
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;
    std::vector<caf::PdmFieldHandle*> fields;

    object->fields(fields);
    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];

        std::vector<caf::PdmObjectHandle*> children;
        fieldHandle->childObjects(&children);
        for (size_t childIdx = 0; childIdx < children.size(); childIdx++)
        {
            caf::PdmObjectHandle* child = children[childIdx];
            caf::PdmXmlObjectHandle* xmlObjHandle = xmlObj(child);

            QString subContext = context + xmlObjHandle->classKeyword() + "/";
            readFieldsFromApplicationStore(child, subContext);
        }

        if (children.size() == 0)
        {
            QString key = context + fieldHandle->keyword();
            if (settings.contains(key))
            {
                QVariant val = settings.value(key);

                caf::PdmValueField* valueField = dynamic_cast<caf::PdmValueField*>(fieldHandle);
                CAF_ASSERT(valueField);
                valueField->setFromQVariant(val);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmSettings::writeFieldsToApplicationStore(const caf::PdmObjectHandle* object, const QString context)
{
    CAF_ASSERT(object);

    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(), 
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);

    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];

        std::vector<caf::PdmObjectHandle*> children;
        fieldHandle->childObjects(&children);
        for (size_t childIdx = 0; childIdx < children.size(); childIdx++)
        {
            caf::PdmObjectHandle* child = children[childIdx];
            QString subContext;
            if (context.isEmpty())
            {
                caf::PdmXmlObjectHandle* xmlObjHandle = xmlObj(child);

                subContext = xmlObjHandle->classKeyword() + "/";
            }

            writeFieldsToApplicationStore(child, subContext);
        }

        if (children.size() == 0)
        {
            caf::PdmValueField* valueField = dynamic_cast<caf::PdmValueField*>(fieldHandle);
            CAF_ASSERT(valueField);
            settings.setValue(context + fieldHandle->keyword(), valueField->toQVariant());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmSettings::readValueFieldsFromApplicationStore(caf::PdmObjectHandle* object, const QString folderName /*= ""*/)
{
    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(), 
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    if ( folderName != "" )
    {
        settings.beginGroup(folderName);
    }

    std::vector<caf::PdmFieldHandle*> fields;

    object->fields(fields);
    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];
        caf::PdmValueField* valueField = dynamic_cast<caf::PdmValueField*>(fieldHandle);

        if (valueField)
        {
            QString key = fieldHandle->keyword();
            if ( settings.contains(key) )
            {
                QVariant val = settings.value(key);

                QString fieldText = "<Element>" + val.toString() + "</Element>";
                QXmlStreamReader reader(fieldText);

                // Make stream point to the text data for the field
                reader.readNext(); // StartDocument 
                reader.readNext(); // StartElement
                reader.readNext(); // Characters
                fieldHandle->xmlCapability()->readFieldData(reader, nullptr);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmSettings::writeValueFieldsToApplicationStore(const caf::PdmObjectHandle* object, const QString folderName /*= ""*/)
{
    CAF_ASSERT(object);

    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(), 
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    if ( folderName != "" )
    {
        settings.beginGroup(folderName);
    }

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);

    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];
        caf::PdmValueField* valueField = dynamic_cast<caf::PdmValueField*>(fieldHandle);
        if (valueField)
        {
            QString fieldText;
            QXmlStreamWriter writer(&fieldText);

            fieldHandle->xmlCapability()->writeFieldData(writer);
            settings.setValue(fieldHandle->keyword(), fieldText);
        }
    }
}

} // namespace caf
