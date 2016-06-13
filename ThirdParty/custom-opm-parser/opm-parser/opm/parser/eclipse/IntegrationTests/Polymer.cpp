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
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyviscTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlymaxTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyrockTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyadsTable.hpp>

using namespace Opm;




BOOST_AUTO_TEST_CASE( parse_polymer_tables ) {
    ParserPtr parser(new Parser());
    DeckPtr deck = parser->parseFile("testdata/integration_tests/POLYMER/POLY.inc", ParseContext());
    Opm::TableManager tables( *deck );
    const TableContainer& plymax = tables.getPlymaxTables();
    const TableContainer& plyrock = tables.getPlyrockTables();
    const TableContainer& plyads = tables.getPlyadsTables();
    const TableContainer& plyvis = tables.getPlyviscTables();

    BOOST_CHECK_EQUAL( plymax.size() , 1U );
    BOOST_CHECK_EQUAL( plyrock.size() , 1U );
    BOOST_CHECK_EQUAL( plyvis.size() , 1U );
    BOOST_CHECK_EQUAL( plyads.size() , 1U );

    {
        const Opm::PlymaxTable& table0 = plymax.getTable<Opm::PlymaxTable>(0);
        BOOST_CHECK_EQUAL( table0.numColumns() , 2U );
        BOOST_CHECK_EQUAL( table0.getPolymerConcentrationColumn()[0] , 3.0 );
        BOOST_CHECK_EQUAL( table0.getMaxPolymerConcentrationColumn()[0] , 0.0 );
    }

    {
        const Opm::PlyviscTable& table0 = plyvis.getTable<Opm::PlyviscTable>(0);
        BOOST_CHECK_EQUAL( table0.numColumns() , 2U );
        BOOST_CHECK_EQUAL( table0.getPolymerConcentrationColumn()[5] , 3.0 );
        BOOST_CHECK_EQUAL( table0.getViscosityMultiplierColumn()[5] , 48.0 );
    }

    {
        const Opm::PlyrockTable& table0 = plyrock.getTable<Opm::PlyrockTable>(0);
        BOOST_CHECK_EQUAL( table0.numColumns() , 5U );
        BOOST_CHECK_EQUAL( table0.getDeadPoreVolumeColumn()[0] , 0.05 );
        BOOST_CHECK_EQUAL( table0.getMaxAdsorbtionColumn()[0] , 0.000025 );
    }

    {
        const Opm::PlyadsTable& table0 = plyads.getTable<Opm::PlyadsTable>(0);
        BOOST_CHECK_EQUAL( table0.numColumns() , 2U );
        BOOST_CHECK_EQUAL( table0.getPolymerConcentrationColumn()[8] , 3.0 );
        BOOST_CHECK_EQUAL( table0.getAdsorbedPolymerColumn()[8] , 0.000025 );
    }
}
