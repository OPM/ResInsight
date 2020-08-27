/*
  Copyright 2019 Equinor ASA.

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

#include <stdexcept>
#include <algorithm>

#define BOOST_TEST_MODULE WLIST_TEST

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WList.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WListManager.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(CreateWLIST) {
    Opm::WList wlist;
    BOOST_CHECK_EQUAL(wlist.size(), 0);
    wlist.add("W1");
    BOOST_CHECK_EQUAL(wlist.size(), 1);


    wlist.del("NO_SUCH_WELL");
    BOOST_CHECK_EQUAL(wlist.size(), 1);

    wlist.del("W1");
    BOOST_CHECK_EQUAL(wlist.size(), 0);

    wlist.add("W1");
    wlist.add("W2");
    wlist.add("W3");

    auto wells = wlist.wells();
    BOOST_CHECK_EQUAL(wells.size(), 3);
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "W1") != wells.end());
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "W2") != wells.end());
    BOOST_CHECK( std::find(wells.begin(), wells.end(), "W3") != wells.end());

    std::vector<std::string> wells2;
    for (const auto& well : wlist)
        wells2.push_back(well);

    BOOST_CHECK_EQUAL(wells2.size(), 3);
    BOOST_CHECK( std::find(wells2.begin(), wells2.end(), "W1") != wells2.end());
    BOOST_CHECK( std::find(wells2.begin(), wells2.end(), "W2") != wells2.end());
    BOOST_CHECK( std::find(wells2.begin(), wells2.end(), "W3") != wells2.end());
}


BOOST_AUTO_TEST_CASE(WLISTManager) {
    Opm::WListManager wlm;
    BOOST_CHECK(!wlm.hasList("NO_SUCH_LIST"));


    {
        auto& wlist1 = wlm.newList("LIST1");
        wlist1.add("A");
        wlist1.add("B");
        wlist1.add("C");
    }

    // If a new list is added with the same name as an existing list the old
    // list is dropped and a new list is created.
    {
        auto& wlist1 = wlm.newList("LIST1");
        BOOST_CHECK_EQUAL(wlist1.size(), 0);
    }
    auto& wlist1 = wlm.newList("LIST1");
    auto& wlist2 = wlm.newList("LIST2");

    wlist1.add("W1");
    wlist1.add("W2");
    wlist1.add("W3");

    wlist2.add("W1");
    wlist2.add("W2");
    wlist2.add("W3");

    // The delWell operation will work across all well lists.
    wlm.delWell("W1");
    BOOST_CHECK( std::find(wlist1.begin(), wlist1.end(), "W1") == wlist1.end());
    BOOST_CHECK( std::find(wlist2.begin(), wlist2.end(), "W1") == wlist2.end());
}


static std::string WELSPECS() {
    return
        "WELSPECS\n"
        "  \'W1\'  \'OP\'  1 1 1.0 \'OIL\' 7* /\n"
        "  \'W2\'  \'OP\'  2 1 1.0 \'OIL\' 7* /\n"
        "  \'W3\'  \'OP\'  3 1 1.0 \'OIL\' 7* /\n"
        "  \'W4\'  \'OP\'  4 1 1.0 \'OIL\' 7* /\n"
        "  \'C1\'  \'OP\'  3 1 1.0 \'OIL\' 7* /\n"
        "  \'C2\'  \'OP\'  4 1 1.0 \'OIL\' 7* /\n"
        "/\n";
}

static Opm::Schedule createSchedule(const std::string& schedule) {
    Opm::Parser parser;
    std::string input =
        "START             -- 0 \n"
        "10 MAI 2007 / \n"
        "SCHEDULE\n"+ schedule;

    /*
        "SCHEDULE\n"
        "WELSPECS\n"
        "     \'W_1\'        \'OP\'   30   37  3.33       \'OIL\'  7* /   \n"
        "/ \n"
        "DATES             -- 1\n"
        " 10  \'JUN\'  2007 / \n"
        "/\n"
        "DATES             -- 2,3\n"
        "  10  JLY 2007 / \n"
        "   10  AUG 2007 / \n"
        "/\n"
        "WELSPECS\n"
        "     \'WX2\'        \'OP\'   30   37  3.33       \'OIL\'  7* /   \n"
        "     \'W_3\'        \'OP\'   20   51  3.92       \'OIL\'  7* /  \n"
        "/\n";
    */

    auto deck = parser.parseString(input);
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    auto python = std::make_shared<Python>();
    return Schedule(deck, grid , fp, runspec, python );
}


BOOST_AUTO_TEST_CASE(WlistFromDeck) {
    std::string no_wlist = WELSPECS();
    no_wlist +=
        "DATES\n"
        "10 JLY 2007 /\n"
        "10 AUG 2007 /\n"
        "/\n";


    Opm::Schedule sched = createSchedule(no_wlist);
    auto& wlm = sched.getWListManager(1);
    BOOST_CHECK(!wlm.hasList("LIST1"));
}

