/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RifOpmDeckFileTools.h"

#include "RifOpmFlowDeckFile.h"

namespace RifOpmDeckFileTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> datesInDeckFile( const std::string& deckFileName )
{
    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( deckFileName );
    if ( !loadResult.has_value() )
    {
        return {};
    }
    std::vector<QDateTime> dates;
    // Extract dates from the deck file
    for ( auto tt : deckFile.dates() )
    {
        dates.push_back( QDateTime::fromSecsSinceEpoch( tt ) );
    }

    return dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> wellGroupsInFileDeck( const std::string& deckFileName )
{
    RifOpmFlowDeckFile deckFile;
    auto               loadResult = deckFile.loadDeck( deckFileName );
    if ( !loadResult.has_value() )
    {
        return {};
    }

    std::vector<std::string> groups;
    for ( auto& grp : deckFile.wellGroupsInFile() )
    {
        groups.push_back( grp );
    }
    return groups;
}

} // namespace RifOpmDeckFileTools
