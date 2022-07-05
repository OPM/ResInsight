/*
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

#define BOOST_TEST_MODULE Aggregate_Connection_Data

#include <opm/output/eclipse/AggregateConnectionData.hpp>

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateMSWData.hpp>
#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/output/eclipse/VectorItems/connection.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>

#include <opm/io/eclipse/rst/connection.hpp>
#include <opm/io/eclipse/rst/header.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/common/utility/TimeService.hpp>

#include <cstddef>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

struct MockIH
{
    MockIH(const int numWells,

	   const int nsegWell 	  =  1,  // E100
	   const int ncwMax       = 20,
	   const int iConnPerConn =  25,  // NICONZ
	   const int sConnPerConn =  41,  // NSCONZ
	   const int xConnPerConn =  58);  // NXCONZ


    std::vector<int> value;

    using Sz = std::vector<int>::size_type;

    Sz nwells;
    Sz nsegwl;
    Sz nsegmx;
    Sz nswlmx;
    Sz ncwmax;
    Sz niconz;
    Sz nsconz;
    Sz nxconz;
};

MockIH::MockIH(const int numWells,
	       const int nsegWell,
	       const int /* ncwMax */,
	       const int iConnPerConn,
	       const int sConnPerConn,
	       const int xConnPerConn)
    : value(411, 0)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::intehead;

    this->nwells = this->value[Ix::NWELLS] = numWells;

    this->nsegwl = this->value[Ix::NSEGWL] = nsegWell;
    this->ncwmax = this->value[Ix::NCWMAX] = 20;
    this->nswlmx = this->value[Ix::NSWLMX] = 1;
    this->nsegmx = this->value[Ix::NSEGMX] = 32;
    this->niconz = this->value[Ix::NICONZ] = iConnPerConn;
    this->nsconz = this->value[Ix::NSCONZ] = sConnPerConn;
    this->nxconz = this->value[Ix::NXCONZ] = xConnPerConn;
}

