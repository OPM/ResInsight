/*
  Copyright 2021 Equinor ASA.

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

#define BOOST_TEST_MODULE ScheduleTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/ScheduleState.hpp>
#include <opm/input/eclipse/Schedule/OilVaporizationProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/GasLiftOpt.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Well/WellMatcher.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvg.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/input/eclipse/Schedule/Group/GuideRateConfig.hpp>
#include <opm/input/eclipse/Schedule/Group/GuideRate.hpp>

using namespace Opm;

std::string deck0 = R"(
START             -- 0
10 MAI 2007 /
GRID
PORO
    1000*0.1 /
PERMX
    1000*1 /
PERMY
    1000*0.1 /
PERMZ
    1000*0.01 /
SCHEDULE

DATES             -- 1
 10  JUN 2007 /
/

DATES             -- 2
 15  JUN 2007 /
/

DATES             -- 3
 20  JUN 2007 /
/

DATES             -- 4
 10  JUL 2007 /
/


DATES             -- 5
 10  AUG 2007 /
/


DATES             -- 6
 10  SEP 2007 /
/

DATES             -- 7
 10  NOV 2007 /
/

)";



std::string WTEST_deck = R"(
START             -- 0
10 MAI 2007 /
GRID
PORO
    1000*0.1 /
PERMX
    1000*1 /
PERMY
    1000*0.1 /
PERMZ
    1000*0.01 /
SCHEDULE
WELSPECS
     'DEFAULT'    'OP'   30   37  3.33       'OIL'  7*/
     'ALLOW'      'OP'   30   37  3.33       'OIL'  3*  YES /
     'BAN'        'OP'   20   51  3.92       'OIL'  3*  NO /
     'W1'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'W2'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'W3'         'OP'   20   51  3.92       'OIL'  3*  NO /
/

COMPDAT
 'BAN'  1  1   1   1 'OPEN' 1*    1.168   0.311   107.872 1*  1*  'Z'  21.925 /
/

WCONHIST
     'BAN'      'OPEN'      'RESV'      0.000      0.000      0.000  5* /
/

WTEST
   'ALLOW'   1   'PE' /
/

WLIST
  '*ILIST'  'NEW'  W1 /
  '*ILIST'  'ADD'  W2 /
/

DATES             -- 1
 10  JUN 2007 /
/

DATES             -- 2
 15  JUN 2007 /
/

DATES             -- 3
 20  JUN 2007 /
/


WTEST
   'ALLOW'  1  '' /
   'BAN'    1  'DGC' /
/

WLIST
  '*ILIST'  'ADD'  W3 /
/


WCONHIST
     'BAN'      'OPEN'      'RESV'      1.000      0.000      0.000  5* /
/

DATES             -- 4
 10  JUL 2007 /
/

WELSPECS
     'I1'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'I2'         'OP'   20   51  3.92       'OIL'  3*  NO /
     'I3'         'OP'   20   51  3.92       'OIL'  3*  NO /
/


WCONPROD
     'BAN'      'OPEN'      'ORAT'      0.000      0.000      0.000  5* /
/


DATES             -- 5
 10  AUG 2007 /
/

WCONINJH
     'BAN'      'WATER'      1*      0 /
/

DATES             -- 6
 10  SEP 2007 /
/

WELOPEN
 'BAN' OPEN /
/

DATES             -- 7
 10  NOV 2007 /
/

WCONINJH
     'BAN'      'WATER'      1*      1.0 /
/
)";

std::string GCONSALE_deck = R"(
START             -- 0
10 MAI 2007 /
SCHEDULE

GRUPTREE
   'G1'  'FIELD' /
   'G2'  'FIELD' /
/

GCONSALE
'G1' 50000 55000 45000 WELL /
/

GCONSUMP
'G1' 20 50 'a_node' /
'G2' 30 60 /
/

DATES             -- 1
 10  JUN 2007 /
/

DATES             -- 2
 15  JUN 2007 /
/

DATES             -- 3
 20  JUN 2007 /
/

