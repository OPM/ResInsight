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
#include <set>
#include <string>
#include <vector>

namespace Opm
{
class FileDeck;
class DeckKeyword;
class DeckItem;
class DeckRecord;
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

    int  mergeKeywordAtPosition( int deckPosition, const Opm::DeckKeyword& keyword );
    bool mergeKeywordAtTimeStep( int timeStep, const Opm::DeckKeyword& keyword, std::string insertAfterKeyword = "" );

    bool openWellAtTimeStep( int timeStep, Opm::DeckKeyword& openKeyword );
    bool openWellAtDeckPosition( int deckPosition, Opm::DeckKeyword& openKeyword );

    bool restartAtTimeStep( int timeStep, std::string deckName );
    bool stopAtTimeStep( int timeStep );

    std::vector<std::string>        keywords( bool includeDates = true );
    std::optional<Opm::DeckKeyword> findKeyword( const std::string& keyword );
    bool                            hasDatesKeyword();
    bool                            isRestartFile();
    std::vector<std::string>        dateStrings();
    std::vector<std::time_t>        dates();
    bool                            appendDateKeywords( const std::vector<std::time_t>& dates );

    std::set<std::string> wellGroupsInFile();

    std::vector<int> welldims();
    bool             setWelldims( int maxWells, int maxConnections, int maxGroups, int maxWellsInGroup );

    std::vector<int> wsegdims();
    bool             setWsegdims( int maxMSWells, int maxSegmentsPerWell, int maxBranchesPerWell );

    std::vector<int> regdims();
    bool             setRegdims( int maxRegions, int maxRegionDefinitions, int maxRegionFlowConnections, int maxFIPRegions );
    bool             ensureRegdimsKeyword();

    bool addIncludeKeyword( std::string section, std::string keyword, std::string filePath );

    bool replaceKeywordData( const std::string& keyword, const std::vector<double>& data );
    bool replaceKeywordData( const std::string& keyword, const std::vector<int>& data );

    bool replaceKeyword( const std::string& section, const Opm::DeckKeyword& keyword );

private:
    void splitDatesIfNecessary();

private:
    std::unique_ptr<Opm::FileDeck> m_fileDeck;
};
