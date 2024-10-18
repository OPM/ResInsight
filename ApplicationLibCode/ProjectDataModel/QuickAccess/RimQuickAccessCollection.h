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
#include "cafPdmPtrArrayField.h"

class RimFieldQuickAccess;
class RimFieldQuickAccessGroup;
class RimFieldReference;

//==================================================================================================
///
///
//==================================================================================================
class RimQuickAccessCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimQuickAccessCollection();

    static RimQuickAccessCollection* instance();

    void addQuickAccessFieldsRecursively( caf::PdmObjectHandle* object );
    void addQuickAccessFields( caf::PdmObjectHandle* object );

    void addQuickAccessField( const RimFieldReference& fieldReference );

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    RimFieldQuickAccessGroup* findOrCreateGroup( caf::PdmObjectHandle* object, const QString& groupName );

    void deleteMarkedObjects();

    static void    updateGroupName( RimFieldQuickAccessGroup* group );
    static QString defaultGroupName();

private:
    caf::PdmChildArrayField<RimFieldQuickAccessGroup*> m_fieldQuickAccesGroups;
};
