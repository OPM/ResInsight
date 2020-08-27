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

#define BOOST_TEST_MODULE Aggregate_Group_Data
#include <opm/output/eclipse/AggregateGroupData.hpp>

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/output/data/Wells.hpp>

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

	   const int igrpPerGrp	 = 101,  // no of data elements per group in IGRP array
	   const int sgrpPerGrp  = 112,  // number of data elements per group in SGRP array
	   const int xgrpPerGrp  = 180,  // number of data elements per group in XGRP array
	   const int zgrpPerGrp  =   5);  // number of data elements per group in XGRP array


    std::vector<int> value;

    using Sz = std::vector<int>::size_type;

    Sz nwells;
    Sz nwgmax;
    Sz ngmaxz;
    Sz nigrpz;
    Sz nsgrpz;
    Sz nxgrpz;
    Sz nzgrpz;
};

MockIH::MockIH(const int numWells,
	       const int igrpPerGrp,
	       const int sgrpPerGrp,
	       const int xgrpPerGrp,
	       const int zgrpPerGrp)
    : value(411, 0)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::intehead;

    this->nwells = this->value[Ix::NWELLS] = numWells;

    this->ngmaxz = this->value[Ix::NGMAXZ] = 5;
    this->nwgmax = this->value[Ix::NWGMAX] = 4;
    this->nigrpz = this->value[Ix::NIGRPZ] = igrpPerGrp;
    this->nsgrpz = this->value[Ix::NSGRPZ] = sgrpPerGrp;
    this->nxgrpz = this->value[Ix::NXGRPZ] = xgrpPerGrp;
    this->nzgrpz = this->value[Ix::NZGRPZ] = zgrpPerGrp;
}

namespace {
    Opm::Deck first_sim()
    {
        // Mostly copy of tests/FIRST_SIM.DATA
        const std::string input = std::string {
            R"~(
RUNSPEC

TITLE
2 PRODUCERS  AND INJECTORS, 2 WELL GROUPS AND ONE INTERMEDIATE GROUP LEVEL  BELOW THE FIELD LEVEL

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
 4  20  4  2  /

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


SUMMARY      ===========================================================

FOPR

WOPR
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

GRUPTREE
 'GRP1' 'FIELD' /
 'WGRP1' 'GRP1' /
 'WGRP2' 'GRP1' /
/

WELSPECS
 'PROD1' 'WGRP1' 1 5 7030 'OIL' 0.0  'STD'  'STOP'  /
 'PROD2' 'WGRP2' 1 5 7030 'OIL' 0.0  'STD'  'STOP'  /
 'WINJ1'  'WGRP1' 10 1 7030 'WAT' 0.0  'STD'  'STOP'   /
 'WINJ2'  'WGRP2' 10 1 7030 'WAT' 0.0  'STD'  'STOP'   /
/

COMPDAT

 'PROD1' 1 5 2 2   3*  0.2   3*  'X' /
 'PROD1' 2 5 2 2   3*  0.2   3*  'X' /
 'PROD1' 3 5 2 2   3*  0.2   3*  'X' /
 'PROD2' 4 5 2 2   3*  0.2   3*  'X' /
 'PROD2' 5 5 2 2   3*  0.2   3*  'X' /

 'WINJ1' 10 1  9 9   3*  0.2   3*  'X' /
 'WINJ1'   9 1  9 9   3*  0.2   3*  'X' /
 'WINJ1'   8 1  9 9   3*  0.2   3*  'X' /
 'WINJ2'   7 1  9 9   3*  0.2   3*  'X' /
 'WINJ2'   6 1  9 9   3*  0.2   3*  'X' /
/


WCONPROD
 'PROD1' 'OPEN' 'LRAT'  3*  1200  1*  2500  1*  /
 'PROD2' 'OPEN' 'LRAT'  3*    800  1*  2500  1*  /
 /

WCONINJE
 'WINJ1' 'WAT' 'OPEN' 'BHP'  1*  1200  3500  1*  /
 'WINJ2' 'WAT' 'OPEN' 'BHP'  1*    800  3500  1*  /
 /


TUNING
 /
 /
 /

TSTEP
 4
/


END

)~" };

        return Opm::Parser{}.parseString(input);
    }

