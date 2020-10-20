/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "cafPdmPtrField.h"

class RimIntersectionResultDefinition;

class RimIntersectionResultsDefinitionCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersectionResultsDefinitionCollection();
    ~RimIntersectionResultsDefinitionCollection() override;

    bool isActive() const;

    std::vector<RimIntersectionResultDefinition*> intersectionResultsDefinitions() const;

    void appendIntersectionResultDefinition( RimIntersectionResultDefinition* interResDef );
    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    caf::PdmFieldHandle* objectToggleField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    virtual void         initAfterRead() override;

private:
    caf::PdmField<bool>                                       m_isActive;
    caf::PdmChildArrayField<RimIntersectionResultDefinition*> m_intersectionResultsDefs;
};
