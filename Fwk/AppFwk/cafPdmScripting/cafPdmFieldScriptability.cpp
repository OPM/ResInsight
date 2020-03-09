//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cafPdmFieldScriptability.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmPythonGenerator.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldScriptability::PdmFieldScriptability( caf::PdmFieldHandle* owner, const QString& scriptFieldName, bool giveOwnership )
{
    m_IOWriteable     = true;
    m_owner           = owner;
    m_scriptFieldName = scriptFieldName;
    owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldScriptability::~PdmFieldScriptability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmFieldScriptability::scriptFieldName() const
{
    return m_scriptFieldName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldScriptability::isIOWriteable() const
{
    return m_IOWriteable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptability::setIOWriteable( bool writeable )
{
    m_IOWriteable = writeable;
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation that doesn't offer any script IO for the field
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptability::readFromField( QTextStream& outputStream,
                                           bool         quoteStrings /*= true*/,
                                           bool         quoteNonBuiltins /*= false*/ ) const
{
}

//--------------------------------------------------------------------------------------------------
/// Empty default implementation that doesn't offer any script IO for the field
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptability::writeToField( QTextStream&              inputStream,
                                          caf::PdmObjectFactory*    objectFactory,
                                          caf::PdmScriptIOMessages* errorMessageContainer,
                                          bool                      stringsAreQuoted /*= true*/,
                                          caf::PdmObjectHandle*     existingObjectsRoot /*= nullptr*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptability::addToField( caf::PdmFieldHandle* field, const QString& fieldName )
{
    if ( field->template capability<PdmFieldScriptability>() == nullptr )
    {
        new PdmFieldScriptability( field, fieldName, true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmFieldScriptability::helpString( const QString& existingTooltip, const QString& keyword )
{
    QString snake_case = caf::PdmPythonGenerator::camelToSnakeCase( keyword );

    QString helpString = QString( "Available through python/rips as the attribute '%1'" ).arg( snake_case );

    if ( !existingTooltip.isEmpty() ) return existingTooltip + "\n\n" + helpString;
    return helpString;
}
