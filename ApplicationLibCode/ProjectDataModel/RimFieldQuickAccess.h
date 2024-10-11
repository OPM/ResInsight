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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimFieldReference;

//==================================================================================================
///
/// This class is used to store a reference to a field in a PdmObject, and is similar to caf::PdmPtrField<caf::PdmObjectHandle*> that is
/// used for a non-owning reference to an object. Consider creating a caf::PdmPtrField<caf::PdmFieldHandle*> instead of this class.
///
/// Investigate if PdmFieldCapability::attributes can be used to store the field name for a caf::PdmPtrField<caf::PdmObjectHandle*>
///
//==================================================================================================
class RimFieldQuickAccess : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFieldQuickAccess();

    void                 setField( caf::PdmFieldHandle* field );
    caf::PdmFieldHandle* field() const;

    bool toBeDeleted() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    caf::PdmChildField<RimFieldReference*> m_fieldReference;

    caf::PdmField<bool> m_selectObjectButton;
    caf::PdmField<bool> m_removeObjectButton;
    bool                m_toBeDeleted;
};
