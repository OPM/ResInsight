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

#include "RimCheckableNamedObject.h"
#include "RimMswCompletionParameters.h"

#include "RiaEclipseUnitTools.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h"    

#include "cvfColor3.h"

class RimFishbonesMultipleSubs;
class RimFishboneWellPathCollection;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimFishbonesCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFishbonesCollection();

    RimFishboneWellPathCollection*    wellPathCollection() const;
    void                              appendFishbonesSubs(RimFishbonesMultipleSubs* subs);
    const RimMswCompletionParameters* mswParameters() const;

    std::vector<RimFishbonesMultipleSubs*> activeFishbonesSubs() const;
    std::vector<RimFishbonesMultipleSubs*> fishbonesSubs() const;

    void         recalculateStartMD();
    double       startMD() const;
    double       mainBoreSkinFactor() const { return m_skinFactor; }
    double       mainBoreDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const;    
    void         setUnitSystemSpecificDefaults();

protected:
    void        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void        initAfterRead() override;

private:
    cvf::Color3f nextFishbonesColor() const;

private:
    caf::PdmChildArrayField<RimFishbonesMultipleSubs*>  m_fishbonesSubs;
    caf::PdmChildField<RimFishboneWellPathCollection*>  m_wellPathCollection;

    caf::PdmField<double>                               m_startMD;
    caf::PdmField<double>                               m_skinFactor;
    caf::PdmField<double>                               m_mainBoreDiameter;
    caf::PdmChildField<RimMswCompletionParameters*>     m_mswParameters;
    bool                                                manuallyModifiedStartMD;

    caf::PdmField<double>                               m_linerDiameter_OBSOLETE;
    caf::PdmField<double>                               m_roughnessFactor_OBSOLETE;

    caf::PdmField<RimMswCompletionParameters::PressureDropEnum>   m_pressureDrop_OBSOLETE;
    caf::PdmField<RimMswCompletionParameters::LengthAndDepthEnum> m_lengthAndDepth_OBSOLETE;
};
