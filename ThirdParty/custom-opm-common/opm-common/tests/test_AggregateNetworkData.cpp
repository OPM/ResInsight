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
#include <opm/output/eclipse/AggregateNetworkData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>

#include <opm/io/eclipse/OutputStream.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include <cstddef>

namespace {


    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);
    }

    Opm::SummaryState sum_state()
    {
        auto state = Opm::SummaryState{Opm::TimeService::now()};

        state.update_well_var("P1", "WOPR", 3342.673828);
        state.update_well_var("P1", "WWPR", 0.000005);
        state.update_well_var("P1", "WGPR", 334267.375);
        state.update_well_var("P1", "WGLIR", 111000.);

        state.update_well_var("P2", "WOPR", 3882.443848);
        state.update_well_var("P2", "WWPR", 0.000002);
        state.update_well_var("P2", "WGPR", 672736.9375);
        state.update_well_var("P2", "WGLIR", 99666.);

        state.update_well_var("P3", "WOPR", 3000.000000);
        state.update_well_var("P3", "WWPR", 0.000002);
        state.update_well_var("P3", "WGPR", 529658.8125);
        state.update_well_var("P3", "WGLIR", 55000.);

        state.update_group_var("B1", "GPR", 81.6848);
        state.update_group_var("N1", "GPR", 72.);
        state.update_group_var("N2", "GPR", 69.);
        state.update_group_var("PLAT-A", "GPR", 67.);
        state.update_group_var("B2", "GPR", 79.0666);
        state.update_group_var("M1", "GPR", 72.);

        return state;
    }

    std::string pad8(const std::string& s) {
        return s + std::string( 8 - s.size(), ' ');
    }
}

struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   ( deck )
        , grid { deck }
        , python( std::make_shared<Opm::Python>() )
        , sched (deck, es, python )
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;
};

// =====================================================================

BOOST_AUTO_TEST_SUITE(Aggregate_Network)