    Opm::SummaryState sim_state()
    {
        auto state = Opm::SummaryState{std::chrono::system_clock::now()};

        state.update("GOPR:GRP1",   235.);
        state.update("GGPR:GRP1",   100237.);
        state.update("GWPR:GRP1",   239.);
        state.update("GOPGR:GRP1",  345.6);
        state.update("GWPGR:GRP1",  456.7);
        state.update("GGPGR:GRP1",  567.8);
        state.update("GVPGR:GRP1",  678.9);
        state.update("GOIGR:GRP1", 0.123);
        state.update("GWIGR:GRP1", 1234.5);
        state.update("GGIGR:GRP1", 2345.6);

        state.update("GOPR:WGRP1",   23.);
        state.update("GGPR:WGRP1",   50237.);
        state.update("GWPR:WGRP1",   29.);
        state.update("GOPGR:WGRP1",  456.7);
        state.update("GWPGR:WGRP1",  567.8);
        state.update("GGPGR:WGRP1",  678.9);
        state.update("GVPGR:WGRP1",  789.1);
        state.update("GOIGR:WGRP1", 1.23);
        state.update("GWIGR:WGRP1", 2345.6);
        state.update("GGIGR:WGRP1", 3456.7);

        state.update("GOPR:WGRP2",   43.);
        state.update("GGPR:WGRP2",   70237.);
        state.update("GWPR:WGRP2",   59.);
        state.update("GOPGR:WGRP2",  56.7);
        state.update("GWPGR:WGRP2",  67.8);
        state.update("GGPGR:WGRP2",  78.9);
        state.update("GVPGR:WGRP2",  89.1);
        state.update("GOIGR:WGRP2", 12.3);
        state.update("GWIGR:WGRP2", 345.6);
        state.update("GGIGR:WGRP2", 456.7);

        state.update("FOPR",   3456.);
        state.update("FGPR",   2003456.);
        state.update("FWPR",   5678.);

        return state;
    }
}

struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   ( deck )
        , python( std::make_shared<Opm::Python>() )
        , sched (deck, es, python )
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;
};

// =====================================================================

BOOST_AUTO_TEST_SUITE(Aggregate_Group)


// test dimensions of multisegment data
BOOST_AUTO_TEST_CASE (Constructor)
{
    const auto ih = MockIH{ 5 };

    const auto agrpd = Opm::RestartIO::Helpers::AggregateGroupData{ ih.value };

    BOOST_CHECK_EQUAL(agrpd.getIGroup().size(), ih.ngmaxz * ih.nigrpz);
    BOOST_CHECK_EQUAL(agrpd.getSGroup().size(), ih.ngmaxz * ih.nsgrpz);
    BOOST_CHECK_EQUAL(agrpd.getXGroup().size(), ih.ngmaxz * ih.nxgrpz);
    BOOST_CHECK_EQUAL(agrpd.getZGroup().size(), ih.ngmaxz * ih.nzgrpz);
}