namespace {
    Opm::Deck first_sim()
    {
        // Mostly copy of tests/FIRST_SIM.DATA
        const auto input = std::string {
            R"~(
RUNSPEC

TITLE
 TWO MULTI-LATERAL WELLS; WINJUCER AND INJECTOR - MULTI-SEGMENT BRANCHES

DIMENS
 10  5  10  /


OIL

WATER

GAS

DISGAS

FIELD

TABDIMS
 1  1  15  15  2  15  /

EQLDIMS
 2  /

WELLDIMS
 3  20  1  3  /

WSEGDIMS
 1  32  5  /

UNIFIN
UNIFOUT

--FMTIN
--FMTOUT

START
 1 'JAN' 2015 /

-- RPTRUNSP

GRID        =========================================================

--NOGGF
BOX
 1 10 1 5 1 1 /

TOPS
50*7000 /

BOX
1 10  1 5 1 10 /

DXV
10*100 /
DYV
5*100  /
DZV
2*20 100 7*20 /

EQUALS
-- 'DX'     100  /
-- 'DY'     100  /
 'PERMX'  50   /
 'PERMZ'  5   /
-- 'DZ'     20   /
 'PORO'   0.2  /
-- 'TOPS'   7000   1 10  1 5  1 1  /
-- 'DZ'     100    1 10  1 5  3 3  /
-- 'PORO'   0.0    1 10  1 5  3 3  /
 /

COPY
  PERMX PERMY /
 /


PORO
   500*0.15 /

RPTGRID
  -- Report Levels for Grid Section Data
  --
 /

EDIT

EQUALS
 'PORV'   0.0    7 7  1 1  9 9  /
/

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


RPTPROPS
-- PROPS Reporting Options
--
/

REGIONS    ===========================================================


FIPNUM

  100*1
  400*2
/

EQLNUM

  100*1
  400*2
/

RPTREGS

    /

SOLUTION    ============================================================

EQUIL
 7020.00 2700.00 7990.00  .00000 7020.00  .00000     0      0       5 /
 7200.00 3700.00 7300.00  .00000 7000.00  .00000     1      0       5 /

RSVD       2 TABLES    3 NODES IN EACH           FIELD   12:00 17 AUG 83
   7000.0  1.0000
   7990.0  1.0000
/
   7000.0  1.0000
   7400.0  1.0000
/

RPTRST
-- Restart File Output Control
--
'BASIC=2' 'FLOWS' 'POT' 'PRES' /

--RPTSOL
--
-- Initialisation Print Output
--
--'PRES' 'SOIL' 'SWAT' 'SGAS' 'RS' 'RESTART=1' 'FIP=2' 'EQUIL' 'RSVD' /

SUMMARY      ===========================================================

FOPR

WOPR
 'WINJ'
 /

FGPR

FWPR

FWIR

FWCT

FGOR

--RUNSUM

ALL

MSUMLINS

MSUMNEWT

SEPARATE

SCHEDULE     ===========================================================

DEBUG
   1 3   /

DRSDT
   1.0E20  /

RPTSCHED
  'PRES'  'SWAT'  'SGAS'  'RESTART=1'  'RS'  'WELLS=2'  'SUMMARY=2'
  'CPU=2' 'WELSPECS'   'NEWTON=2' /

NOECHO


ECHO

WELSPECS
 'PROD' 'G' 1 5 7030 'OIL' 0.0  'STD'  'STOP'  /
 'WINJ' 'G' 10 1 7030 'WAT' 0.0  'STD'  'STOP'   /
/

COMPDAT

 'PROD' 1 5 2 2   3*  0.2   3*  'X' /
 'PROD' 2 5 2 2   3*  0.2   3*  'X' /
 'PROD' 3 5 2 2   3*  0.2   3*  'X' /   -- This connection is explicitly made inactive in the InactiveCell test
 'PROD' 4 5 2 2   3*  0.2   3*  'X' /
 'PROD' 5 5 2 2   3*  0.2   3*  'X' /
 'WINJ' 10 1  9 9   3*  0.2   3*  'X' /
 'WINJ'  9 1  9 9   3*  0.2   3*  'X' /
 'WINJ'  8 1  9 9   3*  0.2   3*  'X' /
 'WINJ'  7 1  9 9   3*  0.2   3*  'X' /
 'WINJ'  6 1  9 9   3*  0.2   3*  'X' /
/

WELSEGS

-- Name    Dep 1   Tlen 1  Vol 1
  'PROD'   7010      10    0.31   'INC' /

-- First   Last   Branch   Outlet  Length   Depth  Diam  Ruff  Area  Vol
-- Seg     Seg    Num      Seg              Chang
-- Main Stem
    2       12     1        1         20     20    0.2   1.E-3  1*   1* /
-- Top Branch
    13      13     2        3         50      0    0.2   1.E-3  1*   1* /
    14      17     2        13       100      0    0.2   1.E-3  1*   1* /
 /

COMPSEGS

-- Name
  'PROD' /

-- I  J  K  Brn  Start   End     Dirn   End
--          No   Length  Length  Penet  Range
-- Top Branch
  1  5  2  2         30      130     'X'    3* /
  2  5  2  2        130      230     'X'    3* /
  3  5  2  2        230      330     'X'    3* /
  4  5  2  2        330      430     'X'    3* /
  5  5  2  2        430      530     'X'    3* /
-- Middle Branch
 /


WCONPROD
 'PROD' 'OPEN' 'LRAT'  3*  2000  1*  2500  1*  /
 /

WCONINJE
 'WINJ' 'WAT' 'OPEN' 'BHP'  1*  20000  3500  1*  /
 /


TUNING
 /
 /
 /

TSTEP
 2 2
/


END


)~" };

        return Opm::Parser{}.parseString(input);
    }