// test dimensions of multisegment data
BOOST_AUTO_TEST_CASE (Constructor)
{
    namespace VI = ::Opm::RestartIO::Helpers::VectorItems;
    const auto simCase = SimulationCase{first_sim("TEST_NETWORK_ALL.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec   = es.runspec();
    Opm::SummaryState st = sum_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    //const auto& ioConfig = es.getIOConfig();
    const auto& units    = es.getUnits();


    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t{1};

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
        createInteHead(es, grid, sched, secs_elapsed,
                       rptStep, rptStep+1, rptStep);

    auto networkData = Opm::RestartIO::Helpers::AggregateNetworkData(ih);
    networkData.captureDeclaredNetworkData(es, sched, units, rptStep, st, ih);

    BOOST_CHECK_EQUAL(static_cast<int>(networkData.getINode().size()), ih[VI::NINODE] * ih[VI::NODMAX]);
    BOOST_CHECK_EQUAL(static_cast<int>(networkData.getIBran().size()), ih[VI::NIBRAN] * ih[VI::NBRMAX]);
    BOOST_CHECK_EQUAL(static_cast<int>(networkData.getINobr().size()), ih[VI::NINOBR]);
    BOOST_CHECK_EQUAL(static_cast<int>(networkData.getZNode().size()), ih[VI::NZNODE] * ih[VI::NODMAX]);
    BOOST_CHECK_EQUAL(static_cast<int>(networkData.getRNode().size()), ih[VI::NRNODE] * ih[VI::NODMAX]);
    BOOST_CHECK_EQUAL(static_cast<int>(networkData.getRBran().size()), ih[VI::NRBRAN] * ih[VI::NBRMAX]);

    //INode-parameters
    const auto& iNode = networkData.getINode();
    auto start = 0*ih[VI::NINODE];
    BOOST_CHECK_EQUAL(iNode[start + 0], 1);
    BOOST_CHECK_EQUAL(iNode[start + 1], 1);
    BOOST_CHECK_EQUAL(iNode[start + 2], 4);
    BOOST_CHECK_EQUAL(iNode[start + 3], 0);
    BOOST_CHECK_EQUAL(iNode[start + 4], 1);

    start = 1*ih[VI::NINODE];
    BOOST_CHECK_EQUAL(iNode[start + 0], 2);
    BOOST_CHECK_EQUAL(iNode[start + 1], 2);
    BOOST_CHECK_EQUAL(iNode[start + 2], 3);
    BOOST_CHECK_EQUAL(iNode[start + 3], 0);
    BOOST_CHECK_EQUAL(iNode[start + 4], 1);

    start = 2*ih[VI::NINODE];
    BOOST_CHECK_EQUAL(iNode[start + 0], 3);
    BOOST_CHECK_EQUAL(iNode[start + 1], 4);
    BOOST_CHECK_EQUAL(iNode[start + 2], 1);
    BOOST_CHECK_EQUAL(iNode[start + 3], 0);
    BOOST_CHECK_EQUAL(iNode[start + 4], 1);

    start = 3*ih[VI::NINODE];
    BOOST_CHECK_EQUAL(iNode[start + 0], 1);
    BOOST_CHECK_EQUAL(iNode[start + 1], 7);
    BOOST_CHECK_EQUAL(iNode[start + 2], 2);
    BOOST_CHECK_EQUAL(iNode[start + 3], 1);
    BOOST_CHECK_EQUAL(iNode[start + 4], 1);

    start = 4*ih[VI::NINODE];
    BOOST_CHECK_EQUAL(iNode[start + 0], 1);
    BOOST_CHECK_EQUAL(iNode[start + 1], 8);
    BOOST_CHECK_EQUAL(iNode[start + 2], 6);
    BOOST_CHECK_EQUAL(iNode[start + 3], 0);
    BOOST_CHECK_EQUAL(iNode[start + 4], 1);

    start = 5*ih[VI::NINODE];
    BOOST_CHECK_EQUAL(iNode[start + 0], 2);
    BOOST_CHECK_EQUAL(iNode[start + 1], 9);
    BOOST_CHECK_EQUAL(iNode[start + 2], 5);
    BOOST_CHECK_EQUAL(iNode[start + 3], 0);
    BOOST_CHECK_EQUAL(iNode[start + 4], 1);

    //IBran-parameters
    const auto& iBran = networkData.getIBran();
    start = 0*ih[VI::NIBRAN];
    BOOST_CHECK_EQUAL(iBran[start + 0], 1);
    BOOST_CHECK_EQUAL(iBran[start + 1], 2);
    BOOST_CHECK_EQUAL(iBran[start + 2], 3);

    start = 1*ih[VI::NIBRAN];
    BOOST_CHECK_EQUAL(iBran[start + 0], 2);
    BOOST_CHECK_EQUAL(iBran[start + 1], 3);
    BOOST_CHECK_EQUAL(iBran[start + 2], 8);

    start = 2*ih[VI::NIBRAN];
    BOOST_CHECK_EQUAL(iBran[start + 0], 3);
    BOOST_CHECK_EQUAL(iBran[start + 1], 4);
    BOOST_CHECK_EQUAL(iBran[start + 2], 7);

    start = 3*ih[VI::NIBRAN];
    BOOST_CHECK_EQUAL(iBran[start + 0], 5);
    BOOST_CHECK_EQUAL(iBran[start + 1], 6);
    BOOST_CHECK_EQUAL(iBran[start + 2], 3);

    start = 4*ih[VI::NIBRAN];
    BOOST_CHECK_EQUAL(iBran[start + 0], 6);
    BOOST_CHECK_EQUAL(iBran[start + 1], 3);
    BOOST_CHECK_EQUAL(iBran[start + 2], 8);

    //ZNode-parameters
    const std::string blank8 = "        ";

    const auto& zNode = networkData.getZNode();
    start = 0*ih[VI::NZNODE];
    BOOST_CHECK_EQUAL(zNode[start + 0].c_str(), pad8("B1"));
    BOOST_CHECK_EQUAL(zNode[start + 1].c_str(), blank8);

    start = 1*ih[VI::NZNODE];
    BOOST_CHECK_EQUAL(zNode[start + 0].c_str(), pad8("N1"));
    BOOST_CHECK_EQUAL(zNode[start + 1].c_str(), blank8);

    start = 2*ih[VI::NZNODE];
    BOOST_CHECK_EQUAL(zNode[start + 0].c_str(), pad8("N2"));
    BOOST_CHECK_EQUAL(zNode[start + 1].c_str(), blank8);

    start = 3*ih[VI::NZNODE];
    BOOST_CHECK_EQUAL(zNode[start + 0].c_str(), pad8("PLAT-A"));
    BOOST_CHECK_EQUAL(zNode[start + 1].c_str(), blank8);

    start = 4*ih[VI::NZNODE];
    BOOST_CHECK_EQUAL(zNode[start + 0].c_str(), pad8("B2"));
    BOOST_CHECK_EQUAL(zNode[start + 1].c_str(), blank8);

    start = 5*ih[VI::NZNODE];
    BOOST_CHECK_EQUAL(zNode[start + 0].c_str(), pad8("M1"));
    BOOST_CHECK_EQUAL(zNode[start + 1].c_str(), blank8);

    //INobr-parameters
    const auto& iNobr = networkData.getINobr();
    start = 0;
    BOOST_CHECK_EQUAL(iNobr[start + 0], -1);
    BOOST_CHECK_EQUAL(iNobr[start + 1],  1);
    BOOST_CHECK_EQUAL(iNobr[start + 2], -2);
    BOOST_CHECK_EQUAL(iNobr[start + 3],  2);
    BOOST_CHECK_EQUAL(iNobr[start + 4], -3);
    BOOST_CHECK_EQUAL(iNobr[start + 5],  5);
    BOOST_CHECK_EQUAL(iNobr[start + 6],  3);
    BOOST_CHECK_EQUAL(iNobr[start + 7], -4);
    BOOST_CHECK_EQUAL(iNobr[start + 8],  4);
    BOOST_CHECK_EQUAL(iNobr[start + 9], -5);

    //RNode-parameters
    const auto& rNode = networkData.getRNode();
    start = 0*ih[VI::NRNODE];
    BOOST_CHECK_CLOSE(rNode[start + 0], 81.6848, 1.0e-10);
    //BOOST_CHECK_CLOSE(rNode[start + 1], 1., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 2], 50., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 15], 1., 1.0e-10);

    start = 1*ih[VI::NRNODE];
    BOOST_CHECK_CLOSE(rNode[start + 0], 72., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 1], 1., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 2], 67., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 15], 1., 1.0e-10);

    start = 2*ih[VI::NRNODE];
    BOOST_CHECK_CLOSE(rNode[start + 0], 69., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 1], 1., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 2], 67., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 15], 1., 1.0e-10);

    start = 3*ih[VI::NRNODE];
    BOOST_CHECK_CLOSE(rNode[start + 0], 67., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 1], 0., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 2], 67., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 15], 1., 1.0e-10);

    start = 4*ih[VI::NRNODE];
    BOOST_CHECK_CLOSE(rNode[start + 0], 79.0666, 1.0e-10);
    //BOOST_CHECK_CLOSE(rNode[start + 1], 1., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 2], 52., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 15], 1., 1.0e-10);

    start = 5*ih[VI::NRNODE];
    BOOST_CHECK_CLOSE(rNode[start + 0], 72., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 1], 1., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 2], 67., 1.0e-10);
    BOOST_CHECK_CLOSE(rNode[start + 15], 1., 1.0e-10);


    //RNode-parameters
    const auto& rBran = networkData.getRBran();
    start = 0*ih[VI::NRBRAN];
    BOOST_CHECK_CLOSE(rBran[start + 0], 6342.673828, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 1], 7.0e-06, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 2], 1029926.1875, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 8], 841.77298664275565, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 9], 0.91567670595811512, 1.0e-10);

    start = 2*ih[VI::NRBRAN];
    BOOST_CHECK_CLOSE(rBran[start + 0], 10225.117676, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 1], 9.0e-06, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 2], 1802329.125, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 8], 841.97309189645352, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 9], 0.91752948909927878, 1.0e-10);

    start = 3*ih[VI::NRBRAN];
    BOOST_CHECK_CLOSE(rBran[start + 0], 3882.4438479999999, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 1], 1.9999999999999999e-06, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 2], 772402.9375, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 8], 842.3, 1.0e-10);
    BOOST_CHECK_CLOSE(rBran[start + 9], 0.92, 1.0e-10);
}

BOOST_AUTO_TEST_SUITE_END()
