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
    setDefault( RimKeywordBcprop::Type::FREE );
}

template <>
void caf::AppEnum<RimKeywordBcprop::Component>::setUp()
{
    addItem( RimKeywordBcprop::Component::GAS, "GAS", "GAS" );
    addItem( RimKeywordBcprop::Component::OIL, "OIL", "OIL" );
    addItem( RimKeywordBcprop::Component::WATER, "WATER", "WATER" );
    addItem( RimKeywordBcprop::Component::SOLVENT, "SOLVENT", "SOLVENT" );
    addItem( RimKeywordBcprop::Component::POLYMER, "POLYMER", "POLYMER" );
    addItem( RimKeywordBcprop::Component::MICR, "MICR", "MICR" );
    addItem( RimKeywordBcprop::Component::OXYG, "OXYG", "OXYG" );
    addItem( RimKeywordBcprop::Component::UREA, "UREA", "UREA" );
    addItem( RimKeywordBcprop::Component::NONE, "NONE", "NONE" );
    setDefault( RimKeywordBcprop::Component::NONE );
}

template <>
void caf::AppEnum<RimKeywordBcprop::MechType>::setUp()
{
    addItem( RimKeywordBcprop::MechType::FREE, "FREE", "FREE" );
    addItem( RimKeywordBcprop::MechType::FIXED, "FIXED", "FIXED" );
    addItem( RimKeywordBcprop::MechType::NONE, "NONE", "NONE" );
    setDefault( RimKeywordBcprop::MechType::NONE );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordBcprop::RimKeywordBcprop()
{
    CAF_PDM_InitObject( "BCPROP Keyword" );
    CAF_PDM_InitField( &m_index, "index", 0, "Index" );
    m_index.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_type, "type", caf::AppEnum<Type>( Type::FREE ), "Type" );
    CAF_PDM_InitField( &m_component, "component", caf::AppEnum<Component>( Component::NONE ), "Comp." );
    CAF_PDM_InitField( &m_rate, "rate", 0.0, "Rate" );
    CAF_PDM_InitFieldNoDefault( &m_press, "press", "Pressure" );
    CAF_PDM_InitFieldNoDefault( &m_temp, "temp", "Temp." );
    CAF_PDM_InitFieldNoDefault( &m_mechType, "mechType", "Mech. Type" );
    CAF_PDM_InitField( &m_fixedX, "fixedX", (size_t)1, "Fixed X" );
    CAF_PDM_InitField( &m_fixedY, "fixedY", (size_t)1, "Fixed Y" );
    CAF_PDM_InitField( &m_fixedZ, "fixedZ", (size_t)1, "Fixed Z" );
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
void RimKeywordBcprop::setIndex( int index )
{
    m_index = index;
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
    uiGroup->add( &m_mechType );
    uiGroup->add( &m_fixedX );
    uiGroup->add( &m_fixedY );
    uiGroup->add( &m_fixedZ );
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
    if ( m_type() != RimKeywordBcprop::Type::FREE )
    {
        items.push_back( RifOpmDeckTools::item( B::COMPONENT::itemName, m_component().text().toStdString() ) );
        items.push_back( RifOpmDeckTools::item( B::RATE::itemName, m_rate() ) );
        items.push_back( RifOpmDeckTools::optionalItem( B::PRESSURE::itemName, m_press() ) );
        items.push_back( RifOpmDeckTools::optionalItem( B::TEMPERATURE::itemName, m_temp() ) );
        items.push_back( RifOpmDeckTools::item( B::MECHTYPE::itemName, m_mechType().text().toStdString() ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDX::itemName, m_fixedX() ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDY::itemName, m_fixedY() ) );
        items.push_back( RifOpmDeckTools::item( B::FIXEDZ::itemName, m_fixedZ() ) );
    }

    Opm::DeckKeyword kw( ( Opm::ParserKeywords::BCPROP() ) );
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    return kw;
}
