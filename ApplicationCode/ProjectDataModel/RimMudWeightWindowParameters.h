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

#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimGeoMechCase;

//==================================================================================================
///
///
//==================================================================================================
class RimMudWeightWindowParameters : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SourceType
    {
        FIXED = 0,
        PER_ELEMENT,
        GRID
    };

    enum class ParameterType
    {
        WELL_DEVIATION,
        WELL_AZIMUTH,
        UCS,
        POISSONS_RATIO,
        K0_FG,
        OBG0
    };

    enum class UpperLimitType
    {
        FG,
        SH_MIN
    };

    enum class LowerLimitType
    {
        PORE_PRESSURE,
        MAX_OF_PORE_PRESSURE_AND_SFG
    };

    enum class FractureGradientCalculationType
    {
        DERIVED_FROM_K0FG,
        PROPORTIONAL_TO_SH
    };

    enum class NonReservoirPorePressureType
    {
        HYDROSTATIC,
        PER_ELEMENT
    };

    RimMudWeightWindowParameters( void );

    SourceType wellDeviationType() const;
    double     wellDeviation() const;
    QString    wellDeviationAddress() const;

    SourceType wellAzimuthType() const;
    double     wellAzimuth() const;
    QString    wellAzimuthAddress() const;

    SourceType ucsType() const;
    double     ucs() const;
    QString    ucsAddress() const;

    SourceType poissonsRatioType() const;
    double     poissonsRatio() const;
    QString    poissonsRatioAddress() const;

    SourceType K0_FG_Type() const;
    double     K0_FG() const;
    QString    K0_FGAddress() const;

    double airGap() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineGroup( caf::PdmUiOrdering&                      uiOrdering,
                      const QString&                           title,
                      caf::PdmField<caf::AppEnum<SourceType>>* typeField,
                      caf::PdmField<double>*                   fixedField,
                      caf::PdmField<QString>*                  addressField );

    void handleFieldChanged( RimGeoMechCase*                          geoMechCase,
                             ParameterType                            parameterType,
                             caf::PdmField<caf::AppEnum<SourceType>>* typeField,
                             caf::PdmField<double>*                   fixedField,
                             caf::PdmField<QString>*                  addressField,
                             bool                                     typeFieldChanged );

private:
    caf::PdmField<caf::AppEnum<SourceType>> m_wellDeviationType;
    caf::PdmField<double>                   m_wellDeviationFixed;
    caf::PdmField<QString>                  m_wellDeviationAddress;

    caf::PdmField<caf::AppEnum<SourceType>> m_wellAzimuthType;
    caf::PdmField<double>                   m_wellAzimuthFixed;
    caf::PdmField<QString>                  m_wellAzimuthAddress;

    caf::PdmField<caf::AppEnum<SourceType>> m_UCSType;
    caf::PdmField<double>                   m_UCSFixed;
    caf::PdmField<QString>                  m_UCSAddress;

    caf::PdmField<caf::AppEnum<SourceType>> m_poissonsRatioType;
    caf::PdmField<double>                   m_poissonsRatioFixed;
    caf::PdmField<QString>                  m_poissonsRatioAddress;

    caf::PdmField<caf::AppEnum<SourceType>> m_K0_FGType;
    caf::PdmField<double>                   m_K0_FGFixed;
    caf::PdmField<QString>                  m_K0_FGAddress;

    caf::PdmField<caf::AppEnum<SourceType>> m_obg0Type;
    caf::PdmField<double>                   m_obg0Fixed;
    caf::PdmField<QString>                  m_obg0Address;

    caf::PdmField<double> m_airGap;
    caf::PdmField<double> m_shMultiplier;
    caf::PdmField<double> m_userDefinedPPNonReservoir;

    caf::PdmField<caf::AppEnum<UpperLimitType>>                  m_upperLimitType;
    caf::PdmField<caf::AppEnum<LowerLimitType>>                  m_lowerLimitType;
    caf::PdmField<caf::AppEnum<FractureGradientCalculationType>> m_fractureGradientCalculationType;
    caf::PdmField<caf::AppEnum<NonReservoirPorePressureType>>    m_porePressureNonReservoirSource;

    caf::PdmField<int> m_referenceLayer;
};
