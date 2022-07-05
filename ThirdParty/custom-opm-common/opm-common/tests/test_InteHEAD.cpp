/*
  Copyright 2018 Statoil IT

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

#define BOOST_TEST_MODULE InteHEAD_Vector

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>

#include <opm/io/eclipse/rst/header.hpp>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iterator>
#include <numeric>              // partial_sum()
#include <string>
#include <vector>

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

namespace {
std::vector<double> elapsedTime(const Opm::Schedule& sched)
{
    auto elapsed = std::vector<double>{};

    elapsed.reserve(sched.size());
    elapsed.push_back(0.0);

    for (auto nstep = sched.size() - 1,
              step  = 0*nstep; step < nstep; ++step)
    {
        elapsed.push_back(sched.stepLength(step));
    }

    std::partial_sum(std::begin(elapsed), std::end(elapsed),
                     std::begin(elapsed));

    return elapsed;
}

    void expectDate(const Opm::RestartIO::InteHEAD::TimePoint& tp,
                    const int year, const int month, const int day)
    {
        BOOST_CHECK_EQUAL(tp.year  , year);
        BOOST_CHECK_EQUAL(tp.month , month);
        BOOST_CHECK_EQUAL(tp.day   , day);

        BOOST_CHECK_EQUAL(tp.hour        , 0);
        BOOST_CHECK_EQUAL(tp.minute      , 0);
        BOOST_CHECK_EQUAL(tp.second      , 0);
        BOOST_CHECK_EQUAL(tp.microseconds, 0);
    }

    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);
    }

} // Anonymous


//int main(int argc, char* argv[])
struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   { deck }
        , grid { deck }
        , python{ std::make_shared<Opm::Python>() }
        , sched{ deck, es, python }
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;

};


BOOST_AUTO_TEST_SUITE(Member_Functions)

BOOST_AUTO_TEST_CASE(Dimensions_Individual)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
        .dimensions(100, 60, 15);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NX], 100);
    BOOST_CHECK_EQUAL(v[VI::intehead::NY],  60);
    BOOST_CHECK_EQUAL(v[VI::intehead::NZ],  15);
}

BOOST_AUTO_TEST_CASE(Dimensions_Array)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
        .dimensions({ {100, 60, 15} });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NX], 100);
    BOOST_CHECK_EQUAL(v[VI::intehead::NY],  60);
    BOOST_CHECK_EQUAL(v[VI::intehead::NZ],  15);
}

BOOST_AUTO_TEST_CASE(NumActive)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
        .numActive(72390);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NACTIV], 72390);
}

BOOST_AUTO_TEST_CASE(UnitConventions)
{
    auto ih = Opm::RestartIO::InteHEAD{};

    // Metric
    {
        ih.unitConventions(Opm::UnitSystem::newMETRIC());

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::UNIT], 1);
    }

    // Field
    {
        ih.unitConventions(Opm::UnitSystem::newFIELD());

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::UNIT], 2);
    }

    // Lab
    {
        ih.unitConventions(Opm::UnitSystem::newLAB());

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::UNIT], 3);
    }

    // PVT-M
    {
        ih.unitConventions(Opm::UnitSystem::newPVT_M());

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::UNIT], 4);
    }
}

BOOST_AUTO_TEST_CASE(WellTableDimensions)
{
    const auto numWells        = 17;
    const auto maxPerf         = 29;
    const auto maxWellsInGroup  =  3;
    const auto maxGroupInField = 14;
    const auto maxWellsInField = 25;
    const auto mxwlstprwel = 3;
    const auto mxdynwlst = 4;

    const auto ih = Opm::RestartIO::InteHEAD{}
        .wellTableDimensions({
            numWells, maxPerf, maxWellsInGroup, maxGroupInField, maxWellsInField,
            mxwlstprwel, mxdynwlst
        });

    const auto& v = ih.data();
    const auto nwgmax = std::max(maxWellsInGroup, maxGroupInField);

    BOOST_CHECK_EQUAL(v[VI::intehead::NWELLS], numWells);
    BOOST_CHECK_EQUAL(v[VI::intehead::NCWMAX], maxPerf);
    BOOST_CHECK_EQUAL(v[VI::intehead::NWGMAX], nwgmax);
    BOOST_CHECK_EQUAL(v[VI::intehead::NGMAXZ], maxGroupInField + 1);
    BOOST_CHECK_EQUAL(v[VI::intehead::NWMAXZ], maxWellsInField);
    BOOST_CHECK_EQUAL(v[VI::intehead::MXWLSTPRWELL], mxwlstprwel);
    BOOST_CHECK_EQUAL(v[VI::intehead::MAXDYNWELLST], mxdynwlst);
}

BOOST_AUTO_TEST_CASE(CalendarDate)
{
    // 2015-04-09T11:22:33.987654+0000

    const auto ih = Opm::RestartIO::InteHEAD{}
        .calendarDate({
            2015, 4, 9, 11, 22, 33, 987654,
        });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[67 - 1], 2015); // Year
    BOOST_CHECK_EQUAL(v[66 - 1],    4); // Month
    BOOST_CHECK_EQUAL(v[65 - 1],    9); // Day

    BOOST_CHECK_EQUAL(v[207 - 1], 11); // Hour
    BOOST_CHECK_EQUAL(v[208 - 1], 22); // Minute
    BOOST_CHECK_EQUAL(v[411 - 1], 33987654); // Second (in microseconds)
}

BOOST_AUTO_TEST_CASE(ActivePhases)
{
    using Ph = Opm::RestartIO::InteHEAD::Phases;
    auto  ih = Opm::RestartIO::InteHEAD{};

    // Oil
    {
        ih.activePhases(Ph{ 1, 0, 0 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 1);
    }

    // Water
    {
        ih.activePhases(Ph{ 0, 1, 0 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 2);
    }

    // Gas
    {
        ih.activePhases(Ph{ 0, 0, 1 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 4);
    }

    // Oil/Water
    {
        ih.activePhases(Ph{ 1, 1, 0 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 3);
    }

    // Oil/Gas
    {
        ih.activePhases(Ph{ 1, 0, 1 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 5);
    }

    // Water/Gas
    {
        ih.activePhases(Ph{ 0, 1, 1 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 6);
    }

    // Oil/Water/Gas
    {
        ih.activePhases(Ph{ 1, 1, 1 });

        const auto& v = ih.data();

        BOOST_CHECK_EQUAL(v[VI::intehead::PHASE], 7);
    }
}

BOOST_AUTO_TEST_CASE(NWell_Parameters)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
      .params_NWELZ(27, 18, 28, 1);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NIWELZ], 27);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSWELZ], 18);
    BOOST_CHECK_EQUAL(v[VI::intehead::NXWELZ], 28);
    BOOST_CHECK_EQUAL(v[VI::intehead::NZWELZ],  1);
}

BOOST_AUTO_TEST_CASE(NConn_Parameters)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
        .params_NCON(31, 41, 59);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NICONZ], 31);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSCONZ], 41);
    BOOST_CHECK_EQUAL(v[VI::intehead::NXCONZ], 59);
}

BOOST_AUTO_TEST_CASE(GroupSize_Parameters)
{
    // https://oeis.org/A001620
    const auto ih = Opm::RestartIO::InteHEAD{}
        .params_GRPZ({
            { 577, 215, 664, 901 }
        });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NIGRPZ], 577);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSGRPZ], 215);
    BOOST_CHECK_EQUAL(v[VI::intehead::NXGRPZ], 664);
    BOOST_CHECK_EQUAL(v[VI::intehead::NZGRPZ], 901);
}

BOOST_AUTO_TEST_CASE(Analytic_Aquifer_Parameters)
{
    // https://oeis.org/A001622
    const auto aqudims = Opm::RestartIO::InteHEAD::AquiferDims {
        1,                      // numAquifers
        61,                     // maxNumAquifers
        803,                    // maxNumAquiferConn
        3988,                   // maxNumActiveAquiferConn
        74989,                  // maxAquiferID
        484820,                 // numNumericAquiferRecords
        45868,                  // numIntAquiferElem
        3436,                   // numRealAquiferElem
        563,                    // numDoubAquiferElem
        81,                     // numNumericAquiferIntElem
        17,                     // numNumericAquiferDoubleElem
        720,                    // numIntConnElem
        30,                     // numRealConnElem
        9,                      // numDoubConnElem
    };

    const auto ih = Opm::RestartIO::InteHEAD{}
        .aquiferDimensions(aqudims);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NAQUIF], 1);
    BOOST_CHECK_EQUAL(v[VI::intehead::NCAMAX], 803);
    BOOST_CHECK_EQUAL(v[VI::intehead::NIAAQZ], 45868);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSAAQZ], 3436);
    BOOST_CHECK_EQUAL(v[VI::intehead::NXAAQZ], 563);
    BOOST_CHECK_EQUAL(v[VI::intehead::NICAQZ], 720);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSCAQZ], 30);
    BOOST_CHECK_EQUAL(v[VI::intehead::NACAQZ], 9);

    BOOST_CHECK_EQUAL(v[VI::intehead::MAX_ACT_ANLYTIC_AQUCONN], 3988);
    BOOST_CHECK_EQUAL(v[VI::intehead::MAX_AN_AQUIFER_ID], 74989);
    BOOST_CHECK_EQUAL(v[VI::intehead::AQU_UNKNOWN_1], 1);
    BOOST_CHECK_EQUAL(v[VI::intehead::MAX_ANALYTIC_AQUIFERS], 61);

    BOOST_CHECK_EQUAL(v[VI::intehead::NIIAQN], 81);
    BOOST_CHECK_EQUAL(v[VI::intehead::NIRAQN], 17);
    BOOST_CHECK_EQUAL(v[VI::intehead::NUM_AQUNUM_RECORDS], 484820);
}

BOOST_AUTO_TEST_CASE(Time_and_report_step)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
        .stepParam(12, 2);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NUM_SOLVER_STEPS], 12); // TSTEP (1st argument to stepParam)
    BOOST_CHECK_EQUAL(v[VI::intehead::REPORT_STEP],       2); // REP_STEP (2nd argument to stepParam)
}

BOOST_AUTO_TEST_CASE(Tuning_param)
{
    const auto newtmx	= 17;
    const auto newtmn	=  5;
    const auto litmax	= 102;
    const auto litmin	= 20;
    const auto mxwsit	= 8;
    const auto mxwpit	= 6;
    const auto wseg_max_restart   = 49;

    const auto ih = Opm::RestartIO::InteHEAD{}
        .tuningParam({
            newtmx, newtmn, litmax, litmin, mxwsit, mxwpit, wseg_max_restart
        });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NEWTMX], newtmx);
    BOOST_CHECK_EQUAL(v[VI::intehead::NEWTMN], newtmn);
    BOOST_CHECK_EQUAL(v[VI::intehead::LITMAX], litmax);
    BOOST_CHECK_EQUAL(v[VI::intehead::LITMIN], litmin);
    BOOST_CHECK_EQUAL(v[VI::intehead::MXWSIT], mxwsit);
    BOOST_CHECK_EQUAL(v[VI::intehead::MXWPIT], mxwpit);
    BOOST_CHECK_EQUAL(v[VI::intehead::WSEGITR_IT2], wseg_max_restart);
}

BOOST_AUTO_TEST_CASE(Various_Parameters)
{
    const auto ih = Opm::RestartIO::InteHEAD{}
        .variousParam(2015, 100);

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::VERSION], 2015); // VERSION
    BOOST_CHECK_EQUAL(v[VI::intehead::IPROG], 100);    // IPROG
    BOOST_CHECK_EQUAL(v[ 76],   5); // IH_076
    BOOST_CHECK_EQUAL(v[101],   1); // IH_101
}

BOOST_AUTO_TEST_CASE(wellSegDimensions)
{
    const auto nsegwl = 3;
    const auto nswlmx = 4;
    const auto nsegmx = 5;
    const auto nlbrmx = 6;
    const auto nisegz = 7;
    const auto nrsegz = 8;
    const auto nilbrz = 9;

    const auto ih = Opm::RestartIO::InteHEAD{}
        .wellSegDimensions({
            nsegwl, nswlmx, nsegmx, nlbrmx, nisegz, nrsegz, nilbrz
        });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NSEGWL], nsegwl);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSWLMX], nswlmx);
    BOOST_CHECK_EQUAL(v[VI::intehead::NSEGMX], nsegmx);
    BOOST_CHECK_EQUAL(v[VI::intehead::NLBRMX], nlbrmx);
    BOOST_CHECK_EQUAL(v[VI::intehead::NISEGZ], nisegz);
    BOOST_CHECK_EQUAL(v[VI::intehead::NRSEGZ], nrsegz);
    BOOST_CHECK_EQUAL(v[VI::intehead::NILBRZ], nilbrz);
}

BOOST_AUTO_TEST_CASE(regionDimensions)
{
    const auto ntfip  = 12;
    const auto nmfipr = 22;
    const auto nrfreg = 5;
    const auto ntfreg = 6;
    const auto nplmix = 7;

    const auto ih = Opm::RestartIO::InteHEAD{}
        .regionDimensions({
            ntfip, nmfipr, nrfreg, ntfreg, nplmix
        });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NTFIP], ntfip);
    BOOST_CHECK_EQUAL(v[VI::intehead::NMFIPR], nmfipr);
}

BOOST_AUTO_TEST_CASE(rockOptions)
{
    const auto ttyp  = 5;

    const auto ih = Opm::RestartIO::InteHEAD{}
        .rockOpts({
            ttyp
        });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::ROCKOPTS_TABTYP], ttyp);
}

BOOST_AUTO_TEST_CASE(ngroups)
{
    const auto ngroup  = 8;

    const auto ih = Opm::RestartIO::InteHEAD{}
        .ngroups({ ngroup });

    const auto& v = ih.data();

    BOOST_CHECK_EQUAL(v[VI::intehead::NGRP], ngroup);
}

static Opm::Schedule make_schedule(const std::string& deck_string) {
    const auto& deck = Opm::Parser{}.parseString(deck_string);
    auto python = std::make_shared<Opm::Python>();
    Opm::EclipseGrid grid(10,10,10);
    Opm::TableManager table ( deck );
    Opm::FieldPropsManager fp( deck, Opm::Phases{true, true, true}, grid, table);
    Opm::Runspec runspec (deck);
    return Opm::Schedule(deck, grid , fp, runspec, python);
}

BOOST_AUTO_TEST_CASE(SimulationDate)
{
    const auto input = std::string { R"(
RUNSPEC

START
  1 JAN 2000
/

SCHEDULE

DATES
  1 'JAN' 2001 /
/

TSTEP
--Advance the simulater for TEN years:
  10*365.0D0 /
)"  };

    auto sched = make_schedule(input);

    const auto start = sched.getStartTime();
    const auto elapsed = elapsedTime(sched);

    auto checkDate = [start, &elapsed]
        (const std::vector<double>::size_type i,
         const std::array<int, 3>&            expectYMD) -> void
    {
        using ::Opm::RestartIO::getSimulationTimePoint;

        expectDate(getSimulationTimePoint(start, elapsed[i]),
                   expectYMD[0], expectYMD[1], expectYMD[2]);
    };

    // START
    checkDate(0, { 2000, 1, 1 });  // Start   == 2000-01-01

    // DATES (2000 being leap year is immaterial)
    checkDate(1, { 2001, 1, 1 });  // RStep 1 == 2000-01-01 -> 2001-01-01

    // TSTEP
    checkDate(2, { 2002, 1, 1 });  // RStep 2 == 2001-01-01 -> 2002-01-01
    checkDate(3, { 2003, 1, 1 });  // RStep 3 == 2002-01-01 -> 2003-01-01
    checkDate(4, { 2004, 1, 1 });  // RStep 4 == 2003-01-01 -> 2004-01-01

    // Leap year: 2004
    checkDate(5, { 2004, 12, 31 }); // RStep 5 == 2004-01-01 -> 2004-12-31
    checkDate(6, { 2005, 12, 31 }); // RStep 6 == 2004-12-31 -> 2005-12-31
    checkDate(7, { 2006, 12, 31 }); // RStep 7 == 2005-12-31 -> 2006-12-31
    checkDate(8, { 2007, 12, 31 }); // RStep 8 == 2006-12-31 -> 2007-12-31

    // Leap year: 2008
    checkDate( 9, { 2008, 12, 30 }); // RStep  9 == 2007-12-31 -> 2008-12-30
    checkDate(10, { 2009, 12, 30 }); // RStep 10 == 2008-12-30 -> 2009-12-30
    checkDate(11, { 2010, 12, 30 }); // RStep 11 == 2009-12-30 -> 2010-12-30
}

BOOST_AUTO_TEST_SUITE_END() // Member_Functions

// =====================================================================

BOOST_AUTO_TEST_SUITE(Transfer_Protocol)

BOOST_AUTO_TEST_CASE(TestHeader) {
    using Ph = Opm::RestartIO::InteHEAD::Phases;

    const auto nx = 10;
    const auto ny = 11;
    const auto nz = 12;
    const auto nactive = 1345;
    const auto numWells        = 17;
    const auto maxPerf         = 29;
    const auto maxWellsInGroup  =  3;
    const auto maxGroupInField = 14;
    const auto maxWellsInField = 25;
    const auto year = 2010;
    const auto month = 1;
    const auto mday = 27;
    const auto hour = 11;
    const auto minute = 22;
    const auto seconds = 30;
    const auto mseconds = 1234;
    const auto total_mseconds = seconds * 1000000 + mseconds;
    const auto phase_sum = 7;
    const auto niwelz = 10;
    const auto nswelz = 11;
    const auto nxwelz = 12;
    const auto nzwelz = 13;
    const auto niconz = 77;
    const auto nsconz = 88;
    const auto nxconz = 99;
    const auto nigrpz = 21;
    const auto nsgrpz = 22;
    const auto nxgrpz = 23;
    const auto nzgrpz = 24;
    const auto ncamax = 1;
    const auto niaaqz = 11;
    const auto nsaaqz = 111;
    const auto nxaaqz = 11111;
    const auto nicaqz = 111111;
    const auto nscaqz = 1111111;
    const auto nacaqz = 11111111;
    const auto niiaqn = 22222222;
    const auto niraqn = 2222222;
    const auto numaqn = 222222;
    const auto tstep  = 78;
    const auto report_step = 12;
    const auto newtmx	= 17;
    const auto newtmn	=  5;
    const auto litmax	= 102;
    const auto litmin	= 20;
    const auto mxwsit	= 8;
    const auto mxwpit	= 6;
    const auto version = 2015;
    const auto iprog = 100;
    const auto nsegwl = 3;
    const auto nswlmx = 4;
    const auto nsegmx = 5;
    const auto nlbrmx = 6;
    const auto nisegz = 7;
    const auto nrsegz = 8;
    const auto nilbrz = 9;
    const auto ntfip  = 12;
    const auto nmfipr = 22;
    const auto ngroup  = 8;

    const auto aqudims = Opm::RestartIO::InteHEAD::AquiferDims {
        1, 61, ncamax, 3988, 74989,
        numaqn, niaaqz, nsaaqz, nxaaqz,
        niiaqn, niraqn,
        nicaqz, nscaqz, nacaqz
    };

    auto unit_system = Opm::UnitSystem::newMETRIC();
    auto ih = Opm::RestartIO::InteHEAD{}
         .dimensions(nx, ny, nz)
         .numActive(nactive)
         .unitConventions(unit_system)
         .wellTableDimensions({ numWells, maxPerf, maxWellsInGroup,
                                maxGroupInField, maxWellsInField, 0, 0 })
         .calendarDate({year, month, mday, hour, minute, seconds, mseconds})
         .activePhases(Ph{1,1,1})
         .params_NWELZ(niwelz, nswelz, nxwelz, nzwelz)
         .params_NCON(niconz, nsconz, nxconz)
         .params_GRPZ({nigrpz, nsgrpz, nxgrpz, nzgrpz})
         .aquiferDimensions(aqudims)
         .stepParam(tstep, report_step)
         .tuningParam({newtmx, newtmn, litmax, litmin, mxwsit, mxwpit, 0})
         .variousParam(version, iprog)
         .wellSegDimensions({nsegwl, nswlmx, nsegmx, nlbrmx, nisegz, nrsegz, nilbrz})
         .regionDimensions({ntfip, nmfipr, 0,0,0})
         .ngroups({ngroup});

    Opm::Runspec runspec;
    Opm::RestartIO::RstHeader header(runspec, unit_system, ih.data(), std::vector<bool>(100), std::vector<double>(1000));
    BOOST_CHECK_EQUAL(header.nx, nx);
    BOOST_CHECK_EQUAL(header.ny, ny);
    BOOST_CHECK_EQUAL(header.nactive, nactive);
    BOOST_CHECK_EQUAL(header.num_wells, numWells);
    BOOST_CHECK_EQUAL(header.ncwmax, maxPerf);
    BOOST_CHECK_EQUAL(header.max_wells_in_group, std::max(maxWellsInGroup , maxGroupInField));
    BOOST_CHECK_EQUAL(header.max_groups_in_field, maxGroupInField + 1);
    BOOST_CHECK_EQUAL(header.max_wells_in_field, maxWellsInField);
    BOOST_CHECK_EQUAL(header.year, year);
    BOOST_CHECK_EQUAL(header.month, month);
    BOOST_CHECK_EQUAL(header.mday, mday);
    BOOST_CHECK_EQUAL(header.hour, hour);
    BOOST_CHECK_EQUAL(header.minute, minute);
    BOOST_CHECK_EQUAL(header.microsecond, total_mseconds);
    BOOST_CHECK_EQUAL(header.phase_sum , phase_sum);
    BOOST_CHECK_EQUAL(header.niwelz, niwelz);
    BOOST_CHECK_EQUAL(header.nswelz, nswelz);
    BOOST_CHECK_EQUAL(header.nxwelz, nxwelz);
    BOOST_CHECK_EQUAL(header.nzwelz, nzwelz);
    BOOST_CHECK_EQUAL(header.niconz, niconz);
    BOOST_CHECK_EQUAL(header.nsconz, nsconz);
    BOOST_CHECK_EQUAL(header.nxconz, nxconz);
    BOOST_CHECK_EQUAL(header.nigrpz, nigrpz);
    BOOST_CHECK_EQUAL(header.nsgrpz, nsgrpz);
    BOOST_CHECK_EQUAL(header.nxgrpz, nxgrpz);
    BOOST_CHECK_EQUAL(header.nzgrpz, nzgrpz);
    BOOST_CHECK_EQUAL(header.ncamax, ncamax);
    BOOST_CHECK_EQUAL(header.niaaqz, niaaqz);
    BOOST_CHECK_EQUAL(header.nsaaqz, nsaaqz);
    BOOST_CHECK_EQUAL(header.nxaaqz, nxaaqz);
    BOOST_CHECK_EQUAL(header.nicaqz, nicaqz);
    BOOST_CHECK_EQUAL(header.nscaqz, nscaqz);
    BOOST_CHECK_EQUAL(header.nacaqz, nacaqz);
    BOOST_CHECK_EQUAL(header.tstep, tstep);
    BOOST_CHECK_EQUAL(header.report_step, report_step);
    BOOST_CHECK_EQUAL(header.newtmx, newtmx);
    BOOST_CHECK_EQUAL(header.newtmn, newtmn);
    BOOST_CHECK_EQUAL(header.litmax, litmax);
    BOOST_CHECK_EQUAL(header.litmin, litmin);
    BOOST_CHECK_EQUAL(header.mxwsit, mxwsit);
    BOOST_CHECK_EQUAL(header.mxwpit, mxwpit);
    BOOST_CHECK_EQUAL(header.version, version);
    BOOST_CHECK_EQUAL(header.iprog, iprog);
    BOOST_CHECK_EQUAL(header.nsegwl, nsegwl);
    BOOST_CHECK_EQUAL(header.nswlmx, nswlmx);
    BOOST_CHECK_EQUAL(header.nsegmx, nsegmx);
    BOOST_CHECK_EQUAL(header.nlbrmx, nlbrmx);
    BOOST_CHECK_EQUAL(header.nisegz, nisegz);
    BOOST_CHECK_EQUAL(header.nilbrz, nilbrz);
    BOOST_CHECK_EQUAL(header.ntfip, ntfip);
    BOOST_CHECK_EQUAL(header.nmfipr, nmfipr);
    BOOST_CHECK_EQUAL(header.ngroup, ngroup);
}


BOOST_AUTO_TEST_CASE(Netbalan)
{
    const auto simCase = SimulationCase{first_sim("5_NETWORK_MODEL5_STDW_NETBAL_PACK.DATA")};

    Opm::EclipseState es    = simCase.es;
    Opm::EclipseGrid  grid   = simCase.grid;

    Opm::Schedule     sched = simCase.sched;
    const auto& start_time = sched.getStartTime();
    double simTime = start_time + 2.E09;

    const std::size_t report_step = 1;
    const std::size_t lookup_step = report_step - 1;

    const auto ih = Opm::RestartIO::Helpers::
            createInteHead(es, grid, sched, simTime,
                           report_step, // Should really be number of timesteps
                           report_step, lookup_step);


    const auto& v = ih.data();

    namespace VI = Opm::RestartIO::Helpers::VectorItems;

    BOOST_CHECK_EQUAL(v[VI::intehead::NETBALAN_3], 13);
    BOOST_CHECK_EQUAL(v[VI::intehead::NETBALAN_5], 14);

}



BOOST_AUTO_TEST_SUITE_END() // Transfer_Protocol
