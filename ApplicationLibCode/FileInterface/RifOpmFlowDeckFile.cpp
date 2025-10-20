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

#include "RifOpmDeckTools.h"

#include "RigEclipseResultTools.h"

#include "opm/common/utility/TimeService.hpp"
#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/FileDeck.hpp"
#include "opm/input/eclipse/Parser/ErrorGuard.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/D.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/E.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/G.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/I.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/O.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/R.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include "cvfStructGrid.h"

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
    int currentStep = 0;

    // locate dates keyword for the selected step
    for ( auto it = fileDeck->start(); it != fileDeck->stop(); it++ )
    {
        auto& kw = fileDeck->operator[]( it );
        if ( kw.name() != Opm::ParserKeywords::DATES::keywordName ) continue;

        if ( currentStep == timeStep )
        {
            return ++it;
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
    auto startIdx = internal::locateTimeStep( fileDeck, timeStep );
    if ( startIdx.has_value() )
    {
        auto idx = startIdx.value();
        idx--;
        // locate keyword for the selected step, break if another date is found
        for ( auto it = idx; it != fileDeck->start(); it-- )
        {
            auto& kw = fileDeck->operator[]( it );
            if ( kw.name() == Opm::ParserKeywords::DATES::keywordName )
            {
                break;
            }
            else if ( kw.name() == Opm::ParserKeywords::SCHEDULE::keywordName )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::optional<Opm::FileDeck::Index> positionToIndex( int deckPosition, std::unique_ptr<Opm::FileDeck>& fileDeck )
{
    auto it = fileDeck->start();
    it      = it + deckPosition;
    if ( it != fileDeck->stop() ) return it;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::string datesKeywordToString( const Opm::DeckKeyword& kw )
{
    using D = Opm::ParserKeywords::DATES;
    if ( kw.name() == D::keywordName )
    {
        const auto& rec   = kw.getRecord( 0 );
        int         day   = rec.getItem<D::DAY>().get<int>( 0 );
        auto        month = rec.getItem<D::MONTH>().get<std::string>( 0 );
        int         year  = rec.getItem<D::YEAR>().get<int>( 0 );

        std::string dateStr;
        if ( rec.size() > 3 )
        {
            auto time = rec.getItem<D::TIME>().get<std::string>( 0 );
            dateStr   = std::format( "{}/{}/{} - {}", day, month, year, time );
        }
        else
        {
            dateStr = std::format( "{}/{}/{}", day, month, year );
        }
        return dateStr;
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static Opm::ParseContext defaultParseContext()
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
static bool insertKeywordAtIndex( std::unique_ptr<Opm::FileDeck>& fileDeck, Opm::DeckKeyword& keyword, Opm::FileDeck::Index insertIdx )
{
    Opm::ErrorGuard errors{};

    try
    {
        fileDeck->insert( insertIdx, keyword );
    }
    catch ( ... )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::optional<Opm::FileDeck::Index> findSectionInsertionPoint( std::unique_ptr<Opm::FileDeck>& fileDeck, const std::string& section )
{
    // Find the specified section
    auto sectionIdx = fileDeck->find( section );
    if ( !sectionIdx.has_value() )
    {
        return std::nullopt; // Section not found
    }

    auto insertIdx = sectionIdx.value();
    insertIdx++; // Start after the section keyword

    // Find a good insertion point within the section
    for ( auto it = insertIdx; it != fileDeck->stop(); it++ )
    {
        auto& kw = fileDeck->operator[]( it );

        // Stop if we hit another major section
        if ( kw.name() == Opm::ParserKeywords::RUNSPEC::keywordName || kw.name() == Opm::ParserKeywords::GRID::keywordName ||
             kw.name() == Opm::ParserKeywords::EDIT::keywordName || kw.name() == Opm::ParserKeywords::REGIONS::keywordName ||
             kw.name() == Opm::ParserKeywords::SOLUTION::keywordName || kw.name() == Opm::ParserKeywords::SUMMARY::keywordName ||
             kw.name() == Opm::ParserKeywords::SCHEDULE::keywordName )
        {
            insertIdx = it;
            break;
        }

        // Keep moving forward in the current section
        insertIdx = it;
        insertIdx++;
    }

    return insertIdx;
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

    try
    {
        auto deck = Opm::Parser{}.parseFile( filename, internal::defaultParseContext(), errors );

        m_fileDeck = std::make_unique<Opm::FileDeck>( deck );

        splitDatesIfNecessary();
    }
    catch ( ... )
    {
        m_fileDeck.reset();
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Make sure all dates keywords are single record, to make it easier to insert new things
//--------------------------------------------------------------------------------------------------
void RifOpmFlowDeckFile::splitDatesIfNecessary()
{
    using D = Opm::ParserKeywords::DATES;
    if ( m_fileDeck.get() == nullptr ) return;

    bool keepSplitting = true;
    while ( keepSplitting )
    {
        keepSplitting = false;
        for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
        {
            auto& kw = m_fileDeck->operator[]( it );
            if ( ( kw.name() == D::keywordName ) && ( kw.size() > 1 ) )
            {
                // split this keyword into multiple keywords
                int                           records = (int)kw.size();
                std::vector<Opm::DeckKeyword> newKeywords;

                for ( int i = 0; i < records; i++ )
                {
                    Opm::DeckKeyword newKw( kw.emptyStructuralCopy() );
                    Opm::DeckRecord  newRecToAdd( kw.getRecord( i ) );
                    newKw.addRecord( std::move( newRecToAdd ) );
                    newKeywords.push_back( newKw );
                }

                std::reverse( newKeywords.begin(), newKeywords.end() );
                // erase old kw and insert the new ones at the same index
                m_fileDeck->erase( it );
                for ( auto& newKw : newKeywords )
                {
                    m_fileDeck->insert( it, newKw );
                }
                // need to restart the search as the iterators could be invalid
                keepSplitting = true;
                break;
            }
        }
    }
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
bool RifOpmFlowDeckFile::mergeWellDeckAtTimeStep( int timeStep, std::string filename )
{
    Opm::ErrorGuard errors{};

    auto deckToMerge = Opm::Parser{}.parseFile( filename, internal::defaultParseContext(), errors );

    const auto welspecsIndexes = deckToMerge.index( Opm::ParserKeywords::WELSPECS::keywordName );
    if ( welspecsIndexes.empty() ) return false;
    auto&           mergeWelspecsKw = deckToMerge[welspecsIndexes[0]];
    Opm::DeckRecord newWelspecRecToAdd( mergeWelspecsKw.getRecord( 0 ) );

    const auto compdatIndexes = deckToMerge.index( Opm::ParserKeywords::COMPDAT::keywordName );
    if ( compdatIndexes.empty() ) return false;
    auto& mergeCompdatKw = deckToMerge[compdatIndexes[0]];

    auto additionalConnections = mergeCompdatKw.size();
    auto welldims              = this->welldims();

    auto dateIdx = internal::locateTimeStep( m_fileDeck, timeStep );
    if ( dateIdx.has_value() )
    {
        auto insertIdx = dateIdx.value();
        // Insert new well into WELSPECS in the selected date section
        auto existingKwFound = internal::locateKeywordAtTimeStep( m_fileDeck, timeStep, Opm::ParserKeywords::WELSPECS::keywordName );
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
            m_fileDeck->insert( insertIdx, mergeWelspecsKw );
            insertIdx++;
        }

        // Insert new well data into COMPDAT in the selected date section
        existingKwFound = internal::locateKeywordAtTimeStep( m_fileDeck, timeStep, Opm::ParserKeywords::COMPDAT::keywordName );
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
            m_fileDeck->insert( insertIdx, mergeCompdatKw );
        }

        // increase wells and connections in welldims to make sure they are big enough
        return setWelldims( (int)welldims[0] + 1, (int)( welldims[1] + additionalConnections ), (int)welldims[2] + 1, (int)welldims[3] + 1 );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///  Returns updated position due to new inserts if successful, < 0 if failure
//--------------------------------------------------------------------------------------------------
int RifOpmFlowDeckFile::mergeWellDeckAtPosition( int position, std::string filename )
{
    Opm::ErrorGuard errors{};

    auto deckToMerge = Opm::Parser{}.parseFile( filename, internal::defaultParseContext(), errors );

    const auto welspecsIndexes = deckToMerge.index( Opm::ParserKeywords::WELSPECS::keywordName );
    if ( welspecsIndexes.empty() ) return -1;
    auto&           mergeWelspecsKw = deckToMerge[welspecsIndexes[0]];
    Opm::DeckRecord newWelspecRecToAdd( mergeWelspecsKw.getRecord( 0 ) );

    const auto compdatIndexes = deckToMerge.index( Opm::ParserKeywords::COMPDAT::keywordName );
    if ( compdatIndexes.empty() ) return -1;
    auto& mergeCompdatKw = deckToMerge[compdatIndexes[0]];

    auto additionalConnections = mergeCompdatKw.size();
    auto welldims              = this->welldims();

    auto insertIdx = internal::positionToIndex( position, m_fileDeck );
    if ( !insertIdx.has_value() )
    {
        return -1;
    }

    // Insert new well into main WELSPECS
    const auto foundWelspecs = m_fileDeck->find( Opm::ParserKeywords::WELSPECS::keywordName );
    if ( foundWelspecs.has_value() )
    {
        auto& existing_idx = foundWelspecs.value();
        auto& welspecs_kw  = m_fileDeck->operator[]( existing_idx );

        Opm::DeckKeyword newWelspecsKw( welspecs_kw );
        newWelspecsKw.addRecord( std::move( newWelspecRecToAdd ) );

        m_fileDeck->erase( existing_idx );
        m_fileDeck->insert( existing_idx, newWelspecsKw );
    }
    else
    {
        // existing kw not found, insert a new one
        m_fileDeck->insert( insertIdx.value(), mergeCompdatKw );
        // update index to keep keyword order
        insertIdx.value() = insertIdx.value() + 1;
        position++;
    }

    // Insert new well data into main COMPDAT
    const auto foundCompdat = m_fileDeck->find( Opm::ParserKeywords::COMPDAT::keywordName );
    if ( foundCompdat.has_value() )
    {
        auto& existing_idx = foundCompdat.value();
        auto& compdat_kw   = m_fileDeck->operator[]( existing_idx );

        Opm::DeckKeyword newCompdatKw( compdat_kw );

        for ( size_t i = 0; i < mergeCompdatKw.size(); i++ )
        {
            Opm::DeckRecord newRecToAdd( mergeCompdatKw.getRecord( i ) );
            newCompdatKw.addRecord( std::move( newRecToAdd ) );
        }

        m_fileDeck->erase( existing_idx );
        m_fileDeck->insert( existing_idx, newCompdatKw );
    }
    else
    {
        // existing kw not found, insert a new one
        m_fileDeck->insert( insertIdx.value(), mergeCompdatKw );
        position++;
    }

    // increase wells and connections in welldims to make sure they are big enough
    if ( setWelldims( (int)welldims[0] + 1, (int)( welldims[1] + additionalConnections ), (int)welldims[2] + 1, (int)welldims[3] + 1 ) )
    {
        return position;
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::openWellAtTimeStep( int timeStep, Opm::DeckKeyword& openKeyword )
{
    auto dateIndex = internal::locateTimeStep( m_fileDeck, timeStep + 1 );
    if ( dateIndex.has_value() )
    {
        auto insertIdx = dateIndex.value();
        insertIdx--;
        return internal::insertKeywordAtIndex( m_fileDeck, openKeyword, insertIdx );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::openWellAtDeckPosition( int deckPosition, Opm::DeckKeyword& openKeyword )
{
    auto index = internal::positionToIndex( deckPosition, m_fileDeck );
    if ( index.has_value() )
    {
        return internal::insertKeywordAtIndex( m_fileDeck, openKeyword, index.value() );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOpmFlowDeckFile::keywords( bool includeDates )
{
    std::vector<std::string> values;

    if ( m_fileDeck.get() == nullptr ) return values;

    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() == Opm::ParserKeywords::DATES::keywordName && includeDates )
        {
            std::string dateStr = kw.name() + " " + internal::datesKeywordToString( kw );
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
    auto idx = m_fileDeck->find( Opm::ParserKeywords::DATES::keywordName );
    return idx.has_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::isRestartFile()
{
    if ( m_fileDeck.get() == nullptr ) return false;
    auto idx = m_fileDeck->find( Opm::ParserKeywords::RESTART::keywordName );
    return idx.has_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RifOpmFlowDeckFile::wellGroupsInFile()
{
    std::set<std::string> values;

    if ( m_fileDeck.get() == nullptr ) return values;

    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() == Opm::ParserKeywords::WELSPECS::keywordName )
        {
            for ( int i = 0; i < (int)kw.size(); i++ )
            {
                auto& item = kw.getRecord( i ).getItem( Opm::ParserKeywords::WELSPECS::GROUP::itemName );
                values.insert( item.get<std::string>( 0 ) );
            }
        }
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifOpmFlowDeckFile::dateStrings()
{
    std::vector<std::string> values;

    if ( m_fileDeck.get() == nullptr ) return values;

    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() == Opm::ParserKeywords::DATES::keywordName )
        {
            std::string dateStr = kw.name() + " " + internal::datesKeywordToString( kw );
            values.push_back( internal::datesKeywordToString( kw ) );
        }
    }
    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::time_t> RifOpmFlowDeckFile::dates()
{
    std::vector<std::time_t> values;
    if ( m_fileDeck.get() == nullptr ) return values;
    for ( auto it = m_fileDeck->start(); it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() == Opm::ParserKeywords::DATES::keywordName )
        {
            const auto& rec = kw.getRecord( 0 );
            values.push_back( Opm::TimeService::timeFromEclipse( rec ) );
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::appendDateKeywords( const std::vector<std::time_t>& dates )
{
    if ( m_fileDeck.get() == nullptr ) return false;

    auto kws = keywords();
    if ( kws.empty() ) return false;

    auto lastIdx = internal::positionToIndex( (int)kws.size() - 1, m_fileDeck );
    if ( !lastIdx.has_value() ) return false;

    auto lastKw = m_fileDeck->operator[]( lastIdx.value() );
    m_fileDeck->erase( lastIdx.value() );

    std::vector<std::time_t> revDates;
    std::reverse_copy( dates.begin(), dates.end(), std::back_inserter( revDates ) );

    for ( auto dt : revDates )
    {
        Opm::DeckKeyword newKw( ( Opm::ParserKeywords::DATES() ) );
        Opm::DeckRecord  newRec;

        std::tm* lt = localtime( &dt );

        std::string month = Opm::TimeService::eclipseMonthNames().at( lt->tm_mon + 1 );

        newRec.addItem( RifOpmDeckTools::item( Opm::ParserKeywords::DATES::DAY::itemName, lt->tm_mday ) );
        newRec.addItem( RifOpmDeckTools::item( Opm::ParserKeywords::DATES::MONTH::itemName, month ) );
        newRec.addItem( RifOpmDeckTools::item( Opm::ParserKeywords::DATES::YEAR::itemName, lt->tm_year + 1900 ) );
        newRec.addItem( RifOpmDeckTools::defaultItem( Opm::ParserKeywords::DATES::TIME::itemName ) );

        newKw.addRecord( std::move( newRec ) );

        m_fileDeck->insert( lastIdx.value(), newKw );
    }

    m_fileDeck->insert( lastIdx.value(), lastKw );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifOpmFlowDeckFile::welldims()
{
    using W = Opm::ParserKeywords::WELLDIMS;
    if ( m_fileDeck.get() == nullptr ) return {};
    auto idx = m_fileDeck->find( W::keywordName );
    if ( idx.has_value() )
    {
        std::vector<int> dims;

        auto&       kw  = m_fileDeck->operator[]( idx.value() );
        const auto& rec = kw.getRecord( 0 );
        dims.push_back( rec.getItem<W::MAXWELLS>().get<int>( 0 ) );
        dims.push_back( rec.getItem<W::MAXCONN>().get<int>( 0 ) );
        dims.push_back( rec.getItem<W::MAXGROUPS>().get<int>( 0 ) );
        dims.push_back( rec.getItem<W::MAX_GROUPSIZE>().get<int>( 0 ) );

        return dims;
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::setWelldims( int maxWells, int maxConnections, int maxGroups, int maxWellsInGroup )
{
    using W = Opm::ParserKeywords::WELLDIMS;
    if ( m_fileDeck.get() == nullptr ) return false;
    auto idx = m_fileDeck->find( W::keywordName );
    if ( idx.has_value() )
    {
        auto& oldkw = m_fileDeck->operator[]( idx.value() );

        Opm::DeckKeyword newKw( Opm::ParserKeyword( oldkw.name() ) );

        newKw.addRecord( Opm::DeckRecord{ { RifOpmDeckTools::item( W::MAXWELLS::itemName, maxWells ),
                                            RifOpmDeckTools::item( W::MAXCONN::itemName, maxConnections ),
                                            RifOpmDeckTools::item( W::MAXGROUPS::itemName, maxGroups ),
                                            RifOpmDeckTools::item( W::MAX_GROUPSIZE::itemName, maxWellsInGroup ) } } );

        m_fileDeck->erase( idx.value() );
        m_fileDeck->insert( idx.value(), newKw );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifOpmFlowDeckFile::regdims()
{
    using R = Opm::ParserKeywords::REGDIMS;
    if ( m_fileDeck.get() == nullptr ) return {};
    auto idx = m_fileDeck->find( R::keywordName );
    if ( idx.has_value() )
    {
        std::vector<int> dims;

        auto&       kw  = m_fileDeck->operator[]( idx.value() );
        const auto& rec = kw.getRecord( 0 );
        dims.push_back( rec.getItem( R::NTFIP::itemName ).get<int>( 0 ) );
        dims.push_back( rec.getItem( R::NMFIPR::itemName ).get<int>( 0 ) );
        dims.push_back( rec.getItem( R::NRFREG::itemName ).get<int>( 0 ) );
        dims.push_back( rec.getItem( R::NTFREG::itemName ).get<int>( 0 ) );

        return dims;
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::setRegdims( int maxRegions, int maxRegionDefinitions, int maxRegionFlowConnections, int maxFIPRegions )
{
    using R = Opm::ParserKeywords::REGDIMS;
    if ( m_fileDeck.get() == nullptr ) return false;
    auto idx = m_fileDeck->find( R::keywordName );
    if ( idx.has_value() )
    {
        auto& oldkw = m_fileDeck->operator[]( idx.value() );

        Opm::DeckKeyword newKw( Opm::ParserKeyword( oldkw.name() ) );
        newKw.addRecord( Opm::DeckRecord{ { RifOpmDeckTools::item( R::NTFIP::itemName, maxRegions ),
                                            RifOpmDeckTools::item( R::NMFIPR::itemName, maxRegionDefinitions ),
                                            RifOpmDeckTools::item( R::NRFREG::itemName, maxRegionFlowConnections ),
                                            RifOpmDeckTools::item( R::NTFREG::itemName, maxFIPRegions ) } } );

        m_fileDeck->erase( idx.value() );
        m_fileDeck->insert( idx.value(), newKw );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::ensureRegdimsKeyword()
{
    using R = Opm::ParserKeywords::REGDIMS;
    if ( m_fileDeck.get() == nullptr ) return false;

    // Check if REGDIMS already exists
    auto idx = m_fileDeck->find( R::keywordName );
    if ( idx.has_value() )
    {
        return true; // Already exists
    }

    // Find RUNSPEC section to add REGDIMS
    auto runspecIdx = m_fileDeck->find( Opm::ParserKeywords::RUNSPEC::keywordName );
    if ( !runspecIdx.has_value() ) return false; // Cannot add REGDIMS without RUNSPEC section

    // Look for a good insertion point after RUNSPEC and before GRID section
    auto insertIdx = runspecIdx.value();
    insertIdx++; // Start after RUNSPEC keyword

    // Try to find GRID or other sections that should come after REGDIMS
    for ( auto it = insertIdx; it != m_fileDeck->stop(); it++ )
    {
        auto& kw = m_fileDeck->operator[]( it );
        if ( kw.name() == Opm::ParserKeywords::GRID::keywordName || kw.name() == Opm::ParserKeywords::SOLUTION::keywordName ||
             kw.name() == Opm::ParserKeywords::SCHEDULE::keywordName )
        {
            insertIdx = it;
            break;
        }
    }

    // Create REGDIMS keyword with default values: "6* 1 /"
    Opm::DeckKeyword regdimsKw( ( Opm::ParserKeyword( R::keywordName ) ) );
    regdimsKw.addRecord( Opm::DeckRecord{ { RifOpmDeckTools::item( R::NTFIP::itemName, 1 ),
                                            RifOpmDeckTools::item( R::NMFIPR::itemName, 1 ),
                                            RifOpmDeckTools::item( R::NRFREG::itemName, 1 ),
                                            RifOpmDeckTools::item( R::NTFREG::itemName, 1 ) } } );

    m_fileDeck->insert( insertIdx, regdimsKw );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::addIncludeKeyword( std::string section, std::string keyword, std::string filePath )
{
    if ( m_fileDeck.get() == nullptr ) return false;

    // Find insertion point within the section
    auto insertIdx = internal::findSectionInsertionPoint( m_fileDeck, section );
    if ( !insertIdx.has_value() ) return false; // Section not found

    // Create the INCLUDE keyword
    Opm::DeckKeyword includeKw( ( Opm::ParserKeywords::INCLUDE() ) );
    includeKw.addRecord( Opm::DeckRecord{ { RifOpmDeckTools::item( Opm::ParserKeywords::INCLUDE::IncludeFile::itemName, filePath ) } } );

    m_fileDeck->insert( insertIdx.value(), includeKw );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::addOperaterKeyword( std::string          section,
                                             std::string          targetProperty,
                                             int                  regionId,
                                             std::string          equation,
                                             std::string          inputProperty,
                                             std::optional<float> alpha,
                                             std::optional<float> beta )
{
    if ( m_fileDeck.get() == nullptr ) return false;

    // Find insertion point within the section
    auto insertIdx = internal::findSectionInsertionPoint( m_fileDeck, section );
    if ( !insertIdx.has_value() ) return false; // Section not found

    using O = Opm::ParserKeywords::OPERATER;

    // Create the OPERATER keyword
    Opm::DeckKeyword operaterKw( ( Opm::ParserKeywords::OPERATER() ) );

    std::vector<Opm::DeckItem> recordItems;
    recordItems.push_back( RifOpmDeckTools::item( O::TARGET_ARRAY::itemName, targetProperty ) );
    recordItems.push_back( RifOpmDeckTools::item( O::REGION_NUMBER::itemName, regionId ) );
    recordItems.push_back( RifOpmDeckTools::item( O::OPERATION::itemName, equation ) );
    recordItems.push_back( RifOpmDeckTools::item( O::ARRAY_PARAMETER::itemName, inputProperty ) );

    // Add alpha parameter
    if ( alpha.has_value() )
    {
        recordItems.push_back( RifOpmDeckTools::item( O::PARAM1::itemName, std::to_string( alpha.value() ) ) );
    }
    else
    {
        recordItems.push_back( RifOpmDeckTools::defaultItem( O::PARAM1::itemName ) );
    }

    // Add beta parameter
    if ( beta.has_value() )
    {
        recordItems.push_back( RifOpmDeckTools::item( O::PARAM2::itemName, std::to_string( beta.value() ) ) );
    }
    else
    {
        recordItems.push_back( RifOpmDeckTools::defaultItem( O::PARAM2::itemName ) );
    }

    // Add final default item
    recordItems.push_back( RifOpmDeckTools::defaultItem( O::REGION_NAME::itemName ) ); // 1* for the last field

    operaterKw.addRecord( Opm::DeckRecord{ std::move( recordItems ) } );

    m_fileDeck->insert( insertIdx.value(), operaterKw );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::addBcconKeyword( std::string section, const std::vector<RigEclipseResultTools::BorderCellFace>& borderCellFaces )
{
    if ( m_fileDeck.get() == nullptr ) return false;

    if ( borderCellFaces.empty() ) return true; // Nothing to add

    // Find insertion point within the section
    auto insertPos = internal::findSectionInsertionPoint( m_fileDeck, section );
    if ( !insertPos.has_value() )
    {
        return false; // Section not found
    }

    // Helper lambda to convert FaceType to string
    auto faceTypeToString = []( cvf::StructGridInterface::FaceType faceType ) -> std::string
    {
        switch ( faceType )
        {
            case cvf::StructGridInterface::POS_I:
                return "X";
            case cvf::StructGridInterface::NEG_I:
                return "X-";
            case cvf::StructGridInterface::POS_J:
                return "Y";
            case cvf::StructGridInterface::NEG_J:
                return "Y-";
            case cvf::StructGridInterface::POS_K:
                return "Z";
            case cvf::StructGridInterface::NEG_K:
                return "Z-";
            default:
                return "";
        }
    };

    // Create the BCCON keyword using OPM's BCCON parser keyword
    using B = Opm::ParserKeywords::BCCON;

    Opm::DeckKeyword bcconKw( ( Opm::ParserKeywords::BCCON() ) );

    for ( const auto& borderFace : borderCellFaces )
    {
        // Convert from 0-based to 1-based Eclipse indexing
        int i1 = static_cast<int>( borderFace.ijk[0] ) + 1;
        int j1 = static_cast<int>( borderFace.ijk[1] ) + 1;
        int k1 = static_cast<int>( borderFace.ijk[2] ) + 1;

        std::string faceStr = faceTypeToString( borderFace.faceType );

        // Create items for the record using proper BCCON item names
        std::vector<Opm::DeckItem> recordItems;

        recordItems.push_back( RifOpmDeckTools::item( B::INDEX::itemName, borderFace.boundaryCondition ) );
        recordItems.push_back( RifOpmDeckTools::item( B::I1::itemName, i1 ) );
        recordItems.push_back( RifOpmDeckTools::item( B::I2::itemName, i1 ) ); // Same as i1 for single cell
        recordItems.push_back( RifOpmDeckTools::item( B::J1::itemName, j1 ) );
        recordItems.push_back( RifOpmDeckTools::item( B::J2::itemName, j1 ) ); // Same as j1 for single cell
        recordItems.push_back( RifOpmDeckTools::item( B::K1::itemName, k1 ) );
        recordItems.push_back( RifOpmDeckTools::item( B::K2::itemName, k1 ) ); // Same as k1 for single cell
        recordItems.push_back( RifOpmDeckTools::item( B::DIRECTION::itemName, faceStr ) );

        bcconKw.addRecord( Opm::DeckRecord{ std::move( recordItems ) } );
    }

    m_fileDeck->insert( insertPos.value(), bcconKw );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::addBcpropKeyword( std::string                                               section,
                                           const std::vector<RigEclipseResultTools::BorderCellFace>& boundaryConditions,
                                           const std::vector<Opm::DeckRecord>&                       boundaryConditionProperties )
{
    if ( m_fileDeck.get() == nullptr ) return false;

    if ( boundaryConditions.empty() ) return true; // Nothing to add

    // Find insertion point within the section
    auto insertPos = internal::findSectionInsertionPoint( m_fileDeck, section );
    if ( !insertPos.has_value() )
    {
        return false; // Section not found
    }

    // Create the BCPROP keyword using OPM's BCPROP parser keyword
    using B = Opm::ParserKeywords::BCPROP;

    Opm::DeckKeyword bcpropKw( ( Opm::ParserKeywords::BCPROP() ) );

    // Add one entry per boundary condition
    for ( const auto& bc : boundaryConditions )
    {
        if ( bc.boundaryCondition <= 0 ) continue; // Skip entries without a valid boundary condition

        // Find the corresponding property record
        // The properties vector should be indexed by boundaryCondition - 1
        size_t propIndex = static_cast<size_t>( bc.boundaryCondition - 1 );
        if ( propIndex < boundaryConditionProperties.size() )
        {
            const auto& propRecord = boundaryConditionProperties[propIndex];

            // Create a new record with the boundary condition INDEX
            std::vector<Opm::DeckItem> recordItems;

            // Add INDEX field
            recordItems.push_back( RifOpmDeckTools::item( B::INDEX::itemName, bc.boundaryCondition ) );

            // Copy all items from the property record (which doesn't include INDEX)
            for ( size_t i = 0; i < propRecord.size(); ++i )
            {
                recordItems.push_back( propRecord.getItem( i ) );
            }

            bcpropKw.addRecord( Opm::DeckRecord{ std::move( recordItems ) } );
        }
    }

    m_fileDeck->insert( insertPos.value(), bcpropKw );
    return true;
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
        if ( kw.name() != Opm::ParserKeywords::DATES::keywordName ) continue;

        Opm::FileDeck::Index insertIdx( it );

        if ( mswFileData[curTimeStep] != prevFileData )
        {
            auto deck = Opm::Parser{}.parseString( mswFileData[curTimeStep], internal::defaultParseContext(), errors );

            for ( auto kwit = deck.begin(); kwit != deck.end(); kwit++ )
            {
                Opm::DeckKeyword newKw( *kwit );

                if ( ( newKw.name() == Opm::ParserKeywords::WELSPECS::keywordName ) && ( curTimeStep == 0 ) )
                {
                    const auto found = m_fileDeck->find( Opm::ParserKeywords::WELSPECS::keywordName );
                    if ( !found.has_value() )
                    {
                        m_fileDeck->insert( insertIdx, newKw );
                        insertIdx++;
                    }
                    else
                    {
                        Opm::DeckRecord newRecToAdd( newKw.getRecord( 0 ) );

                        auto& existing_idx = found.value();
                        auto& welspecs_kw  = m_fileDeck->operator[]( existing_idx );

                        Opm::DeckKeyword newWelspecsKw( welspecs_kw );
                        newWelspecsKw.addRecord( std::move( newRecToAdd ) );

                        m_fileDeck->erase( existing_idx );
                        m_fileDeck->insert( existing_idx, newWelspecsKw );
                    }
                }
                else if ( ( newKw.name() == Opm::ParserKeywords::COMPDAT::keywordName ) && ( curTimeStep == 0 ) )
                {
                    const auto found = m_fileDeck->find( Opm::ParserKeywords::COMPDAT::keywordName );
                    if ( !found.has_value() )
                    {
                        m_fileDeck->insert( insertIdx, newKw );
                        insertIdx++;
                    }
                    else
                    {
                        auto& existing_idx = found.value();
                        auto& compdat_kw   = m_fileDeck->operator[]( existing_idx );

                        Opm::DeckKeyword newCompdatKw( compdat_kw );

                        for ( size_t i = 0; i < newKw.size(); i++ )
                        {
                            Opm::DeckRecord newRecToAdd( newKw.getRecord( i ) );
                            newCompdatKw.addRecord( std::move( newRecToAdd ) );
                        }

                        m_fileDeck->erase( existing_idx );
                        m_fileDeck->insert( existing_idx, newCompdatKw );
                    }
                }
                else
                {
                    m_fileDeck->insert( insertIdx, newKw );
                    insertIdx++;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::stopAtTimeStep( int timeStep )
{
    if ( !m_fileDeck ) return false;

    auto dateIdx = internal::locateTimeStep( m_fileDeck, timeStep );
    if ( dateIdx.has_value() )
    {
        Opm::DeckKeyword newKw( ( Opm::ParserKeywords::END() ) );
        m_fileDeck->insert( dateIdx.value(), newKw );
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::replaceKeywordData( const std::string& keyword, const std::vector<double>& data )
{
    try
    {
        if ( m_fileDeck.get() == nullptr ) return false;

        // Find the keyword in the deck
        auto keywordIdx = m_fileDeck->find( keyword );
        if ( !keywordIdx.has_value() ) return false;

        // Get the existing keyword to preserve location
        const auto& existingKw = m_fileDeck->operator[]( keywordIdx.value() );

        // Create a new keyword with the same name and location
        Opm::DeckKeyword newKw( existingKw.location(), keyword );
        newKw.setDataKeyword( true );

        // Keywords ending with "NUM" should be integers (e.g., SATNUM, PVTNUM, EQLNUM)
        bool isIntType = keyword.size() >= 3 && keyword.substr( keyword.size() - 3 ) == "NUM";

        // Create a record with the new data
        Opm::DeckRecord record;
        if ( isIntType )
        {
            // Create integer item
            Opm::DeckItem item( keyword, int() );
            for ( const auto& value : data )
            {
                item.push_back( static_cast<int>( std::round( value ) ) );
            }
            record.addItem( std::move( item ) );
        }
        else
        {
            // Create double item
            std::vector<Opm::Dimension> active_dimensions;
            std::vector<Opm::Dimension> default_dimensions;
            Opm::DeckItem               item( keyword, double(), active_dimensions, default_dimensions );
            for ( const auto& value : data )
            {
                item.push_back( value );
            }
            record.addItem( std::move( item ) );
        }

        newKw.addRecord( std::move( record ) );

        // Replace the keyword in the deck
        m_fileDeck->erase( keywordIdx.value() );
        m_fileDeck->insert( keywordIdx.value(), newKw );

        return true;
    }
    catch ( std::exception& )
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpmFlowDeckFile::replaceKeywordData( const std::string& keyword, const std::vector<int>& data )
{
    try
    {
        if ( m_fileDeck.get() == nullptr ) return false;

        // Find the keyword in the deck
        auto keywordIdx = m_fileDeck->find( keyword );
        if ( !keywordIdx.has_value() ) return false;

        // Get the existing keyword to preserve location
        const auto& existingKw = m_fileDeck->operator[]( keywordIdx.value() );

        // Create a new keyword with the same name and location
        Opm::DeckKeyword newKw( existingKw.location(), keyword );
        newKw.setDataKeyword( true );

        // Create a record with the new data
        Opm::DeckRecord record;
        // Create integer item
        Opm::DeckItem item( keyword, int() );
        for ( const auto& value : data )
        {
            item.push_back( value );
        }
        record.addItem( std::move( item ) );

        newKw.addRecord( std::move( record ) );

        // Replace the keyword in the deck
        m_fileDeck->erase( keywordIdx.value() );
        m_fileDeck->insert( keywordIdx.value(), newKw );

        return true;
    }
    catch ( std::exception& )
    {
        return false;
    }
}
