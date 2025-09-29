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

#pragma once

#include <ctime>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Opm
{
class FileDeck;
class DeckItem;
class ParseContext;
} // namespace Opm

//==================================================================================================
///
///
//==================================================================================================
class RifOpmFlowDeckFile
{
public:
    RifOpmFlowDeckFile();
    ~RifOpmFlowDeckFile();

    bool loadDeck( std::string filename );
    bool saveDeck( std::string folder, std::string filename );

    bool mergeWellDeckAtTimeStep( int timeStep, std::string filename );
    int  mergeWellDeckAtPosition( int position, std::string filename );

    bool mergeMswData( std::vector<std::string>& mswFileData );

    bool openWellAtTimeStep( int timeStep, std::string openText );
    bool openWellAtDeckPosition( int deckPosition, std::string openText );

    bool restartAtTimeStep( int timeStep, std::string deckName );
    bool stopAtTimeStep( int timeStep );

    std::vector<std::string> keywords();
    bool                     hasDatesKeyword();
    bool                     isRestartFile();
    std::vector<std::string> dateStrings();
    std::vector<std::time_t> dates();

    std::vector<int> welldims();
    bool             setWelldims( int maxWells, int maxConnections, int maxGroups, int maxWellsInGroup );

    std::vector<int>         regdims();
    bool                     setRegdims( int maxRegions, int maxRegionDefinitions, int maxRegionFlowConnections, int maxFIPRegions );
    bool                     ensureRegdimsKeyword();

    bool                     addIncludeKeyword( std::string section, std::string keyword, std::string filePath );
    bool                     addOperaterKeyword( std::string section, std::string targetProperty, int regionId, std::string equation, std::string inputProperty, std::optional<float> alpha, std::optional<float> beta );

private:
    Opm::DeckItem item( std::string name, std::string value );
    Opm::DeckItem item( std::string name, int value );
    Opm::DeckItem defaultItem( std::string name, int cols );

    void splitDatesIfNecessary();

private:
    std::unique_ptr<Opm::FileDeck> m_fileDeck;
};
