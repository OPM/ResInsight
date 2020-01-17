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
#include "RimWbsParameters.h"

#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimWellLogFile.h"
#include "RimWellPath.h"

CAF_PDM_SOURCE_INIT( RimWbsParameters, "WbsParameters" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWbsParameters::RimWbsParameters()
{
    CAF_PDM_InitObject( "Well Bore Stability Parameters", ":/WellLogPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_porePressureSource,
                                "PorePressureSource",
                                "Pore Pressure",
                                "",
                                "Data source for Pore Pressure",
                                "" );

    CAF_PDM_InitFieldNoDefault( &m_porePressureShaleSource,
                                "PorePressureShaleSource",
                                "Shale Pore Pressure",
                                "",
                                "Data source for Pore Pressure in Shale",
                                "" );

    CAF_PDM_InitFieldNoDefault( &m_poissonRatioSource,
                                "PoissionRatioSource",
                                "Poisson Ratio",
                                "",
                                "Data source for Poisson Ratio",
                                "" );

    CAF_PDM_InitFieldNoDefault( &m_ucsSource, "UcsSource", "Uniaxial Compressive Strength", "", "Data source for UCS", "" );

    CAF_PDM_InitFieldNoDefault( &m_OBG0Source, "OBG0Source", "Initial Overburden Gradient", "", "Data source for OBG0", "" );
    CAF_PDM_InitFieldNoDefault( &m_DFSource, "DFSource", "Depletion Factor (DF)", "", "Data source for Depletion Factor", "" );

    CAF_PDM_InitFieldNoDefault( &m_K0SHSource,
                                "K0SHSource",
                                "K0_SH",
                                "",
                                "SH in Shale (Matthews & Kelly) = K0_SH * (OBG0-PP0) + PP0 + DF * (PP-PP0)\nK0_SH = "
                                "(SH - PP)/(OBG-PP)",
                                "" );

    CAF_PDM_InitFieldNoDefault( &m_FGShaleSource, "FGShaleSource", "FG in Shale Calculation", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_K0FGSource,
                                "K0FGSource",
                                "K0_FG",
                                "",
                                "FG in shale = K0_FG * (OBG0-PP0)\nK0_FG = (FG-PP)/(OBG-PP)",
                                "" );

    CAF_PDM_InitField( &m_userDefinedPPShale, "UserPPShale", 1.05, "Multiplier of hydrostatic PP", "", "", "" );

    CAF_PDM_InitField( &m_userDefinedPoissionRatio,
                       "UserPoissionRatio",
                       0.35,
                       "User Defined Poisson Ratio",
                       "",
                       "User Defined Poisson Ratio",
                       "" );
    // Typical UCS: http://ceae.colorado.edu/~amadei/CVEN5768/PDF/NOTES8.pdf
    // Typical UCS for Shale is 5 - 100 MPa -> 50 - 1000 bar.
    CAF_PDM_InitField( &m_userDefinedUcs, "UserUcs", 100.0, "User Defined UCS [bar]", "", "User Defined UCS [bar]", "" );

    CAF_PDM_InitField( &m_userDefinedDF, "UserDF", 0.7, "User Defined DF", "", "User Defined Depletion Factor", "" );
    CAF_PDM_InitField( &m_userDefinedK0FG, "UserK0FG", 0.75, "User Defined K0_FG", "", "", "" );
    CAF_PDM_InitField( &m_userDefinedK0SH, "UserK0SH", 0.65, "User Defined K0_SH", "", "", "" );
    CAF_PDM_InitField( &m_FGShaleMultiplier,
                       "FGMultiplier",
                       1.05,
                       "SH Multiplier for FG in Shale",
                       "",
                       "FG in Shale = Multiplier * SH",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_geoMechCase, "GeoMechCase", "GeoMechCase", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "WellPath", "", "", "" );
    CAF_PDM_InitField( &m_timeStep, "TimeStep", -1, "TimeStep", "", "", "" );

    m_parameterSourceFields = {{RigWbsParameter::PP_Sand(), &m_porePressureSource},
                               {RigWbsParameter::PP_Shale(), &m_porePressureShaleSource},
                               {RigWbsParameter::poissonRatio(), &m_poissonRatioSource},
                               {RigWbsParameter::UCS(), &m_ucsSource},
                               {RigWbsParameter::OBG0(), &m_OBG0Source},
                               {RigWbsParameter::DF(), &m_DFSource},
                               {RigWbsParameter::K0_FG(), &m_K0FGSource},
                               {RigWbsParameter::K0_SH(), &m_K0SHSource},
                               {RigWbsParameter::FG_Shale(), &m_FGShaleSource}};

    m_userDefinedValueFields = {{RigWbsParameter::PP_Shale(), &m_userDefinedPPShale},
                                {RigWbsParameter::poissonRatio(), &m_userDefinedPoissionRatio},
                                {RigWbsParameter::UCS(), &m_userDefinedUcs},
                                {RigWbsParameter::DF(), &m_userDefinedDF},
                                {RigWbsParameter::K0_FG(), &m_userDefinedK0FG},
                                {RigWbsParameter::K0_SH(), &m_userDefinedK0SH},
                                {RigWbsParameter::FG_Shale(), &m_FGShaleMultiplier}};

    for ( auto parameterFieldPair : m_parameterSourceFields )
    {
        auto sources = parameterFieldPair.first.sources();
        if ( !sources.empty() )
        {
            setParameterSource( parameterFieldPair.first, sources.front() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWbsParameters::RimWbsParameters( const RimWbsParameters& copyFrom )
{
    *this = copyFrom;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWbsParameters& RimWbsParameters::operator=( const RimWbsParameters& copyFrom )
{
    m_geoMechCase = copyFrom.m_geoMechCase();
    m_wellPath    = copyFrom.m_wellPath();
    m_timeStep    = copyFrom.m_timeStep();

    for ( auto parameterSourcePair : m_parameterSourceFields )
    {
        setParameterSource( parameterSourcePair.first, copyFrom.parameterSource( parameterSourcePair.first ) );
    }
    for ( auto parameterUserDefinedValuePair : m_userDefinedValueFields )
    {
        setUserDefinedValue( parameterUserDefinedValuePair.first,
                             copyFrom.userDefinedValue( parameterUserDefinedValuePair.first ) );
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWbsParameters::~RimWbsParameters() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::setGeoMechCase( RimGeoMechCase* geoMechCase )
{
    m_geoMechCase = geoMechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::applyWbsParametersToExtractor( RigGeoMechWellLogExtractor* extractor )
{
    for ( auto parameterSourcePair : m_parameterSourceFields )
    {
        extractor->setWbsParametersSource( parameterSourcePair.first, ( *parameterSourcePair.second )() );
    }
    for ( auto parameterUserDefinedValuePair : m_userDefinedValueFields )
    {
        extractor->setWbsUserDefinedValue( parameterUserDefinedValuePair.first,
                                           ( *parameterUserDefinedValuePair.second ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWbsParameters::ParameterSource RimWbsParameters::parameterSource( const RigWbsParameter& parameter ) const
{
    auto field = sourceField( parameter );
    if ( field )
    {
        return ( *field )();
    }
    return RigWbsParameter::INVALID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWbsParameters::userDefinedValue( const RigWbsParameter& parameter ) const
{
    auto it = m_userDefinedValueFields.find( parameter );
    if ( it != m_userDefinedValueFields.end() )
    {
        return *it->second;
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::setParameterSource( const RigWbsParameter& parameter, ParameterSource source )
{
    auto field = sourceField( parameter );
    if ( field )
    {
        *field = source;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::setUserDefinedValue( const RigWbsParameter& parameter, double value )
{
    auto it = m_userDefinedValueFields.find( parameter );
    if ( it != m_userDefinedValueFields.end() )
    {
        it->second->setValue( value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmField<RimWbsParameters::ParameterSourceEnum>* RimWbsParameters::sourceField( const RigWbsParameter& parameter ) const
{
    auto it = m_parameterSourceFields.find( parameter );
    if ( it != m_parameterSourceFields.end() )
    {
        return it->second;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWbsParameters::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                       bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    std::set<RigWbsParameter> allParameters = RigWbsParameter::allParameters();

    for ( const RigWbsParameter& parameter : allParameters )
    {
        caf::PdmField<ParameterSourceEnum>* field = sourceField( parameter );
        if ( field == fieldNeedingOptions )
        {
            std::vector<ParameterSource> sources = supportedSources( parameter );
            for ( int i = 0; i < (int)sources.size(); ++i )
            {
                if ( parameter.exclusiveOptions() || i == (int)sources.size() - 1 ||
                     sources[i] == RigWbsParameter::HYDROSTATIC )
                {
                    options.push_back( caf::PdmOptionItemInfo( ParameterSourceEnum::uiText( sources[i] ), sources[i] ) );
                }
                else
                {
                    QStringList cumulativeSourceLabels;
                    for ( int j = i; j < (int)sources.size(); ++j )
                    {
                        int index = 1 + ( j - i );
                        cumulativeSourceLabels.push_back(
                            QString( "%1. %2" ).arg( index ).arg( ParameterSourceEnum::uiText( sources[j] ) ) );
                    }
                    options.push_back( caf::PdmOptionItemInfo( cumulativeSourceLabels.join( ", " ), sources[i] ) );
                }
            }
            break;
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::loadDataAndUpdate()
{
    std::set<RigWbsParameter> allParameters = RigWbsParameter::allParameters();

    for ( const RigWbsParameter& parameter : allParameters )
    {
        caf::PdmField<ParameterSourceEnum>* field = sourceField( parameter );
        if ( field )
        {
            assignValidSource( field, supportedSources( parameter ) );
        }
    }
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_porePressureSource );
    uiOrdering.add( &m_porePressureShaleSource );
    if ( m_porePressureShaleSource == RigWbsParameter::USER_DEFINED )
    {
        uiOrdering.add( &m_userDefinedPPShale );
    }
    uiOrdering.add( &m_poissonRatioSource );
    uiOrdering.add( &m_userDefinedPoissionRatio );
    uiOrdering.add( &m_ucsSource );
    uiOrdering.add( &m_userDefinedUcs );
    uiOrdering.add( &m_OBG0Source );
    uiOrdering.add( &m_DFSource );
    uiOrdering.add( &m_userDefinedDF );
    uiOrdering.add( &m_K0SHSource );
    uiOrdering.add( &m_userDefinedK0SH );
    uiOrdering.add( &m_FGShaleSource );
    if ( m_FGShaleSource == RigWbsParameter::PROPORTIONAL_TO_SH )
    {
        uiOrdering.add( &m_FGShaleMultiplier );
    }
    else
    {
        uiOrdering.add( &m_K0FGSource );
        uiOrdering.add( &m_userDefinedK0FG );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWbsParameters::hasLasFileWithChannel( const QString& channel ) const
{
    if ( m_wellPath && !RimWellLogFile::findMdAndChannelValuesForWellPath( m_wellPath, channel ).empty() )
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWbsParameters::hasElementPropertyEntry( const RigFemResultAddress& resAddr ) const
{
    RigFemPartResultsCollection* femPartResults = nullptr;
    if ( m_geoMechCase && m_timeStep > 0 )
    {
        femPartResults = m_geoMechCase->geoMechData()->femPartResults();
        if ( femPartResults )
        {
            return !femPartResults->resultValues( resAddr, 0, m_timeStep ).empty();
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWbsParameters::assignValidSource( caf::PdmField<ParameterSourceEnum>* parameterSourceField,
                                          const std::vector<ParameterSource>& validSources )
{
    CAF_ASSERT( parameterSourceField );
    if ( std::find( validSources.begin(), validSources.end(), ( *parameterSourceField )() ) == validSources.end() )
    {
        *parameterSourceField = validSources.front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWbsParameters::ParameterSource> RimWbsParameters::supportedSources( const RigWbsParameter& parameter ) const
{
    std::vector<RigGeoMechWellLogExtractor::WbsParameterSource> sources;

    for ( auto source : parameter.sources() )
    {
        if ( source == RigWbsParameter::LAS_FILE )
        {
            if ( hasLasFileWithChannel( parameter.addressString( RigWbsParameter::LAS_FILE ) ) )
            {
                sources.push_back( source );
            }
        }
        else if ( source == RigWbsParameter::ELEMENT_PROPERTY_TABLE )
        {
            RigFemResultAddress resAddr = parameter.femAddress( RigWbsParameter::ELEMENT_PROPERTY_TABLE );
            if ( hasElementPropertyEntry( resAddr ) )
            {
                sources.push_back( source );
            }
        }
        else
        {
            sources.push_back( source );
        }
    }
    return sources;
}
