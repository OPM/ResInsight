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
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/output/eclipse/VectorItems/msw.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/io/eclipse/rst/segment.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include <cstddef>

namespace {

namespace VI = ::Opm::RestartIO::Helpers::VectorItems;

Opm::Deck first_sim(std::string fname) {
    return Opm::Parser {} .parseFile(fname);
}

Opm::SummaryState sim_state()
{
     auto state = Opm::SummaryState{Opm::TimeService::now()};

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

    state.update_well_var("WINJ", "WBHP",  234.);
    return state;
}
Opm::data::Wells wr()
{
    using o = ::Opm::data::Rates::opt;

    auto xw = ::Opm::data::Wells {};

    {
        xw["PROD"].rates
        .set(o::wat, 1.0)
        .set(o::oil, 2.0)
        .set(o::gas, 3.0);
        xw["PROD"].bhp = 213.0;
        double qo = 5.;
        double qw = 4.;
        double qg = 50.;
        int firstConnectedCell = 90; // zero-based linear index of (1,5,2)
        for (int i = 0; i < 5; i++) {
            xw["PROD"].connections.emplace_back();
            auto& c = xw["PROD"].connections.back();

            c.rates.set(o::wat, qw*(float(i)+1.))
            .set(o::oil, qo*(float(i)+1.))
            .set(o::gas, qg*(float(i)+1.));

            c.index = firstConnectedCell + i;
        }
        auto seg = Opm::data::Segment {};
        for (std::size_t i = 1; i < 5; i++) {
            xw["PROD"].segments.insert(std::pair<std::size_t,Opm::data::Segment>(i,seg));
        }
        xw["WINJ"].bhp = 234.0;

        xw["WINJ"].rates.set(o::wat, 5.0);
        xw["WINJ"].rates.set(o::oil, 0.0);
        xw["WINJ"].rates.set(o::gas, 0.0);
        qw = 7.;
        firstConnectedCell = 409; // zero-based linear index of (10,1,9)
        for (int i = 0; i < 5; i++) {
            xw["WINJ"].connections.emplace_back();
            auto& c = xw["WINJ"].connections.back();

            c.rates.set(o::wat, qw*(float(i)+1.))
            .set(o::oil, 0.)
            .set(o::gas, 0.);

            c.index = firstConnectedCell - i;
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
    const auto simCase = SimulationCase {first_sim("TEST_AGGREGATE_MSW.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec   = es.runspec();
    Opm::SummaryState st = sim_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;

    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t {1};

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
                    createInteHead(es, grid, sched, secs_elapsed,
                                   rptStep, rptStep+1, rptStep);

    const auto amswd = Opm::RestartIO::Helpers::AggregateMSWData { ih };
    const auto nswlmx = VI::intehead::NSWLMX;
    const auto nsegmx = VI::intehead::NSEGMX;
    const auto nisegz = VI::intehead::NISEGZ;
    const auto nrsegz = VI::intehead::NRSEGZ;
    const auto nlbrmx = VI::intehead::NLBRMX;
    const auto nilbrz = VI::intehead::NILBRZ;
    BOOST_CHECK_EQUAL(static_cast<int>(amswd.getISeg().size()), ih[nswlmx] * ih[nsegmx] * ih[nisegz]);
    BOOST_CHECK_EQUAL(static_cast<int>(amswd.getRSeg().size()), ih[nswlmx] * ih[nsegmx] * ih[nrsegz]);
    BOOST_CHECK_EQUAL(static_cast<int>(amswd.getILBs().size()), ih[nswlmx] * ih[nlbrmx]);
    BOOST_CHECK_EQUAL(static_cast<int>(amswd.getILBr().size()), ih[nswlmx] * ih[nlbrmx] * ih[nilbrz]);
}


BOOST_AUTO_TEST_CASE (Declared_MSW_Data)
{

    const auto simCase = SimulationCase {first_sim("TEST_AGGREGATE_MSW.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec   = es.runspec();
    Opm::SummaryState smry = sim_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    const auto& units    = es.getUnits();


    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t {1};

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
                    createInteHead(es, grid, sched, secs_elapsed,
                                   rptStep, rptStep+1, rptStep);

    //BOOST_CHECK_EQUAL(ih.nwells, MockIH::Sz{2});

    const Opm::data::Wells wrc = wr();
    auto amswd = Opm::RestartIO::Helpers::AggregateMSWData {ih};
    amswd.captureDeclaredMSWData(simCase.sched,
                                 rptStep,
                                 units,
                                 ih,
                                 grid,
                                 smry,
                                 wrc
                                );

    // ISEG (PROD)
    {
        auto start = 2*ih[VI::intehead::NISEGZ];

        const auto& iSeg = amswd.getISeg();
        BOOST_CHECK_EQUAL(iSeg[start + 0] , 10); // PROD-segment 3, ordered segment
        BOOST_CHECK_EQUAL(iSeg[start + 1] ,  2); // PROD-segment 3, outlet segment
        BOOST_CHECK_EQUAL(iSeg[start + 2] ,  4); // PROD-segment 3, inflow segment current branch
        BOOST_CHECK_EQUAL(iSeg[start + 3] ,  2); // PROD-segment 3, branch number
        BOOST_CHECK_EQUAL(iSeg[start + 4] ,  0); // PROD-segment 3, number of inflow branches
        BOOST_CHECK_EQUAL(iSeg[start + 5] ,  0); // PROD-segment 3, Sum number of inflow branches from first segment to current segment
        BOOST_CHECK_EQUAL(iSeg[start + 6] ,  1); // PROD-segment 3, number of connections in segment
        BOOST_CHECK_EQUAL(iSeg[start + 7] ,  1); // PROD-segment 3, sum of connections with lower segmeent number than current segment
        BOOST_CHECK_EQUAL(iSeg[start + 8] ,  8); // PROD-segment 3, ordered segment

        start = 9*ih[VI::intehead::NISEGZ];

        BOOST_CHECK_EQUAL(iSeg[start + 0] ,  1); // PROD-segment 10, ordered segment
        BOOST_CHECK_EQUAL(iSeg[start + 1] ,  6); // PROD-segment 10, outlet segment
        BOOST_CHECK_EQUAL(iSeg[start + 2] ,  0); // PROD-segment 10, inflow segment current branch
        BOOST_CHECK_EQUAL(iSeg[start + 3] ,  5); // PROD-segment 10, branch number
        BOOST_CHECK_EQUAL(iSeg[start + 4] ,  0); // PROD-segment 10, number of inflow branches
        BOOST_CHECK_EQUAL(iSeg[start + 5] ,  0); // PROD-segment 10, Sum number of inflow branches from first segment to current segment
        BOOST_CHECK_EQUAL(iSeg[start + 6] ,  0); // PROD-segment 10, number of connections in segment
        BOOST_CHECK_EQUAL(iSeg[start + 7] ,  0); // PROD-segment 10, sum of connections with lower segmeent number than current segment
        BOOST_CHECK_EQUAL(iSeg[start + 8] ,  3); // PROD-segment 10, ordered segment

    }

    // ISEG (WINJ)
    {
        auto start =  ih[VI::intehead::NISEGZ]*ih[VI::intehead::NSEGMX] + 13*ih[VI::intehead::NISEGZ];
        const auto& iSeg = amswd.getISeg();
        BOOST_CHECK_EQUAL(iSeg[start + 0] ,  5); // WINJ-segment 14, ordered segment
        BOOST_CHECK_EQUAL(iSeg[start + 1] , 13); // WINJ-segment 14, outlet segment
        BOOST_CHECK_EQUAL(iSeg[start + 2] , 15); // WINJ-segment 14, inflow segment current branch
        BOOST_CHECK_EQUAL(iSeg[start + 3] ,  2); // WINJ-segment 14, branch number
        BOOST_CHECK_EQUAL(iSeg[start + 4] ,  0); // WINJ-segment 14, number of inflow branches
        BOOST_CHECK_EQUAL(iSeg[start + 5] ,  0); // WINJ-segment 14, Sum number of inflow branches from first segment to current segment
        BOOST_CHECK_EQUAL(iSeg[start + 6] ,  1); // WINJ-segment 14, number of connections in segment
        BOOST_CHECK_EQUAL(iSeg[start + 7] ,  1); // WINJ-segment 14, sum of connections with lower segmeent number than current segment
        BOOST_CHECK_EQUAL(iSeg[start + 8] ,  5); // WINJ-segment 14, ordered segment

        start = ih[VI::intehead::NISEGZ]*ih[VI::intehead::NSEGMX] + 16*ih[VI::intehead::NISEGZ];
        BOOST_CHECK_EQUAL(iSeg[start + 0] ,  2); // WINJ-segment 17, ordered segment
        BOOST_CHECK_EQUAL(iSeg[start + 1] , 16); // WINJ-segment 17, outlet segment
        BOOST_CHECK_EQUAL(iSeg[start + 2] , 18); // WINJ-segment 17, inflow segment current branch
        BOOST_CHECK_EQUAL(iSeg[start + 3] ,  2); // WINJ-segment 17, branch number
        BOOST_CHECK_EQUAL(iSeg[start + 4] ,  0); // WINJ-segment 17, number of inflow branches
        BOOST_CHECK_EQUAL(iSeg[start + 5] ,  0); // WINJ-segment 17, Sum number of inflow branches from first segment to current segment
        BOOST_CHECK_EQUAL(iSeg[start + 6] ,  1); // WINJ-segment 17, number of connections in segment
        BOOST_CHECK_EQUAL(iSeg[start + 7] ,  4); // WINJ-segment 17, sum of connections with lower segmeent number than current segment
        BOOST_CHECK_EQUAL(iSeg[start + 8] ,  2); // WINJ-segment 17, ordered segment

    }

    // RSEG (PROD)  + (WINJ)
    {
        // well no 1 - PROD
        const std::string wname = "PROD";
        int segNo = 1;
        // 'stringSegNum' is one-based (1 .. #segments inclusive)
        std::string stringSegNo = std::to_string(segNo);

        const auto  i0 = (segNo-1)*ih[VI::intehead::NRSEGZ];
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

        const auto  i0 = ih[VI::intehead::NRSEGZ]*ih[VI::intehead::NSEGMX] + (segNo-1)*ih[VI::intehead::NRSEGZ];
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
        auto start =  0*ih[VI::intehead::NILBRZ];

        const auto& iLBr = amswd.getILBr();
        //PROD
        BOOST_CHECK_EQUAL(iLBr[start + 0] ,  0); // PROD-branch   1, outlet segment
        BOOST_CHECK_EQUAL(iLBr[start + 1] ,  2); // PROD-branch   1, No of segments in branch
        BOOST_CHECK_EQUAL(iLBr[start + 2] ,  1); // PROD-branch   1, first segment
        BOOST_CHECK_EQUAL(iLBr[start + 3] ,  2); // PROD-branch   1, last segment
        BOOST_CHECK_EQUAL(iLBr[start + 4] ,  0); // PROD-branch   1, branch no - 1
        //PROD
        start =  1*ih[VI::intehead::NILBRZ];
        BOOST_CHECK_EQUAL(iLBr[start + 0] ,  2); // PROD-branch   2, outlet segment
        BOOST_CHECK_EQUAL(iLBr[start + 1] ,  5); // PROD-branch   2, No of segments in branch
        BOOST_CHECK_EQUAL(iLBr[start + 2] ,  3); // PROD-branch   2, first segment
        BOOST_CHECK_EQUAL(iLBr[start + 3] ,  7); // PROD-branch   2, last segment
        BOOST_CHECK_EQUAL(iLBr[start + 4] ,  1); // PROD-branch   2, branch no - 1


        start = ih[VI::intehead::NILBRZ]*ih[VI::intehead::NLBRMX] + 0*ih[VI::intehead::NILBRZ];
        //WINJ
        BOOST_CHECK_EQUAL(iLBr[start + 0] ,  0); // WINJ-branch   1, outlet segment
        BOOST_CHECK_EQUAL(iLBr[start + 1] , 13); // WINJ-branch   1, No of segments in branch
        BOOST_CHECK_EQUAL(iLBr[start + 2] ,  1); // WINJ-branch   1, first segment
        BOOST_CHECK_EQUAL(iLBr[start + 3] , 13); // WINJ-branch   1, last segment
        BOOST_CHECK_EQUAL(iLBr[start + 4] ,  0); // WINJ-branch   1, branch no - 1

        start = ih[VI::intehead::NILBRZ]*ih[VI::intehead::NLBRMX] + 1*ih[VI::intehead::NILBRZ];
        //WINJ
        BOOST_CHECK_EQUAL(iLBr[start + 0] , 13); // WINJ-branch   2, outlet segment
        BOOST_CHECK_EQUAL(iLBr[start + 1] ,  5); // WINJ-branch   2, No of segments in branch
        BOOST_CHECK_EQUAL(iLBr[start + 2] , 14); // WINJ-branch   2, first segment
        BOOST_CHECK_EQUAL(iLBr[start + 3] , 18); // WINJ-branch   2, last segment
        BOOST_CHECK_EQUAL(iLBr[start + 4] ,  1); // WINJ-branch   2, branch no - 1


    }

    // ILBS
    {
        auto start =  0*ih[VI::intehead::NLBRMX];

        const auto& iLBs = amswd.getILBs();
        //PROD
        BOOST_CHECK_EQUAL(iLBs[start + 0] ,   3); // PROD-branch   2, first segment in branch

        start = ih[VI::intehead::NLBRMX] + 0*ih[VI::intehead::NLBRMX];
        //WINJ
        BOOST_CHECK_EQUAL(iLBs[start + 0] ,  14); // WINJ-branch   2, first segment in branch

    }
}


BOOST_AUTO_TEST_CASE(MSW_AICD) {
    const auto simCase = SimulationCase {first_sim("TEST_AGGREGATE_MSW.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec   = es.runspec();
    Opm::SummaryState smry = sim_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    const auto& units    = es.getUnits();


    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t {1};

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
                    createInteHead(es, grid, sched, secs_elapsed,
                                   rptStep, rptStep+1, rptStep);


    const Opm::data::Wells wrc = wr();
    auto amswd = Opm::RestartIO::Helpers::AggregateMSWData {ih};
    amswd.captureDeclaredMSWData(simCase.sched,
                                 rptStep,
                                 units,
                                 ih,
                                 grid,
                                 smry,
                                 wrc
                                );

// ISEG (PROD)
    {
        const auto& iSeg = amswd.getISeg();
        auto start = 7*ih[VI::intehead::NISEGZ];
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::SegmentType],     -8); // PROD-segment 8,
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::ICDScalingMode],   1); // PROD-segment 8,
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::ICDOpenShutFlag],  0); // PROD-segment 8,

        start = 8*ih[VI::intehead::NISEGZ];
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::SegmentType],     -8); // PROD-segment 9,
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::ICDScalingMode],   1); // PROD-segment 9,
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::ICDOpenShutFlag],  0); // PROD-segment 9,

        start = 9*ih[VI::intehead::NISEGZ];
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::SegmentType],     -8); // PROD-segment 10,
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::ICDScalingMode],   0); // PROD-segment 10,
        BOOST_CHECK_EQUAL(iSeg[start + VI::ISeg::index::ICDOpenShutFlag],  0); // PROD-segment 10,



    }

        // RSEG (PROD)
    {
        // well no 1 - PROD
        const auto& rseg = amswd.getRSeg();

        int segNo = 8;
        auto  i0 = (segNo-1)*ih[VI::intehead::NRSEGZ];
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::DeviceBaseStrength], 3.260E-05  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::ScalingFactor], 0.06391  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::CalibrFluidDensity], 63.678 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::CalibrFluidViscosity], 0.48 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::CriticalWaterFraction], 0.5 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::TransitionRegWidth], 0.05 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::MaxEmulsionRatio], 5. , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::FlowRateExponent], 2.1 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::ViscFuncExponent], 1.2  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::MaxValidFlowRate], -2e+20  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::ICDLength], 0.06391 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionOilDensityExponent],  1. , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionWaterDensityExponent], 1.  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionGasDensityExponent], 1. , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionOilViscosityExponent], 1. , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionWaterViscosityExponent], 1. , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionGasViscosityExponent], 1.  , 1.0e-10);

        segNo = 10;
        i0 = (segNo-1)*ih[VI::intehead::NRSEGZ];
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::DeviceBaseStrength], 3.260E-05  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::ScalingFactor], 0.000876 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::CalibrFluidDensity], 63.678 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::CalibrFluidViscosity], 0.48 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::CriticalWaterFraction], 0.53 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::TransitionRegWidth], 0.048 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::MaxEmulsionRatio], 4.89 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::FlowRateExponent], 2.1 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::ViscFuncExponent], 1.2  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::MaxValidFlowRate], 9.876e+06  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::ICDLength], 0.0876 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionOilDensityExponent],  0.92 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionWaterDensityExponent], 0.89  , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionGasDensityExponent], 0.91 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionOilViscosityExponent], 1.01 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionWaterViscosityExponent], 1.02 , 1.0e-10);
        BOOST_CHECK_CLOSE(rseg[i0 + VI::RSeg::index::flowFractionGasViscosityExponent], 1.03  , 1.0e-10);

    }

}


BOOST_AUTO_TEST_CASE(MSW_RST) {
    const auto simCase = SimulationCase {first_sim("TEST_AGGREGATE_MSW.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec   = es.runspec();
    Opm::SummaryState smry = sim_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    const auto& units    = es.getUnits();


    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t {1};

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
                    createInteHead(es, grid, sched, secs_elapsed,
                                   rptStep, rptStep+1, rptStep);


    const Opm::data::Wells wrc = wr();
    auto amswd = Opm::RestartIO::Helpers::AggregateMSWData {ih};
    amswd.captureDeclaredMSWData(simCase.sched,
                                 rptStep,
                                 units,
                                 ih,
                                 grid,
                                 smry,
                                 wrc
                                );

    const auto& iseg = amswd.getISeg();
    const auto& rseg = amswd.getRSeg();
    auto segment = Opm::RestartIO::RstSegment(simCase.es.getUnits(), 1, iseg.data(), rseg.data());
}



BOOST_AUTO_TEST_SUITE_END()
