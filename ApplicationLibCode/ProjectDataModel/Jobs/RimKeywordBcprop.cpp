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

#include "RimKeywordBcprop.h"

#include "RifOpmDeckTools.h"

#include "cafPdmFieldCapability.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"

CAF_PDM_SOURCE_INIT( RimKeywordBcprop, "KeywordBcprop" );

namespace caf
{
template <>
void caf::AppEnum<RimKeywordBcprop::Type>::setUp()
{
    addItem( RimKeywordBcprop::Type::DIRICHLET, "DIRICHLET", "DIRICHLET" );
    addItem( RimKeywordBcprop::Type::FREE, "FREE", "FREE" );
    addItem( RimKeywordBcprop::Type::RATE, "RATE", "RATE" );
    addItem( RimKeywordBcprop::Type::THERMAL, "THERMAL", "THERMAL" );
    addItem( RimKeywordBcprop::Type::NONE, "NONE", "NONE" );
    setDefault( RimKeywordBcprop::Type::NONE );
}

template <>
void caf::AppEnum<RimKeywordBcprop::Component>::setUp()
{
    addItem( RimKeywordBcprop::Component::GAS, "GAS", "GAS" );
    addItem( RimKeywordBcprop::Component::OIL, "OIL", "OIL" );
    addItem( RimKeywordBcprop::Component::WATER, "WATER", "WATER" );
    addItem( RimKeywordBcprop::Component::SOLVET, "SOLVET", "SOLVET" );
    addItem( RimKeywordBcprop::Component::POLYMER, "POLYMER", "POLYMER" );
    addItem( RimKeywordBcprop::Component::MICR, "MICR", "MICR" );
    addItem( RimKeywordBcprop::Component::OXYG, "OXYG", "OXYG" );
    addItem( RimKeywordBcprop::Component::UREA, "UREA", "UREA" );
    addItem( RimKeywordBcprop::Component::NONE, "NONE", "NONE" );
    setDefault( RimKeywordBcprop::Component::NONE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordBcprop::RimKeywordBcprop()
{
    CAF_PDM_InitObject( "BCPROP Keyword" );
    CAF_PDM_InitField( &m_index, "index", 0, "Index" );
    CAF_PDM_InitField( &m_type, "type", caf::AppEnum<Type>( Type::NONE ), "Type" );
    CAF_PDM_InitField( &m_component, "component", caf::AppEnum<Component>( Component::NONE ), "Component" );
    CAF_PDM_InitField( &m_rate, "rate", 0.0, "Rate" );
    CAF_PDM_InitFieldNoDefault( &m_press, "press", "Pressure" );
    CAF_PDM_InitFieldNoDefault( &m_temp, "temp", "Temperature" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordBcprop::~RimKeywordBcprop()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordBcprop::uiOrdering( caf::PdmUiGroup* uiGroup )
{
    uiGroup->add( &m_index );
    uiGroup->add( &m_type );
    uiGroup->add( &m_component );
    uiGroup->add( &m_rate );
    uiGroup->add( &m_press );
    uiGroup->add( &m_temp );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckKeyword RimKeywordBcprop::keyword()
{
    using B = Opm::ParserKeywords::BCPROP;

    std::vector<Opm::DeckItem> items;

    items.push_back( RifOpmDeckTools::item( B::INDEX::itemName, m_index() ) );
    items.push_back( RifOpmDeckTools::item( B::TYPE::itemName, m_type().text().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( B::COMPONENT::itemName, m_component().text().toStdString() ) );
    items.push_back( RifOpmDeckTools::item( B::RATE::itemName, m_rate() ) );
    items.push_back( m_press().has_value() ? RifOpmDeckTools::item( B::PRESSURE::itemName, m_press().value() )
                                           : RifOpmDeckTools::defaultItem( B::PRESSURE::itemName ) );
    items.push_back( m_temp().has_value() ? RifOpmDeckTools::item( B::TEMPERATURE::itemName, m_temp().value() )
                                          : RifOpmDeckTools::defaultItem( B::TEMPERATURE::itemName ) );

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::BCPROP() ) );
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    return kw;
}
