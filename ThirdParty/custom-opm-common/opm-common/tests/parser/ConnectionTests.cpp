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

#include <cstddef>
#include <stdexcept>
#include <iostream>

#define BOOST_TEST_MODULE CompletionTests
#include <boost/test/unit_test.hpp>

#include <opm/common/utility/ActiveGridCells.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

#include <opm/input/eclipse/Schedule/Well/Connection.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/Schedule/CompletedCells.hpp>
#include <opm/input/eclipse/Schedule/ScheduleGrid.hpp>

#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>

#include <opm/input/eclipse/Units/Units.hpp>

namespace {
    double cp_rm3_per_db()
    {
        return Opm::prefix::centi*Opm::unit::Poise * Opm::unit::cubic(Opm::unit::meter)
            / (Opm::unit::day * Opm::unit::barsa);
    }

Opm::WellConnections loadCOMPDAT(const std::string& compdat_keyword) {
    Opm::EclipseGrid grid(10,10,10);
    Opm::TableManager tables;
    Opm::Parser parser;
    const auto deck = parser.parseString(compdat_keyword);
    Opm::FieldPropsManager field_props(deck, Opm::Phases{true, true, true}, grid, Opm::TableManager());
    const auto& keyword = deck["COMPDAT"][0];
    Opm::WellConnections connections(Opm::Connection::Order::TRACK, 10,10);
    Opm::CompletedCells cells(grid);
    for (const auto& rec : keyword)
        connections.loadCOMPDAT(rec, Opm::ScheduleGrid(grid, field_props, cells), "WELL", {});

    return connections;
}
}

namespace Opm {

inline std::ostream& operator<<( std::ostream& stream, const Connection& c ) {
    return stream << "(" << c.getI() << "," << c.getJ() << "," << c.getK() << ")";
}

inline std::ostream& operator<<( std::ostream& stream, const WellConnections& cs ) {
    stream << "{ ";
    for( const auto& c : cs ) stream << c << " ";
    return stream << "}";
}

}





BOOST_AUTO_TEST_CASE(CreateWellConnectionsOK) {
    Opm::WellConnections completionSet(Opm::Connection::Order::TRACK, 1,1);
    BOOST_CHECK_MESSAGE( completionSet.empty(), "Default-constructed completion set must be empty" );
    BOOST_CHECK_EQUAL( 0U , completionSet.size() );
    BOOST_CHECK(!completionSet.allConnectionsShut());
}



BOOST_AUTO_TEST_CASE(AddCompletionSizeCorrect) {
    auto dir = Opm::Connection::Direction::Z;
    const auto kind = Opm::Connection::CTFKind::DeckValue;
    Opm::WellConnections completionSet(Opm::Connection::Order::TRACK, 1,1);
    Opm::Connection completion1( 10,10,10, 100, 1, 0.0, Opm::Connection::State::OPEN , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);
    Opm::Connection completion2( 10,10,11, 102, 1, 0.0, Opm::Connection::State::SHUT , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);
    completionSet.add( completion1 );
    BOOST_CHECK_EQUAL( 1U , completionSet.size() );
    BOOST_CHECK_MESSAGE( !completionSet.empty(), "Non-empty completion set must not be empty" );

    completionSet.add( completion2 );
    BOOST_CHECK_EQUAL( 2U , completionSet.size() );

    BOOST_CHECK_EQUAL( completion1 , completionSet.get(0) );
}


BOOST_AUTO_TEST_CASE(WellConnectionsGetOutOfRangeThrows) {
    auto dir = Opm::Connection::Direction::Z;
    const auto kind = Opm::Connection::CTFKind::DeckValue;
    Opm::Connection completion1( 10,10,10, 100, 1, 0.0, Opm::Connection::State::OPEN , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0,true);
    Opm::Connection completion2( 10,10,11, 102, 1, 0.0, Opm::Connection::State::SHUT , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0,true);
    Opm::WellConnections completionSet(Opm::Connection::Order::TRACK, 1,1);
    completionSet.add( completion1 );
    BOOST_CHECK_EQUAL( 1U , completionSet.size() );

    completionSet.add( completion2 );
    BOOST_CHECK_EQUAL( 2U , completionSet.size() );

    BOOST_CHECK_THROW( completionSet.get(10) , std::out_of_range );
}





