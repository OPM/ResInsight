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
#pragma once

#include "cafPdmFieldCapability.h"
#include <QString>

class QTextStream;

namespace caf
{
class PdmFieldHandle;
class PdmObjectFactory;
class PdmObjectHandle;
class PdmScriptIOMessages;

class PdmAbstractFieldScriptingCapability : public PdmFieldCapability
{
public:
    PdmAbstractFieldScriptingCapability( caf::PdmFieldHandle* owner, const QString& scriptFieldName, bool giveOwnership );
    virtual ~PdmAbstractFieldScriptingCapability();

    const QString scriptFieldName() const;

    bool isIOWriteable() const;
    void setIOWriteable( bool writeable );

    virtual void
                 readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const = 0;
    virtual void writeToField( QTextStream&              inputStream,
                               caf::PdmObjectFactory*    objectFactory,
                               caf::PdmScriptIOMessages* errorMessageContainer,
                               bool                      stringsAreQuoted    = true,
                               caf::PdmObjectHandle*     existingObjectsRoot = nullptr ) = 0;

    static QString helpString( const QString& existingTooltip, const QString& keyword );

protected:
    PdmFieldHandle* m_owner;
    QString         m_scriptFieldName;
    bool            m_IOWriteable;
};

} // namespace caf
