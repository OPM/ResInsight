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

#include "RimMudWeightWindowParameters.h"

#include "RiaLogging.h"

#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimMudWeightWindowParameters, "RimMudWeightWindowParameters" );

namespace caf
{
template <>
void caf::AppEnum<RimMudWeightWindowParameters::SourceType>::setUp()
{
    addItem( RimMudWeightWindowParameters::SourceType::FIXED, "FIXED", "Fixed" );
    addItem( RimMudWeightWindowParameters::SourceType::PER_ELEMENT, "PER_ELEMENT", "From element properties" );
    addItem( RimMudWeightWindowParameters::SourceType::GRID, "GRID", "Grid" );
    setDefault( RimMudWeightWindowParameters::SourceType::FIXED );
}

template <>
void caf::AppEnum<RimMudWeightWindowParameters::ParameterType>::setUp()
{
    addItem( RimMudWeightWindowParameters::ParameterType::WELL_DEVIATION, "WELL_DEVIATION", "Well Deviation" );
    addItem( RimMudWeightWindowParameters::ParameterType::WELL_AZIMUTH, "WELL_AZIMUTH", "Well Azimuth" );
    addItem( RimMudWeightWindowParameters::ParameterType::UCS, "UCS", "UCS" );
    addItem( RimMudWeightWindowParameters::ParameterType::POISSONS_RATIO, "POISSONS_RARIO", "Poisson's Ratio" );
    addItem( RimMudWeightWindowParameters::ParameterType::K0_FG, "K0_FG", "K0 FG" );
    addItem( RimMudWeightWindowParameters::ParameterType::OBG0, "OBG0", "Initial Overburden Gradient" );
    setDefault( RimMudWeightWindowParameters::ParameterType::WELL_DEVIATION );
}

template <>
void caf::AppEnum<RimMudWeightWindowParameters::UpperLimitType>::setUp()
{
    addItem( RimMudWeightWindowParameters::UpperLimitType::FG, "FG", "Fracture Gradient" );
    addItem( RimMudWeightWindowParameters::UpperLimitType::SH_MIN, "SH_MIN", "Minimum Horizontal Stress" );
    setDefault( RimMudWeightWindowParameters::UpperLimitType::FG );
}

template <>
void caf::AppEnum<RimMudWeightWindowParameters::LowerLimitType>::setUp()
{
    addItem( RimMudWeightWindowParameters::LowerLimitType::PORE_PRESSURE, "PORE_PRESSURE", "Pore Pressure" );
    addItem( RimMudWeightWindowParameters::LowerLimitType::MAX_OF_PORE_PRESSURE_AND_SFG,
             "MAX_OF_PORE_PRESSURE_AND_SFG",
             "Maximum of Pore Pressure and SFG" );
    setDefault( RimMudWeightWindowParameters::LowerLimitType::PORE_PRESSURE );
}

template <>
void caf::AppEnum<RimMudWeightWindowParameters::FractureGradientCalculationType>::setUp()
{
    addItem( RimMudWeightWindowParameters::FractureGradientCalculationType::DERIVED_FROM_K0FG,
             "DERIVED_FROM_K0FG",
             "FG derived from K0_FG" );
    addItem( RimMudWeightWindowParameters::FractureGradientCalculationType::PROPORTIONAL_TO_SH,
             "PROPORTIONAL_TO_SH",
             "Proportional to SH" );
    setDefault( RimMudWeightWindowParameters::FractureGradientCalculationType::DERIVED_FROM_K0FG );
}

template <>
void caf::AppEnum<RimMudWeightWindowParameters::NonReservoirPorePressureType>::setUp()
{
    addItem( RimMudWeightWindowParameters::NonReservoirPorePressureType::HYDROSTATIC, "HYDROSTATIC", "Hydrostatic" );
    addItem( RimMudWeightWindowParameters::NonReservoirPorePressureType::PER_ELEMENT,
             "PER_ELEMENT",
             "From element properties" );
    setDefault( RimMudWeightWindowParameters::NonReservoirPorePressureType::HYDROSTATIC );
}

} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::RimMudWeightWindowParameters( void )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Mud Weight Window Parameters",
                                                    "",
                                                    "",
                                                    "The Mud Weight Window Calculation Parameters",
                                                    "MudWeightWindowParameters",
                                                    "" );

    caf::AppEnum<SourceType> defaultSourceType = RimMudWeightWindowParameters::SourceType::FIXED;

    CAF_PDM_InitField( &m_wellDeviationType, "WellDeviationSourceType", defaultSourceType, "Well Deviation", "", "", "" );
    CAF_PDM_InitField( &m_wellDeviationFixed, "WellDeviationFixed", 0.0, "Fixed Well Deviation", "", "", "" );
    m_wellDeviationFixed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellDeviationAddress, "WellDeviationAddress", QString( "" ), "Value", "", "", "" );
    m_wellDeviationAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellAzimuthType, "WellAzimuthSourceType", defaultSourceType, "Well Azimuth", "", "", "" );
    CAF_PDM_InitField( &m_wellAzimuthFixed, "WellAzimuthFixed", 0.0, "Fixed Well Azimuth", "", "", "" );
    m_wellAzimuthFixed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellAzimuthAddress, "WellAzimuthAddress", QString( "" ), "Value", "", "", "" );
    m_wellAzimuthAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_UCSType, "UCSSourceType", defaultSourceType, "UCS [Bar]", "", "", "" );
    CAF_PDM_InitField( &m_UCSFixed, "UCSFixed", 100.0, "Fixed UCS [Bar]", "", "", "" );
    m_UCSFixed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_UCSAddress, "UCSAddress", QString( "" ), "Value", "", "", "" );
    m_UCSAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_poissonsRatioType, "PoissonsRatioSourceType", defaultSourceType, "Poisson's Ratio", "", "", "" );
    CAF_PDM_InitField( &m_poissonsRatioFixed, "PoissonsRatioFixed", 0.35, "Fixed Possion's Ratio", "", "", "" );
    m_poissonsRatioFixed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_poissonsRatioAddress, "PoissonsRatioAddress", QString( "" ), "Value", "", "", "" );
    m_poissonsRatioAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_K0_FGType, "K0_FGSourceType", defaultSourceType, "K0 FG", "", "", "" );
    CAF_PDM_InitField( &m_K0_FGFixed, "K0_FGFixed", 0.75, "Fixed K0_FG", "", "", "" );
    m_K0_FGFixed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_K0_FGAddress, "K0_FGAddress", QString( "" ), "Value", "", "", "" );
    m_K0_FGAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    caf::AppEnum<SourceType> defaultOBG0SourceType = RimMudWeightWindowParameters::SourceType::GRID;
    CAF_PDM_InitField( &m_obg0Type, "obg0SourceType", defaultOBG0SourceType, "Initial Overburden Gradient", "", "", "" );
    CAF_PDM_InitField( &m_obg0Fixed, "obg0Fixed", 0.75, "Fixed Initial Overburden Gradient", "", "", "" );
    m_obg0Fixed.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_obg0Address, "obg0Address", QString( "" ), "Value", "", "", "" );
    m_obg0Address.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    m_parameterFields[RimMudWeightWindowParameters::ParameterType::WELL_DEVIATION] =
        std::make_tuple( &m_wellDeviationType, &m_wellDeviationFixed, &m_wellDeviationAddress );
    m_parameterFields[RimMudWeightWindowParameters::ParameterType::WELL_AZIMUTH] =
        std::make_tuple( &m_wellAzimuthType, &m_wellAzimuthFixed, &m_wellAzimuthAddress );
    m_parameterFields[RimMudWeightWindowParameters::ParameterType::UCS] =
        std::make_tuple( &m_UCSType, &m_UCSFixed, &m_UCSAddress );
    m_parameterFields[RimMudWeightWindowParameters::ParameterType::POISSONS_RATIO] =
        std::make_tuple( &m_poissonsRatioType, &m_poissonsRatioFixed, &m_poissonsRatioAddress );
    m_parameterFields[RimMudWeightWindowParameters::ParameterType::K0_FG] =
        std::make_tuple( &m_K0_FGType, &m_K0_FGFixed, &m_K0_FGAddress );
    m_parameterFields[RimMudWeightWindowParameters::ParameterType::OBG0] =
        std::make_tuple( &m_obg0Type, &m_obg0Fixed, &m_obg0Address );

    CAF_PDM_InitField( &m_airGap, "AirGap", 0.0, "Air Gap", "", "", "" );

    CAF_PDM_InitField( &m_shMultiplier, "SHMultiplier", 1.05, "SH Multplier for FG in Shale", "", "", "" );

    caf::AppEnum<UpperLimitType> defaultUpperLimitType = RimMudWeightWindowParameters::UpperLimitType::FG;
    CAF_PDM_InitField( &m_upperLimitType, "UpperLimitType", defaultUpperLimitType, "Upper Limit Type", "", "", "" );

    caf::AppEnum<LowerLimitType> defaultLowerLimitType =
        RimMudWeightWindowParameters::LowerLimitType::MAX_OF_PORE_PRESSURE_AND_SFG;
    CAF_PDM_InitField( &m_lowerLimitType, "LowerLimitType", defaultLowerLimitType, "Lower Limit Type", "", "", "" );

    caf::AppEnum<FractureGradientCalculationType> defaultFractureGradientCalculationType =
        RimMudWeightWindowParameters::FractureGradientCalculationType::DERIVED_FROM_K0FG;
    CAF_PDM_InitField( &m_fractureGradientCalculationType,
                       "FractureGradientCalculationType",
                       defaultFractureGradientCalculationType,
                       "FG in Shale Calculation",
                       "",
                       "",
                       "" );

    caf::AppEnum<NonReservoirPorePressureType> defaultNonReservoirPorePressureType =
        RimMudWeightWindowParameters::NonReservoirPorePressureType::HYDROSTATIC;
    CAF_PDM_InitField( &m_porePressureNonReservoirSource,
                       "PorePressureNonReservoirSource",
                       defaultNonReservoirPorePressureType,
                       "Non-Reservoir Pore Pressure",
                       "",
                       "Data source for Non-Reservoir Pore Pressure",
                       "" );
    CAF_PDM_InitField( &m_userDefinedPPNonReservoir, "UserPPNonReservoir", 1.0, "  Multiplier of hydrostatic PP", "", "", "" );
    CAF_PDM_InitField( &m_porePressureNonReservoirAddress, "PPNonReservoirAddress", QString( "" ), "Value", "", "", "" );
    m_porePressureNonReservoirAddress.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_referenceLayer, "ReferenceLayer", -1, "Reference Layer", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::SourceType RimMudWeightWindowParameters::wellDeviationType() const
{
    return m_wellDeviationType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMudWeightWindowParameters::wellDeviation() const
{
    return m_wellDeviationFixed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMudWeightWindowParameters::wellDeviationAddress() const
{
    return m_wellDeviationAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::SourceType RimMudWeightWindowParameters::wellAzimuthType() const
{
    return m_wellAzimuthType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMudWeightWindowParameters::wellAzimuth() const
{
    return m_wellAzimuthFixed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMudWeightWindowParameters::wellAzimuthAddress() const
{
    return m_wellAzimuthAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::SourceType RimMudWeightWindowParameters::ucsType() const
{
    return m_UCSType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMudWeightWindowParameters::ucs() const
{
    return m_UCSFixed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMudWeightWindowParameters::ucsAddress() const
{
    return m_UCSAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::SourceType RimMudWeightWindowParameters::poissonsRatioType() const
{
    return m_poissonsRatioType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMudWeightWindowParameters::poissonsRatio() const
{
    return m_poissonsRatioFixed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMudWeightWindowParameters::poissonsRatioAddress() const
{
    return m_poissonsRatioAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::SourceType RimMudWeightWindowParameters::K0_FG_Type() const
{
    return m_K0_FGType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMudWeightWindowParameters::K0_FG() const
{
    return m_K0_FGFixed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimMudWeightWindowParameters::K0_FGAddress() const
{
    return m_K0_FGAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimMudWeightWindowParameters::airGap() const
{
    return m_airGap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMudWeightWindowParameters::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                     const QVariant&            oldValue,
                                                     const QVariant&            newValue )
{
    RimGeoMechCase* geoMechCase = nullptr;
    firstAncestorOrThisOfType( geoMechCase );
    if ( !geoMechCase )
    {
        return;
    }

    if ( changedField == &m_wellDeviationFixed || changedField == &m_wellDeviationType ||
         changedField == &m_wellDeviationAddress )
    {
        handleFieldChanged( geoMechCase,
                            ParameterType::WELL_DEVIATION,
                            &m_wellDeviationType,
                            &m_wellDeviationFixed,
                            &m_wellDeviationAddress,
                            changedField == &m_wellDeviationType );
    }
    else if ( changedField == &m_wellAzimuthFixed || changedField == &m_wellAzimuthType ||
              changedField == &m_wellAzimuthAddress )
    {
        handleFieldChanged( geoMechCase,
                            ParameterType::WELL_AZIMUTH,
                            &m_wellAzimuthType,
                            &m_wellAzimuthFixed,
                            &m_wellAzimuthAddress,
                            changedField == &m_wellAzimuthType );
    }
    else if ( changedField == &m_UCSFixed || changedField == &m_UCSType || changedField == &m_UCSAddress )
    {
        handleFieldChanged( geoMechCase, ParameterType::UCS, &m_UCSType, &m_UCSFixed, &m_UCSAddress, changedField == &m_UCSType );
    }
    else if ( changedField == &m_poissonsRatioFixed || changedField == &m_poissonsRatioType ||
              changedField == &m_poissonsRatioAddress )
    {
        handleFieldChanged( geoMechCase,
                            ParameterType::POISSONS_RATIO,
                            &m_poissonsRatioType,
                            &m_poissonsRatioFixed,
                            &m_poissonsRatioAddress,
                            changedField == &m_poissonsRatioType );
    }
    else if ( changedField == &m_K0_FGFixed || changedField == &m_K0_FGType || changedField == &m_K0_FGAddress )
    {
        handleFieldChanged( geoMechCase,
                            ParameterType::K0_FG,
                            &m_K0_FGType,
                            &m_K0_FGFixed,
                            &m_K0_FGAddress,
                            changedField == &m_K0_FGType );
    }
    else if ( changedField == &m_obg0Fixed || changedField == &m_obg0Type || changedField == &m_obg0Address )
    {
        handleFieldChanged( geoMechCase, ParameterType::OBG0, &m_obg0Type, &m_obg0Fixed, &m_obg0Address, changedField == &m_obg0Type );
    }
    else if ( changedField == &m_airGap || changedField == &m_upperLimitType || changedField == &m_lowerLimitType ||
              changedField == &m_referenceLayer || changedField == &m_fractureGradientCalculationType ||
              changedField == &m_shMultiplier || changedField == &m_porePressureNonReservoirSource ||
              changedField == &m_userDefinedPPNonReservoir || changedField == &m_porePressureNonReservoirAddress )
    {
        RigGeoMechCaseData* rigCaseData = geoMechCase->geoMechData();
        if ( rigCaseData && rigCaseData->femPartResults() )
        {
            rigCaseData->femPartResults()->setMudWeightWindowParameters( m_airGap,
                                                                         m_upperLimitType.value(),
                                                                         m_lowerLimitType.value(),
                                                                         m_referenceLayer,
                                                                         m_fractureGradientCalculationType.value(),
                                                                         m_shMultiplier,
                                                                         m_porePressureNonReservoirSource.value(),
                                                                         m_userDefinedPPNonReservoir,
                                                                         m_porePressureNonReservoirAddress );
            geoMechCase->updateConnectedViews();
            geoMechCase->settingsChanged.send();
        }
    }
}

void RimMudWeightWindowParameters::handleFieldChanged( RimGeoMechCase*                          geoMechCase,
                                                       ParameterType                            parameterType,
                                                       caf::PdmField<caf::AppEnum<SourceType>>* typeField,
                                                       caf::PdmField<double>*                   fixedField,
                                                       caf::PdmField<QString>*                  addressField,
                                                       bool                                     typeFieldChanged )

{
    RigGeoMechCaseData* rigCaseData = geoMechCase->geoMechData();

    if ( rigCaseData && rigCaseData->femPartResults() )
    {
        if ( typeField->value() == RimMudWeightWindowParameters::SourceType::FIXED ||
             typeField->value() == RimMudWeightWindowParameters::SourceType::GRID )
        {
            rigCaseData->femPartResults()->setCalculationParameters( parameterType, "", fixedField->value() );
        }
        else if ( typeField->value() == RimMudWeightWindowParameters::SourceType::PER_ELEMENT )
        {
            if ( typeFieldChanged )
            {
                // Show info message to user when selecting "from file" option before
                // an element property has been imported
                std::vector<std::string> elementProperties = geoMechCase->possibleElementPropertyFieldNames();
                if ( elementProperties.empty() )
                {
                    QString title = caf::AppEnum<ParameterType>::uiText( parameterType );
                    QString importMessage =
                        QString( "Please import '%1' from file by "
                                 "selecting 'Import Element Property Table' on the Geomechanical Model." )
                            .arg( title );
                    RiaLogging::info( importMessage );
                    // Set back to default value
                    *typeField = typeField->defaultValue();
                    return;
                }
            }

            if ( addressField->value().isEmpty() )
            {
                // Automatically select the first available property element if empty
                std::vector<std::string> elementProperties = geoMechCase->possibleElementPropertyFieldNames();
                if ( !elementProperties.empty() )
                {
                    *addressField = QString::fromStdString( elementProperties[0] );
                }
            }

            rigCaseData->femPartResults()->setCalculationParameters( parameterType,
                                                                     addressField->value(),
                                                                     fixedField->value() );
        }
    }

    geoMechCase->updateConnectedViews();
    geoMechCase->settingsChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMudWeightWindowParameters::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    defineGroup( uiOrdering, "Well Deviation", &m_wellDeviationType, &m_wellDeviationFixed, &m_wellDeviationAddress );
    defineGroup( uiOrdering, "Well Azimuth", &m_wellAzimuthType, &m_wellAzimuthFixed, &m_wellAzimuthAddress );
    defineGroup( uiOrdering, "UCS", &m_UCSType, &m_UCSFixed, &m_UCSAddress );
    defineGroup( uiOrdering, "Poisson's Ratio", &m_poissonsRatioType, &m_poissonsRatioFixed, &m_poissonsRatioAddress );

    RimGeoMechCase* geoMechCase = nullptr;
    firstAncestorOrThisOfType( geoMechCase );
    if ( !geoMechCase )
    {
        return;
    }

    RigGeoMechCaseData* rigCaseData = geoMechCase->geoMechData();

    if ( rigCaseData && m_referenceLayer == -1 )
    {
        m_referenceLayer =
            (int)rigCaseData->femParts()->part( 0 )->getOrCreateStructGrid()->reservoirIJKBoundingBox().first.z();
    }

    uiOrdering.add( &m_fractureGradientCalculationType );
    uiOrdering.add( &m_shMultiplier );
    defineGroup( uiOrdering, "K0 for Fracture Gradient Factor for Shale", &m_K0_FGType, &m_K0_FGFixed, &m_K0_FGAddress );

    m_shMultiplier.uiCapability()->setUiHidden( m_fractureGradientCalculationType !=
                                                FractureGradientCalculationType::PROPORTIONAL_TO_SH );

    bool isDerivedFromK0_FG = m_fractureGradientCalculationType == FractureGradientCalculationType::DERIVED_FROM_K0FG;
    m_K0_FGType.uiCapability()->setUiHidden( !isDerivedFromK0_FG );
    m_K0_FGFixed.uiCapability()->setUiHidden(
        !( isDerivedFromK0_FG && m_K0_FGType == RimMudWeightWindowParameters::SourceType::FIXED ) );
    m_K0_FGAddress.uiCapability()->setUiHidden(
        !( isDerivedFromK0_FG && m_K0_FGType == RimMudWeightWindowParameters::SourceType::PER_ELEMENT ) );

    defineGroup( uiOrdering, "Initial Overburden Gradient", &m_obg0Type, &m_obg0Fixed, &m_obg0Address );
    m_obg0Type.uiCapability()->setUiHidden( !isDerivedFromK0_FG );
    m_obg0Fixed.uiCapability()->setUiHidden( true );
    m_obg0Address.uiCapability()->setUiHidden(
        !( isDerivedFromK0_FG && m_obg0Type == RimMudWeightWindowParameters::SourceType::PER_ELEMENT ) );

    bool ppNonResPerElement =
        ( m_porePressureNonReservoirSource == RimMudWeightWindowParameters::NonReservoirPorePressureType::PER_ELEMENT );
    m_userDefinedPPNonReservoir.uiCapability()->setUiHidden( ppNonResPerElement );
    m_porePressureNonReservoirAddress.uiCapability()->setUiHidden( !ppNonResPerElement );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMudWeightWindowParameters::defineGroup( caf::PdmUiOrdering&                      uiOrdering,
                                                const QString&                           title,
                                                caf::PdmField<caf::AppEnum<SourceType>>* typeField,
                                                caf::PdmField<double>*                   fixedField,
                                                caf::PdmField<QString>*                  addressField )
{
    uiOrdering.add( typeField );
    uiOrdering.add( fixedField );
    uiOrdering.add( addressField );

    fixedField->uiCapability()->setUiHidden( *typeField != RimMudWeightWindowParameters::SourceType::FIXED );
    addressField->uiCapability()->setUiHidden( *typeField != RimMudWeightWindowParameters::SourceType::PER_ELEMENT );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMudWeightWindowParameters::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_wellDeviationFixed || field == &m_wellAzimuthFixed )
    {
        auto uiDoubleValueEditorAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( uiDoubleValueEditorAttr )
        {
            uiDoubleValueEditorAttr->m_validator = new QDoubleValidator( 0.0, 360.0, 3 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimMudWeightWindowParameters::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    RimGeoMechCase* geoMechCase = nullptr;
    firstAncestorOrThisOfType( geoMechCase );

    if ( geoMechCase != nullptr )
    {
        if ( fieldNeedingOptions == &m_obg0Type )
        {
            std::vector<SourceType> sourceTypes = { SourceType::GRID, SourceType::PER_ELEMENT };
            for ( auto sourceType : sourceTypes )
            {
                options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<SourceType>::uiText( sourceType ), sourceType ) );
            }
        }
        else if ( fieldNeedingOptions == &m_wellDeviationType || fieldNeedingOptions == &m_wellAzimuthType ||
                  fieldNeedingOptions == &m_UCSType || fieldNeedingOptions == &m_poissonsRatioType ||
                  fieldNeedingOptions == &m_K0_FGType )
        {
            std::vector<SourceType> sourceTypes = { SourceType::FIXED, SourceType::PER_ELEMENT };
            for ( auto sourceType : sourceTypes )
            {
                options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<SourceType>::uiText( sourceType ), sourceType ) );
            }
        }
        else if ( fieldNeedingOptions == &m_wellDeviationAddress || fieldNeedingOptions == &m_wellAzimuthAddress ||
                  fieldNeedingOptions == &m_UCSAddress || fieldNeedingOptions == &m_poissonsRatioAddress ||
                  fieldNeedingOptions == &m_K0_FGAddress || fieldNeedingOptions == &m_obg0Address ||
                  fieldNeedingOptions == &m_porePressureNonReservoirAddress )
        {
            std::vector<std::string> elementProperties = geoMechCase->possibleElementPropertyFieldNames();

            std::vector<caf::FilePath> elementPropertyFileNames = geoMechCase->elementPropertyFileNames();
            std::vector<QString>       paths;
            for ( auto path : elementPropertyFileNames )
            {
                paths.push_back( path.path() );
            }

            std::map<std::string, QString> addressesInFile =
                geoMechCase->geoMechData()->femPartResults()->addressesInElementPropertyFiles( paths );

            for ( const std::string& elementProperty : elementProperties )
            {
                QString result   = QString::fromStdString( elementProperty );
                QString filename = geoMechCase->findFileNameForElementProperty( elementProperty, addressesInFile );
                options.push_back( caf::PdmOptionItemInfo( result + " (" + filename + ")", result ) );
            }
        }
        else if ( fieldNeedingOptions == &m_referenceLayer )
        {
            if ( geoMechCase->geoMechData() )
            {
                size_t kCount =
                    geoMechCase->geoMechData()->femParts()->part( 0 )->getOrCreateStructGrid()->gridPointCountK() - 1;
                for ( size_t layerIdx = 0; layerIdx < kCount; ++layerIdx )
                {
                    options.push_back( caf::PdmOptionItemInfo( QString::number( layerIdx + 1 ), (int)layerIdx ) );
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMudWeightWindowParameters::updateFemPartResults() const
{
    RimGeoMechCase* geoMechCase = nullptr;
    firstAncestorOrThisOfType( geoMechCase );
    if ( !geoMechCase )
    {
        return;
    }

    RigGeoMechCaseData* rigCaseData = geoMechCase->geoMechData();
    if ( !rigCaseData )
    {
        return;
    }

    for ( size_t i = 0; i < caf::AppEnum<ParameterType>::size(); ++i )
    {
        updateFemPartsForParameter( caf::AppEnum<ParameterType>::fromIndex( i ), rigCaseData );
    }

    // Make sure the reference layer is valid
    int referenceLayer = m_referenceLayer();
    if ( referenceLayer == -1 )
    {
        referenceLayer =
            (int)rigCaseData->femParts()->part( 0 )->getOrCreateStructGrid()->reservoirIJKBoundingBox().first.z();
    }

    rigCaseData->femPartResults()->setMudWeightWindowParameters( m_airGap,
                                                                 m_upperLimitType.value(),
                                                                 m_lowerLimitType.value(),
                                                                 referenceLayer,
                                                                 m_fractureGradientCalculationType.value(),
                                                                 m_shMultiplier,
                                                                 m_porePressureNonReservoirSource.value(),
                                                                 m_userDefinedPPNonReservoir,
                                                                 m_porePressureNonReservoirAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimMudWeightWindowParameters::updateFemPartsForParameter( ParameterType       parameterType,
                                                               RigGeoMechCaseData* rigCaseData ) const
{
    auto it = m_parameterFields.find( parameterType );
    if ( it == m_parameterFields.end() ) return;

    caf::PdmField<caf::AppEnum<SourceType>>* typeField    = std::get<0>( it->second );
    caf::PdmField<double>*                   fixedField   = std::get<1>( it->second );
    caf::PdmField<QString>*                  addressField = std::get<2>( it->second );

    if ( rigCaseData->femPartResults() )
    {
        if ( typeField->value() == RimMudWeightWindowParameters::SourceType::FIXED ||
             typeField->value() == RimMudWeightWindowParameters::SourceType::GRID )
        {
            rigCaseData->femPartResults()->setCalculationParameters( parameterType, "", fixedField->value() );
        }
        else if ( typeField->value() == RimMudWeightWindowParameters::SourceType::PER_ELEMENT )
        {
            rigCaseData->femPartResults()->setCalculationParameters( parameterType,
                                                                     addressField->value(),
                                                                     fixedField->value() );
        }
    }
}
