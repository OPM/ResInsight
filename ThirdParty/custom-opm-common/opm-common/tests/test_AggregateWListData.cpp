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
#include <opm/output/eclipse/AggregateWListData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/input/eclipse/Schedule/Well/WList.hpp>

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

BOOST_AUTO_TEST_SUITE(Aggregate_WList)


// test dimensions for IWLS and ZWLS plus the vectors for different cases
BOOST_AUTO_TEST_CASE (Constructor)
{
    namespace VI = ::Opm::RestartIO::Helpers::VectorItems;
    const auto simCase = SimulationCase{first_sim("TEST_WLIST.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;

    // Report Step 2  (3.07.20)
    {
        const auto simStep = std::size_t {1};
        double secs_elapsed = 3.1536E07;
        const auto ih
            = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, simStep, simStep + 1, simStep);

        auto wListData = Opm::RestartIO::Helpers::AggregateWListData(ih);
        wListData.captureDeclaredWListData(sched, simStep, ih);

        BOOST_CHECK_EQUAL(static_cast<int>(wListData.getIWls().size()),
                          ih[VI::intehead::NWMAXZ] * ih[VI::intehead::MXWLSTPRWELL]);
        BOOST_CHECK_EQUAL(static_cast<int>(wListData.getZWls().size()),
                          ih[VI::intehead::NWMAXZ] * ih[VI::intehead::MXWLSTPRWELL]);

        // IWls-parameters
        auto iWLs = wListData.getIWls();
        auto start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        // ZWLs-parameters
        const std::string blank8 = "        ";

        auto zWLs = wListData.getZWls();
        start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);
    }

    // Report Step 3  (5.07.20)
    {
        const auto simStep = std::size_t {2};
        double secs_elapsed = 3.1536E07;
        const auto ih
            = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, simStep, simStep + 1, simStep);

        auto wListData = Opm::RestartIO::Helpers::AggregateWListData(ih);
        wListData.captureDeclaredWListData(sched, simStep, ih);

        // IWls-parameters
        auto iWLs = wListData.getIWls();
        auto start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 3);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        // ZWLs-parameters
        const std::string blank8 = "        ";

        auto zWLs = wListData.getZWls();
        start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);
    }

    // Report Step 4  (1.08.20)
    {
        const auto simStep = std::size_t {3};
        double secs_elapsed = 3.1536E07;
        const auto ih
            = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, simStep, simStep + 1, simStep);

        auto wListData = Opm::RestartIO::Helpers::AggregateWListData(ih);
        wListData.captureDeclaredWListData(sched, simStep, ih);

        // IWls-parameters
        auto iWLs = wListData.getIWls();
        auto start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 3);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        // ZWLs-parameters
        const std::string blank8 = "        ";

        auto zWLs = wListData.getZWls();
        start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*INJ1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);
    }

        // Report Step 6  (20.08.20)
    {
        const auto simStep = std::size_t {5};
        double secs_elapsed = 3.1536E07;
        const auto ih
            = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, simStep, simStep + 1, simStep);

        auto wListData = Opm::RestartIO::Helpers::AggregateWListData(ih);
        wListData.captureDeclaredWListData(sched, simStep, ih);

        // IWls-parameters
        auto iWLs = wListData.getIWls();
        auto start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 3);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        // ZWLs-parameters
        const std::string blank8 = "        ";

        auto zWLs = wListData.getZWls();
        start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*INJ1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);
    }

    // Report Step 8  (10.09.20)
    {
        const auto simStep = std::size_t {7};
        double secs_elapsed = 3.1536E07;
        const auto ih
            = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, simStep, simStep + 1, simStep);

        auto wListData = Opm::RestartIO::Helpers::AggregateWListData(ih);
        wListData.captureDeclaredWListData(sched, simStep, ih);

        // IWls-parameters
        auto iWLs = wListData.getIWls();
        auto start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 3);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        // ZWLs-parameters
        const std::string blank8 = "        ";

        auto zWLs = wListData.getZWls();
        start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*INJ1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);
    }

        // Report Step 9  (10.09.20)
    {
        const auto simStep = std::size_t {8};
        double secs_elapsed = 3.1536E07;
        const auto ih
            = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, simStep, simStep + 1, simStep);

        auto wListData = Opm::RestartIO::Helpers::AggregateWListData(ih);
        wListData.captureDeclaredWListData(sched, simStep, ih);

        // IWls-parameters
        auto iWLs = wListData.getIWls();
        auto start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 2);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(iWLs[start + 0], 1);
        BOOST_CHECK_EQUAL(iWLs[start + 1], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 2], 0);
        BOOST_CHECK_EQUAL(iWLs[start + 3], 0);

        // ZWLs-parameters
        const std::string blank8 = "        ";

        auto zWLs = wListData.getZWls();
        start = 0 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*PRD2"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 1 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 2 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), pad8("*PRD1"));
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);

        start = 3 * ih[VI::intehead::MXWLSTPRWELL];
        BOOST_CHECK_EQUAL(zWLs[start + 0].c_str(), pad8("*INJ1"));
        BOOST_CHECK_EQUAL(zWLs[start + 1].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 2].c_str(), blank8);
        BOOST_CHECK_EQUAL(zWLs[start + 3].c_str(), blank8);
    }
}

BOOST_AUTO_TEST_SUITE_END()