    std::pair<Opm::data::Wells, Opm::SummaryState> wr(const Opm::Schedule& sched)
    {
        using o = ::Opm::data::Rates::opt;

        auto xw = ::Opm::data::Wells {};
        Opm::SummaryState sum_state(Opm::TimeService::now());

        {
            xw["PROD"].rates.set(o::wat, 1.0).set(o::oil, 2.0).set(o::gas, 3.0);
            xw["PROD"].bhp = 213.0;

            const double qv = 12.34;
            {
                const double qw =  4.0;
                const double qo =  5.0;
                const double qg = 50.0;

                const auto& well = sched.getWell("PROD", 0);
                const auto& connections = well.getConnections();
                for (int i = 0; i < 5; i++) {
                    auto& c = xw["PROD"].connections.emplace_back();

                    c.rates.set(o::wat, qw * (float(i) + 1.0))
                        .set(o::oil, qo * (float(i) + 1.0))
                        .set(o::gas, qg * (float(i) + 1.0));
                    c.pressure = 215.0;
                    c.index = connections[i].global_index();
                    c.trans_factor = connections[i].CF();

                    const auto& global_index = connections[i].global_index();
                    sum_state.update_conn_var("PROD", "CWPR", global_index + 1, qw * (i + 1));
                    sum_state.update_conn_var("PROD", "COPR", global_index + 1, qo * (i + 1));
                    sum_state.update_conn_var("PROD", "CGPR", global_index + 1, qg * (i + 1));
                    sum_state.update_conn_var("PROD", "CVPR", global_index + 1, qv * (i + 1));

                    sum_state.update_conn_var("PROD", "COPT", global_index + 1, qo * (i + 1) * 2.0);
                    sum_state.update_conn_var("PROD", "CWPT", global_index + 1, qw * (i + 1) * 2.0);
                    sum_state.update_conn_var("PROD", "CGPT", global_index + 1, qg * (i + 1) * 2.0);
                    sum_state.update_conn_var("PROD", "CVPT", global_index + 1, qv * (i + 1) * 2.0);

                    sum_state.update_conn_var("PROD", "CGOR", global_index + 1, qg / qo);

                    sum_state.update_conn_var("PROD", "CPR",  global_index + 1, 215.0);
                }

                auto seg = Opm::data::Segment {};
                for (std::size_t i = 1; i < 5; i++) {
                    xw["PROD"].segments.insert(std::pair<std::size_t, Opm::data::Segment>(i, seg));
                }
            }

            {
                const auto& well = sched.getWell("WINJ", 0);
                const auto& connections = well.getConnections();
                xw["WINJ"].bhp = 234.0;

                xw["WINJ"].rates.set(o::wat, 5.0);
                xw["WINJ"].rates.set(o::oil, 0.0);
                xw["WINJ"].rates.set(o::gas, 0.0);

                const double qw = 7.0;
                for (int i = 0; i < 4; i++) {
                    xw["WINJ"].connections.emplace_back();
                    auto& c = xw["WINJ"].connections.back();

                    c.rates.set(o::wat, qw * (float(i) + 1.0)).set(o::oil, 0.0).set(o::gas, 0.0);
                    c.pressure = 218.0;
                    c.index = connections[i].global_index();
                    c.trans_factor = connections[i].CF();

                    const auto& global_index = connections[i].global_index();
                    sum_state.update_conn_var("WINJ", "CWIR", global_index+ 1, qw*(i + 1));
                    sum_state.update_conn_var("WINJ", "COIR", global_index+ 1, 0.0);
                    sum_state.update_conn_var("WINJ", "CGIR", global_index+ 1, 0.0);
                    sum_state.update_conn_var("WINJ", "CVIR", global_index+ 1, qv*(i + 1));

                    sum_state.update_conn_var("WINJ", "COIT", global_index + 1, 543.21 * (i + 1));
                    sum_state.update_conn_var("WINJ", "CWIT", global_index + 1, qw * (i + 1) * 2.0);
                    sum_state.update_conn_var("WINJ", "CGIT", global_index + 1, 9876.54 * (i + 1));
                    sum_state.update_conn_var("WINJ", "CVIT", global_index + 1, qv * (i + 1) * 2.0);

                    sum_state.update_conn_var("WINJ", "CPR", global_index + 1, 218.0);
                }
            }
        }

        return { std::move(xw), std::move(sum_state) };
    }
} // namespace

struct SimulationCase {
    explicit SimulationCase(const Opm::Deck& deck)
        : es   (deck)
        , grid (deck)
        , sched(deck, es, std::make_shared<Opm::Python>())
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid grid;
    Opm::Schedule sched;
};

