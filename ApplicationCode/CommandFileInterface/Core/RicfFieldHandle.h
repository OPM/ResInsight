/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "cafPdmFieldScriptability.h"

#include <QString>

namespace caf
{
class PdmObjectFactory;
class PdmFieldHandle;
} // namespace caf

class RicfMessages;

class QTextStream;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfFieldHandle : public caf::PdmFieldScriptability
{
public:
    RicfFieldHandle( caf::PdmFieldHandle* owner, const QString& scriptFieldName, bool giveOwnership );
    ~RicfFieldHandle() override;

    virtual void readFieldData( QTextStream&           inputStream,
                                caf::PdmObjectFactory* objectFactory,
                                RicfMessages*          errorMessageContainer,
                                bool                   stringsAreQuoted = true ) = 0;

private:
    caf::PdmFieldHandle* m_owner;
    QString              m_fieldName;
    bool                 m_IOWriteable;
};