GRUPTREE
   'G3'  'G2' /
/

GCONSALE
'G1' 12345 12345 12345 WELL /
/

GCONSUMP
'G1' 10 77 'b_node' /
'G2' 10 77 /
/

DATES             -- 4
 10  JUL 2007 /
/

DATES             -- 5
 10  AUG 2007 /
/

DATES             -- 6
 10  SEP 2007 /
/

DATES             -- 7
 10  NOV 2007 /
/



)";

std::string VFP_deck1 = R"(
START             -- 0
10 MAI 2007 /
SCHEDULE

VFPINJ
5  32.9   WAT   THP METRIC   BHP /
1 3 5 /
7 11 /
1 1.5 2.5 3.5 /
2 4.5 5.5 6.5 /

DATES             -- 1
 10  JUN 2007 /
/

DATES             -- 2
 15  JUN 2007 /
/

DATES             -- 3
 20  JUN 2007 /
/

VFPINJ
5  32.9   WAT   THP METRIC   BHP /
1 3 5 /
7 11 /
1 1.5 2.5 3.4 /
2 4.5 5.5 6.4 /

DATES             -- 4
 10  JUL 2007 /
/

DATES             -- 5
 10  AUG 2007 /
/

DATES             -- 6
 10  SEP 2007 /
/

DATES             -- 7
 10  NOV 2007 /
/
)";




static Schedule make_schedule(const std::string& deck_string) {
    const auto& deck = Parser{}.parseString(deck_string);
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    return Schedule(deck, grid , fp, runspec, python);
}



BOOST_AUTO_TEST_CASE(SerializeWTest) {
    auto sched = make_schedule(WTEST_deck);
    auto sched0 = make_schedule(deck0);
    auto wtest1 = sched[0].wtest_config();
    auto wtest2 = sched[3].wtest_config();

    {
        std::vector<Opm::WellTestConfig> value_list;
        std::vector<std::size_t> index_list;
        sched.pack_state<Opm::WellTestConfig>( value_list, index_list );
        BOOST_CHECK_EQUAL( value_list.size(), 2 );

        sched0.unpack_state<Opm::WellTestConfig>( value_list, index_list );
    }
    BOOST_CHECK( wtest1 == sched0[0].wtest_config());
    BOOST_CHECK( wtest1 == sched0[1].wtest_config());
    BOOST_CHECK( wtest1 == sched0[2].wtest_config());

    BOOST_CHECK( wtest2 == sched0[3].wtest_config());
    BOOST_CHECK( wtest2 == sched0[4].wtest_config());
    BOOST_CHECK( wtest2 == sched0[5].wtest_config());
}

BOOST_AUTO_TEST_CASE(SerializeWList) {
    auto sched = make_schedule(WTEST_deck);
    auto sched0 = make_schedule(deck0);
    auto wlm1 = sched[0].wlist_manager();
    auto wlm2 = sched[3].wlist_manager();

    {
        std::vector<Opm::WListManager> value_list;
        std::vector<std::size_t> index_list;
        sched.pack_state<Opm::WListManager>( value_list, index_list);
        BOOST_CHECK_EQUAL( value_list.size(), 2 );
        sched0.unpack_state<Opm::WListManager>( value_list, index_list );
    }
    BOOST_CHECK( wlm1 == sched0[0].wlist_manager());
    BOOST_CHECK( wlm1 == sched0[1].wlist_manager());
    BOOST_CHECK( wlm1 == sched0[2].wlist_manager());

    BOOST_CHECK( wlm2 == sched0[3].wlist_manager());
    BOOST_CHECK( wlm2 == sched0[4].wlist_manager());
    BOOST_CHECK( wlm2 == sched0[5].wlist_manager());
}