// =====================================================================

BOOST_AUTO_TEST_SUITE(Aggregate_ConnData)

// test dimensions of Connection data
BOOST_AUTO_TEST_CASE (Constructor)
{
    const auto ih = MockIH{ 5 };

    const auto amconn = Opm::RestartIO::Helpers::AggregateConnectionData{ ih.value };

    BOOST_CHECK_EQUAL(amconn.getIConn().size(), ih.nwells * ih.ncwmax * ih.niconz);
    BOOST_CHECK_EQUAL(amconn.getSConn().size(), ih.nwells * ih.ncwmax * ih.nsconz);
    BOOST_CHECK_EQUAL(amconn.getXConn().size(), ih.nwells * ih.ncwmax * ih.nxconz);
}

BOOST_AUTO_TEST_CASE(Declared_Connection_Data)
{
    const auto simCase = SimulationCase {first_sim()};

    // Report Step 1: 2115-01-01 --> 2015-01-03
    const auto rptStep = std::size_t {1};

    const auto ih = MockIH {static_cast<int>(simCase.sched.getWells(rptStep).size())};

    BOOST_CHECK_EQUAL(ih.nwells, MockIH::Sz {2});

    const auto& [wrc, sum_state] = wr(simCase.sched);
    auto amconn = Opm::RestartIO::Helpers::AggregateConnectionData {ih.value};
    amconn.captureDeclaredConnData(simCase.sched, simCase.grid, simCase.es.getUnits(), wrc, sum_state, rptStep);

    // ICONN (PROD)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::IConn::index;

        auto start = 0 * ih.niconz;

        const auto& iConn = amconn.getIConn();
        BOOST_CHECK_EQUAL(iConn[start + Ix::SeqIndex], 1); // PROD-connection 1, sequence number
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellI], 1); // PROD-connection 1, Cell I
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellJ], 5); // PROD-connection 1, Cell J
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellK], 2); // PROD-connection 1, Cell K
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnStat], 1); // PROD-connection 1, ConnStat
        BOOST_CHECK_EQUAL(iConn[start + Ix::Drainage], 0); // PROD-connection 1, Drainage saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::Imbibition], 0); // PROD-connection 1, Imbibition saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::ComplNum], 1); // PROD-connection 1, Complum number
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnDir], 1); // PROD-connection 1, Connection direction
        BOOST_CHECK_EQUAL(iConn[start + Ix::Segment], 13); // PROD-connection 1, Segment ID for direction

        start = 3 * ih.niconz;
        BOOST_CHECK_EQUAL(iConn[start + Ix::SeqIndex], 4); // PROD-connection 4, sequence number
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellI], 4); // PROD-connection 4, Cell I
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellJ], 5); // PROD-connection 4, Cell J
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellK], 2); // PROD-connection 4, Cell K
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnStat], 1); // PROD-connection 4, ConnStat
        BOOST_CHECK_EQUAL(iConn[start + Ix::Drainage], 0); // PROD-connection 4, Drainage saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::Imbibition], 0); // PROD-connection 4, Imbibition saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::ComplNum], 4); // PROD-connection 4, Complum number
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnDir], 1); // PROD-connection 4, Connection direction
        BOOST_CHECK_EQUAL(iConn[start + Ix::Segment], 16); // PROD-connection 4, Segment ID for direction

        // ICONN (WINJ)
        start = ih.ncwmax * ih.niconz;
        BOOST_CHECK_EQUAL(iConn[start + Ix::SeqIndex], 1); // WINJ-connection 1, sequence number
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellI], 10); // WINJ-connection 1, Cell I
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellJ], 1); // WINJ-connection 1, Cell J
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellK], 9); // WINJ-connection 1, Cell K
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnStat], 1); // WINJ-connection 1, ConnStat
        BOOST_CHECK_EQUAL(iConn[start + Ix::Drainage], 0); // WINJ-connection 1, Drainage saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::Imbibition], 0); // WINJ-connection 1, Imbibition saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::ComplNum], 1); // WINJ-connection 1, Complum number
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnDir], 1); // WINJ-connection 1, Connection direction
        BOOST_CHECK_EQUAL(iConn[start + Ix::Segment], 0); // WINJ-connection 1, Segment ID for direction

        start = ih.ncwmax * ih.niconz + 2 * ih.niconz;
        BOOST_CHECK_EQUAL(iConn[start + Ix::SeqIndex], 3); // WINJ-connection 3, sequence number
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellI], 8); // WINJ-connection 3, Cell I
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellJ], 1); // WINJ-connection 3, Cell J
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellK], 9); // WINJ-connection 3, Cell K
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnStat], 1); // WINJ-connection 3, ConnStat
        BOOST_CHECK_EQUAL(iConn[start + Ix::Drainage], 0); // WINJ-connection 3, Drainage saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::Imbibition], 0); // WINJ-connection 3, Imbibition saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::ComplNum], 3); // WINJ-connection 3, Complum number
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnDir], 1); // WINJ-connection 3, Connection direction
        BOOST_CHECK_EQUAL(iConn[start + Ix::Segment], 0); // WINJ-connection 3, Segment ID for direction

        start = ih.ncwmax * ih.niconz + 3 * ih.niconz;
        BOOST_CHECK_EQUAL(iConn[start + Ix::SeqIndex], 4); // WINJ-connection 4, sequence number
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellI], 6); // WINJ-connection 4, Cell I
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellJ], 1); // WINJ-connection 4, Cell J
        BOOST_CHECK_EQUAL(iConn[start + Ix::CellK], 9); // WINJ-connection 4, Cell K
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnStat], 1); // WINJ-connection 4, ConnStat
        BOOST_CHECK_EQUAL(iConn[start + Ix::Drainage], 0); // WINJ-connection 4, Drainage saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::Imbibition], 0); // WINJ-connection 4, Imbibition saturation table
        BOOST_CHECK_EQUAL(iConn[start + Ix::ComplNum], 4); // WINJ-connection 4, Complum number
        BOOST_CHECK_EQUAL(iConn[start + Ix::ConnDir], 1); // WINJ-connection 4, Connection direction
        BOOST_CHECK_EQUAL(iConn[start + Ix::Segment], 0); // WINJ-connection 4, Segment ID for direction
    }

    // SCONN (PROD)  + (WINJ)
    {
        // well no 1 - PROD
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::SConn::index;
        const auto& sconn = amconn.getSConn();
        int connNo = 1;
        auto i0 = (connNo - 1) * ih.nsconz;
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::ConnTrans], 2.55826545, 1.0e-5); // PROD - conn 1 : Transmissibility factor
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::Depth], 7050., 1.0e-5); // PROD - conn 1 : Centre depth
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::Diameter], 0.20, 1.0e-5); // PROD - conn 1 : diameter
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::EffectiveKH], 1581.13879, 1.0e-5); // PROD - conn 1 : effective kh-product
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::item12], 2.55826545, 1.0e-5); // PROD - conn 1 : Transmissibility factor
        BOOST_CHECK_CLOSE(
            sconn[i0 + Ix::SegDistEnd], 130., 1.0e-5); // PROD - conn 1 : Distance to end of connection in segment
        BOOST_CHECK_CLOSE(
            sconn[i0 + Ix::SegDistStart], 30., 1.0e-5); // PROD - conn 1 : Distance to start of connection in segment

        // Well no 2 - WINJ well
        connNo = 3;
        i0 = ih.ncwmax * ih.nsconz + (connNo - 1) * ih.nsconz;
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::ConnTrans], 2.55826545, 1.0e-5); // WINJ - conn 3 : Transmissibility factor
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::Depth], 7250., 1.0e-5); // WINJ - conn 3 : Centre depth
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::Diameter], 0.20, 1.0e-5); // WINJ - conn 3 : diameter
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::EffectiveKH], 1581.13879, 1.0e-5); // WINJ - conn 3 : effective kh-product
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::item12], 2.55826545, 1.0e-5); // WINJ - conn 3 : Transmissibility factor
        BOOST_CHECK_CLOSE(
            sconn[i0 + Ix::SegDistEnd], 0., 1.0e-5); // WINJ - conn 3 : Distance to end of connection in segment
        BOOST_CHECK_CLOSE(
            sconn[i0 + Ix::SegDistStart], 0., 1.0e-5); // WINJ - conn 3 : Distance to start of connection in segment

        connNo = 4;
        i0 = ih.ncwmax * ih.nsconz + (connNo - 1) * ih.nsconz;
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::ConnTrans], 2.55826545, 1.0e-5); // WINJ - conn 4 : Transmissibility factor
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::Depth], 7250., 1.0e-5); // WINJ - conn 4 : Centre depth
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::Diameter], 0.20, 1.0e-5); // WINJ - conn 4 : diameter
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::EffectiveKH], 1581.13879, 1.0e-5); // WINJ - conn 4 : effective kh-product
        BOOST_CHECK_CLOSE(sconn[i0 + Ix::item12], 2.55826545, 1.0e-5); // WINJ - conn 4 : Transmissibility factor
        BOOST_CHECK_CLOSE(
            sconn[i0 + Ix::SegDistEnd], 0., 1.0e-5); // WINJ - conn 4 : Distance to end of connection in segment
        BOOST_CHECK_CLOSE(
            sconn[i0 + Ix::SegDistStart], 0., 1.0e-5); // WINJ - conn 4 : Distance to start of connection in segment
    }

    // XCONN (PROD)  + (WINJ)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XConn::index;
        const auto& xconn = amconn.getXConn();

        // PROD well
        int connNo = 1;
        auto i0 = (connNo - 1) * ih.nxconz;
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilRate], 5.0 * (float(connNo)),
                          1.0e-5); // PROD - conn 1 : Surface oil rate
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WaterRate], 4.0 * (float(connNo)),
                          1.0e-5); // PROD - conn 1 : Surface water rate
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasRate], 50.0 * (float(connNo)),
                          1.0e-5); // PROD - conn 1 : Surface gas rate
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::ResVRate],
                          12.34 * static_cast<float>(connNo),
                          1.0e-5); // PROD - conn 1 : Reservoir volume rate

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilPrTotal],
                          5.0 * static_cast<float>(connNo) * 2.0,
                          1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WatPrTotal],
                          4.0 * static_cast<float>(connNo) * 2.0,
                          1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasPrTotal],
                          50.0 * static_cast<float>(connNo) * 2.0,
                          1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::VoidPrTotal],
                          12.34 * static_cast<float>(connNo) * 2.0,
                          1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GORatio],
                          50.0 / 5.0,
                          1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilInjTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WatInjTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasInjTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::VoidInjTotal], 0.0, 1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::Pressure], 215.0, 1.0e-5); // PROD - conn 1 : Connection pressure

        // WINJ well
        connNo = 3;
        i0 = ih.ncwmax * ih.nxconz + (connNo - 1) * ih.nxconz;
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WaterRate],
                          -7.0 * (float(connNo)),
                          1.0e-5); // WINJ - conn 3 : Surface water rate
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::Pressure], 218., 1.0e-5); // WINJ - conn 3 : Connection pressure
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::ResVRate],
                          -12.34 * static_cast<float>(connNo),
                          1.0e-5); // WINJ - conn 3 : Reservoir volume rate

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilPrTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WatPrTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasPrTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::VoidPrTotal], 0.0, 1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GORatio], 0.0, 1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilInjTotal] , 543.21 * connNo, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WatInjTotal] , 7.0 * connNo * 2.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasInjTotal] , 9876.54 * connNo, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::VoidInjTotal], 12.34 * connNo * 2.0, 1.0e-5);

        connNo = 4;
        i0 = ih.ncwmax * ih.nxconz + (connNo - 1) * ih.nxconz;
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WaterRate],
                          -7.0 * (float(connNo)),
                          1.0e-5); // WINJ - conn 3 : Surface water rate
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::Pressure], 218., 1.0e-5); // WINJ - conn 3 : Connection pressure
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::ResVRate],
                          -12.34 * static_cast<float>(connNo),
                          1.0e-5); // WINJ - conn 3 : Reservoir volume rate

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilPrTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WatPrTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasPrTotal] , 0.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::VoidPrTotal], 0.0, 1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GORatio], 0.0, 1.0e-5);

        BOOST_CHECK_CLOSE(xconn[i0 + Ix::OilInjTotal] , 543.21 * connNo, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::WatInjTotal] , 7.0 * connNo * 2.0, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::GasInjTotal] , 9876.54 * connNo, 1.0e-5);
        BOOST_CHECK_CLOSE(xconn[i0 + Ix::VoidInjTotal], 12.34 * connNo * 2.0, 1.0e-5);
    }
}

