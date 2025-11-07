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

#include "RifOpmDeckTools.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Units/Dimension.hpp"

namespace RifOpmDeckTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem item( std::string name, std::string value )
{
    Opm::DeckItem item1( name, value );
    item1.push_back( value );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem optionalItem( std::string name, std::optional<std::string> value )
{
    if ( value.has_value() )
    {
        return item( name, value.value() );
    }
    return defaultItem( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem item( std::string name, int value )
{
    Opm::DeckItem item1( name, value );
    item1.push_back( value );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem optionalItem( std::string name, std::optional<int> value )
{
    if ( value.has_value() )
    {
        return item( name, value.value() );
    }
    return defaultItem( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem item( std::string name, size_t value )
{
    return item( name, (int)value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem optionalItem( std::string name, std::optional<size_t> value )
{
    if ( value.has_value() )
    {
        return item( name, value.value() );
    }
    return defaultItem( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem defaultItem( std::string name )
{
    Opm::DeckItem item1( name, 0 );
    item1.push_backDummyDefault<int>( 1 );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem item( std::string name, double value )
{
    std::vector<Opm::Dimension> active_dimensions;
    std::vector<Opm::Dimension> default_dimensions;

    Opm::DeckItem item1( name, double(), active_dimensions, default_dimensions );
    item1.push_back( value );
    return item1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem optionalItem( std::string name, std::optional<double> value )
{
    if ( value.has_value() )
    {
        return item( name, value.value() );
    }
    return defaultItem( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem optionalItem( std::string name, std::optional<float> value )
{
    if ( value.has_value() )
    {
        return item( name, (double)value.value() );
    }
    return defaultItem( name );
}

} // namespace RifOpmDeckTools
