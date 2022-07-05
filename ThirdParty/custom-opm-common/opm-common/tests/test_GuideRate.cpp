/*
  Copyright (c) 2020 Equinor ASA

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

#define BOOST_TEST_MODULE test_GuideRate

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Schedule/Group/GuideRate.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/input/eclipse/Units/Units.hpp>

#include <memory>
#include <string>
#include <utility>

#include <stddef.h>

namespace {
    struct Setup
    {
        explicit Setup(const std::string& input)
            : Setup { Opm::Parser{}.parseString(input) }
        {}

        explicit Setup(const Opm::Deck& deck)
            : es    { deck }
            , sched { deck, es, std::make_shared<const Opm::Python>() }
            , gr    { sched }
        {}

        Opm::EclipseState es;
        Opm::Schedule     sched;
        Opm::GuideRate    gr;
    };

    Setup case_10x10x10_model4(const double damping_factor = 0.5)
    {
        const auto prolog = std::string { R"(RUNSPEC
START
  4 'AUG' 2020 /

TITLE
  Check GUIDERAT Formula Implementation

DIMENS
  10 10 10 /

OIL
GAS
WATER
DISGAS
VAPOIL

METRIC

TABDIMS
/

WELLDIMS
3 10 2 2 /

GRID

DXV
  10*100 /

DYV
  10*100 /

DZV
  10*5 /

DEPTHZ
  121*2000 /

PERMX
  1000*100 /

COPY
  PERMX PERMY /
  PERMX PERMZ /
/

MULTIPLY
  PERMZ 0.1 /
/

PORO
  1000*0.3 /

SOLUTION

PRESSURE
  1000*320 /
SOIL
  1000*0.85 /
SWAT
  1000*0.12 /
SGAS
  1000*0.03 /
RS
  1000*226.0 /
RV
  1000*0.0 /

SCHEDULE

WELSPECS
  P1 P 10 7 2002.5 OIL /
  P2 P 7 10 2002.5 OIL /
  I1 I 2  2 2002.5 GAS /
/

COMPDAT
  P1 2* 1 10 OPEN 1* 1* 0.5 /
  P2 2* 1 10 OPEN 1* 1* 0.5 /
  I1 2* 1 10 OPEN 1* 1* 0.5 /
/

WCONINJE
  'I1' 'GAS'  'OPEN'  'RATE'  200  1*  450.0 /
/

)"  };

        const auto guiderat = std::string { R"(
-- GR_{oil} = WOPP / (0.5 + (WWPP / WOPP))
-- with a user-specified damping/time-delay factor (default 0.5).
GUIDERAT
--1   2   3   4   5   6   7  8  9          10
  1.0 OIL 1.0 0.5 1.0 1.0 1* 1* YES )" } + std::to_string(damping_factor) + " /\n";

        const auto epilog = std::string { R"(
WCONPROD
  P* OPEN GRUP 150 100 15E+3 250 1* 50 25 /
/

WCONINJE
  I1 GAS OPEN RATE 20.0E+3 1* 500 350 /
/

GCONPROD
  P 'ORAT' 200.0 150.0 100.0E+3 1* 1* YES 1* FORM /
/

DATES
  5 'AUG' 2020 /
 10 'AUG' 2020 /
 20 'AUG' 2020 /
  1 'SEP' 2020 /
  1 'OCT' 2020 /
  1 'NOV' 2020 /
  1 'DEC' 2020 /
  1 'JAN' 2021 /
/

END
)" };

        return Setup { prolog + guiderat + epilog };
}
} // Namespace anonymous

// ======================================================================

BOOST_AUTO_TEST_SUITE(GuideRate_Calculations)

BOOST_AUTO_TEST_CASE(P1_First)
{
    auto cse = case_10x10x10_model4();

    const auto wopp = 1.0;
    const auto wgpp = 5.0;
    const auto wwpp = 0.1;
    const auto stm  = 0.0;
    const auto rpt  = size_t{1};

    cse.gr.updateGuideRateExpiration(stm, rpt);
    cse.gr.compute("P1", rpt, stm, wopp, wgpp, wwpp);

    const auto orat = 2.0;
    const auto grat = 4.0;      // == 2 * orat
    const auto wrat = 1.0;      // == orat / 2

    const auto expect_gr_oil = 1.0 / (0.5 + 0.1/1.0); // wopp / (0.5 + wwpp/wopp)

    // GR_{oil}
    {
        const auto grval = cse.gr.get("P1", Opm::Well::GuideRateTarget::OIL, { orat, grat, wrat });

        BOOST_CHECK_CLOSE(grval, expect_gr_oil, 1.0e-5);
    }

    // GR_{gas}
    {
        const auto grval = cse.gr.get("P1", Opm::Well::GuideRateTarget::GAS, { orat, grat, wrat });

        const auto expect = (grat / orat) * expect_gr_oil;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{water}
    {
        const auto grval = cse.gr.get("P1", Opm::Well::GuideRateTarget::WAT, { orat, grat, wrat });

        const auto expect = (wrat / orat) * expect_gr_oil;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }
}

BOOST_AUTO_TEST_CASE(P2_Second)
{
    auto cse = case_10x10x10_model4();

    {
        const auto wopp = 1.0;
        const auto wgpp = 5.0;
        const auto wwpp = 0.1;
        const auto stm  = 0.0;
        const auto rpt  = size_t{1};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P2", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 10.0;
        const auto wgpp = 50.0;
        const auto wwpp = 1.0;
        const auto stm  = 10.0*Opm::unit::second; // Before recalculation delay
        const auto rpt  = size_t{1};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P2", rpt, stm, wopp, wgpp, wwpp);
    }

    const auto orat = 2.0;
    const auto grat = 4.0;      // == 2 * orat
    const auto wrat = 1.0;      // == orat / 2

    const auto expect_gr_oil_1 = 1.0 / (0.5 + 0.1/1.0); // wopp_1 / (0.5 + wwpp_1/wopp_1)

    // GR_{oil}
    {
        const auto grval = cse.gr.get("P2", Opm::Well::GuideRateTarget::OIL, { orat, grat, wrat });

        BOOST_CHECK_CLOSE(grval, expect_gr_oil_1, 1.0e-5);
    }

    // GR_{gas}
    {
        const auto grval = cse.gr.get("P2", Opm::Well::GuideRateTarget::GAS, { orat, grat, wrat });

        const auto expect = (grat / orat) * expect_gr_oil_1;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{water}
    {
        const auto grval = cse.gr.get("P2", Opm::Well::GuideRateTarget::WAT, { orat, grat, wrat });

        const auto expect = (wrat / orat) * expect_gr_oil_1;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    {
        const auto wopp = 10.0;
        const auto wgpp = 50.0;
        const auto wwpp = 1.0;
        const auto stm  = 10.0*Opm::unit::day; // After recalculation delay
        const auto rpt  = size_t{3};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P2", rpt, stm, wopp, wgpp, wwpp);
    }

    const auto expect_gr_oil_2 = 10.0 / (0.5 + 1.0/10.0); // wopp_2 / (0.5 + wwpp_2/wopp_2)

    // GR_{oil}
    {
        const auto grval = cse.gr.get("P2", Opm::Well::GuideRateTarget::OIL, { orat, grat, wrat });

        const auto expect = 0.5*expect_gr_oil_2 + 0.5*expect_gr_oil_1;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{gas}
    {
        const auto grval = cse.gr.get("P2", Opm::Well::GuideRateTarget::GAS, { orat, grat, wrat });

        const auto expect = (grat / orat) * (0.5*expect_gr_oil_2 + 0.5*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{water}
    {
        const auto grval = cse.gr.get("P2", Opm::Well::GuideRateTarget::WAT, { orat, grat, wrat });

        const auto expect = (wrat / orat) * (0.5*expect_gr_oil_2 + 0.5*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }
}

BOOST_AUTO_TEST_CASE(P_Third)
{
    auto cse = case_10x10x10_model4();

    {
        const auto wopp = 1.0;
        const auto wgpp = 5.0;
        const auto wwpp = 0.1;
        const auto stm  = 0.0;
        const auto rpt  = size_t{1};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 10.0;
        const auto wgpp = 50.0;
        const auto wwpp = 1.0;
        const auto stm  = 10.0*Opm::unit::day;
        const auto rpt  = size_t{3};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 20.0;
        const auto wgpp = 100.0;
        const auto wwpp = 10.0;
        const auto stm  = 20.0*Opm::unit::day;
        const auto rpt  = size_t{4};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    const auto expect_gr_oil_1 =  1.0 / (0.5 +  0.1/ 1.0); // wopp_1 / (0.5 + wwpp_1/wopp_1)
    const auto expect_gr_oil_2 = 10.0 / (0.5 +  1.0/10.0); // wopp_2 / (0.5 + wwpp_2/wopp_2)
    const auto expect_gr_oil_3 = 20.0 / (0.5 + 10.0/20.0); // wopp_3 / (0.5 + wwpp_3/wopp_3)

    const auto orat = 2.0;
    const auto grat = 4.0;      // == 2 * orat
    const auto wrat = 1.0;      // == orat / 2

    // GR_{oil}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::OIL, { orat, grat, wrat });

        const auto expect = 0.5*expect_gr_oil_3 + 0.5*0.5*expect_gr_oil_2 + 0.5*0.5*expect_gr_oil_1;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{gas}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::GAS, { orat, grat, wrat });

        const auto expect = (grat / orat) * (0.5*expect_gr_oil_3 + 0.5*0.5*expect_gr_oil_2 + 0.5*0.5*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{water}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::WAT, { orat, grat, wrat });

        const auto expect = (wrat / orat) * (0.5*expect_gr_oil_3 + 0.5*0.5*expect_gr_oil_2 + 0.5*0.5*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }
}

BOOST_AUTO_TEST_CASE(P_Third_df01)
{
    auto cse = case_10x10x10_model4(0.1);

    {
        const auto wopp = 1.0;
        const auto wgpp = 5.0;
        const auto wwpp = 0.1;
        const auto stm  = 0.0;
        const auto rpt  = size_t{1};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 10.0;
        const auto wgpp = 50.0;
        const auto wwpp = 1.0;
        const auto stm  = 10.0*Opm::unit::day;
        const auto rpt  = size_t{3};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 20.0;
        const auto wgpp = 100.0;
        const auto wwpp = 10.0;
        const auto stm  = 20.0*Opm::unit::day;
        const auto rpt  = size_t{4};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    const auto expect_gr_oil_1 =  1.0 / (0.5 +  0.1/ 1.0); // wopp_1 / (0.5 + wwpp_1/wopp_1)
    const auto expect_gr_oil_2 = 10.0 / (0.5 +  1.0/10.0); // wopp_2 / (0.5 + wwpp_2/wopp_2)
    const auto expect_gr_oil_3 = 20.0 / (0.5 + 10.0/20.0); // wopp_3 / (0.5 + wwpp_3/wopp_3)

    const auto orat = 2.0;
    const auto grat = 4.0;      // == 2 * orat
    const auto wrat = 1.0;      // == orat / 2

    // GR_{oil}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::OIL, { orat, grat, wrat });

        const auto expect = 0.1*expect_gr_oil_3 + 0.1*0.9*expect_gr_oil_2 + 0.9*0.9*expect_gr_oil_1;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{gas}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::GAS, { orat, grat, wrat });

        const auto expect = (grat / orat) * (0.1*expect_gr_oil_3 + 0.1*0.9*expect_gr_oil_2 + 0.9*0.9*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{water}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::WAT, { orat, grat, wrat });

        const auto expect = (wrat / orat) * (0.1*expect_gr_oil_3 + 0.1*0.9*expect_gr_oil_2 + 0.9*0.9*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }
}

BOOST_AUTO_TEST_CASE(P_Third_df09)
{
    auto cse = case_10x10x10_model4(0.9);

    {
        const auto wopp = 1.0;
        const auto wgpp = 5.0;
        const auto wwpp = 0.1;
        const auto stm  = 0.0;
        const auto rpt  = size_t{1};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 10.0;
        const auto wgpp = 50.0;
        const auto wwpp = 1.0;
        const auto stm  = 10.0*Opm::unit::day;
        const auto rpt  = size_t{3};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    {
        const auto wopp = 20.0;
        const auto wgpp = 100.0;
        const auto wwpp = 10.0;
        const auto stm  = 20.0*Opm::unit::day;
        const auto rpt  = size_t{4};

        cse.gr.updateGuideRateExpiration(stm, rpt);
        cse.gr.compute("P", rpt, stm, wopp, wgpp, wwpp);
    }

    const auto expect_gr_oil_1 =  1.0 / (0.5 +  0.1/ 1.0); // wopp_1 / (0.5 + wwpp_1/wopp_1)
    const auto expect_gr_oil_2 = 10.0 / (0.5 +  1.0/10.0); // wopp_2 / (0.5 + wwpp_2/wopp_2)
    const auto expect_gr_oil_3 = 20.0 / (0.5 + 10.0/20.0); // wopp_3 / (0.5 + wwpp_3/wopp_3)

    const auto orat = 2.0;
    const auto grat = 4.0;      // == 2 * orat
    const auto wrat = 1.0;      // == orat / 2

    // GR_{oil}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::OIL, { orat, grat, wrat });

        const auto expect = 0.9*expect_gr_oil_3 + 0.9*0.1*expect_gr_oil_2 + 0.1*0.1*expect_gr_oil_1;
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{gas}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::GAS, { orat, grat, wrat });

        const auto expect = (grat / orat) * (0.9*expect_gr_oil_3 + 0.9*0.1*expect_gr_oil_2 + 0.1*0.1*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    // GR_{water}
    {
        const auto grval = cse.gr.get("P", Opm::Group::GuideRateProdTarget::WAT, { orat, grat, wrat });

        const auto expect = (wrat / orat) * (0.9*expect_gr_oil_3 + 0.9*0.1*expect_gr_oil_2 + 0.1*0.1*expect_gr_oil_1);
        BOOST_CHECK_CLOSE(grval, expect, 1.0e-5);
    }

    const auto& sched = cse.sched;
    auto wi = sched.getWell("I1", 0);
    wi.updateWellGuideRate(true, 1.0, Opm::Well::GuideRateTarget::RAT, 1.0);
    BOOST_CHECK( wi.getGuideRatePhase() == Opm::Well::GuideRateTarget::GAS );
}

BOOST_AUTO_TEST_SUITE_END() // GuideRate_Calculations
