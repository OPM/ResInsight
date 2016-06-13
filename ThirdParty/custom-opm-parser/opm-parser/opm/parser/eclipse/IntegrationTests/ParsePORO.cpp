/*
  Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Units/ConversionFactors.hpp>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(ParsePOROandPERMX) {
    ParserPtr parser(new Parser());
    std::string poroFile("testdata/integration_tests/PORO/PORO1");
    DeckPtr deck =  parser->parseFile(poroFile, ParseContext());
    const auto& kw1 = deck->getKeyword("PORO" , 0);
    const auto& kw2 = deck->getKeyword("PERMX" , 0);

    BOOST_CHECK_THROW( kw1.getIntData() , std::logic_error );
    BOOST_CHECK_THROW( kw1.getStringData() , std::logic_error );

    {
        const std::vector<double>& poro = kw1.getRawDoubleData();
        BOOST_CHECK_EQUAL( 440U , poro.size() );
        BOOST_CHECK_EQUAL( 0.233782813 , poro[0]);
        BOOST_CHECK_EQUAL( 0.251224369 , poro[1]);
        BOOST_CHECK_EQUAL( 0.155628711 , poro[439]);
    }

    {
        const std::vector<double>& permx = kw2.getSIDoubleData();
        const std::vector<double>& permxRAW = kw2.getRawDoubleData();
        BOOST_CHECK_EQUAL( 1000U , permx.size() );
        BOOST_CHECK_EQUAL( 1000U , permxRAW.size() );

        BOOST_CHECK_CLOSE( Metric::Permeability * 1 , permx[0] , 0.001);
        BOOST_CHECK_CLOSE( Metric::Permeability * 2 , permx[1] , 0.001);
        BOOST_CHECK_CLOSE( Metric::Permeability * 3 , permx[2] , 0.001);
        BOOST_CHECK_CLOSE( Metric::Permeability * 10, permx[999] , 0.001);
    }
}
