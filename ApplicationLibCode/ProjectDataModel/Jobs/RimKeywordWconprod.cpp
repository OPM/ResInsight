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

#include "RifOpmDeckTools.h"

#include "cafPdmFieldCapability.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

CAF_PDM_SOURCE_INIT( RimKeywordWconprod, "KeywordWconprod" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordWconprod::RimKeywordWconprod()
{
    CAF_PDM_InitObject( "WCONPROD Keyword" );
    CAF_PDM_InitField( &m_status, "status", QString( "OPEN" ), "Well Status" );
    CAF_PDM_InitField( &m_target, "target", QString( "LRAT" ), "Target Production Phase" );
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
void RimKeywordWconprod::uiOrdering( caf::PdmUiGroup* uiGroup )
{
    uiGroup->add( &m_status );
    uiGroup->add( &m_target );
    uiGroup->add( &m_orat );
    uiGroup->add( &m_wrat );
    uiGroup->add( &m_grat );
    uiGroup->add( &m_lrat );
    uiGroup->add( &m_resv );
    uiGroup->add( &m_bhp );
    uiGroup->add( &m_thp );
    uiGroup->add( &m_vfptab );
    uiGroup->add( &m_alqWell );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword RimKeywordWconprod::keyword( const QString& wellName )
{
    using W = Opm::ParserKeywords::WCONPROD;

    std::vector<Opm::DeckItem> items;

    items.push_back( RifOpmDeckTools::item( W::WELL::itemName, wellName.toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::STATUS::itemName, m_status().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::CMODE::itemName, m_target().toStdString() ) );
    items.push_back( m_orat().has_value() ? RifOpmDeckTools::item( W::ORAT::itemName, m_orat().value() )
                                          : RifOpmDeckTools::defaultItem( W::ORAT::itemName ) );
    items.push_back( m_wrat().has_value() ? RifOpmDeckTools::item( W::WRAT::itemName, m_wrat().value() )
                                          : RifOpmDeckTools::defaultItem( W::WRAT::itemName ) );
    items.push_back( m_grat().has_value() ? RifOpmDeckTools::item( W::GRAT::itemName, m_grat().value() )
                                          : RifOpmDeckTools::defaultItem( W::GRAT::itemName ) );
    items.push_back( m_lrat().has_value() ? RifOpmDeckTools::item( W::LRAT::itemName, m_lrat().value() )
                                          : RifOpmDeckTools::defaultItem( W::LRAT::itemName ) );
    items.push_back( m_resv().has_value() ? RifOpmDeckTools::item( W::RESV::itemName, m_resv().value() )
                                          : RifOpmDeckTools::defaultItem( W::RESV::itemName ) );
    items.push_back( m_bhp().has_value() ? RifOpmDeckTools::item( W::BHP::itemName, m_bhp().value() )
                                         : RifOpmDeckTools::defaultItem( W::BHP::itemName ) );
    items.push_back( m_thp().has_value() ? RifOpmDeckTools::item( W::THP::itemName, m_thp().value() )
                                         : RifOpmDeckTools::defaultItem( W::THP::itemName ) );
    items.push_back( m_vfptab().has_value() ? RifOpmDeckTools::item( W::VFP_TABLE::itemName, m_vfptab().value() )
                                            : RifOpmDeckTools::defaultItem( W::VFP_TABLE::itemName ) );
    items.push_back( m_alqWell().has_value() ? RifOpmDeckTools::item( W::ALQ::itemName, m_alqWell().value() )
                                             : RifOpmDeckTools::defaultItem( W::ALQ::itemName ) );

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::WCONPROD() ) );
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    return kw;
}
