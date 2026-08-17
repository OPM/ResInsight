/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaOpmKeywordTools.h"

#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserItem.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserRecord.hpp"

#include <iterator>
#include <set>

namespace
{
const Opm::Parser& sharedParser()
{
    static const Opm::Parser parser( true );
    return parser;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RiaOpmKeywordInfo> RiaOpmKeywordTools::keywordInfo( const QString& keywordName )
{
    const QString keyword  = keywordName.toUpper();
    const auto&   parser   = sharedParser();
    const auto    deckName = keyword.toStdString();

    if ( !parser.isRecognizedKeyword( deckName ) ) return std::nullopt;

    const auto& parserKeyword = parser.getParserKeywordFromDeckName( deckName );

    RiaOpmKeywordInfo info;
    info.name = QString::fromStdString( parserKeyword.getName() );

    const auto recordCount = static_cast<size_t>( std::distance( parserKeyword.begin(), parserKeyword.end() ) );
    if ( recordCount == 1 )
    {
        const auto& record         = parserKeyword.getRecord( 0 );
        info.acceptsArbitraryItems = record.size() == 1 && record.get( 0 ).sizeType() == Opm::ParserItem::item_size::ALL;
    }

    std::set<std::string> seenNames;
    for ( const auto& record : parserKeyword )
    {
        for ( const auto& item : record )
        {
            if ( seenNames.insert( item.name() ).second )
            {
                info.itemNames.push_back( QString::fromStdString( item.name() ) );
            }
        }
    }

    return info;
}
