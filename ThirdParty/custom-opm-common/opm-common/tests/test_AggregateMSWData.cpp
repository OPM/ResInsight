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

#define BOOST_TEST_MODULE Aggregate_MSW_Data
#include <opm/output/eclipse/AggregateMSWData.hpp>

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/io/eclipse/rst/segment.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>

#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include <cstddef>

struct MockIH
{
    MockIH(const int numWells,

	   const int nsegWell 	 =   2,  // E100
	   const int isegPerWell =  22,  // E100
	   const int rsegPerWell = 146,  // E100
	   const int ilbsPerWell =   5,  // E100
	   const int ilbrPerWell =  10); // E100


    std::vector<int> value;

    using Sz = std::vector<int>::size_type;

    Sz nwells;

    Sz nsegwl;
    Sz nswlmx;
    Sz nsegmx;
    Sz nlbrmx;
    Sz nisegz;
    Sz nrsegz;
    Sz nilbrz;
};

MockIH::MockIH(const int numWells,
	       const int nsegWell,
	       const int isegPerWell,
	       const int rsegPerWell,
	       const int ilbsPerWell,
	       const int ilbrPerWell )
    : value(411, 0)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::intehead;

    this->nwells = this->value[Ix::NWELLS] = numWells;

    this->nsegwl = this->value[Ix::NSEGWL] = nsegWell;
    this->nswlmx = this->value[Ix::NSWLMX] = 2;
    this->nsegmx = this->value[Ix::NSEGMX] = 32;
    this->nisegz = this->value[Ix::NISEGZ] = isegPerWell;
    this->nrsegz = this->value[Ix::NRSEGZ] = rsegPerWell;
    this->nlbrmx = this->value[Ix::NLBRMX] = ilbsPerWell;
    this->nilbrz = this->value[Ix::NILBRZ] = ilbrPerWell;
}

