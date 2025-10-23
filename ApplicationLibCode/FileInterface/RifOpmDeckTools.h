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

#include <string>

namespace Opm
{
class DeckItem;
} // namespace Opm

namespace RifOpmDeckTools
{
Opm::DeckItem item( std::string name, std::string value );
Opm::DeckItem item( std::string name, int value );
Opm::DeckItem item( std::string name, size_t value );
Opm::DeckItem item( std::string name, double value );
Opm::DeckItem defaultItem( std::string name, int columns = 1 );

} // namespace RifOpmDeckTools
