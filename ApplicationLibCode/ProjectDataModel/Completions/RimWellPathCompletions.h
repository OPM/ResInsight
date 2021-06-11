/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimWellPathCompletionSettings.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "gsl/gsl"

class RimFishbonesCollection;
class RimPerforationCollection;
class RimStimPlanModelCollection;
class RimWellPathComponentInterface;
class RimWellPathFracture;
class RimWellPathFractureCollection;
class RimWellPathValve;

//==================================================================================================
///
///
//==================================================================================================
class RimWellPathCompletions : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathCompletions();

    RimFishbonesCollection*        fishbonesCollection() const;
    RimPerforationCollection*      perforationCollection() const;
    RimWellPathFractureCollection* fractureCollection() const;
    RimStimPlanModelCollection*    stimPlanModelCollection() const;

    std::vector<const RimWellPathComponentInterface*> allCompletions() const;
    bool                                              hasCompletions() const;
    void                                              setUnitSystemSpecificDefaults();

    std::vector<RimWellPathValve*>    valves() const;
    std::vector<RimWellPathFracture*> allFractures() const;
    std::vector<RimWellPathFracture*> activeFractures() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void initAfterRead() override;

private:
    void applyToSettings( gsl::not_null<RimWellPathCompletionSettings*> settings );

private:
    caf::PdmChildField<RimFishbonesCollection*>        m_fishbonesCollection;
    caf::PdmChildField<RimPerforationCollection*>      m_perforationCollection;
    caf::PdmChildField<RimWellPathFractureCollection*> m_fractureCollection;
    caf::PdmChildField<RimStimPlanModelCollection*>    m_stimPlanModelCollection;

private:
    /////////////////////
    // OBSOLETE FIELDS //
    /////////////////////
    caf::PdmField<QString> m_wellNameForExport_OBSOLETE;
    caf::PdmField<QString> m_wellGroupName_OBSOLETE;

    caf::PdmField<QString>                                                m_referenceDepth_OBSOLETE;
    caf::PdmField<RimWellPathCompletionSettings::WellTypeEnum>            m_preferredFluidPhase_OBSOLETE;
    caf::PdmField<QString>                                                m_drainageRadiusForPI_OBSOLETE;
    caf::PdmField<RimWellPathCompletionSettings::GasInflowEnum>           m_gasInflowEquation_OBSOLETE;
    caf::PdmField<RimWellPathCompletionSettings::AutomaticWellShutInEnum> m_automaticWellShutIn_OBSOLETE;
    caf::PdmField<bool>                                                   m_allowWellCrossFlow_OBSOLETE;
    caf::PdmField<int>                                                    m_wellBoreFluidPVTTable_OBSOLETE;
    caf::PdmField<RimWellPathCompletionSettings::HydrostaticDensityEnum>  m_hydrostaticDensity_OBSOLETE;
    caf::PdmField<int>                                                    m_fluidInPlaceRegion_OBSOLETE;
};
