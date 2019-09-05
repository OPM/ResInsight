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

#include "RimWellLogPlot.h"

#include "RigGeoMechWellLogExtractor.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <set>

class RimGeoMechCase;
class RimWellPath;

class RimWellBoreStabilityPlot : public RimWellLogPlot
{
    CAF_PDM_HEADER_INIT;

public:
    typedef caf::AppEnum<RigGeoMechWellLogExtractor::WbsParameterSource> ParameterSourceEnum;

public:
    RimWellBoreStabilityPlot();

    RigGeoMechWellLogExtractor::WbsParameterSource porePressureSource() const;
    RigGeoMechWellLogExtractor::WbsParameterSource poissonRatioSource() const;
    RigGeoMechWellLogExtractor::WbsParameterSource ucsSource() const;

    double userDefinedPoissonRatio() const;
    double userDefinedUcs() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

private:
    caf::PdmField<ParameterSourceEnum> m_porePressureSource;
    caf::PdmField<ParameterSourceEnum> m_poissonRatioSource;
    caf::PdmField<ParameterSourceEnum> m_ucsSource;

    caf::PdmField<double> m_userDefinedPoissionRatio;
    caf::PdmField<double> m_userDefinedUcs;
};
