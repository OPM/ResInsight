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
#include "RigWbsParameter.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <set>

class RimGeoMechCase;
class RimWellPath;

class RimWellBoreStabilityPlot : public RimWellLogPlot
{
    CAF_PDM_HEADER_INIT;

    using ParameterSource     = RigGeoMechWellLogExtractor::WbsParameterSource;
    using ParameterSourceEnum = RigGeoMechWellLogExtractor::WbsParameterSourceEnum;

public:
    RimWellBoreStabilityPlot();

    void applyWbsParametersToExtractor( RigGeoMechWellLogExtractor* extractor );

    RimWellBoreStabilityPlot::ParameterSource parameterSource( const RigWbsParameter& parameter ) const;
    double                                    userDefinedValue( const RigWbsParameter& parameter ) const;

    void setParameterSource( const RigWbsParameter& parameter, RimWellBoreStabilityPlot::ParameterSource );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

    void assignValidSource( caf::PdmField<ParameterSourceEnum>* parameterSourceField,
                            const std::vector<ParameterSource>& validSources );
    void onLoadDataAndUpdate() override;

    bool hasLasFileWithChannel( const QString& channel ) const;
    bool hasElementPropertyEntry( const RigFemResultAddress& resAddr ) const;

    caf::PdmField<ParameterSourceEnum>* sourceField( const RigWbsParameter& parameter ) const;

    std::vector<ParameterSource> supportedSources( const RigWbsParameter& parameter ) const;

private:
    caf::PdmField<ParameterSourceEnum> m_porePressureSource;
    caf::PdmField<ParameterSourceEnum> m_porePressureShaleSource;
    caf::PdmField<ParameterSourceEnum> m_poissonRatioSource;
    caf::PdmField<ParameterSourceEnum> m_ucsSource;
    caf::PdmField<ParameterSourceEnum> m_OBG0Source;
    caf::PdmField<ParameterSourceEnum> m_DFSource;
    caf::PdmField<ParameterSourceEnum> m_K0FGSource;
    caf::PdmField<ParameterSourceEnum> m_K0SHSource;
    caf::PdmField<ParameterSourceEnum> m_FGShaleSource;

    caf::PdmField<double> m_userDefinedPPShale;
    caf::PdmField<double> m_userDefinedPoissionRatio;
    caf::PdmField<double> m_userDefinedUcs;
    caf::PdmField<double> m_userDefinedDF;
    caf::PdmField<double> m_userDefinedK0FG;
    caf::PdmField<double> m_userDefinedK0SH;
    caf::PdmField<double> m_FGShaleMultiplier;

    std::map<RigWbsParameter, caf::PdmField<ParameterSourceEnum>*> m_parameterSourceFields;
    std::map<RigWbsParameter, caf::PdmField<double>*>              m_userDefinedValueFields;
};
