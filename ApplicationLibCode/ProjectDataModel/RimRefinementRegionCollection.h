/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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
#include "cafVecIjk.h"

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <QString>

class RigRefinement;
class RimEclipseCase;
class RimRefinementRegion;

//==================================================================================================
///
/// Collection of RimRefinementRegion objects attached to a RimEclipseView. Combines selected
/// regions into a single RigRefinement for use in sector-model export.
///
//==================================================================================================
class RimRefinementRegionCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimRefinementRegionCollection();

    bool isActive() const;

    std::vector<RimRefinementRegion*> regions() const;
    std::vector<RimRefinementRegion*> activeRegions() const;

    RimRefinementRegion* addNewRegion( RimEclipseCase* eclipseCase );
    void                 addRegion( RimRefinementRegion* region );
    void                 removeRegion( RimRefinementRegion* region );

    // Combine the given regions into a single RigRefinement sized to the supplied sector
    // bounds (0-based, inclusive). Regions with disagreeing per-axis refinement on a shared
    // cell-index produce an error. Returns either the refinement or a user-readable error message.
    static std::variant<std::unique_ptr<RigRefinement>, QString>
        combineRefinements( const caf::VecIjk0&                            sectorMin,
                            const caf::VecIjk0&                            sectorMax,
                            const std::vector<RimRefinementRegion*>&       regions );

    caf::PdmFieldHandle* objectToggleField() override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<bool>                           m_isActive;
    caf::PdmChildArrayField<RimRefinementRegion*> m_regions;
};
