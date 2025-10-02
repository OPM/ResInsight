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

#include "RimKeywordWconinje.h"

#include "RifOpmDeckTools.h"

#include "cafPdmFieldCapability.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

CAF_PDM_SOURCE_INIT( RimKeywordWconinje, "KeywordWconinje" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconinje::RimKeywordWconinje()
{
    CAF_PDM_InitObject( "WCONINJE Keyword" );
    CAF_PDM_InitField( &m_type, "type", QString( "WAT" ), "Injection Type" );
    CAF_PDM_InitField( &m_status, "status", QString( "OPEN" ), "Well Status" );
    CAF_PDM_InitField( &m_target, "target", QString( "RATE" ), "Target Injection Control Mode" );
    CAF_PDM_InitFieldNoDefault( &m_rate, "rate", "Max Surface Injection Rate" );
    CAF_PDM_InitFieldNoDefault( &m_resv, "resv", "Max Reservoir Volume Rate" );
    CAF_PDM_InitFieldNoDefault( &m_bhp, "bhp", "Max Bottom Hole Pressure" );
    CAF_PDM_InitFieldNoDefault( &m_thp, "thp", "Max Tubing Head Pressure" );
    CAF_PDM_InitFieldNoDefault( &m_vfptab, "vfptab", "VFP Table Index" );
    CAF_PDM_InitFieldNoDefault( &m_rsrvinj, "rsrvinj", "Gas<->Oil ratio" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconinje::~RimKeywordWconinje()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimKeywordWconinje::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_status )
    {
        options.push_back( caf::PdmOptionItemInfo( "OPEN", QVariant::fromValue( QString( "OPEN" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "STOP", QVariant::fromValue( QString( "STOP" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "SHUT", QVariant::fromValue( QString( "SHUT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "AUTO", QVariant::fromValue( QString( "AUTO" ) ) ) );
    }
    else if ( fieldNeedingOptions == &m_type )
    {
        options.push_back( caf::PdmOptionItemInfo( "WATER", QVariant::fromValue( QString( "WAT" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "GAS", QVariant::fromValue( QString( "GAS" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "OIL", QVariant::fromValue( QString( "OIL" ) ) ) );
    }
    else if ( fieldNeedingOptions == &m_target )
    {
        options.push_back( caf::PdmOptionItemInfo( "RATE", QVariant::fromValue( QString( "RATE" ) ) ) );
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
void RimKeywordWconinje::uiOrdering( caf::PdmUiGroup* uiGroup )
{
    uiGroup->add( &m_type );
    uiGroup->add( &m_status );
    uiGroup->add( &m_target );
    uiGroup->add( &m_resv );
    uiGroup->add( &m_bhp );
    uiGroup->add( &m_thp );
    uiGroup->add( &m_vfptab );
    uiGroup->add( &m_rsrvinj );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword RimKeywordWconinje::keyword( const QString& wellName )
{
    using W = Opm::ParserKeywords::WCONINJE;

    std::vector<Opm::DeckItem> items;

    items.push_back( RifOpmDeckTools::item( W::WELL::itemName, wellName.toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::TYPE::itemName, m_type().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::STATUS::itemName, m_status().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::CMODE::itemName, m_target().toStdString() ) );
    items.push_back( m_resv().has_value() ? RifOpmDeckTools::item( W::RESV::itemName, m_resv().value() )
                                          : RifOpmDeckTools::defaultItem( W::RESV::itemName ) );
    items.push_back( m_bhp().has_value() ? RifOpmDeckTools::item( W::BHP::itemName, m_bhp().value() )
                                         : RifOpmDeckTools::defaultItem( W::BHP::itemName ) );
    items.push_back( m_thp().has_value() ? RifOpmDeckTools::item( W::THP::itemName, m_thp().value() )
                                         : RifOpmDeckTools::defaultItem( W::THP::itemName ) );
    items.push_back( m_vfptab().has_value() ? RifOpmDeckTools::item( W::VFP_TABLE::itemName, m_vfptab().value() )
                                            : RifOpmDeckTools::defaultItem( W::VFP_TABLE::itemName ) );
    items.push_back( m_rsrvinj().has_value() ? RifOpmDeckTools::item( W::VAPOIL_C::itemName, m_rsrvinj().value() )
                                             : RifOpmDeckTools::defaultItem( W::VAPOIL_C::itemName ) );

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::WCONINJE() ) );
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    return kw;
}