BOOST_AUTO_TEST_CASE(InactiveCell) {
    auto simCase = SimulationCase{first_sim()};
    const auto rptStep = std::size_t{1};
    const auto ih = MockIH {static_cast<int>(simCase.sched.getWells(rptStep).size())};
    const auto& [wrc, sum_state] = wr(simCase.sched);
    auto conn0 = Opm::RestartIO::Helpers::AggregateConnectionData{ih.value};
    conn0.captureDeclaredConnData(simCase.sched,
                                  simCase.grid,
                                  simCase.es.getUnits(),
                                  wrc,
                                  sum_state,
                                  rptStep
                                  );

    std::vector<int> actnum(500, 1);
    // Here we deactive the cell holding connection number 2.
    actnum[simCase.grid.getGlobalIndex(2,4,1)] = 0;
    simCase.grid.resetACTNUM(actnum);

    auto conn1 = Opm::RestartIO::Helpers::AggregateConnectionData{ih.value};
    conn1.captureDeclaredConnData(simCase.sched,
                                  simCase.grid,
                                  simCase.es.getUnits(),
                                  wrc,
                                  sum_state,
                                  rptStep
                                  );

    const std::size_t num_test_connections = 4;

    {
        using IC = ::Opm::RestartIO::Helpers::VectorItems::IConn::index;
        const auto iconn0 = conn0.getIConn();
        const auto iconn1 = conn1.getIConn();
        for (std::size_t conn_index = 0; conn_index < num_test_connections; conn_index++) {
            std::size_t offset1 = conn_index * ih.niconz;
            std::size_t offset0 = offset1;

            if (conn_index >= 2)
                offset0 += ih.niconz;

            for (std::size_t elm_index = 0; elm_index < ih.niconz; elm_index++) {
                if (elm_index == IC::SeqIndex && conn_index >= 2) {
                    // Comparing the connection ID - which should be different;
                    BOOST_CHECK_EQUAL(iconn1[offset1 + elm_index] + 1 , iconn0[offset0 + elm_index]);
                } else
                    BOOST_CHECK_EQUAL(iconn1[offset1 + elm_index], iconn0[offset0 + elm_index]);
            }
        }
    }


    {
        const auto sconn0 = conn0.getSConn();
        const auto sconn1 = conn1.getSConn();
        for (std::size_t conn_index = 0; conn_index < num_test_connections; conn_index++) {
            std::size_t offset1 = conn_index * ih.nsconz;
            std::size_t offset0 = offset1;

            if (conn_index >= 2)
                offset0 += ih.nsconz;

            for (std::size_t elm_index = 0; elm_index < ih.nsconz; elm_index++)
                BOOST_CHECK_EQUAL(sconn1[offset1 + elm_index], sconn0[offset0 + elm_index]);
        }
    }

    {
        const auto xconn0 = conn0.getXConn();
        const auto xconn1 = conn1.getXConn();
        for (std::size_t conn_index = 0; conn_index < num_test_connections; conn_index++) {
            std::size_t offset1 = conn_index * ih.nxconz;
            std::size_t offset0 = offset1;

            if (conn_index >= 2)
                offset0 += ih.nxconz;

            for (std::size_t elm_index = 0; elm_index < ih.nxconz; elm_index++)
                BOOST_CHECK_EQUAL(xconn1[offset1 + elm_index], xconn0[offset0 + elm_index]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
