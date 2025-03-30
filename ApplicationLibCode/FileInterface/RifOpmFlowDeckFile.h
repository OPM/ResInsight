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

#include <memory>
#include <string>

namespace Opm
{
class FileDeck;
class DeckItem;
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

    bool mergeWellDeck( std::string filename );

private:
    Opm::DeckItem item( std::string name, std::string value );
    Opm::DeckItem item( std::string name, int value );
    Opm::DeckItem defaultItem( std::string name, int cols );

private:
    std::unique_ptr<Opm::FileDeck> m_fileDeck;
};
