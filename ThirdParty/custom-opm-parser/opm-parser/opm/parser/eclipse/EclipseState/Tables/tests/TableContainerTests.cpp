/*
  Copyright (C) 2013 by Andreas Lauser

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE TableContainerTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableContainer.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>

#include <string>
#include <memory>

std::shared_ptr<const Opm::Deck> createSWOFDeck() {
    const char *deckData =
        "TABDIMS\n"
        " 2 /\n"
        "\n"
        "SWOF\n"
        " 1 2 3 4\n"
        " 5 6 7 8 /\n"
        " 9 10 11 12 /\n";

    Opm::Parser parser;
    Opm::DeckConstPtr deck(parser.parseString(deckData, Opm::ParseContext()));
    return deck;
}

BOOST_AUTO_TEST_CASE( CreateContainer ) {
    std::vector<std::string> columnNames{"A", "B", "C", "D"};
    Opm::DeckConstPtr deck = createSWOFDeck();
    Opm::TableContainer container(10);
    BOOST_CHECK( container.empty() );
    BOOST_CHECK_EQUAL( 0 , container.size() );
    BOOST_CHECK_EQUAL( false , container.hasTable( 1 ));

    std::shared_ptr<Opm::SimpleTable> table = std::make_shared<Opm::SwofTable>( deck->getKeyword("SWOF").getRecord(0).getItem(0) );
    BOOST_CHECK_THROW( container.addTable( 10 , table ), std::invalid_argument );
    container.addTable( 6 , table );
    BOOST_CHECK_EQUAL( 1 , container.size() );

    BOOST_CHECK_EQUAL( table.get() , &(container[6]));
    BOOST_CHECK_EQUAL( table.get() , &(container[9]));

    BOOST_CHECK_THROW( container[5] , std::invalid_argument );
    BOOST_CHECK_THROW( container[10] , std::invalid_argument );
}

