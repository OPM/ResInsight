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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimGridView;
class RimFieldQuickAccess;

//==================================================================================================
///
///
//==================================================================================================
class RimFieldQuickAccessGroup : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFieldQuickAccessGroup();

    RimGridView* ownerView() const;
    void         setOwnerView( RimGridView* owner );

    void addFields( const std::vector<caf::PdmFieldHandle*>& fields );
    void addField( caf::PdmFieldHandle* field );

    std::vector<RimFieldQuickAccess*> fieldQuickAccesses() const;
    caf::PdmObjectHandle*             groupOwner() const;

    void removeFieldQuickAccess( RimFieldQuickAccess* fieldQuickAccess );

private:
    void addFieldQuickAccess( RimFieldQuickAccess* fieldQuickAccess );
    bool findField( const caf::PdmFieldHandle* field ) const;

    bool isOwnerViewMatching( caf::PdmFieldHandle* field );

private:
    caf::PdmChildArrayField<RimFieldQuickAccess*> m_fieldQuickAccess;
    caf::PdmPtrField<RimGridView*>                m_ownerView;
};