BOOST_AUTO_TEST_CASE (Declared_Group_Data)
{
    const auto simCase = SimulationCase{first_sim()};

    // Report Step 1: 2115-01-01 --> 2015-01-05
    const auto rptStep = std::size_t{1};

    const auto ih = MockIH {
        static_cast<int>(simCase.sched.getWells(rptStep).size())
    };

    BOOST_CHECK_EQUAL(ih.nwells, MockIH::Sz{4});

    const auto smry = sim_state();
    const auto& units    = simCase.es.getUnits();
    auto agrpd = Opm::RestartIO::Helpers::AggregateGroupData{ih.value};
    agrpd.captureDeclaredGroupData(simCase.sched, units, rptStep, smry,
            ih.value);

    // IGRP (PROD)
    {
        auto start = 0*ih.nigrpz;

        const auto& iGrp = agrpd.getIGroup();
        BOOST_CHECK_EQUAL(iGrp[start + 0] ,  2); // Group GRP1 - Child group number one - equal to group WGRP1 (no 2)
        BOOST_CHECK_EQUAL(iGrp[start + 1] ,  3); // Group GRP1 - Child group number two - equal to group WGRP2 (no 3)
        BOOST_CHECK_EQUAL(iGrp[start + 4] ,  2); // Group GRP1 - No of child groups
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 26] ,  1); // Group GRP1 - Group type (well group = 0, node group = 1)
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 27] ,  1); // Group GRP1 - Group level (FIELD level is 0)
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 28] ,  5); // Group GRP1 - index of parent group (= 0 for FIELD)

        start = 1*ih.nigrpz;
        BOOST_CHECK_EQUAL(iGrp[start + 0] ,  1); // Group WGRP1 - Child well number one - equal to well PROD1 (no 1)
        BOOST_CHECK_EQUAL(iGrp[start + 1] ,  3); // Group WGRP1 - Child well number two - equal to well WINJ1 (no 3)
        BOOST_CHECK_EQUAL(iGrp[start + 4] ,  2); // Group WGRP1 - No of child wells
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 26] ,  0); // Group WGRP1 - Group type (well group = 0, node group = 1)
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 27] ,  2); // Group WGRP1 - Group level (FIELD level is 0)
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 28] ,  1); // Group GRP1 - index of parent group (= 0 for FIELD)

        start = (ih.ngmaxz-1)*ih.nigrpz;
        BOOST_CHECK_EQUAL(iGrp[start + 0] ,  1); // Group FIELD - Child group number one - equal to group GRP1
        BOOST_CHECK_EQUAL(iGrp[start + 4] ,  1); // Group FIELD - No of child groups
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 26] ,  1); // Group FIELD - Group type (well group = 0, node group = 1)
        BOOST_CHECK_EQUAL(iGrp[start + ih.nwgmax + 27] ,  0); // Group FIELD - Group level (FIELD level is 0)
    }


    // XGRP (PROD)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::XGroup::index;
        auto start = 0*ih.nxgrpz;

        const auto& xGrp = agrpd.getXGroup();
        BOOST_CHECK_EQUAL(xGrp[start + 0] ,  235.); // Group GRP1 - GOPR
        BOOST_CHECK_EQUAL(xGrp[start + 1] ,  239.); // Group GRP1 - GWPR
        BOOST_CHECK_EQUAL(xGrp[start + 2] ,  100237.); // Group GRP1 - GGPR

        BOOST_CHECK_CLOSE(xGrp[start + Ix::OilPrGuideRate], 345.6, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::OilPrGuideRate],
                          xGrp[start + Ix::OilPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::WatPrGuideRate], 456.7, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::WatPrGuideRate],
                          xGrp[start + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::GasPrGuideRate], 567.8, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::GasPrGuideRate],
                          xGrp[start + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::VoidPrGuideRate], 678.9, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::VoidPrGuideRate],
                          xGrp[start + Ix::VoidPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::OilInjGuideRate], 0.123, 1.0e-10);
        BOOST_CHECK_CLOSE(xGrp[start + Ix::WatInjGuideRate], 1234.5, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::WatInjGuideRate],
                          xGrp[start + Ix::WatInjGuideRate_2]);
        BOOST_CHECK_CLOSE(xGrp[start + Ix::GasInjGuideRate], 2345.6, 1.0e-10);

        start = 1*ih.nxgrpz;
        BOOST_CHECK_EQUAL(xGrp[start + 0] ,  23.); // Group WGRP1 - GOPR
        BOOST_CHECK_EQUAL(xGrp[start + 1] ,  29.); // Group WGRP1 - GWPR
        BOOST_CHECK_EQUAL(xGrp[start + 2] ,  50237.); // Group WGRP1 - GGPR

        BOOST_CHECK_CLOSE(xGrp[start + Ix::OilPrGuideRate], 456.7, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::OilPrGuideRate],
                          xGrp[start + Ix::OilPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::WatPrGuideRate], 567.8, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::WatPrGuideRate],
                          xGrp[start + Ix::WatPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::GasPrGuideRate], 678.9, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::GasPrGuideRate],
                          xGrp[start + Ix::GasPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::VoidPrGuideRate], 789.1, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::VoidPrGuideRate],
                          xGrp[start + Ix::VoidPrGuideRate_2]);

        BOOST_CHECK_CLOSE(xGrp[start + Ix::OilInjGuideRate], 1.23, 1.0e-10);
        BOOST_CHECK_CLOSE(xGrp[start + Ix::WatInjGuideRate], 2345.6, 1.0e-10);
        BOOST_CHECK_EQUAL(xGrp[start + Ix::WatInjGuideRate],
                          xGrp[start + Ix::WatInjGuideRate_2]);
        BOOST_CHECK_CLOSE(xGrp[start + Ix::GasInjGuideRate], 3456.7, 1.0e-10);

        start = 2*ih.nxgrpz;
        BOOST_CHECK_EQUAL(xGrp[start + 0] ,  43.); // Group WGRP2 - GOPR
        BOOST_CHECK_EQUAL(xGrp[start + 1] ,  59.); // Group WGRP2 - GWPR
        BOOST_CHECK_EQUAL(xGrp[start + 2] ,  70237.); // Group WGRP2 - GGPR


        start = (ih.ngmaxz-1)*ih.nxgrpz;
        BOOST_CHECK_EQUAL(xGrp[start + 0] ,  3456.); // Group FIELD - FOPR
        BOOST_CHECK_EQUAL(xGrp[start + 1] ,  5678.); // Group FIELD - FWPR
        BOOST_CHECK_EQUAL(xGrp[start + 2] ,  2003456.); // Group FIELD - FGPR
    }

        // ZGRP (PROD)
    {
        auto start = 0*ih.nzgrpz;

        const auto& zGrp = agrpd.getZGroup();
        BOOST_CHECK_EQUAL(zGrp[start + 0].c_str() ,  "GRP1    "); // Group GRP1 - GOPR

        start = 1*ih.nzgrpz;
        BOOST_CHECK_EQUAL(zGrp[start + 0].c_str() ,  "WGRP1   "); // Group WGRP1 - GOPR

        start = 2*ih.nzgrpz;
        BOOST_CHECK_EQUAL(zGrp[start + 0].c_str() ,  "WGRP2   "); // Group WGRP2 - GOPR

        start = (ih.ngmaxz-1)*ih.nzgrpz;
        BOOST_CHECK_EQUAL(zGrp[start + 0].c_str() ,  "FIELD   "); // Group FIELD - FOPR
    }


}


BOOST_AUTO_TEST_SUITE_END()
