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
#include "opm/input/eclipse/Parser/ParserKeywords/D.hpp"

#include <format>
#include <memory>
#include <optional>

namespace internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::optional<Opm::FileDeck::Index> locateTimeStep( std::unique_ptr<Opm::FileDeck>& fileDeck, int timeStep )
{
    Opm::ErrorGuard errors{};

    int currentStep = 1;

    // locate dates keyword for the selected step
    for ( auto it = fileDeck->start(); it != fileDeck->stop(); it++ )
    {
        auto& kw = fileDeck->operator[]( it );
        if ( kw.name() != "DATES" ) continue;

        if ( currentStep == timeStep )
        {
            return it;
        }
        currentStep++;
    }
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::optional<Opm::FileDeck::Index> locateKeywordAtTimeStep( std::unique_ptr<Opm::FileDeck>& fileDeck, int timeStep, std::string keyword )
{
    Opm::ErrorGuard errors{};

    auto startPos = internal::locateTimeStep( fileDeck, timeStep );
    if ( startPos.has_value() )
    {
        auto startIdx = startPos.value();
        startIdx--;
        // locate keyword for the selected step, break if another date is found
        for ( auto it = startIdx; it != fileDeck->start(); it-- )
        {
            auto& kw = fileDeck->operator[]( it );
            if ( kw.name() == "DATES" )
            {
                break;
            }
            else if ( kw.name() == "SCHEDULE" )
            {
                break;
            }
            else if ( kw.name() == keyword )
            {
                return it;
            }
        }
    }
    return std::nullopt;
}
} // namespace internal

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
    Opm::ErrorGuard errors{};

    auto deck = Opm::Parser{}.parseFile( filename, defaultParseContext(), errors );

    m_fileDeck = std::make_unique<Opm::FileDeck>( deck );

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
bool RifOpmFlowDeckFile::mergeWellDeck( int timeStep, std::string filename )
{
    Opm::ErrorGuard errors{};

    auto deckToMerge = Opm::Parser{}.parseFile( filename, defaultParseContext(), errors );

    const auto welspecsIndexes = deckToMerge.index( "WELSPECS" );
    if ( welspecsIndexes.empty() ) return false;
    auto&           mergeWelspecsKw = deckToMerge[welspecsIndexes[0]];
    Opm::DeckRecord newWelspecRecToAdd( mergeWelspecsKw.getRecord( 0 ) );

    const auto compdatIndexes = deckToMerge.index( "COMPDAT" );
    if ( compdatIndexes.empty() ) return false;
    auto& mergeCompdatKw = deckToMerge[compdatIndexes[0]];

    auto datePos = internal::locateTimeStep( m_fileDeck, timeStep );
    if ( datePos.has_value() )
    {
        auto insertPos = datePos.value();
        // Insert new well into WELSPECS in the selected date section
        auto existingKwFound = internal::locateKeywordAtTimeStep( m_fileDeck, timeStep, "WELSPECS" );
        if ( existingKwFound.has_value() )
        {
            auto& welspecs_kw = m_fileDeck->operator[]( existingKwFound.value() );

            Opm::DeckKeyword newWelspecsKw( welspecs_kw );
            newWelspecsKw.addRecord( std::move( newWelspecRecToAdd ) );

            m_fileDeck->erase( existingKwFound.value() );
            m_fileDeck->insert( existingKwFound.value(), newWelspecsKw );
        }
        else
        {
            // existing kw not found, insert a new one
            m_fileDeck->insert( insertPos, mergeWelspecsKw );
            insertPos++;
        }

        // Insert new well data into COMPDAT in the selected date section
        existingKwFound = internal::locateKeywordAtTimeStep( m_fileDeck, timeStep, "COMPDAT" );
        if ( existingKwFound.has_value() )
        {
            auto& compdat_kw = m_fileDeck->operator[]( existingKwFound.value() );

            Opm::DeckKeyword newCompdatKw( compdat_kw );
            for ( size_t i = 0; i < mergeCompdatKw.size(); i++ )
            {
                Opm::DeckRecord newRecToAdd( mergeCompdatKw.getRecord( i ) );
                newCompdatKw.addRecord( std::move( newRecToAdd ) );
            }

            m_fileDeck->erase( existingKwFound.value() );
            m_fileDeck->insert( existingKwFound.value(), newCompdatKw );
        }
        else
        {
            // existing kw not found, insert a new one
            m_fileDeck->insert( insertPos, mergeCompdatKw );
        }
    }
    else
    {
        // Insert new well into main WELSPECS
        {
            const auto foundWelspecs = m_fileDeck->find( "WELSPECS" );
            if ( !foundWelspecs.has_value() ) return false;
            auto& existing_pos = foundWelspecs.value();
            auto& welspecs_kw  = m_fileDeck->operator[]( existing_pos );

            Opm::DeckKeyword newWelspecsKw( welspecs_kw );
            newWelspecsKw.addRecord( std::move( newWelspecRecToAdd ) );

            m_fileDeck->erase( existing_pos );
            m_fileDeck->insert( existing_pos, newWelspecsKw );
        }

        // Insert new well data into main COMPDAT
        {
            const auto foundCompdat = m_fileDeck->find( "COMPDAT" );
            if ( !foundCompdat.has_value() ) return false;
            auto& existing_pos = foundCompdat.value();
            auto& compdat_kw   = m_fileDeck->operator[]( existing_pos );

            Opm::DeckKeyword newCompdatKw( compdat_kw );

            for ( size_t i = 0; i < mergeCompdatKw.size(); i++ )
            {
                Opm::DeckRecord newRecToAdd( mergeCompdatKw.getRecord( i ) );
                newCompdatKw.addRecord( std::move( newRecToAdd ) );
            }

            m_fileDeck->erase( existing_pos );
            m_fileDeck->insert( existing_pos, newCompdatKw );
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::openWellAtTimeStep( int timeStep, std::string openText )
{
    Opm::ErrorGuard errors{};

    auto datePos = internal::locateTimeStep( m_fileDeck, timeStep );
    if ( datePos.has_value() )
    {
        auto             deck = Opm::Parser{}.parseString( openText, defaultParseContext(), errors );
        Opm::DeckKeyword newKw( *deck.begin() );

        m_fileDeck->insert( datePos.value(), newKw );

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::ParseContext RifOpmFlowDeckFile::defaultParseContext()
{
    // Use the same default ParseContext as flow.
    Opm::ParseContext pc( Opm::InputErrorAction::WARN );
    pc.update( Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputErrorAction::IGNORE );
    pc.update( Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputErrorAction::WARN );
    pc.update( Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputErrorAction::WARN );
    pc.update( Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputErrorAction::WARN );

    return pc;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::openWellAtDeckPosition( int deckPosition, std::string openText )
{
    Opm::ErrorGuard errors{};

    // locate position in file deck
    int currentPosition = 0;
    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        if ( currentPosition == deckPosition )
        {
            auto             deck = Opm::Parser{}.parseString( openText, defaultParseContext(), errors );
            Opm::DeckKeyword newKw( *deck.begin() );

            m_fileDeck->insert( it, newKw );

            return true;
        }
        currentPosition++;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOpmFlowDeckFile::keywords()
{
    std::vector<std::string> values;

    if ( m_fileDeck.get() == nullptr ) return values;

    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() == "DATES" )
        {
            const auto& rec   = kw.getRecord( 0 );
            int         day   = rec.getItem( "DAY" ).get<int>( 0 );
            auto        month = rec.getItem( "MONTH" ).getTrimmedString( 0 );
            int         year  = rec.getItem( "YEAR" ).get<int>( 0 );

            std::string dateStr = std::format( "DATES  ({}/{}/{})", day, month, year );
            values.push_back( dateStr );
        }
        else
        {
            values.push_back( kw.name() );
        }
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::hasDatesKeyword()
{
    if ( m_fileDeck.get() == nullptr ) return false;
    auto pos = m_fileDeck->find( "DATES" );
    return pos.has_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::mergeMswData( std::vector<std::string>& mswFileData )
{
    Opm::ErrorGuard errors{};

    int         curTimeStep = 0;
    int         maxSteps    = (int)mswFileData.size();
    std::string prevFileData;

    // locate dates keyword for the selected step
    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() != "DATES" ) continue;

        Opm::FileDeck::Index insertPos( it );

        if ( mswFileData[curTimeStep] != prevFileData )
        {
            auto deck = Opm::Parser{}.parseString( mswFileData[curTimeStep], defaultParseContext(), errors );

            for ( auto kwit = deck.begin(); kwit != deck.end(); kwit++ )
            {
                Opm::DeckKeyword newKw( *kwit );

                if ( ( newKw.name() == "WELSPECS" ) && ( curTimeStep == 0 ) )
                {
                    const auto found = m_fileDeck->find( "WELSPECS" );
                    if ( !found.has_value() )
                    {
                        m_fileDeck->insert( insertPos, newKw );
                        insertPos++;
                    }
                    else
                    {
                        Opm::DeckRecord newRecToAdd( newKw.getRecord( 0 ) );

                        auto& existing_pos = found.value();
                        auto& welspecs_kw  = m_fileDeck->operator[]( existing_pos );

                        Opm::DeckKeyword newWelspecsKw( welspecs_kw );
                        newWelspecsKw.addRecord( std::move( newRecToAdd ) );

                        m_fileDeck->erase( existing_pos );
                        m_fileDeck->insert( existing_pos, newWelspecsKw );
                    }
                }
                else if ( ( newKw.name() == "COMPDAT" ) && ( curTimeStep == 0 ) )
                {
                    const auto found = m_fileDeck->find( "COMPDAT" );
                    if ( !found.has_value() )
                    {
                        m_fileDeck->insert( insertPos, newKw );
                        insertPos++;
                    }
                    else
                    {
                        auto& existing_pos = found.value();
                        auto& compdat_kw   = m_fileDeck->operator[]( existing_pos );

                        Opm::DeckKeyword newCompdatKw( compdat_kw );

                        for ( size_t i = 0; i < newKw.size(); i++ )
                        {
                            Opm::DeckRecord newRecToAdd( newKw.getRecord( i ) );
                            newCompdatKw.addRecord( std::move( newRecToAdd ) );
                        }

                        m_fileDeck->erase( existing_pos );
                        m_fileDeck->insert( existing_pos, newCompdatKw );
                    }
                }
                else
                {
                    m_fileDeck->insert( insertPos, newKw );
                    insertPos++;
                }
            }

            prevFileData = mswFileData[curTimeStep];
        }

        curTimeStep++;
        if ( curTimeStep >= maxSteps ) break;
    }

    return curTimeStep > 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::restartAtTimeStep( int timeStep, std::string deckName )
{
    if ( !m_fileDeck ) return false;

    m_fileDeck->rst_solution( deckName, timeStep );
    m_fileDeck->insert_skiprest();
    return true;
}