BOOST_AUTO_TEST_CASE(SerializeGCONSALE) {
    auto sched  = make_schedule(GCONSALE_deck);
    auto sched0 = make_schedule(deck0);
    auto gconsale1 = sched[0].gconsale.get();
    auto gconsale2 = sched[3].gconsale.get();

    {
        std::vector<Opm::GConSale> value_list;
        std::vector<std::size_t> index_list;
        sched.pack_state<Opm::GConSale>( value_list, index_list );
        BOOST_CHECK_EQUAL( value_list.size(), 2 );
        sched0.unpack_state<Opm::GConSale>( value_list, index_list );
    }

    BOOST_CHECK( gconsale1 == sched0[0].gconsale());
    BOOST_CHECK( gconsale1 == sched0[1].gconsale());
    BOOST_CHECK( gconsale1 == sched0[2].gconsale());

    BOOST_CHECK( gconsale2 == sched0[3].gconsale());
    BOOST_CHECK( gconsale2 == sched0[4].gconsale());
    BOOST_CHECK( gconsale2 == sched0[5].gconsale());
}

BOOST_AUTO_TEST_CASE(SerializeGCONSUMP) {
    auto sched  = make_schedule(GCONSALE_deck);
    auto sched0 = make_schedule(deck0);
    auto gconsump1 = sched[0].gconsump.get();
    auto gconsump2 = sched[3].gconsump.get();

    {
        std::vector<Opm::GConSump> value_list;
        std::vector<std::size_t> index_list;
        sched.pack_state<Opm::GConSump>( value_list, index_list );
        BOOST_CHECK_EQUAL( value_list.size(), 2 );
        sched0.unpack_state<Opm::GConSump>( value_list, index_list );
    }

    BOOST_CHECK( gconsump1 == sched0[0].gconsump());
    BOOST_CHECK( gconsump1 == sched0[1].gconsump());
    BOOST_CHECK( gconsump1 == sched0[2].gconsump());

    BOOST_CHECK( gconsump2 == sched0[3].gconsump());
    BOOST_CHECK( gconsump2 == sched0[4].gconsump());
    BOOST_CHECK( gconsump2 == sched0[5].gconsump());
}


BOOST_AUTO_TEST_CASE(SerializeVFP) {
    auto sched = make_schedule(VFP_deck1);
    auto sched0 = make_schedule(deck0);
    auto vfpinj1 = sched[0].vfpinj;
    auto vfpinj2 = sched[3].vfpinj;

    {
        std::vector<Opm::VFPInjTable> value_list;
        std::vector<std::size_t> index_list;
        sched.pack_map<int, Opm::VFPInjTable>( value_list, index_list );
        BOOST_CHECK_EQUAL( value_list.size(), 2 );
        sched0.unpack_map<int, Opm::VFPInjTable>( value_list, index_list );
    }

    BOOST_CHECK( vfpinj1 == sched0[0].vfpinj);
    BOOST_CHECK( vfpinj1 == sched0[1].vfpinj);
    BOOST_CHECK( vfpinj1 == sched0[2].vfpinj);

    BOOST_CHECK( vfpinj2 == sched0[3].vfpinj);
    BOOST_CHECK( vfpinj2 == sched0[4].vfpinj);
    BOOST_CHECK( vfpinj2 == sched0[5].vfpinj);
}

BOOST_AUTO_TEST_CASE(SerializeGROUPS) {
    auto sched = make_schedule(GCONSALE_deck);
    auto sched0 = make_schedule(deck0);
    auto groups1 = sched[0].groups;
    auto groups2 = sched[3].groups;

    {
        std::vector<Opm::Group> value_list;
        std::vector<std::size_t> index_list;
        sched.pack_map<std::string, Opm::Group>( value_list, index_list );
        BOOST_CHECK_EQUAL( value_list.size(), 5 );
        sched0.unpack_map<std::string, Opm::Group>( value_list, index_list );
    }

    BOOST_CHECK( groups1 == sched0[0].groups);
    BOOST_CHECK( groups1 == sched0[1].groups);
    BOOST_CHECK( groups1 == sched0[2].groups);

    BOOST_CHECK( groups2 == sched0[3].groups);
    BOOST_CHECK( groups2 == sched0[4].groups);
    BOOST_CHECK( groups2 == sched0[5].groups);
}








