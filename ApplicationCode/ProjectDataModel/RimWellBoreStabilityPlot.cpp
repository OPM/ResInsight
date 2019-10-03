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
#include "RimWellBoreStabilityPlot.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimTools.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogFile.h"

#include "cafPdmBase.h"
#include "cafPdmObject.h"
#include "cafPdmUiGroup.h"

namespace caf
{
template <>
void RimWellBoreStabilityPlot::ParameterSourceEnum::setUp()
{
    addItem( RigGeoMechWellLogExtractor::AUTO, "AUTO", "Automatic" );
    addItem( RigGeoMechWellLogExtractor::GRID, "GRID", "Grid" );
    addItem( RigGeoMechWellLogExtractor::LAS_FILE, "LAS_FILE", "LAS File" );
    addItem( RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE, "ELEMENT_PROPERTY_TABLE", "Element Property Table" );
    addItem( RigGeoMechWellLogExtractor::USER_DEFINED, "USER_DEFINED", "User Defined" );
    addItem( RigGeoMechWellLogExtractor::HYDROSTATIC_PP, "HYDROSTATIC_PP", "Hydrostatic" );
    setDefault( RigGeoMechWellLogExtractor::AUTO );
}
} // End namespace caf

CAF_PDM_SOURCE_INIT( RimWellBoreStabilityPlot, "WellBoreStabilityPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellBoreStabilityPlot::RimWellBoreStabilityPlot()
{
    CAF_PDM_InitObject( "Well Bore Stability Plot", ":/WellLogPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_porePressureSource,
                                "PorePressureSource",
                                "Pore Pressure",
                                "",
                                "Data source for Pore Pressure",
                                "" );
    CAF_PDM_InitFieldNoDefault( &m_poissonRatioSource,
                                "PoissionRatioSource",
                                "Poisson Ratio",
                                "",
                                "Data source for Poisson Ratio",
                                "" );
    CAF_PDM_InitFieldNoDefault( &m_ucsSource, "UcsSource", "Uniaxial Compressive Strength", "", "Data source for UCS", "" );

    CAF_PDM_InitField( &m_userDefinedPoissionRatio,
                       "UserPoissionRatio",
                       0.25,
                       "User defined Poisson Ratio",
                       "",
                       "User defined Poisson Ratio",
                       "" );
    // Typical UCS: http://ceae.colorado.edu/~amadei/CVEN5768/PDF/NOTES8.pdf
    // Typical UCS for Shale is 5 - 100 MPa -> 50 - 1000 bar.
    CAF_PDM_InitField( &m_userDefinedUcs, "UserUcs", 100.0, "User defined UCS [bar]", "", "User defined UCS [bar]", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::WbsParameterSource RimWellBoreStabilityPlot::porePressureSource() const
{
    return m_porePressureSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::WbsParameterSource RimWellBoreStabilityPlot::poissonRatioSource() const
{
    return m_poissonRatioSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::WbsParameterSource RimWellBoreStabilityPlot::ucsSource() const
{
    return m_ucsSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellBoreStabilityPlot::userDefinedPoissonRatio() const
{
    return m_userDefinedPoissionRatio();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellBoreStabilityPlot::userDefinedUcs() const
{
    return m_userDefinedUcs();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_commonDataSource->uiOrdering( uiConfigName, uiOrdering );

    caf::PdmUiGroup* parameterSources = uiOrdering.addNewGroup( "Parameter Sources" );
    parameterSources->add( &m_porePressureSource );
    parameterSources->add( &m_poissonRatioSource );
    parameterSources->add( &m_userDefinedPoissionRatio );
    parameterSources->add( &m_ucsSource );
    parameterSources->add( &m_userDefinedUcs );

    uiOrderingForDepthAxis( uiOrdering );
    uiOrderingForPlotSettings( uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellBoreStabilityPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimWellLogPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_porePressureSource )
    {
        for ( auto source : supportedSourcesForPorePressure() )
        {
            options.push_back( caf::PdmOptionItemInfo( ParameterSourceEnum::uiText( source ), source ) );
        }
    }
    else if ( fieldNeedingOptions == &m_poissonRatioSource )
    {
        for ( auto source : supportedSourcesForPoisson() )
        {
            options.push_back( caf::PdmOptionItemInfo( ParameterSourceEnum::uiText( source ), source ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ucsSource )
    {
        for ( auto source : supportedSourcesForUcs() )
        {
            options.push_back( caf::PdmOptionItemInfo( ParameterSourceEnum::uiText( source ), source ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    RimWellLogPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_porePressureSource || changedField == &m_poissonRatioSource || changedField == &m_ucsSource ||
         changedField == &m_userDefinedPoissionRatio || changedField == &m_userDefinedUcs )
    {
        this->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellBoreStabilityPlot::onLoadDataAndUpdate()
{
    if ( !supportedSourcesForPorePressure().count( m_porePressureSource() ) )
    {
        m_porePressureSource = RigGeoMechWellLogExtractor::AUTO;
    }

    if ( !supportedSourcesForPoisson().count( m_poissonRatioSource() ) )
    {
        m_poissonRatioSource = RigGeoMechWellLogExtractor::AUTO;
    }

    if ( !supportedSourcesForUcs().count( m_ucsSource() ) )
    {
        m_ucsSource = RigGeoMechWellLogExtractor::AUTO;
    }

    RimWellLogPlot::onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellBoreStabilityPlot::hasLasFileWithChannel( const QString& channel ) const
{
    RimWellPath* wellPath = m_commonDataSource->wellPathToApply();
    if ( wellPath && !RimWellLogFile::findMdAndChannelValuesForWellPath( wellPath, channel ).empty() )
    {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellBoreStabilityPlot::hasElementPropertyEntry( const RigFemResultAddress& resAddr ) const
{
    int             timeStep    = m_commonDataSource->timeStepToApply();
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( m_commonDataSource->caseToApply() );

    RigFemPartResultsCollection* femPartResults = nullptr;
    if ( geoMechCase && timeStep > 0 )
    {
        femPartResults = geoMechCase->geoMechData()->femPartResults();
        if ( femPartResults )
        {
            return !femPartResults->resultValues( resAddr, 0, timeStep ).empty();
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigGeoMechWellLogExtractor::WbsParameterSource> RimWellBoreStabilityPlot::supportedSourcesForPorePressure() const
{
    std::set<RigGeoMechWellLogExtractor::WbsParameterSource> sources;

    for ( auto source : RigGeoMechWellLogExtractor::supportedSourcesForPorePressure() )
    {
        if ( source == RigGeoMechWellLogExtractor::LAS_FILE )
        {
            if ( hasLasFileWithChannel( "PP" ) )
            {
                sources.insert( source );
            }
        }
        else if ( source == RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE )
        {
            RigFemResultAddress resAddr( RIG_ELEMENT, "POR", "" );
            if ( hasElementPropertyEntry( resAddr ) )
            {
                sources.insert( source );
            }
        }
        else
        {
            sources.insert( source );
        }
    }
    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigGeoMechWellLogExtractor::WbsParameterSource> RimWellBoreStabilityPlot::supportedSourcesForPoisson() const
{
    std::set<RigGeoMechWellLogExtractor::WbsParameterSource> sources;

    for ( auto source : RigGeoMechWellLogExtractor::supportedSourcesForPoissonRatio() )
    {
        if ( source == RigGeoMechWellLogExtractor::LAS_FILE )
        {
            if ( hasLasFileWithChannel( "POISSON_RATIO" ) )
            {
                sources.insert( source );
            }
        }
        else if ( source == RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE )
        {
            RigFemResultAddress resAddr( RIG_ELEMENT, "RATIO", "" );
            if ( hasElementPropertyEntry( resAddr ) )
            {
                sources.insert( source );
            }
        }
        else
        {
            sources.insert( source );
        }
    }
    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigGeoMechWellLogExtractor::WbsParameterSource> RimWellBoreStabilityPlot::supportedSourcesForUcs() const
{
    std::set<RigGeoMechWellLogExtractor::WbsParameterSource> sources;

    for ( auto source : RigGeoMechWellLogExtractor::supportedSourcesForUcs() )
    {
        if ( source == RigGeoMechWellLogExtractor::LAS_FILE )
        {
            if ( hasLasFileWithChannel( "UCS" ) )
            {
                sources.insert( source );
            }
        }
        else if ( source == RigGeoMechWellLogExtractor::ELEMENT_PROPERTY_TABLE )
        {
            RigFemResultAddress resAddr( RIG_ELEMENT, "UCS", "" );
            if ( hasElementPropertyEntry( resAddr ) )
            {
                sources.insert( source );
            }
        }
        else
        {
            sources.insert( source );
        }
    }
    return sources;
}
