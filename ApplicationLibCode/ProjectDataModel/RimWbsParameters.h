/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigGeoMechWellLogExtractor.h"
#include "RigWbsParameter.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <map>

class RimGeoMechCase;
class RimWellPath;

class RimWbsParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using ParameterSource     = RigGeoMechWellLogExtractor::WbsParameterSource;
    using ParameterSourceEnum = RigGeoMechWellLogExtractor::WbsParameterSourceEnum;

public:
    RimWbsParameters();
    ~RimWbsParameters() override;

    RimWbsParameters( const RimWbsParameters& copyFrom );
    RimWbsParameters& operator=( const RimWbsParameters& copyFrom );

    void setGeoMechCase( RimGeoMechCase* geoMechCase );
    void setWellPath( RimWellPath* wellPath );
    void setTimeStep( int timeStep );

    void applyWbsParametersToExtractor( RigGeoMechWellLogExtractor* extractor );

    ParameterSource parameterSource( const RigWbsParameter& parameter ) const;
    double          userDefinedValue( const RigWbsParameter& parameter ) const;

    void setParameterSource( const RigWbsParameter& parameter, ParameterSource source );
    void setUserDefinedValue( const RigWbsParameter& parameter, double value );

    caf::PdmField<ParameterSourceEnum>* sourceField( const RigWbsParameter& parameter ) const;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;

    void loadDataAndUpdate();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    bool hasLasFileWithChannel( const QString& channel ) const;
    bool hasElementPropertyEntry( const RigFemResultAddress& resAddr ) const;

    void assignValidSource( caf::PdmField<ParameterSourceEnum>* parameterSourceField,
                            const std::vector<ParameterSource>& validSources );

    std::vector<ParameterSource> supportedSources( const RigWbsParameter& parameter ) const;

private:
    caf::PdmField<ParameterSourceEnum> m_porePressureSource;
    caf::PdmField<ParameterSourceEnum> m_porePressureNonReservoirSource;
    caf::PdmField<ParameterSourceEnum> m_poissonRatioSource;
    caf::PdmField<ParameterSourceEnum> m_ucsSource;
    caf::PdmField<ParameterSourceEnum> m_OBG0Source;
    caf::PdmField<ParameterSourceEnum> m_DFSource;
    caf::PdmField<ParameterSourceEnum> m_K0FGSource;
    caf::PdmField<ParameterSourceEnum> m_K0SHSource;
    caf::PdmField<ParameterSourceEnum> m_FGShaleSource;
    caf::PdmField<ParameterSourceEnum> m_waterDensitySource;

    caf::PdmField<double> m_userDefinedPPShale;
    caf::PdmField<double> m_userDefinedPoissionRatio;
    caf::PdmField<double> m_userDefinedUcs;
    caf::PdmField<double> m_userDefinedDF;
    caf::PdmField<double> m_userDefinedK0FG;
    caf::PdmField<double> m_userDefinedK0SH;
    caf::PdmField<double> m_FGShaleMultiplier;
    caf::PdmField<double> m_userDefinedDensity;

    caf::PdmPtrField<RimGeoMechCase*> m_geoMechCase;
    caf::PdmPtrField<RimWellPath*>    m_wellPath;
    caf::PdmField<int>                m_timeStep;

    std::map<RigWbsParameter, caf::PdmField<ParameterSourceEnum>*> m_parameterSourceFields;
    std::map<RigWbsParameter, caf::PdmField<double>*>              m_userDefinedValueFields;
};
