/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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
#include "cafPdmChildField.h"
#include "cafPdmObject.h"

#include <vector>

class RimFaultDistanceCollection;
class RimWellTargetMapping;

namespace caf
{
class CmdFeatureMenuBuilder;
}

//==================================================================================================
/// Case-level "Data Analytics" folder. Owns the analytics objects (fault distance results and
/// well target mappings) and exposes a right-click menu for creating new ones.
//==================================================================================================
class RimDataAnalyticsCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataAnalyticsCollection();

    RimFaultDistanceCollection* faultDistanceCollection() const;

    void                               addWellTargetMapping( RimWellTargetMapping* wellTargetMapping );
    std::vector<RimWellTargetMapping*> wellTargetMappings() const;

    bool isEmpty() const;

private:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    caf::PdmChildField<RimFaultDistanceCollection*> m_faultDistanceCollection;
    caf::PdmChildArrayField<RimWellTargetMapping*>  m_wellTargetMappings;
};
