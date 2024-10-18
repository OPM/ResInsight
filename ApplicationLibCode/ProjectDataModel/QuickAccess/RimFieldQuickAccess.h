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

#include "RimNamedObject.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class RimFieldReference;

//==================================================================================================
///
///
//==================================================================================================
class RimFieldQuickAccess : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFieldQuickAccess();

    void                 setField( caf::PdmFieldHandle* field );
    caf::PdmFieldHandle* field() const;

    bool markedForRemoval() const;

    void onSel();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void onSelectObjectButton();
    void onRemoveObjectButton();

private:
    caf::PdmChildField<RimFieldReference*> m_fieldReference;

    caf::PdmProxyValueField<bool> m_selectObjectButton;
    caf::PdmProxyValueField<bool> m_removeItemButton;

    bool m_markedForRemoval;
};
