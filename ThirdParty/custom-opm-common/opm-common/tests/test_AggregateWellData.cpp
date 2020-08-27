/*
  Copyright 2019 Equinor
  Copyright 2018 Statoil ASA

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

#define BOOST_TEST_MODULE Aggregate_Well_Data

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/output/eclipse/AggregateConnectionData.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Action/State.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/io/eclipse/rst/well.hpp>
#include <opm/io/eclipse/rst/header.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/State.hpp>

#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

struct MockIH
{
    MockIH(const int numWells,
           const int iwelPerWell = 155,  // E100
           const int swelPerWell = 122,  // E100
           const int xwelPerWell = 130,  // E100
           const int zwelPerWell =   3); // E100

    std::vector<int> value;

    using Sz = std::vector<int>::size_type;

    Sz nwells;
    Sz niwelz;
    Sz nswelz;
    Sz nxwelz;
    Sz nzwelz;
};

MockIH::MockIH(const int numWells,
               const int iwelPerWell,
               const int swelPerWell,
               const int xwelPerWell,
               const int zwelPerWell)
    : value(411, 0)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::intehead;

    this->nwells = this->value[Ix::NWELLS] = numWells;
    this->niwelz = this->value[Ix::NIWELZ] = iwelPerWell;
    this->nswelz = this->value[Ix::NSWELZ] = swelPerWell;
    this->nxwelz = this->value[Ix::NXWELZ] = xwelPerWell;
    this->nzwelz = this->value[Ix::NZWELZ] = zwelPerWell;
}

namespace {
    Opm::Deck first_sim()
    {
        // Mostly copy of tests/FIRST_SIM.DATA
        const auto input = std::string {
            R"~(
RUNSPEC
OIL
GAS
WATER
DISGAS
UNIFOUT
UNIFIN
DIMENS
 10 10 10 /
WELLDIMS
 6  20  1  6  /
TABDIMS
 1  1  15  15  2  15  /
FIELD
EQLDIMS
 1  /

GRID
DXV
10*100. /
DYV
10*100. /
DZV
10*100. /
TOPS
100*7000. /

PORO
1000*0.2 /

PERMX
1000*100. /

PERMY
1000*100. /

PERMZ
1000*10. /


PROPS       ==========================================================

-- WATER RELATIVE PERMEABILITY AND CAPILLARY PRESSURE ARE TABULATED AS
-- A FUNCTION OF WATER SATURATION.
--
--  SWAT   KRW   PCOW
SWFN

    0.12  0       0
    1.0   0.00001 0  /

-- SIMILARLY FOR GAS
--
--  SGAS   KRG   PCOG
SGFN

    0     0       0
    0.02  0       0
    0.05  0.005   0
    0.12  0.025   0
    0.2   0.075   0
    0.25  0.125   0
    0.3   0.19    0
    0.4   0.41    0
    0.45  0.6     0
    0.5   0.72    0
    0.6   0.87    0
    0.7   0.94    0
    0.85  0.98    0
    1.0   1.0     0
/

-- OIL RELATIVE PERMEABILITY IS TABULATED AGAINST OIL SATURATION
-- FOR OIL-WATER AND OIL-GAS-CONNATE WATER CASES
--
--  SOIL     KROW     KROG
SOF3

    0        0        0
    0.18     0        0
    0.28     0.0001   0.0001
    0.38     0.001    0.001
    0.43     0.01     0.01
    0.48     0.021    0.021
    0.58     0.09     0.09
    0.63     0.2      0.2
    0.68     0.35     0.35
    0.76     0.7      0.7
    0.83     0.98     0.98
    0.86     0.997    0.997
    0.879    1        1
    0.88     1        1    /


-- PVT PROPERTIES OF WATER
--
--    REF. PRES. REF. FVF  COMPRESSIBILITY  REF VISCOSITY  VISCOSIBILITY
PVTW
       4014.7     1.029        3.13D-6           0.31            0 /

-- ROCK COMPRESSIBILITY
--
--    REF. PRES   COMPRESSIBILITY
ROCK
        14.7          3.0D-6          /

-- SURFACE DENSITIES OF RESERVOIR FLUIDS
--
--        OIL   WATER   GAS
DENSITY
         49.1   64.79  0.06054  /

-- PVT PROPERTIES OF DRY GAS (NO VAPOURISED OIL)
-- WE WOULD USE PVTG TO SPECIFY THE PROPERTIES OF WET GAS
--
--   PGAS   BGAS   VISGAS
PVDG
     14.7 166.666   0.008
    264.7  12.093   0.0096
    514.7   6.274   0.0112
   1014.7   3.197   0.014
   2014.7   1.614   0.0189
   2514.7   1.294   0.0208
   3014.7   1.080   0.0228
   4014.7   0.811   0.0268
   5014.7   0.649   0.0309
   9014.7   0.386   0.047   /

-- PVT PROPERTIES OF LIVE OIL (WITH DISSOLVED GAS)
-- WE WOULD USE PVDO TO SPECIFY THE PROPERTIES OF DEAD OIL
--
-- FOR EACH VALUE OF RS THE SATURATION PRESSURE, FVF AND VISCOSITY
-- ARE SPECIFIED. FOR RS=1.27 AND 1.618, THE FVF AND VISCOSITY OF
-- UNDERSATURATED OIL ARE DEFINED AS A FUNCTION OF PRESSURE. DATA
-- FOR UNDERSATURATED OIL MAY BE SUPPLIED FOR ANY RS, BUT MUST BE
-- SUPPLIED FOR THE HIGHEST RS (1.618).
--
--   RS      POIL  FVFO  VISO
PVTO
    0.001    14.7 1.062  1.04    /
    0.0905  264.7 1.15   0.975   /
    0.18    514.7 1.207  0.91    /
    0.371  1014.7 1.295  0.83    /
    0.636  2014.7 1.435  0.695   /
    0.775  2514.7 1.5    0.641   /
    0.93   3014.7 1.565  0.594   /
    1.270  4014.7 1.695  0.51
           5014.7 1.671  0.549
           9014.7 1.579  0.74    /
    1.618  5014.7 1.827  0.449
           9014.7 1.726  0.605   /
/


REGIONS    ===========================================================


FIPNUM

  1000*1
/

EQLNUM

  1000*1
/


SOLUTION    ============================================================

EQUIL
7020.00 2700.00 7990.00  .00000 7200.00  .00000     0      0       5 /

RSVD       2 TABLES    3 NODES IN EACH           FIELD   12:00 17 AUG 83
   7000.0  1.0000
   7990.0  1.0000
/

SCHEDULE
RPTRST
BASIC=1
/
DATES             -- 1
 10  OKT 2008 /
/
WELSPECS
      'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
      'OP_2'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
      'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
      'OP_2'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 /
      'OP_1'  9  9   3   3 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/
WCONPROD
      'OP_1' 'OPEN' 'ORAT' 20000  4* 1000 /
/
WCONINJE
      'OP_2' 'GAS' 'OPEN' 'RATE' 100 200 400 /
/

DATES             -- 2
 20  JAN 2011 /
/
WELSPECS
      'OP_3'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
      'OP_3'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/

WELOPEN
      'OP_1' 'STOP' /
/
WCONPROD
      'OP_3' 'OPEN' 'ORAT' 20000  4* 1000 /
/
WCONINJE
      'OP_2' 'WATER' 'OPEN' 'RATE' 100 200 400 /
/

DATES             -- 3
 15  JUN 2013 /
/
COMPDAT
      'OP_2'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
      'OP_1'  9  9   7  7 'SHUT' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/

DATES             -- 4
 22  APR 2014 /
/
WELSPECS
      'OP_4'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
      'OP_4'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
      'OP_3'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/
WCONPROD
      'OP_4' 'OPEN' 'ORAT' 20000  4* 1000 /
/

DATES             -- 5
 30  AUG 2014 /
/
WELSPECS
      'OP_5'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
      'OP_5'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/
WCONPROD
      'OP_5' 'OPEN' 'ORAT' 20000  4* 1000 /
/

DATES             -- 6
 15  SEP 2014 /
/
WCONPROD
      'OP_3' 'SHUT' 'ORAT' 20000  4* 1000 /
/

DATES             -- 7
 9  OCT 2014 /
/
WELSPECS
      'OP_6'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  /
/
COMPDAT
      'OP_6'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/
WCONPROD
      'OP_6' 'OPEN' 'ORAT' 20000  4* 1000 /
/
TSTEP            -- 8
10 /
)~" };

        return Opm::Parser{}.parseString(input);
    }

    Opm::SummaryState sim_state()
    {
        auto state = Opm::SummaryState{std::chrono::system_clock::now()};

        state.update("WOPR:OP_1" ,    1.0);
        state.update("WWPR:OP_1" ,    2.0);
        state.update("WGPR:OP_1" ,    3.0);
        state.update("WVPR:OP_1" ,    4.0);
        state.update("WOPT:OP_1" ,   10.0);
        state.update("WWPT:OP_1" ,   20.0);
        state.update("WGPT:OP_1" ,   30.0);
        state.update("WVPT:OP_1" ,   40.0);
        state.update("WWIR:OP_1" ,    0.0);
        state.update("WGIR:OP_1" ,    0.0);
        state.update("WWIT:OP_1" ,    0.0);
        state.update("WGIT:OP_1" ,    0.0);
        state.update("WVIT:OP_1" ,    0.0);
        state.update("WWCT:OP_1" ,    0.625);
        state.update("WGOR:OP_1" ,  234.5);
        state.update("WBHP:OP_1" ,  314.15);
        state.update("WTHP:OP_1" ,  123.45);
        state.update("WOPTH:OP_1",  345.6);
        state.update("WWPTH:OP_1",  456.7);
        state.update("WGPTH:OP_1",  567.8);
        state.update("WWITH:OP_1",    0.0);
        state.update("WGITH:OP_1",    0.0);
        state.update("WGVIR:OP_1",    0.0);
        state.update("WWVIR:OP_1",    0.0);
        state.update("WOPGR:OP_1",    4.9);
        state.update("WWPGR:OP_1",    3.8);
        state.update("WGPGR:OP_1",    2.7);
        state.update("WVPGR:OP_1",    6.1);

        state.update("WOPR:OP_2" ,    0.0);
        state.update("WWPR:OP_2" ,    0.0);
        state.update("WGPR:OP_2" ,    0.0);
        state.update("WVPR:OP_2" ,    0.0);
        state.update("WOPT:OP_2" ,    0.0);
        state.update("WWPT:OP_2" ,    0.0);
        state.update("WGPT:OP_2" ,    0.0);
        state.update("WVPT:OP_2" ,    0.0);
        state.update("WWIR:OP_2" ,  100.0);
        state.update("WGIR:OP_2" ,  200.0);
        state.update("WWIT:OP_2" , 1000.0);
        state.update("WGIT:OP_2" , 2000.0);
        state.update("WVIT:OP_2" , 1234.5);
        state.update("WWCT:OP_2" ,    0.0);
        state.update("WGOR:OP_2" ,    0.0);
        state.update("WBHP:OP_2" ,  400.6);
        state.update("WTHP:OP_2" ,  234.5);
        state.update("WOPTH:OP_2",    0.0);
        state.update("WWPTH:OP_2",    0.0);
        state.update("WGPTH:OP_2",    0.0);
        state.update("WWITH:OP_2", 1515.0);
        state.update("WGITH:OP_2", 3030.0);
        state.update("WGVIR:OP_2", 1234.0);
        state.update("WWVIR:OP_2", 4321.0);
        state.update("WOIGR:OP_2",    4.9);
        state.update("WWIGR:OP_2",    3.8);
        state.update("WGIGR:OP_2",    2.7);
        state.update("WVIGR:OP_2",    6.1);

        state.update("WOPR:OP_3" ,   11.0);
        state.update("WWPR:OP_3" ,   12.0);
        state.update("WGPR:OP_3" ,   13.0);
        state.update("WVPR:OP_3" ,   14.0);
        state.update("WOPT:OP_3" ,  110.0);
        state.update("WWPT:OP_3" ,  120.0);
        state.update("WGPT:OP_3" ,  130.0);
        state.update("WVPT:OP_3" ,  140.0);
        state.update("WWIR:OP_3" ,    0.0);
        state.update("WGIR:OP_3" ,    0.0);
        state.update("WWIT:OP_3" ,    0.0);
        state.update("WGIT:OP_3" ,    0.0);
        state.update("WVIT:OP_3" ,    0.0);
        state.update("WWCT:OP_3" ,    0.0625);
        state.update("WGOR:OP_3" , 1234.5);
        state.update("WBHP:OP_3" ,  314.15);
        state.update("WTHP:OP_3" ,  246.9);
        state.update("WOPTH:OP_3", 2345.6);
        state.update("WWPTH:OP_3", 3456.7);
        state.update("WGPTH:OP_3", 4567.8);
        state.update("WWITH:OP_3",    0.0);
        state.update("WGITH:OP_3",    0.0);
        state.update("WGVIR:OP_3",    0.0);
        state.update("WWVIR:OP_3",   43.21);
        state.update("WOPGR:OP_3",    49.0);
        state.update("WWPGR:OP_3",    38.9);
        state.update("WGPGR:OP_3",    27.8);
        state.update("WVPGR:OP_3",    61.2);

        return state;
    }

    Opm::data::WellRates well_rates_1()
    {
        using o = ::Opm::data::Rates::opt;

        auto xw = ::Opm::data::WellRates{};

        {
            xw["OP_1"].rates
                .set(o::wat, 1.0)
                .set(o::oil, 2.0)
                .set(o::gas, 3.0);

            xw["OP_1"].connections.emplace_back();
            auto& c = xw["OP_1"].connections.back();

            c.rates.set(o::wat, 1.0)
                   .set(o::oil, 2.0)
                   .set(o::gas, 3.0);
            auto& curr = xw["OP_1"].current_control;
            curr.isProducer = true;
            curr.prod = ::Opm::Well::ProducerCMode::GRAT;
        }

        {
            xw["OP_2"].bhp = 234.0;

            xw["OP_2"].rates.set(o::gas, 5.0);
            //xw["OP_2"].connections.emplace_back();

            //auto& c = xw["OP_2"].connections.back();
            //c.rates.set(o::gas, 4.0);
            auto& curr = xw["OP_2"].current_control;
            curr.isProducer = false;
            curr.inj = ::Opm::Well::InjectorCMode::RATE;
        }

        return xw;
    }

    Opm::data::WellRates well_rates_2()
    {
        using o = ::Opm::data::Rates::opt;

        auto xw = ::Opm::data::WellRates{};

        {
            xw["OP_1"].bhp = 150.0;  // Closed

            xw["OP_1"].connections.emplace_back();
            auto& c = xw["OP_1"].connections.back();

            c.rates.set(o::wat, 1.0)
                   .set(o::oil, 2.0)
                   .set(o::gas, 3.0);

            auto& curr = xw["OP_1"].current_control;
            curr.isProducer = true;
            curr.prod = ::Opm::Well::ProducerCMode::NONE;
        }

        {
            xw["OP_2"].bhp = 234.0;

            xw["OP_2"].rates.set(o::wat, 5.0);
            xw["OP_2"].connections.emplace_back();

            auto& c = xw["OP_2"].connections.back();
            c.rates.set(o::wat, 5.0);
        }

        return xw;
    }


} // namespace

struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   { deck }
        , grid { deck }
        , python{ std::make_shared<Opm::Python>()}
        , sched{ deck, es, python }
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;
};

// =====================================================================

BOOST_AUTO_TEST_SUITE(Aggregate_WD)

BOOST_AUTO_TEST_CASE (Constructor)
{
    const auto ih = MockIH{ 5 };

    const auto awd = Opm::RestartIO::Helpers::AggregateWellData{ ih.value };

    BOOST_CHECK_EQUAL(awd.getIWell().size(), ih.nwells * ih.niwelz);
    BOOST_CHECK_EQUAL(awd.getSWell().size(), ih.nwells * ih.nswelz);
    BOOST_CHECK_EQUAL(awd.getXWell().size(), ih.nwells * ih.nxwelz);
    BOOST_CHECK_EQUAL(awd.getZWell().size(), ih.nwells * ih.nzwelz);
}

BOOST_AUTO_TEST_CASE (Declared_Well_Data)
{
    const auto simCase = SimulationCase{first_sim()};
    Opm::Action::State action_state;
    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t{1};

    const auto ih = MockIH {
        static_cast<int>(simCase.sched.getWells(rptStep).size())
    };

    BOOST_CHECK_EQUAL(ih.nwells, MockIH::Sz{2});

    const auto smry = sim_state();
    auto awd = Opm::RestartIO::Helpers::AggregateWellData{ih.value};
    awd.captureDeclaredWellData(simCase.sched,
                                simCase.es.getUnits(), rptStep, action_state, smry, ih.value);

    // IWEL (OP_1)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IWell::index;

        const auto start = 0*ih.niwelz;

        const auto& iwell = awd.getIWell();
        BOOST_CHECK_EQUAL(iwell[start + Ix::IHead] , 9); // OP_1 -> I
        BOOST_CHECK_EQUAL(iwell[start + Ix::JHead] , 9); // OP_1 -> J
        BOOST_CHECK_EQUAL(iwell[start + Ix::FirstK], 1); // OP_1/Head -> K
        BOOST_CHECK_EQUAL(iwell[start + Ix::NConn] , 2); // OP_1 #Compl
        BOOST_CHECK_EQUAL(iwell[start + Ix::WType] , 1); // OP_1 -> Producer
        BOOST_CHECK_EQUAL(iwell[start + Ix::VFPTab], 0); // VFP defaulted -> 0

        // Completion order
        BOOST_CHECK_EQUAL(iwell[start + Ix::CompOrd], 0); // Track ordering (default)

        BOOST_CHECK_EQUAL(iwell[start + Ix::item18], -100); // M2 Magic
        BOOST_CHECK_EQUAL(iwell[start + Ix::item25], -  1); // M2 Magic
        BOOST_CHECK_EQUAL(iwell[start + Ix::item48], -  1); // M2 Magic
        BOOST_CHECK_EQUAL(iwell[start + Ix::item32],    7); // M2 Magic
    }

    // IWEL (OP_2)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IWell::index;

        const auto start = 1*ih.niwelz;

        const auto& iwell = awd.getIWell();
        BOOST_CHECK_EQUAL(iwell[start + Ix::IHead] , 9); // OP_2 -> I
        BOOST_CHECK_EQUAL(iwell[start + Ix::JHead] , 9); // OP_2 -> J
        BOOST_CHECK_EQUAL(iwell[start + Ix::FirstK], 2); // OP_2/Head -> K
        BOOST_CHECK_EQUAL(iwell[start + Ix::NConn] , 1); // OP_2 #Compl
        BOOST_CHECK_EQUAL(iwell[start + Ix::WType] , 4); // OP_2 -> Gas Inj.
        BOOST_CHECK_EQUAL(iwell[start + Ix::VFPTab], 0); // VFP defaulted -> 0

        // Completion order
        BOOST_CHECK_EQUAL(iwell[start + Ix::CompOrd], 0); // Track ordering (default)

        BOOST_CHECK_EQUAL(iwell[start + Ix::item18], -100); // M2 Magic
        BOOST_CHECK_EQUAL(iwell[start + Ix::item25], -  1); // M2 Magic
        BOOST_CHECK_EQUAL(iwell[start + Ix::item48], -  1); // M2 Magic
        BOOST_CHECK_EQUAL(iwell[start + Ix::item32],    7); // M2 Magic
    }

    // SWEL (OP_1)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::SWell::index;

        const auto i0 = 0*ih.nswelz;

        const auto& swell = awd.getSWell();
        BOOST_CHECK_CLOSE(swell[i0 + Ix::OilRateTarget], 20.0e3f, 1.0e-7f);

        // No WRAT limit
        BOOST_CHECK_CLOSE(swell[i0 + Ix::WatRateTarget], 1.0e20f, 1.0e-7f);

        // No GRAT limit
        BOOST_CHECK_CLOSE(swell[i0 + Ix::GasRateTarget], 1.0e20f, 1.0e-7f);

        // LRAT limit derived from ORAT + WRAT (= ORAT + 0.0)
        BOOST_CHECK_CLOSE(swell[i0 + Ix::LiqRateTarget], 1.0e20f, 1.0e-7f);

        // No direct limit, extract value from 'smry' (WVPR:OP_1)
        BOOST_CHECK_CLOSE(swell[i0 + Ix::ResVRateTarget], 1.0e20f, 1.0e-7f);

        // No THP limit
        BOOST_CHECK_CLOSE(swell[i0 + Ix::THPTarget] ,    0.0f, 1.0e-7f);
        BOOST_CHECK_CLOSE(swell[i0 + Ix::BHPTarget] , 1000.0f, 1.0e-7f);
        BOOST_CHECK_CLOSE(swell[i0 + Ix::DatumDepth], 7050.00049f, 1.0e-7f);
    }

    // SWEL (OP_2)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::SWell::index;

        const auto i1 = 1*ih.nswelz;

        const auto& swell = awd.getSWell();
        BOOST_CHECK_CLOSE(swell[i1 + Ix::THPTarget], 1.0e20f, 1.0e-7f);
        BOOST_CHECK_CLOSE(swell[i1 + Ix::BHPTarget],  400.0f, 1.0e-7f);

        BOOST_CHECK_CLOSE(swell[i1 + Ix::DatumDepth], 7150.0f, 1.0e-7f);
    }

    // XWEL (OP_1)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto i0 = 0*ih.nxwelz;

        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::BHPTarget], 1000.0, 1.0e-10);
    }

    // XWEL (OP_2)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto i1 = 1*ih.nxwelz;

        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::BHPTarget], 400.0, 1.0e-10);
    }

    // ZWEL (OP_1)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::ZWell::index;

        const auto i0 = 0*ih.nzwelz;

        const auto& zwell = awd.getZWell();

        BOOST_CHECK_EQUAL(zwell[i0 + Ix::WellName].c_str(), "OP_1    ");
    }

    // ZWEL (OP_2)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::ZWell::index;

        const auto i1 = 1*ih.nzwelz;

        const auto& zwell = awd.getZWell();

        BOOST_CHECK_EQUAL(zwell[i1 + Ix::WellName].c_str(), "OP_2    ");
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (Dynamic_Well_Data_Step1)
{
    const auto simCase = SimulationCase{first_sim()};

    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t{1};

    const auto ih = MockIH {
        static_cast<int>(simCase.sched.getWells(rptStep).size())
    };

    const auto xw   = well_rates_1();
    const auto smry = sim_state();
    auto awd = Opm::RestartIO::Helpers::AggregateWellData{ih.value};

    awd.captureDynamicWellData(simCase.sched, rptStep, xw, smry);

    // IWEL (OP_1)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IWell::index;

        const auto i0 = 0*ih.niwelz;

        const auto& iwell = awd.getIWell();

        BOOST_CHECK_EQUAL(iwell[i0 + Ix::item9 ], iwell[i0 + Ix::ActWCtrl]);
        BOOST_CHECK_EQUAL(iwell[i0 + Ix::item11], 1);
    }

    // IWEL (OP_2)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IWell::index;

        const auto i1 = 1*ih.niwelz;

        const auto& iwell = awd.getIWell();

        //
        // These checks do not work because flow gives a well's status SHUT
        // when all the connections are shut (no flowing connections)
        // This needs to be corrected in flow

        BOOST_CHECK_EQUAL(iwell[i1 + Ix::item9 ], -1); // No flowing conns.
        BOOST_CHECK_EQUAL(iwell[i1 + Ix::item11], -1); // No flowing conns.
    }

    // XWEL (OP_1)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto  i0 = 0*ih.nxwelz;
        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::OilPrRate], 1.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatPrRate], 2.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GasPrRate], 3.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::LiqPrRate], 1.0 + 2.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::VoidPrRate], 4.0, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::TubHeadPr], 123.45, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::FlowBHP], 314.15 , 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatCut] ,   0.625, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GORatio], 234.5  , 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::OilPrTotal], 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatPrTotal], 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GasPrTotal], 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::VoidPrTotal], 40.0, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::item37],
                          xwell[i0 + Ix::WatPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::item38],
                          xwell[i0 + Ix::GasPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistOilPrTotal], 345.6, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistWatPrTotal], 456.7, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistGasPrTotal], 567.8, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistWatInjTotal], 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistGasInjTotal], 0.0, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::PrimGuideRate], 4.9, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::PrimGuideRate], xwell[i0 + Ix::PrimGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatPrGuideRate], 3.8, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::WatPrGuideRate], xwell[i0 + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GasPrGuideRate], 2.7, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::GasPrGuideRate], xwell[i0 + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::VoidPrGuideRate], 6.1, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::VoidPrGuideRate], xwell[i0 + Ix::VoidPrGuideRate_2]);
    }

    // XWEL (OP_2)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto  i1 = 1*ih.nxwelz;
        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::GasPrRate], -200.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::VoidPrRate], -1234.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::TubHeadPr], 234.5, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::FlowBHP], 400.6, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::WatInjTotal], 1000.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::GasInjTotal], 2000.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::VoidInjTotal], 1234.5, 1.0e-10);

        // Bg = VGIR / GIR = 1234.0 / 200.0
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::GasFVF], 6.17, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::item38],
                          xwell[i1 + Ix::GasPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistOilPrTotal] ,    0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistWatPrTotal] ,    0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistGasPrTotal] ,    0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistWatInjTotal], 1515.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistGasInjTotal], 3030.0, 1.0e-10);

        // Gas injector => primary guide rate == gas injection guide rate (with negative sign).
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::PrimGuideRate], -2.7, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::PrimGuideRate], xwell[i1 + Ix::PrimGuideRate_2]);

        // Injector => all phase production guide rates are zero
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::WatPrGuideRate], 0.0, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::WatPrGuideRate], xwell[i1 + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::GasPrGuideRate], 0.0, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::GasPrGuideRate], xwell[i1 + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::VoidPrGuideRate], 0.0, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::VoidPrGuideRate], xwell[i1 + Ix::VoidPrGuideRate_2]);
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (Dynamic_Well_Data_Step2)
{
    const auto simCase = SimulationCase{first_sim()};

    // Report Step 2: 2011-01-20 --> 2013-06-15
    const auto rptStep = std::size_t{2};

    const auto ih = MockIH {
        static_cast<int>(simCase.sched.getWells(rptStep).size())
    };

    const auto xw   = well_rates_2();
    const auto smry = sim_state();
    auto awd = Opm::RestartIO::Helpers::AggregateWellData{ih.value};

    awd.captureDynamicWellData(simCase.sched, rptStep, xw, smry);

    // IWEL (OP_1) -- closed producer
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IWell::index;

        const auto i0 = 0*ih.niwelz;

        const auto& iwell = awd.getIWell();



        BOOST_CHECK_EQUAL(iwell[i0 + Ix::item9] , 0);
        BOOST_CHECK_EQUAL(iwell[i0 + Ix::item11], 0);
    }

    // IWEL (OP_2) -- water injector
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IWell::index;

        const auto i1 = 1*ih.niwelz;

        const auto& iwell = awd.getIWell();

        BOOST_CHECK_EQUAL(iwell[i1 + Ix::item9],
                          iwell[i1 + Ix::ActWCtrl]);
        BOOST_CHECK_EQUAL(iwell[i1 + Ix::item11], 1);
    }

    // XWEL (OP_1) -- closed producer
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto  i0 = 0*ih.nxwelz;
        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::OilPrRate], 1.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatPrRate], 2.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GasPrRate], 3.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::LiqPrRate], 1.0 + 2.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::VoidPrRate], 4.0, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::TubHeadPr], 123.45, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::FlowBHP], 314.15, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatCut] , 0.625, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GORatio], 234.5, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::OilPrTotal], 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatPrTotal], 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GasPrTotal], 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::VoidPrTotal], 40.0, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::item37],
                          xwell[i0 + Ix::WatPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::item38],
                          xwell[i0 + Ix::GasPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistOilPrTotal], 345.6, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistWatPrTotal], 456.7, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i0 + Ix::HistGasPrTotal], 567.8, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::PrimGuideRate], 4.9, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::PrimGuideRate], xwell[i0 + Ix::PrimGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::WatPrGuideRate], 3.8, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::WatPrGuideRate], xwell[i0 + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::GasPrGuideRate], 2.7, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::GasPrGuideRate], xwell[i0 + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i0 + Ix::VoidPrGuideRate], 6.1, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i0 + Ix::VoidPrGuideRate], xwell[i0 + Ix::VoidPrGuideRate_2]);
    }

    // XWEL (OP_2) -- water injector
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto  i1 = 1*ih.nxwelz;
        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::WatPrRate], -100.0, 1.0e-10);

        // Copy of WWIR
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::LiqPrRate],
                          xwell[i1 + Ix::WatPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::TubHeadPr], 234.5, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::FlowBHP], 400.6, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::WatInjTotal], 1000.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::GasInjTotal], 2000.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::VoidInjTotal], 1234.5, 1.0e-10);

        // Copy of WWIR
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::item37],
                          xwell[i1 + Ix::WatPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistOilPrTotal] ,    0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistWatPrTotal] ,    0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistGasPrTotal] ,    0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistWatInjTotal], 1515.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::HistGasInjTotal], 3030.0, 1.0e-10);

        // WWVIR
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::WatVoidPrRate],
                          -4321.0, 1.0e-10);

        // Water injector => primary guide rate == water injection guide rate (with negative sign).
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::PrimGuideRate], -3.8, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::PrimGuideRate], xwell[i1 + Ix::PrimGuideRate_2]);

        // Injector => all phase production guide rates are zero
        BOOST_CHECK_CLOSE(xwell[i1 + Ix::WatPrGuideRate], 0.0, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::WatPrGuideRate], xwell[i1 + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::GasPrGuideRate], 0.0, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::GasPrGuideRate], xwell[i1 + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i1 + Ix::VoidPrGuideRate], 0.0, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i1 + Ix::VoidPrGuideRate], xwell[i1 + Ix::VoidPrGuideRate_2]);
    }

    // XWEL (OP_3) -- producer
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

        const auto  i2 = 2*ih.nxwelz;
        const auto& xwell = awd.getXWell();

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::OilPrRate], 11.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::WatPrRate], 12.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::GasPrRate], 13.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::LiqPrRate], 11.0 + 12.0, 1.0e-10); // LPR
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::VoidPrRate], 14.0, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::TubHeadPr], 246.9, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::FlowBHP], 314.15, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::WatCut] , 0.0625, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::GORatio], 1234.5, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::OilPrTotal], 110.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::WatPrTotal], 120.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::GasPrTotal], 130.0, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::VoidPrTotal], 140.0, 1.0e-10);

        // Copy of WWPR
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::item37],
                          xwell[i2 + Ix::WatPrRate], 1.0e-10);

        // Copy of WGPR
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::item38],
                          xwell[i2 + Ix::GasPrRate], 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::HistOilPrTotal], 2345.6, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::HistWatPrTotal], 3456.7, 1.0e-10);
        BOOST_CHECK_CLOSE(xwell[i2 + Ix::HistGasPrTotal], 4567.8, 1.0e-10);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::PrimGuideRate], 49, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i2 + Ix::PrimGuideRate], xwell[i2 + Ix::PrimGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::WatPrGuideRate], 38.9, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i2 + Ix::WatPrGuideRate], xwell[i2 + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::GasPrGuideRate], 27.8, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i2 + Ix::GasPrGuideRate], xwell[i2 + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xwell[i2 + Ix::VoidPrGuideRate], 61.2, 1.0e-10);
        BOOST_CHECK_EQUAL(xwell[i2 + Ix::VoidPrGuideRate], xwell[i2 + Ix::VoidPrGuideRate_2]);
    }
}


BOOST_AUTO_TEST_CASE(WELL_POD) {
    const auto simCase = SimulationCase{first_sim()};
    const auto& units = simCase.es.getUnits();
    // Report Step 2: 2011-01-20 --> 2013-06-15
    const auto rptStep = std::size_t{2};
    const auto sim_step = rptStep - 1;
    Opm::SummaryState sumState(std::chrono::system_clock::now());
    const auto xw   = well_rates_1();
    Opm::Action::State action_state;

    const auto ih = Opm::RestartIO::Helpers::createInteHead(simCase.es,
                                                            simCase.grid,
                                                            simCase.sched,
                                                            0,
                                                            sim_step,
                                                            sim_step,
                                                            sim_step);

    auto wellData = Opm::RestartIO::Helpers::AggregateWellData(ih);
    wellData.captureDeclaredWellData(simCase.sched, units, sim_step, action_state, sumState, ih);
    wellData.captureDynamicWellData(simCase.sched, sim_step, xw , sumState);

    auto connectionData = Opm::RestartIO::Helpers::AggregateConnectionData(ih);
    connectionData.captureDeclaredConnData(simCase.sched, simCase.grid, units, xw , sim_step);

    const auto& iwel = wellData.getIWell();
    const auto& swel = wellData.getSWell();
    const auto& xwel = wellData.getXWell();
    const auto& zwel8 = wellData.getZWell();

    const auto& icon = connectionData.getIConn();
    const auto& scon = connectionData.getSConn();
    const auto& xcon = connectionData.getXConn();

    Opm::RestartIO::RstHeader header(ih, std::vector<bool>(100), std::vector<double>(1000));
    std::vector<Opm::RestartIO::RstWell> wells;
    std::vector<std::string> zwel;
    for (const auto& s8: zwel8)
        zwel.push_back(s8.c_str());

    for (auto iw = 0; iw < header.num_wells; iw++) {
        std::size_t zwel_offset = header.nzwelz * iw;
        std::size_t iwel_offset = header.niwelz * iw;
        std::size_t swel_offset = header.nswelz * iw;
        std::size_t xwel_offset = header.nxwelz * iw;
        std::size_t icon_offset = header.niconz * header.ncwmax * iw;
        std::size_t scon_offset = header.nsconz * header.ncwmax * iw;
        std::size_t xcon_offset = header.nxconz * header.ncwmax * iw;

        wells.emplace_back(units,
                           header,
                           "GROUP",
                           zwel.data() + zwel_offset,
                           iwel.data() + iwel_offset,
                           swel.data() + swel_offset,
                           xwel.data() + xwel_offset,
                           icon.data() + icon_offset,
                           scon.data() + scon_offset,
                           xcon.data() + xcon_offset);

    }

    // Well OP2
    const auto& well2 = wells[1];
    BOOST_CHECK_EQUAL(well2.k1k2.first, 1);
    BOOST_CHECK_EQUAL(well2.k1k2.second, 1);
    BOOST_CHECK_EQUAL(well2.ij[0], 8);
    BOOST_CHECK_EQUAL(well2.ij[1], 8);
    BOOST_CHECK_EQUAL(well2.name, "OP_2");

    BOOST_CHECK_EQUAL(well2.connections.size(), 1);
    const auto& conn1 = well2.connections[0];
    BOOST_CHECK_EQUAL(conn1.ijk[0], 8);
    BOOST_CHECK_EQUAL(conn1.ijk[1], 8);
    BOOST_CHECK_EQUAL(conn1.ijk[2], 1);
}

BOOST_AUTO_TEST_SUITE_END()