BOOST_AUTO_TEST_CASE(WlistInvalid) {
  std::string wlist_invalid_well = WELSPECS() +
    "WLIST\n"
    " \'*LIST1\' \'NEW\' WELLX /\n"
      "/\n"
    "DATES\n"
    "10 JLY 2007 /\n"
    "10 AUG 2007 /\n"
    "/\n";

  std::string wlist_invalid_action = WELSPECS() +
    "WLIST\n"
    " \'*LIST1\' \'NEWX\' W1 /\n"
    "/\n"
    "DATES\n"
    "10 JLY 2007 /\n"
    "10 AUG 2007 /\n"
    "/\n";

  std::string wlist_invalid_list1 = WELSPECS() +
    "WLIST\n"
    " \'LIST1\' \'NEW\' W1 /\n"
    "/\n"
    "DATES\n"
    "10 JLY 2007 /\n"
    "10 AUG 2007 /\n"
    "/\n";

  std::string wlist_invalid_list2 = WELSPECS() +
    "WLIST\n"
    " \'*LIST1\' \'NEW\' W1 /\n"
    " \'*LIST2\' \'ADD\' W2 /\n"
    "/\n"
    "DATES\n"
    "10 JLY 2007 /\n"
    "10 AUG 2007 /\n"
    "/\n";

  BOOST_CHECK_THROW( createSchedule(wlist_invalid_well), std::invalid_argument);
  BOOST_CHECK_THROW( createSchedule(wlist_invalid_well), std::invalid_argument);
  BOOST_CHECK_THROW( createSchedule(wlist_invalid_action), std::invalid_argument);
  BOOST_CHECK_THROW( createSchedule(wlist_invalid_list1), std::invalid_argument);
  BOOST_CHECK_THROW( createSchedule(wlist_invalid_list2), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Wlist) {
  std::string wlist = WELSPECS() +
      "WLIST\n"
      " \'*LIST1\' \'NEW\' W1 W2 /\n"
      " \'*LIST1\' \'ADD\' W3 W4 /\n"
      " \'*LIST2\' \'NEW\' W1 W3 /\n"
      " \'*LIST4\' \'NEW\' \'*LIST1\' /\n"
      " \'*LIST5\' \'NEW\' \'W*\' /\n"
      " \'*LIST6\' \'NEW\' \'I*\' /\n"
      "/\n"
      "DATES\n"
      "10 JLY 2007 /\n"
      "10 AUG 2007 /\n"
      "/\n"
      "WLIST\n"
      " \'*LIST3\' \'NEW\' /\n"
      " \'*LIST3\' \'MOV\' W1 W3 /\n"
      "/\n";

  auto sched = createSchedule(wlist);
  {
      const auto& wlm = sched.getWListManager(1);
      const auto& wl1 = wlm.getList("*LIST1");
      const auto& wl2 = wlm.getList("*LIST2");
      const auto& wl4 = wlm.getList("*LIST4");
      const auto& wl5 = wlm.getList("*LIST5");
      const auto& wl6 = wlm.getList("*LIST6");

      BOOST_CHECK_EQUAL(wl1.wells().size(), 4 );
      BOOST_CHECK_EQUAL(wl2.wells().size(), 2 );
      BOOST_CHECK_EQUAL(wl4.wells().size(), 4 );
      BOOST_CHECK_EQUAL(wl5.wells().size(), 4 );
      BOOST_CHECK_EQUAL(wl6.wells().size(), 0 );
  }
  {
      const auto& wlm = sched.getWListManager(2);
      const auto& wl1 = wlm.getList("*LIST1");
      const auto& wl2 = wlm.getList("*LIST2");
      const auto& wl3 = wlm.getList("*LIST3");

      BOOST_CHECK_EQUAL(wl1.wells().size(), 2 );
      BOOST_CHECK_EQUAL(wl2.wells().size(), 0 );
      BOOST_CHECK_EQUAL(wl3.wells().size(), 2 );

      BOOST_CHECK( wl1.has("W2"));
      BOOST_CHECK( wl1.has("W4"));

      BOOST_CHECK( !wl2.has("W1"));
      BOOST_CHECK( !wl2.has("W3"));

      BOOST_CHECK( wl3.has("W1"));
      BOOST_CHECK( wl3.has("W3"));

      const auto& wells1 = wlm.wells("*LIST1");
      BOOST_CHECK( wells1 == wl1.wells() );
  }
}

template <typename T>
bool vector_equal(const std::vector<T>& v1, const std::vector<T>& v2) {
    std::unordered_set<T> s1(v1.begin(), v1.end());
    std::unordered_set<T> s2(v2.begin(), v2.end());
    return s1 == s2;
}


BOOST_AUTO_TEST_CASE(WlistPattern) {
  std::string wlist = WELSPECS() +
      "WLIST\n"
      " \'*LIST1\' \'NEW\' W1 W2 /\n"
      " \'*COLL1\' \'NEW\' C1 /\n"
      " \'*BOLL2\' \'NEW\' C2 /\n"
      " \'*LIST2\' \'NEW\' W1 W3 /\n"
      "/\n"
      "DATES\n"
      "10 JLY 2007 /\n"
      "10 AUG 2007 /\n"
      "/\n";

  auto sched = createSchedule(wlist);
  const auto& wlm = sched.getWListManager(1);
  BOOST_CHECK( vector_equal(wlm.wells("*LIST1"), {"W1", "W2"}));
  BOOST_CHECK( vector_equal(wlm.wells("*LIST2"), {"W1", "W3"}));
  BOOST_CHECK( vector_equal(wlm.wells("*LIST*"), {"W1", "W2", "W3"}));
  BOOST_CHECK( vector_equal(wlm.wells("**OLL*"), {"C1", "C2"}));
}
