/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimVfpTableExtractor.h"

#include "cafPdmUiItem.h"
#include "cafUtils.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/V.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::VFPInjTable> RimVfpTableExtractor::extractVfpInjectionTables( const std::string& filename )
{
    std::vector<Opm::VFPInjTable> tables;

    try
    {
		Opm::Parser parser(false);
		const ::Opm::ParserKeywords::VFPINJ kw1;
		const ::Opm::ParserKeywords::VFPIDIMS kw2;

		parser.addParserKeyword(kw1);
		parser.addParserKeyword(kw2);
		
        auto        deck = parser.parseFile( filename );

        std::string myKeyword   = "VFPINJ";
        auto        keywordList = deck.getKeywordList( myKeyword );

        for ( auto kw : keywordList )
        {
            auto name = kw->name();

            Opm::UnitSystem unitSystem;
            {
                const auto& header = kw->getRecord( 0 );

                if ( header.getItem<Opm::ParserKeywords::VFPINJ::UNITS>().hasValue( 0 ) )
                {
                    std::string units_string;
                    units_string = header.getItem<Opm::ParserKeywords::VFPINJ::UNITS>().get<std::string>( 0 );
                    unitSystem   = Opm::UnitSystem( units_string );
                }
            }

            Opm::VFPInjTable table( *kw, unitSystem );
            tables.push_back( table );
        }
    }
    catch ( ... )
    {
    }

    return tables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::VFPProdTable> RimVfpTableExtractor::extractVfpProductionTables( const std::string& filename )
{
    std::vector<Opm::VFPProdTable> tables;

    try
    {
		Opm::Parser parser(false);
		const ::Opm::ParserKeywords::VFPPROD kw1;

		parser.addParserKeyword(kw1);

        auto        deck = parser.parseFile( filename );

        std::string myKeyword   = "VFPPROD";
        auto        keywordList = deck.getKeywordList( myKeyword );

        for ( auto kw : keywordList )
        {
            auto name = kw->name();

            Opm::UnitSystem unitSystem;
            {
                const auto& header = kw->getRecord( 0 );

                if ( header.getItem<Opm::ParserKeywords::VFPPROD::UNITS>().hasValue( 0 ) )
                {
                    std::string units_string;
                    units_string = header.getItem<Opm::ParserKeywords::VFPPROD::UNITS>().get<std::string>( 0 );
                    unitSystem   = Opm::UnitSystem( units_string );
                }
            }

			bool gaslift_opt_active = false;
            Opm::VFPProdTable table( *kw, gaslift_opt_active, unitSystem );
            tables.push_back( table );
        }
    }
    catch ( ... )
    {
    }

    return tables;
}