BOOST_AUTO_TEST_CASE(AddCompletionCopy) {
    Opm::WellConnections completionSet(Opm::Connection::Order::TRACK, 10,10);
    auto dir = Opm::Connection::Direction::Z;
    const auto kind = Opm::Connection::CTFKind::DeckValue;

    Opm::Connection completion1( 10,10,10, 100, 1, 0.0, Opm::Connection::State::OPEN , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);
    Opm::Connection completion2( 10,10,11, 101, 1, 0.0, Opm::Connection::State::SHUT , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);
    Opm::Connection completion3( 10,10,12, 102, 1, 0.0, Opm::Connection::State::SHUT , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);

    completionSet.add( completion1 );
    completionSet.add( completion2 );
    completionSet.add( completion3 );
    BOOST_CHECK_EQUAL( 3U , completionSet.size() );

    auto copy = completionSet;
    BOOST_CHECK_EQUAL( 3U , copy.size() );

    BOOST_CHECK_EQUAL( completion1 , copy.get(0));
    BOOST_CHECK_EQUAL( completion2 , copy.get(1));
    BOOST_CHECK_EQUAL( completion3 , copy.get(2));
}


BOOST_AUTO_TEST_CASE(ActiveCompletions) {
    Opm::EclipseGrid grid(10,20,20);
    auto dir = Opm::Connection::Direction::Z;
    const auto kind = Opm::Connection::CTFKind::Defaulted;
    Opm::WellConnections completions(Opm::Connection::Order::TRACK, 10,10);
    Opm::Connection completion1( 0,0,0, grid.getGlobalIndex(0,0,0), 1, 0.0, Opm::Connection::State::OPEN , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);
    Opm::Connection completion2( 0,0,1, grid.getGlobalIndex(0,0,1), 1, 0.0, Opm::Connection::State::SHUT , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);
    Opm::Connection completion3( 0,0,2, grid.getGlobalIndex(0,0,2), 1, 0.0, Opm::Connection::State::SHUT , 99.88, 355.113, 0.25, 0.0, 0.0, 0.0, 0.0, 0, dir, kind, 0, true);

    completions.add( completion1 );
    completions.add( completion2 );
    completions.add( completion3 );

    std::vector<int> actnum(grid.getCartesianSize(), 1);
    actnum[0] = 0;
    grid.resetACTNUM( actnum);

    Opm::WellConnections active_completions(completions, grid);
    BOOST_CHECK_EQUAL( active_completions.size() , 2U);
    BOOST_CHECK_EQUAL( completion2, active_completions.get(0));
    BOOST_CHECK_EQUAL( completion3, active_completions.get(1));
}

BOOST_AUTO_TEST_CASE(loadCOMPDATTEST) {
    Opm::UnitSystem units(Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC); // Unit system used in deck FIRST_SIM.DATA.
    {
        const std::string deck = R"(GRID

PERMX
  1000*0.10 /

COPY
  'PERMX' 'PERMZ' /
  'PERMX' 'PERMY' /
/
SCHEDULE

COMPDAT
--                                    CF      Diam    Kh      Skin   Df
    'WELL'  1  1   1   1 'OPEN' 1*    1.168   0.311   107.872 1*     1*  'Z'  21.925 /
/)";
        Opm::WellConnections connections = loadCOMPDAT(deck);
        const auto& conn0 = connections[0];
        BOOST_CHECK_EQUAL(conn0.CF(), units.to_si(Opm::UnitSystem::measure::transmissibility, 1.168));
        BOOST_CHECK_EQUAL(conn0.Kh(), units.to_si(Opm::UnitSystem::measure::effective_Kh, 107.872));
        BOOST_CHECK_MESSAGE(conn0.ctfAssignedFromInput(), "CTF Must be Assigned From Input");
    }

    {
        const std::string deck = R"(GRID

PERMX
  1000*0.10 /

COPY
  'PERMX' 'PERMZ' /
  'PERMX' 'PERMY' /
/

SCHEDULE

COMPDAT
--                                CF      Diam    Kh      Skin   Df
'WELL'  1  1   1   1 'OPEN' 1*    1.168   0.311   0       1*     1*  'Z'  21.925 /
/)";
        Opm::WellConnections connections = loadCOMPDAT(deck);
        const auto& conn0 = connections[0];
        BOOST_CHECK_EQUAL(conn0.CF(), units.to_si(Opm::UnitSystem::measure::transmissibility, 1.168));
        BOOST_CHECK_EQUAL(conn0.Kh(), units.to_si(Opm::UnitSystem::measure::effective_Kh, 0.10 * 1.0));
    }
}


