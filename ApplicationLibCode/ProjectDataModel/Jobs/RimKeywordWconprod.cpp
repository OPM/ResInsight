/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimKeywordWconprod.h"

#include "cafPdmFieldCapability.h"

CAF_PDM_SOURCE_INIT( RimKeywordWconprod, "KeywordWconprod" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconprod::RimKeywordWconprod()
{
    CAF_PDM_InitObject( "WCONPROD Keyword" );
    CAF_PDM_InitField( &m_wellName, "wellName", QString(), "Well Name" );
    m_wellName.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_status, "status", QString( "OPEN" ), "Well Status" );
    CAF_PDM_InitField( &m_target, "target", QString(), "Target Production Phase" );
    CAF_PDM_InitFieldNoDefault( &m_orat, "orat", "Max Surface Oil Production Rate" );
    CAF_PDM_InitFieldNoDefault( &m_wrat, "wrat", "Max Surface Water Production Rate" );
    CAF_PDM_InitFieldNoDefault( &m_grat, "grat", "Max Surface Gas Production Rate" );
    CAF_PDM_InitFieldNoDefault( &m_lrat, "lrat", "Max Surface Liquid Production Rate" );
    CAF_PDM_InitFieldNoDefault( &m_resv, "resv", "Max Reservoir Volume Rate" );
    CAF_PDM_InitFieldNoDefault( &m_bhp, "bhp", "Max Bottom Hole Pressure" );
    CAF_PDM_InitFieldNoDefault( &m_thp, "thp", "Min Tubing Head Pressure" );
    CAF_PDM_InitFieldNoDefault( &m_vfptab, "vfptab", "VFP Table Index" );
    CAF_PDM_InitFieldNoDefault( &m_alqWell, "alqWell", "Artificial Lift Quantity" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconprod::~RimKeywordWconprod()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimKeywordWconprod::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_status )
    {
        options.push_back( caf::PdmOptionItemInfo( "OPEN", QVariant::fromValue( QString( "OPEN" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "STOP", QVariant::fromValue( QString( "STOP" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "SHUT", QVariant::fromValue( QString( "SHUT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "AUTO", QVariant::fromValue( QString( "AUTO" ) ) ) );
    }
    else if ( fieldNeedingOptions == &m_target )
    {
        options.push_back( caf::PdmOptionItemInfo( "ORAT", QVariant::fromValue( QString( "ORAT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "WRAT", QVariant::fromValue( QString( "WRAT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "GRAT", QVariant::fromValue( QString( "GRAT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "LRAT", QVariant::fromValue( QString( "LRAT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "RESV", QVariant::fromValue( QString( "RESV" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "BHP", QVariant::fromValue( QString( "BHP" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "THP", QVariant::fromValue( QString( "THP" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "GRUP", QVariant::fromValue( QString( "GRUP" ) ) ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordWconprod::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_wellName );
    uiOrdering.add( &m_status );
    uiOrdering.add( &m_target );
    uiOrdering.add( &m_orat );
    uiOrdering.add( &m_wrat );
    uiOrdering.add( &m_grat );
    uiOrdering.add( &m_lrat );
    uiOrdering.add( &m_resv );
    uiOrdering.add( &m_bhp );
    uiOrdering.add( &m_thp );
    uiOrdering.add( &m_vfptab );
    uiOrdering.add( &m_alqWell );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimKeywordWconprod::recordAsString() const
{
    QString record = m_wellName();

    QString defStr = " 1*";

    if ( !m_status().isEmpty() )
    {
        record += " " + m_status;
    }
    else
    {
        record += defStr;
    }

    record += " " + m_status();
    if ( !m_target().isEmpty() )
    {
        record += " " + m_target();
    }

    record += "/\n";

    return record;
}