namespace {
    Opm::Deck first_sim()
    {
        // Mostly copy of tests/FIRST_SIM.DATA
        const std::string input = std::string {
            R"~(
RUNSPEC

TITLE
 TWO MULTI-LATERAL WELLS; PRODUCER AND INJECTOR - MULTI-SEGMENT BRANCHES

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
 2  32  5  /

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

RPTGRID
  -- Report Levels for Grid Section Data
  --
 /

PORO
   500*0.15 /

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
 7020.00 2700.00 7990.00  .00000 7200.00  .00000     0      0       5 /
 7200.00 3700.00 7300.00  .00000 7100.00  .00000     1      0       5 /

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
 'PROD'
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
 'PROD' 3 5 2 2   3*  0.2   3*  'X' /
 'PROD' 4 5 2 2   3*  0.2   3*  'X' /
 'PROD' 5 5 2 2   3*  0.2   3*  'X' /

'WINJ' 10 1  9 9   3*  0.2   3*  'X' /
 'WINJ'   9 1  9 9   3*  0.2   3*  'X' /
 'WINJ'   8 1  9 9   3*  0.2   3*  'X' /
 'WINJ'   7 1  9 9   3*  0.2   3*  'X' /
 'WINJ'   6 1  9 9   3*  0.2   3*  'X' /
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

WELSEGS

-- Name    Dep 1   Tlen 1  Vol 1
  'WINJ'   7010      10    0.31   'INC' /

-- First   Last   Branch   Outlet  Length   Depth  Diam  Ruff  Area  Vol
-- Seg     Seg    Num      Seg              Chang
-- Main Stem
    2       14     1        1         20     20    0.2   1.E-3  1*   1* /

-- Bottom Branch
    15      15     2        14        50      0    0.2   1.E-3  1*   1* /
    16      19     2        15       100      0    0.2   1.E-3  1*   1* /
 /

COMPSEGS

-- Name
  'WINJ' /

-- I  J  K  Brn  Start   End     Dirn   End
--          No   Length  Length  Penet  Range

-- Bottom Branch
  10  1  9  2        270      370     'X'    3* /
    9  1  9  2        370      470     'X'    3* /
    8  1  9  2        470      570     'X'    3* /
    7  1  9  2        570      670     'X'    3* /
    6  1  9  2        670      770     'X'    3* /
 /


WCONPROD
 'PROD' 'OPEN' 'LRAT'  3*  2000  1*  2500  1*  /
 /

WCONINJE
 'WINJ' 'WAT' 'OPEN' 'RESV'  1*  2000  3500  1*  /
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

        Opm::SummaryState sim_state()
    {
        auto state = Opm::SummaryState{std::chrono::system_clock::now()};

	state.update("SPR:PROD:1",   235.);
	state.update("SPR:PROD:2",   237.);
	state.update("SPR:PROD:3",   239.);
	state.update("SPR:PROD:4",   243.);

	state.update("SOFR:PROD:1",   35.);
	state.update("SOFR:PROD:2",   30.);
	state.update("SOFR:PROD:3",   25.);
	state.update("SOFR:PROD:4",   20.);

	state.update("SGFR:PROD:1",   25.E3);
	state.update("SGFR:PROD:2",   20.E3);
	state.update("SGFR:PROD:3",   15.E3);
	state.update("SGFR:PROD:4",   10.E3);

	state.update("SWFR:PROD:1",  11.);
	state.update("SWFR:PROD:2",  12.);
	state.update("SWFR:PROD:3",  13.);
	state.update("SWFR:PROD:4",  14.);

	state.update("SPR:WINJ:1",   310.);
	state.update("SPR:WINJ:2",   320.);
	state.update("SPR:WINJ:3",   330.);
	state.update("SPR:WINJ:4",   340.);

	state.update("SWFR:WINJ:1",  21.);
	state.update("SWFR:WINJ:2",  22.);
	state.update("SWFR:WINJ:3",  23.);
	state.update("SWFR:WINJ:4",  24.);

	state.update("WBHP:WINJ",  234.);
        return state;
    }
    Opm::data::WellRates wr()
    {
        using o = ::Opm::data::Rates::opt;

        auto xw = ::Opm::data::WellRates{};

        {
            xw["PROD"].rates
                .set(o::wat, 1.0)
                .set(o::oil, 2.0)
                .set(o::gas, 3.0);
	    xw["PROD"].bhp = 213.0;
	    double qo = 5.;
	    double qw = 4.;
	    double qg = 50.;
            for (int i = 0; i < 5; i++) {
		xw["PROD"].connections.emplace_back();
		auto& c = xw["PROD"].connections.back();

		c.rates.set(o::wat, qw*(float(i)+1.))
                   .set(o::oil, qo*(float(i)+1.))
                   .set(o::gas, qg*(float(i)+1.));
	    }
	    auto seg = Opm::data::Segment{};
	    for (std::size_t i = 1; i < 5; i++) {
		xw["PROD"].segments.insert(std::pair<std::size_t,Opm::data::Segment>(i,seg));
	    }
            xw["WINJ"].bhp = 234.0;

            xw["WINJ"].rates.set(o::wat, 5.0);
	    xw["WINJ"].rates.set(o::oil, 0.0);
	    xw["WINJ"].rates.set(o::gas, 0.0);
	    qw = 7.;
            for (int i = 0; i < 5; i++) {
		xw["WINJ"].connections.emplace_back();
		auto& c = xw["WINJ"].connections.back();

		c.rates.set(o::wat, qw*(float(i)+1.))
		       .set(o::oil, 0.)
		       .set(o::gas, 0.);
	    }
        }
        return xw;
    }
}

struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   ( deck )
        , grid ( deck )
        , python( std::make_shared<Opm::Python>() )
        , sched( deck, es, python )
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python>  python;
    Opm::Schedule     sched;
};

// =====================================================================

BOOST_AUTO_TEST_SUITE(Aggregate_MSW)


// test dimensions of multisegment data
BOOST_AUTO_TEST_CASE (Constructor)
{
    const auto ih = MockIH{ 5 };

    const auto amswd = Opm::RestartIO::Helpers::AggregateMSWData{ ih.value };

    BOOST_CHECK_EQUAL(amswd.getISeg().size(), ih.nswlmx * ih.nsegmx * ih.nisegz);
    BOOST_CHECK_EQUAL(amswd.getRSeg().size(), ih.nswlmx * ih.nsegmx * ih.nrsegz);
    BOOST_CHECK_EQUAL(amswd.getILBs().size(), ih.nswlmx * ih.nlbrmx);
    BOOST_CHECK_EQUAL(amswd.getILBr().size(), ih.nswlmx * ih.nlbrmx * ih.nilbrz);
}


BOOST_AUTO_TEST_CASE (Declared_MSW_Data)
{
    const auto simCase = SimulationCase{first_sim()};

    // Report Step 1: 2115-01-01 --> 2015-01-03
    const auto rptStep = std::size_t{1};

    const auto ih = MockIH {
        static_cast<int>(simCase.sched.getWells(rptStep).size())
    };

    BOOST_CHECK_EQUAL(ih.nwells, MockIH::Sz{2});

    const auto smry = sim_state();
    const Opm::data::WellRates wrc = wr();
    auto amswd = Opm::RestartIO::Helpers::AggregateMSWData{ih.value};
    amswd.captureDeclaredMSWData(simCase.sched,
			      rptStep,
                              simCase.es.getUnits(),
			      ih.value,
			      simCase.grid,
			      smry,
			      wrc
			      );

    // ISEG (PROD)
    {
        auto start = 2*ih.nisegz;

        const auto& iSeg = amswd.getISeg();
	BOOST_CHECK_EQUAL(iSeg[start + 0] , 15); // PROD-segment 3, ordered segment
	BOOST_CHECK_EQUAL(iSeg[start + 1] ,  2); // PROD-segment 3, outlet segment
	BOOST_CHECK_EQUAL(iSeg[start + 2] ,  4); // PROD-segment 3, inflow segment current branch
	BOOST_CHECK_EQUAL(iSeg[start + 3] ,  1); // PROD-segment 3, branch number
	BOOST_CHECK_EQUAL(iSeg[start + 4] ,  1); // PROD-segment 3, number of inflow branches
	BOOST_CHECK_EQUAL(iSeg[start + 5] ,  1); // PROD-segment 3, Sum number of inflow branches from first segment to current segment
	BOOST_CHECK_EQUAL(iSeg[start + 6] ,  0); // PROD-segment 3, number of connections in segment
	BOOST_CHECK_EQUAL(iSeg[start + 7] ,  0); // PROD-segment 3, sum of connections with lower segmeent number than current segment
	BOOST_CHECK_EQUAL(iSeg[start + 8] , 15); // PROD-segment 3, ordered segment

	start = 13*ih.nisegz;

	BOOST_CHECK_EQUAL(iSeg[start + 0] ,  4); // PROD-segment 14, ordered segment
	BOOST_CHECK_EQUAL(iSeg[start + 1] , 13); // PROD-segment 14, outlet segment
	BOOST_CHECK_EQUAL(iSeg[start + 2] , 15); // PROD-segment 14, inflow segment current branch
	BOOST_CHECK_EQUAL(iSeg[start + 3] ,  2); // PROD-segment 14, branch number
	BOOST_CHECK_EQUAL(iSeg[start + 4] ,  0); // PROD-segment 14, number of inflow branches
	BOOST_CHECK_EQUAL(iSeg[start + 5] ,  0); // PROD-segment 14, Sum number of inflow branches from first segment to current segment
	BOOST_CHECK_EQUAL(iSeg[start + 6] ,  1); // PROD-segment 14, number of connections in segment
	BOOST_CHECK_EQUAL(iSeg[start + 7] ,  2); // PROD-segment 14, sum of connections with lower segmeent number than current segment
	BOOST_CHECK_EQUAL(iSeg[start + 8] ,  4); // PROD-segment 14, ordered segment

    }

    // ISEG (WINJ)
    {
        auto start = ih.nisegz*ih.nsegmx + 13*ih.nisegz;

        const auto& iSeg = amswd.getISeg();
	BOOST_CHECK_EQUAL(iSeg[start + 0] ,  6); // WINJ-segment 14, ordered segment
	BOOST_CHECK_EQUAL(iSeg[start + 1] , 13); // WINJ-segment 14, outlet segment
	BOOST_CHECK_EQUAL(iSeg[start + 2] ,  0); // WINJ-segment 14, inflow segment current branch
	BOOST_CHECK_EQUAL(iSeg[start + 3] ,  1); // WINJ-segment 14, branch number
	BOOST_CHECK_EQUAL(iSeg[start + 4] ,  1); // WINJ-segment 14, number of inflow branches
	BOOST_CHECK_EQUAL(iSeg[start + 5] ,  1); // WINJ-segment 14, Sum number of inflow branches from first segment to current segment
	BOOST_CHECK_EQUAL(iSeg[start + 6] ,  0); // WINJ-segment 14, number of connections in segment
	BOOST_CHECK_EQUAL(iSeg[start + 7] ,  0); // WINJ-segment 14, sum of connections with lower segmeent number than current segment
	BOOST_CHECK_EQUAL(iSeg[start + 8] ,  6); // WINJ-segment 14, ordered segment

	start = ih.nisegz*ih.nsegmx + 16*ih.nisegz;
	BOOST_CHECK_EQUAL(iSeg[start + 0] ,  3); // WINJ-segment 17, ordered segment
	BOOST_CHECK_EQUAL(iSeg[start + 1] , 16); // WINJ-segment 17, outlet segment
 	BOOST_CHECK_EQUAL(iSeg[start + 2] , 18); // WINJ-segment 17, inflow segment current branch
	BOOST_CHECK_EQUAL(iSeg[start + 3] ,  2); // WINJ-segment 17, branch number
	BOOST_CHECK_EQUAL(iSeg[start + 4] ,  0); // WINJ-segment 17, number of inflow branches
	BOOST_CHECK_EQUAL(iSeg[start + 5] ,  0); // WINJ-segment 17, Sum number of inflow branches from first segment to current segment
	BOOST_CHECK_EQUAL(iSeg[start + 6] ,  1); // WINJ-segment 17, number of connections in segment
	BOOST_CHECK_EQUAL(iSeg[start + 7] ,  3); // WINJ-segment 17, sum of connections with lower segmeent number than current segment
	BOOST_CHECK_EQUAL(iSeg[start + 8] ,  3); // WINJ-segment 17, ordered segment

    }

    // RSEG (PROD)  + (WINJ)
    {
	// well no 1 - PROD
	const std::string wname = "PROD";
	int segNo = 1;
	// 'stringSegNum' is one-based (1 .. #segments inclusive)
	std::string stringSegNo = std::to_string(segNo);

        const auto  i0 = (segNo-1)*ih.nrsegz;
	const auto&  units = simCase.es.getUnits();
	const auto gfactor = (units.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD)
		    ? 0.1781076 : 0.001;
        const auto& rseg = amswd.getRSeg();

	BOOST_CHECK_CLOSE(rseg[i0     ],   10.  , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  1], 7010.  , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  5], 0.31   , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  6],   10.  , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  7], 7010.  , 1.0e-10);

	const double temp_o = smry.get("SOFR:PROD:1");
	const double temp_w = smry.get("SWFR:PROD:1")*0.1;
	const double temp_g = smry.get("SGFR:PROD:1")*gfactor;

	auto t0 = temp_o + temp_w + temp_g;
	double t1 = (std::abs(temp_w) > 0) ? temp_w / t0 : 0.;
	double t2 = (std::abs(temp_g) > 0) ? temp_g / t0 : 0.;

	BOOST_CHECK_CLOSE(rseg[i0 +  8],  t0, 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  9],  t1, 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 + 10],  t2, 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 + 11], 235., 1.0e-10);
    }

    {
    // well no 2 - WINJ
	const std::string wname = "WINJ";
	int segNo = 1;
	// 'stringSegNum' is one-based (1 .. #segments inclusive)
	std::string stringSegNo = std::to_string(segNo);

        const auto  i0 = ih.nrsegz*ih.nsegmx + (segNo-1)*ih.nrsegz;
	const auto&  units = simCase.es.getUnits();
	using M = ::Opm::UnitSystem::measure;
	const auto gfactor = (units.getType() == Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD)
		    ? 0.1781076 : 0.001;
        const auto& rseg = amswd.getRSeg();

	BOOST_CHECK_CLOSE(rseg[i0     ],   10.  , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  1], 7010.  , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  5], 0.31   , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  6],   10.  , 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  7], 7010.  , 1.0e-10);

	const double temp_o = 0.;
	const double temp_w = -units.from_si(M::liquid_surface_rate,105.)*0.1;
	const double temp_g = 0.0*gfactor;

	auto t0 = temp_o + temp_w + temp_g;
	double t1 = (std::abs(temp_w) > 0) ? temp_w / t0 : 0.;
	double t2 = (std::abs(temp_g) > 0) ? temp_g / t0 : 0.;

	BOOST_CHECK_CLOSE(rseg[i0 +  8],  t0, 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 +  9],  t1, 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 + 10],  t2, 1.0e-10);
	BOOST_CHECK_CLOSE(rseg[i0 + 11], 234., 1.0e-10);

    }

    // ILBR
    {
        auto start =  0*ih.nilbrz;

        const auto& iLBr = amswd.getILBr();
	//PROD
	BOOST_CHECK_EQUAL(iLBr[start + 0] ,  0); // PROD-branch   1, outlet segment
	BOOST_CHECK_EQUAL(iLBr[start + 1] , 12); // PROD-branch   1, No of segments in branch
	BOOST_CHECK_EQUAL(iLBr[start + 2] ,  1); // PROD-branch   1, first segment
	BOOST_CHECK_EQUAL(iLBr[start + 3] , 12); // PROD-branch   1, last segment
	BOOST_CHECK_EQUAL(iLBr[start + 4] ,  0); // PROD-branch   1, branch no - 1
	//PROD
	start =  1*ih.nilbrz;
	BOOST_CHECK_EQUAL(iLBr[start + 0] ,  3); // PROD-branch   2, outlet segment
	BOOST_CHECK_EQUAL(iLBr[start + 1] ,  5); // PROD-branch   2, No of segments in branch
	BOOST_CHECK_EQUAL(iLBr[start + 2] , 13); // PROD-branch   2, first segment
	BOOST_CHECK_EQUAL(iLBr[start + 3] , 17); // PROD-branch   2, last segment
	BOOST_CHECK_EQUAL(iLBr[start + 4] ,  1); // PROD-branch   2, branch no - 1


	start = ih.nilbrz*ih.nlbrmx + 0*ih.nilbrz;
	//WINJ
	BOOST_CHECK_EQUAL(iLBr[start + 0] ,  0); // WINJ-branch   1, outlet segment
	BOOST_CHECK_EQUAL(iLBr[start + 1] , 14); // WINJ-branch   1, No of segments in branch
	BOOST_CHECK_EQUAL(iLBr[start + 2] ,  1); // WINJ-branch   1, first segment
	BOOST_CHECK_EQUAL(iLBr[start + 3] , 14); // WINJ-branch   1, last segment
	BOOST_CHECK_EQUAL(iLBr[start + 4] ,  0); // WINJ-branch   1, branch no - 1

	start = ih.nilbrz*ih.nlbrmx + 1*ih.nilbrz;
	//WINJ
	BOOST_CHECK_EQUAL(iLBr[start + 0] , 14); // WINJ-branch   2, outlet segment
	BOOST_CHECK_EQUAL(iLBr[start + 1] ,  5); // WINJ-branch   2, No of segments in branch
	BOOST_CHECK_EQUAL(iLBr[start + 2] , 15); // WINJ-branch   2, first segment
	BOOST_CHECK_EQUAL(iLBr[start + 3] , 19); // WINJ-branch   2, last segment
	BOOST_CHECK_EQUAL(iLBr[start + 4] ,  1); // WINJ-branch   2, branch no - 1


    }

       // ILBS
    {
        auto start =  0*ih.nlbrmx;

        const auto& iLBs = amswd.getILBs();
	//PROD
	BOOST_CHECK_EQUAL(iLBs[start + 0] ,  13); // PROD-branch   2, first segment in branch

	start = ih.nlbrmx + 0*ih.nlbrmx;
	//WINJ
	BOOST_CHECK_EQUAL(iLBs[start + 0] ,  15); // WINJ-branch   2, first segment in branch

    }
}


BOOST_AUTO_TEST_CASE(MSW_RST) {
    const auto simCase = SimulationCase{first_sim()};

    // Report Step 1: 2115-01-01 --> 2015-01-03
    const auto rptStep = std::size_t{1};

    const auto ih = MockIH {
                            static_cast<int>(simCase.sched.getWells(rptStep).size())
    };

    const auto smry = sim_state();
    const Opm::data::WellRates wrc = wr();
    auto amswd = Opm::RestartIO::Helpers::AggregateMSWData{ih.value};
    amswd.captureDeclaredMSWData(simCase.sched,
                                 rptStep,
                                 simCase.es.getUnits(),
                                 ih.value,
                                 simCase.grid,
                                 smry,
                                 wrc
                                 );
    const auto& iseg = amswd.getISeg();
    const auto& rseg = amswd.getRSeg();
    auto segment = Opm::RestartIO::RstSegment(simCase.es.getUnits(), 1, iseg.data(), rseg.data());
}



BOOST_AUTO_TEST_SUITE_END()