BOOST_AUTO_TEST_CASE(loadCOMPDATTESTSPE1) {
    Opm::Parser parser;

    const auto deck = parser.parseFile("SPE1CASE1.DATA");
    auto python = std::make_shared<Opm::Python>();
    Opm::EclipseState state(deck);
    Opm::Schedule sched(deck, state, python);
    const auto& units = deck.getActiveUnitSystem();

    const auto& prod = sched.getWell("PROD", 0);
    const auto& connections = prod.getConnections();
    const auto& conn0 = connections[0];
    /* Expected values come from Eclipse simulation. */
    BOOST_CHECK_CLOSE(conn0.CF(), units.to_si(Opm::UnitSystem::measure::transmissibility, 10.609), 2e-2);
    BOOST_CHECK_CLOSE(conn0.Kh(), units.to_si(Opm::UnitSystem::measure::effective_Kh, 10000), 1e-6);
    BOOST_CHECK_MESSAGE(!conn0.ctfAssignedFromInput(), "Calculated CTF must NOT be assigned from input");
}


struct exp_conn {
    std::string well;
    int ci;
    double CF;
    double Kh;
};

BOOST_AUTO_TEST_CASE(loadCOMPDATTESTSPE9) {
    Opm::Parser parser;

    const auto deck = parser.parseFile("SPE9_CP_PACKED.DATA");
    auto python = std::make_shared<Opm::Python>();
    Opm::EclipseState state(deck);
    Opm::Schedule sched(deck, state, python);
    const auto& units = deck.getActiveUnitSystem();
/*
  The list of the expected values come from the PRT file in an ECLIPSE simulation.
*/
    std::vector<exp_conn> expected = {
  {"INJE1"   ,1 ,     0.166,    111.9},
  {"INJE1"   ,2 ,     0.597,    402.6},
  {"INJE1"   ,3 ,     1.866,   1259.2},
  {"INJE1"   ,4 ,    12.442,   8394.2},
  {"INJE1"   ,5 ,     6.974,   4705.3},

  {"PRODU2"  ,1 ,     0.893,    602.8},
  {"PRODU2"  ,2 ,     3.828,   2582.8},
  {"PRODU2"  ,3 ,     0.563,    380.0},

  {"PRODU3"  ,1 ,     1.322,    892.1},
  {"PRODU3"  ,2 ,     3.416,   2304.4},

  {"PRODU4"  ,1 ,     4.137,   2791.2},
  {"PRODU4"  ,2 ,    66.455,  44834.7},

  {"PRODU5"  ,1 ,     0.391,    264.0},
  {"PRODU5"  ,2 ,     7.282,   4912.6},
  {"PRODU5"  ,3 ,     1.374,    927.3},

  {"PRODU6"  ,1 ,     1.463,    987.3},
  {"PRODU6"  ,2 ,     1.891,   1275.8},

  {"PRODU7"  ,1 ,     1.061,    716.1},
  {"PRODU7"  ,2 ,     5.902,   3982.0},
  {"PRODU7"  ,3 ,     0.593,    400.1},

  {"PRODU8"  ,1 ,     0.993,    670.1},
  {"PRODU8"  ,2 ,    17.759,  11981.5},

  {"PRODU9"  ,1 ,     0.996,    671.9},
  {"PRODU9"  ,2 ,     2.548,   1719.0},

  {"PRODU10" ,1 ,    11.641,   7853.9},
  {"PRODU10" ,2 ,     7.358,   4964.1},
  {"PRODU10" ,3 ,     0.390,    262.8},

  {"PRODU11" ,2 ,     3.536,   2385.6},

  {"PRODU12" ,1 ,     3.028,   2043.1},
  {"PRODU12" ,2 ,     0.301,    202.7},
  {"PRODU12" ,3 ,     0.279,    188.3},

  {"PRODU13" ,2 ,     5.837,   3938.1},

  {"PRODU14" ,1 ,   180.976, 122098.1},
  {"PRODU14" ,2 ,    25.134,  16957.0},
  {"PRODU14" ,3 ,     0.532,    358.7},

  {"PRODU15" ,1 ,     4.125,   2783.1},
  {"PRODU15" ,2 ,     6.431,   4338.7},

  {"PRODU16" ,2 ,     5.892,   3975.0},

  {"PRODU17" ,1 ,    80.655,  54414.9},
  {"PRODU17" ,2 ,     9.098,   6138.3},

  {"PRODU18" ,1 ,     1.267,    855.1},
  {"PRODU18" ,2 ,    18.556,  12518.9},

  {"PRODU19" ,1 ,    15.589,  10517.2},
  {"PRODU19" ,3 ,     1.273,    859.1},

  {"PRODU20" ,1 ,     3.410,   2300.5},
  {"PRODU20" ,2 ,     0.191,    128.8},
  {"PRODU20" ,3 ,     0.249,    168.1},

  {"PRODU21" ,1 ,     0.596,    402.0},
  {"PRODU21" ,2 ,     0.163,    109.9},

  {"PRODU22" ,1 ,     4.021,   2712.8},
  {"PRODU22" ,2 ,     0.663,    447.1},

  {"PRODU23" ,1 ,     1.542,   1040.2},

  {"PRODU24" ,1 ,    78.939,  53257.0},
  {"PRODU24" ,3 ,    17.517,  11817.8},

  {"PRODU25" ,1 ,     3.038,   2049.5},
  {"PRODU25" ,2 ,     0.926,    624.9},
  {"PRODU25" ,3 ,     0.891,    601.3},

  {"PRODU26" ,1 ,     0.770,    519.6},
  {"PRODU26" ,3 ,     0.176,    118.6}};

   for (const auto& ec : expected) {
     const auto& well = sched.getWell(ec.well, 0);
       const auto& connections = well.getConnections();
       const auto& conn = connections[ec.ci - 1];

       BOOST_CHECK_CLOSE( conn.CF(), units.to_si(Opm::UnitSystem::measure::transmissibility, ec.CF), 2e-1);
       BOOST_CHECK_CLOSE( conn.Kh(), units.to_si(Opm::UnitSystem::measure::effective_Kh, ec.Kh), 1e-1);
       BOOST_CHECK_MESSAGE( !conn.ctfAssignedFromInput(), "Calculated SPE9 CTF values must NOT be assigned from input");
   }
}

