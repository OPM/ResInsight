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


#include "cafPdmSettings.h"
#include "cafPdmField.h"

#include <assert.h>


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Settings::readFieldsFromApplicationStore(caf::PdmObject* object)
{
    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(), 
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    QString prefix = object->classKeyword();
    if (!prefix.isEmpty())
    {
        prefix += "/";
    }

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);
    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];

        QString keywordWithPrefix = prefix + fieldHandle->keyword();
        if (settings.contains(keywordWithPrefix))
        {
            QVariant val = settings.value(keywordWithPrefix);
            fieldHandle->setValueFromUi(val);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Settings::writeFieldsToApplicationStore(caf::PdmObject* object)
{
    assert(object);

    // Qt doc :
    //
    // Constructs a QSettings object for accessing settings of the application and organization
    // set previously with a call to QCoreApplication::setOrganizationName(), 
    // QCoreApplication::setOrganizationDomain(), and QCoreApplication::setApplicationName().
    QSettings settings;

    QString prefix = object->classKeyword();
    if (!prefix.isEmpty())
    {
        prefix += "/";
    }

    std::vector<caf::PdmFieldHandle*> fields;
    object->fields(fields);

    size_t i;
    for (i = 0; i < fields.size(); i++)
    {
        caf::PdmFieldHandle* fieldHandle = fields[i];

        QString keywordWithPrefix = prefix + fieldHandle->keyword();
        settings.setValue(keywordWithPrefix, fieldHandle->uiValue());
    }
}


} // namespace caf
