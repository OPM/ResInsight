/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimFieldReference;

//==================================================================================================
///
///
//==================================================================================================
class RimPinnedFieldCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPinnedFieldCollection();

    static RimPinnedFieldCollection* instance();

    void addField( caf::PdmFieldHandle* field );
    void removeField( caf::PdmFieldHandle* field );

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmChildArrayField<RimFieldReference*> m_fieldReferences;
};
