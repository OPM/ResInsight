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

#include "RiaDefines.h"

#include "RimFishbonesDefines.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h"

#include "cvfColor3.h"

class RimFishbones;

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

    void          appendFishbonesSubs( RimFishbones* subs );
    RimFishbones* appendFishbonesSubsAtLocations( const std::vector<double>& subLocations, RimFishbonesDefines::DrillingType drillingType );
    void          setFixedStartMD( double startMD );
    void          setFixedEndMD( double endMD );

    void setUnitSystemSpecificDefaults();
    void computeStartAndEndLocation();

    bool                       hasFishbones() const;
    std::vector<RimFishbones*> activeFishbonesSubs() const;
    std::vector<RimFishbones*> allFishbonesSubs() const;

    double startMD() const;
    double endMD() const;

    double mainBoreSkinFactor() const;
    double mainBoreDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    cvf::Color3f nextFishbonesColor() const;
    double       calculateStartMD() const;
    double       calculateEndMD() const;

private:
    caf::PdmChildArrayField<RimFishbones*> m_fishbones;

    caf::PdmField<caf::AppEnum<RimFishbonesDefines::ValueSource>> m_startMDAuto;
    caf::PdmField<caf::AppEnum<RimFishbonesDefines::ValueSource>> m_endMDAuto;

    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;

    caf::PdmField<double> m_skinFactor;
    caf::PdmField<double> m_mainBoreDiameter;
};