BOOST_AUTO_TEST_CASE(ApplyWellPI) {
    const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
DIMENS
10 10 3 /

START
  5 OCT 2020 /

GRID
DXV
  10*100 /
DYV
  10*100 /
DZV
  3*10 /
DEPTHZ
  121*2000 /

ACTNUM
  100*1
  99*1 0
  100*1
/

PERMX
  300*100 /
PERMY
  300*100 /
PERMZ
  300*100 /
PORO
  300*0.3 /

SCHEDULE
WELSPECS
  'P' 'G' 10 10 2005 'LIQ' /
/

COMPDAT
  'P' 0 0 1 3 OPEN 1 100 /
/

TSTEP
  10
/

END
)");

    const auto es    = Opm::EclipseState{ deck };
    const auto sched = Opm::Schedule{ deck, es };

    const auto expectCF = 100.0*cp_rm3_per_db();

    auto connP = sched.getWell("P", 0).getConnections();
    for (const auto& conn : connP) {
        BOOST_CHECK_CLOSE(conn.CF(), expectCF, 1.0e-10);
    }

    {
        std::vector<bool> scalingApplicable;

        connP.applyWellPIScaling(2.0, scalingApplicable);  // No "prepare" -> no change.
        for (const auto& conn : connP) {
            BOOST_CHECK_CLOSE(conn.CF(), expectCF, 1.0e-10);
        }
    }

    // All CFs scaled by factor 2.
    BOOST_CHECK_MESSAGE( connP.prepareWellPIScaling(), "First call to prepareWellPIScaling must be a state change");
    BOOST_CHECK_MESSAGE(!connP.prepareWellPIScaling(), "Second call to prepareWellPIScaling must NOT be a state change");

    {
        std::vector<bool> scalingApplicable;

        connP.applyWellPIScaling(2.0, scalingApplicable);  // No "prepare" -> no change.
        for (const auto& conn : connP) {
            BOOST_CHECK_CLOSE(conn.CF(), 2.0*expectCF, 1.0e-10);
        }
    }

    // Reset CF -- simulating COMPDAT record (inactive cell)
    connP.addConnection(9, 9, 1, // 10, 10, 2
        199,
        2015.0,
        Opm::Connection::State::OPEN,
        50.0*cp_rm3_per_db(),
        0.123,
        0.234,
        0.157,
        0.0,
        0.0,
        0.0,
        1);

    BOOST_REQUIRE_EQUAL(connP.size(), std::size_t{3});

    BOOST_CHECK_CLOSE(connP[0].CF(),  2.0*expectCF       , 1.0e-10);
    BOOST_CHECK_CLOSE(connP[1].CF(),  2.0*expectCF       , 1.0e-10);
    BOOST_CHECK_CLOSE(connP[2].CF(), 50.0*cp_rm3_per_db(), 1.0e-10);

    // Should not apply to connection whose CF was manually specified
    {
        std::vector<bool> scalingApplicable;
        connP.applyWellPIScaling(2.0, scalingApplicable);

        BOOST_CHECK_CLOSE(connP[0].CF(),  4.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[1].CF(),  4.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[2].CF(), 50.0*cp_rm3_per_db(), 1.0e-10);
    }

    // Prepare new scaling.  Simulating new WELPI record.
    // New scaling applies to all connections.
    BOOST_CHECK_MESSAGE(connP.prepareWellPIScaling(), "Third call to prepareWellPIScaling must be a state change");

    {
        std::vector<bool> scalingApplicable;
        connP.applyWellPIScaling(2.0, scalingApplicable);

        BOOST_CHECK_CLOSE(connP[0].CF(),   8.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[1].CF(),   8.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[2].CF(), 100.0*cp_rm3_per_db(), 1.0e-10);
    }

    // Reset CF -- simulating COMPDAT record (active cell)
    connP.addConnection(8, 9, 1, // 10, 10, 2
        198,
        2015.0,
        Opm::Connection::State::OPEN,
        50.0*cp_rm3_per_db(),
        0.123,
        0.234,
        0.157,
        0.0,
        0.0,
        0.0,
        1);

    BOOST_REQUIRE_EQUAL(connP.size(), std::size_t{4});

    {
        std::vector<bool> scalingApplicable;
        connP.applyWellPIScaling(2.0, scalingApplicable);

        BOOST_CHECK_CLOSE(connP[0].CF(),  16.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[1].CF(),  16.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[2].CF(), 200.0*cp_rm3_per_db(), 1.0e-10);
        BOOST_CHECK_CLOSE(connP[3].CF(),  50.0*cp_rm3_per_db(), 1.0e-10);
    }

    const auto& grid = es.getInputGrid();
    const auto actCells = Opm::ActiveGridCells {
        std::size_t{10}, std::size_t{10}, std::size_t{3},
        grid.getActiveMap().data(),
        grid.getNumActive()
    };

    connP.filter(actCells);

    BOOST_REQUIRE_EQUAL(connP.size(), std::size_t{3});
    BOOST_CHECK_CLOSE(connP[0].CF(), 16.0*expectCF       , 1.0e-10);
    BOOST_CHECK_CLOSE(connP[1].CF(), 16.0*expectCF       , 1.0e-10);
    BOOST_CHECK_CLOSE(connP[2].CF(), 50.0*cp_rm3_per_db(), 1.0e-10);

    {
        std::vector<bool> scalingApplicable;

        connP.applyWellPIScaling(2.0, scalingApplicable);
        BOOST_CHECK_CLOSE(connP[0].CF(), 32.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[1].CF(), 32.0*expectCF       , 1.0e-10);
        BOOST_CHECK_CLOSE(connP[2].CF(), 50.0*cp_rm3_per_db(), 1.0e-10);
    }
}

