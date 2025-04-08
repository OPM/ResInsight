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

#include "RifOpmFlowDeckFile.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/FileDeck.hpp"
#include "opm/input/eclipse/Parser/ErrorGuard.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmFlowDeckFile::RifOpmFlowDeckFile()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmFlowDeckFile::~RifOpmFlowDeckFile()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::loadDeck( std::string filename )
{
    Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );

    // Use the same default ParseContext as flow.
    parseContext.update( Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputErrorAction::IGNORE );
    parseContext.update( Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputErrorAction::WARN );

    Opm::ErrorGuard errors{};

    auto deck = Opm::Parser{}.parseFile( filename, parseContext, errors );

    m_fileDeck.reset( new Opm::FileDeck( deck ) );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::saveDeck( std::string folder, std::string filename )
{
    if ( m_fileDeck.get() != nullptr )
    {
        m_fileDeck->dump( folder, filename, Opm::FileDeck::OutputMode::COPY );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem RifOpmFlowDeckFile::item( std::string name, std::string value )
{
    Opm::DeckItem item1( name, value );
    item1.push_back( value );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem RifOpmFlowDeckFile::item( std::string name, int value )
{
    Opm::DeckItem item1( name, value );
    item1.push_back( value );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem RifOpmFlowDeckFile::defaultItem( std::string name, int cols )
{
    Opm::DeckItem item1( name, 0 );
    item1.push_backDummyDefault<int>( cols );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::mergeWellDeck( std::string filename )
{
    Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );

    // Use the same default ParseContext as flow.
    parseContext.update( Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputErrorAction::IGNORE );
    parseContext.update( Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputErrorAction::WARN );

    Opm::ErrorGuard errors{};

    auto          deck = Opm::Parser{}.parseFile( filename, parseContext, errors );
    Opm::FileDeck wellDeck( deck );

    // Insert new well into WELSPECS
    {
        const auto wellspec2 = wellDeck.find( "WELSPECS" );
        if ( !wellspec2.has_value() ) return false;
        auto& wellspec2_deck = wellDeck.operator[]( wellspec2.value() );

        Opm::DeckRecord newWellRec( wellspec2_deck.getRecord( 0 ) );

        const auto wellspec = m_fileDeck->find( "WELSPECS" );
        if ( !wellspec.has_value() ) return false;
        auto& wellspec_pos  = wellspec.value();
        auto& wellspec_deck = m_fileDeck->operator[]( wellspec_pos );

        Opm::DeckKeyword newWellspec( wellspec_deck );

        newWellspec.addRecord( std::move( newWellRec ) );

        m_fileDeck->erase( wellspec_pos );
        m_fileDeck->insert( wellspec_pos, newWellspec );
    }

    // Insert new well data into COMPDAT
    {
        const auto compdat2 = wellDeck.find( "COMPDAT" );
        if ( !compdat2.has_value() ) return false;
        auto& compdat2_kw = wellDeck.operator[]( compdat2.value() );

        const auto compdat = m_fileDeck->find( "COMPDAT" );
        if ( !compdat.has_value() ) return false;
        auto& compdat_pos = compdat.value();
        auto& compdat_kw  = m_fileDeck->operator[]( compdat_pos );

        Opm::DeckKeyword newCompdat( compdat_kw );

        for ( size_t i = 0; i < compdat2_kw.size(); i++ )
        {
            Opm::DeckRecord newWellRec( compdat2_kw.getRecord( i ) );
            newCompdat.addRecord( std::move( newWellRec ) );
        }

        m_fileDeck->erase( compdat_pos );
        m_fileDeck->insert( compdat_pos, newCompdat );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::openWellAtTimeStep( int timeStep, std::string filename )
{
    Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );

    // Use the same default ParseContext as flow.
    parseContext.update( Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputErrorAction::IGNORE );
    parseContext.update( Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputErrorAction::WARN );

    Opm::ErrorGuard errors{};

    int stepCount = 0;

    // locate dates keyword for the selected step
    Opm::FileDeck::Index* datePos = nullptr;
    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() != "DATES" ) continue;

        if ( stepCount == timeStep )
        {
            datePos = &it;

            auto             deck = Opm::Parser{}.parseFile( filename, parseContext, errors );
            Opm::FileDeck    wellDeck( deck );
            Opm::DeckKeyword newKw( wellDeck.operator[]( wellDeck.start() ) );

            Opm::FileDeck::Index openPos( *datePos );
            openPos--;

            m_fileDeck->insert( openPos, newKw );

            return true;
        }
        stepCount++;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::openWellAtStart( std::string filename )
{
    Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );

    // Use the same default ParseContext as flow.
    parseContext.update( Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputErrorAction::IGNORE );
    parseContext.update( Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputErrorAction::WARN );
    parseContext.update( Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputErrorAction::WARN );

    Opm::ErrorGuard errors{};

    // find position of COMPDAT
    const auto compdat = m_fileDeck->find( "COMPDAT" );
    if ( !compdat.has_value() ) return false;

    // insert open kw after compdat
    Opm::FileDeck::Index openPos( *compdat );
    openPos++;

    auto             deck = Opm::Parser{}.parseFile( filename, parseContext, errors );
    Opm::FileDeck    wellDeck( deck );
    Opm::DeckKeyword openWellKw( wellDeck.operator[]( wellDeck.start() ) );

    m_fileDeck->insert( openPos, openWellKw );

    return true;
}