BOOST_AUTO_TEST_CASE(Completion_From_Global_Connection_Index) {
    const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
START
7 OCT 2020 /

DIMENS
  10 10 3 /

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PERMX
  300*100.0 /
PERMY
  300*100.0 /
PERMZ
  300*10.0 /
PORO
  300*0.3 /

SCHEDULE
WELSPECS
  'P' 'G' 10 10 2005 'LIQ' /
/
COMPDAT
  'P' 0 0 1 1 OPEN 1 100 /
/

TSTEP
  10
/

COMPDAT
  'P' 0 0 2 2 OPEN 1 50 /
/

TSTEP
  10
/

END
)");

    const auto es    = Opm::EclipseState{ deck };
    const auto sched = Opm::Schedule{ deck, es };

    {
        const auto connP = sched.getWell("P", 0).getConnections();

        const auto complnum_100 =
            getCompletionNumberFromGlobalConnectionIndex(connP, 100 - 1);
        const auto complnum_200 =
            getCompletionNumberFromGlobalConnectionIndex(connP, 200 - 1);

        BOOST_CHECK_MESSAGE(  complnum_100.has_value(), "Completion number must be defined at time 0 for connection in cell (10,10,1)");
        BOOST_CHECK_MESSAGE(! complnum_200.has_value(), "Completion number must NOT be defined at time 0 for connection in cell (10,10,2)");

        BOOST_CHECK_EQUAL(complnum_100.value(), 1);
    }

    {
        const auto connP = sched.getWell("P", 1).getConnections();

        const auto complnum_100 =
            getCompletionNumberFromGlobalConnectionIndex(connP, 100 - 1);
        const auto complnum_200 =
            getCompletionNumberFromGlobalConnectionIndex(connP, 200 - 1);

        BOOST_CHECK_MESSAGE(complnum_100.has_value(), "Completion number must be defined at time 0 for connection in cell (10,10,1)");
        BOOST_CHECK_MESSAGE(complnum_200.has_value(), "Completion number must be defined at time 0 for connection in cell (10,10,2)");

        BOOST_CHECK_EQUAL(complnum_100.value(), 1);
        BOOST_CHECK_EQUAL(complnum_200.value(), 2);
    }
}

BOOST_AUTO_TEST_CASE(testReAndConnectionLength) {
    Opm::Parser parser;

    const auto deck = parser.parseFile("SPE1CASE1.DATA");
    auto python = std::make_shared<Opm::Python>();
    Opm::EclipseState state(deck);
    Opm::Schedule sched(deck, state, python);

    const auto& prod = sched.getWell("PROD", 0);
    const auto& connections = prod.getConnections();
    const auto& conn0 = connections[0];
    BOOST_CHECK_CLOSE(conn0.re(), 171.96498506535622 , 2e-2);
    BOOST_CHECK_CLOSE(conn0.connectionLength(),15.239999999999782, 1e-6);
}
