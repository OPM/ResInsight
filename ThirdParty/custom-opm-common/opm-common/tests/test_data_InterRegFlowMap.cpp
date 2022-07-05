/*
  Copyright (c) 2022 Equinor ASA

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

#define BOOST_TEST_MODULE data_InterRegFlowMap

#include <boost/test/unit_test.hpp>

#include <opm/output/data/InterRegFlowMap.hpp>

#include <optional>
#include <stdexcept>
#include <utility>
#include <tuple>

#include "tests/MessageBuffer.cpp"

namespace {
    Opm::data::InterRegFlowMap::FlowRates conn_1()
    {
        using Component = Opm::data::InterRegFlowMap::Component;

        auto rate = Opm::data::InterRegFlowMap::FlowRates{};

        rate[Component::Oil] = 1.0;
        rate[Component::Gas] = 2.0;
        rate[Component::Water] = 3.0;
        rate[Component::Disgas] = 4.0;
        rate[Component::Vapoil] = 5.0;

        return rate;
    }

    Opm::data::InterRegFlowMap::FlowRates conn_2()
    {
        using Component = Opm::data::InterRegFlowMap::Component;

        auto rate = Opm::data::InterRegFlowMap::FlowRates{};

        rate[Component::Oil] = 0.1;
        rate[Component::Gas] = 0.2;
        rate[Component::Water] = 0.3;
        rate[Component::Disgas] = 0.4;
        rate[Component::Vapoil] = 0.5;

        return rate;
    }

    Opm::data::InterRegFlowMap::FlowRates conn_3()
    {
        using Component = Opm::data::InterRegFlowMap::Component;

        auto rate = Opm::data::InterRegFlowMap::FlowRates{};

        rate[Component::Oil] = -0.2;
        rate[Component::Gas] = -0.4;
        rate[Component::Water] = -0.6;
        rate[Component::Disgas] = -0.8;
        rate[Component::Vapoil] = -1.0;

        return rate;
    }
}

BOOST_AUTO_TEST_SUITE(InterRegMap)

BOOST_AUTO_TEST_CASE(Basic)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap = Opm::data::InterRegFlowMap{};
    flowMap.addConnection(0, 1, conn_1());

    // Invalid source region index
    BOOST_CHECK_THROW(flowMap.addConnection(-1, 1, conn_1()),
                      std::logic_error);

    // Invalid destination region index
    BOOST_CHECK_THROW(flowMap.addConnection(1, -1, conn_1()),
                      std::logic_error);

    flowMap.compress(2);

    BOOST_CHECK_EQUAL(flowMap.numRegions(), 2);

    // Invalid source region index
    BOOST_CHECK_THROW(std::ignore = flowMap.getInterRegFlows(-1, 0),
                      std::invalid_argument);

    // Invalid destination region index
    BOOST_CHECK_THROW(std::ignore = flowMap.getInterRegFlows(0, -1),
                      std::invalid_argument);

    // Invalid region index pair (source == destination)
    BOOST_CHECK_THROW(std::ignore = flowMap.getInterRegFlows(0, 0),
                      std::invalid_argument);

    {
        auto flows = flowMap.getInterRegFlows(0, 1729);
        BOOST_CHECK_MESSAGE(! flows.has_value(),
                            "Unregistered region pair must NOT have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 1.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 1.0e-6);
    }
}

BOOST_AUTO_TEST_CASE(Basic_Reverse)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap = Opm::data::InterRegFlowMap{};
    flowMap.addConnection(1, 0, conn_1());

    flowMap.compress(2);

    BOOST_CHECK_EQUAL(flowMap.numRegions(), 2);

    {
        auto flows = flowMap.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), -1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), -2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), -3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), -4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), -5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 0.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 1.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), -1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), -2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), -3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), -4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), -5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 0.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 1.0e-6);
    }
}

BOOST_AUTO_TEST_CASE(Not_All_Pairs)
{
    auto flowMap = Opm::data::InterRegFlowMap{};
    flowMap.addConnection(3, 2, conn_1());
    flowMap.addConnection(3, 1, conn_1());
    flowMap.addConnection(2, 0, conn_1());
    flowMap.addConnection(1, 0, conn_1());

    flowMap.compress(4);

    {
        auto flows = flowMap.getInterRegFlows(0, 1);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair (0,1) must have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(0, 2);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair (0,2) must have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(0, 3);
        BOOST_CHECK_MESSAGE(! flows.has_value(),
                            "Unregistered region pair (0,3) must NOT have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 2);
        BOOST_CHECK_MESSAGE(! flows.has_value(),
                            "Unregistered region pair (1,2) must NOT have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 3);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair (1,3) must have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(2, 3);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair (2,3) must have a value");
    }
}

BOOST_AUTO_TEST_CASE(Clear)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap = Opm::data::InterRegFlowMap{};
    flowMap.addConnection(0, 1, conn_1());

    flowMap.compress(2);

    BOOST_CHECK_EQUAL(flowMap.numRegions(), 2);

    flowMap.clear();

    BOOST_CHECK_EQUAL(flowMap.numRegions(), 0);

    flowMap.addConnection(0, 1, conn_1());

    flowMap.compress(2);

    BOOST_CHECK_EQUAL(flowMap.numRegions(), 2);

    {
        auto flows = flowMap.getInterRegFlows(0, 1729);
        BOOST_CHECK_MESSAGE(! flows.has_value(),
                            "Unregistered region pair must NOT have a value");
    }

    {
        auto flows = flowMap.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 1.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 1.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 1.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 1.0e-6);
    }
}

BOOST_AUTO_TEST_CASE(MultiConn_Contrib)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap = Opm::data::InterRegFlowMap{};
    flowMap.addConnection(0, 1, conn_1());
    flowMap.addConnection(0, 1, conn_2());
    flowMap.addConnection(0, 1, conn_3());

    flowMap.addConnection(2, 1, conn_1());
    flowMap.addConnection(1, 2, conn_2());
    flowMap.addConnection(2, 1, conn_3());

    flowMap.compress(4);

    {
        auto flows = flowMap.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -1.0, 5.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -1.0, 5.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(0, 2);
        BOOST_CHECK_MESSAGE(! flows.has_value(),
                            "Registered region pair must have a value");

    }

    {
        auto flows = flowMap.getInterRegFlows(1, 3);
        BOOST_CHECK_MESSAGE(! flows.has_value(),
                            "Registered region pair must have a value");

    }

    {
        auto flows = flowMap.getInterRegFlows(2, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), -0.7, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), -1.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), -2.1, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), -2.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), -3.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 1.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 1.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 5.0e-6);
    }
}

BOOST_AUTO_TEST_CASE(Merge_MultiConn)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap = Opm::data::InterRegFlowMap{};
    flowMap.addConnection(0, 1, conn_1());
    flowMap.addConnection(0, 1, conn_2());
    flowMap.addConnection(0, 1, conn_3());

    flowMap.compress(4);

    flowMap.addConnection(2, 1, conn_1());
    flowMap.addConnection(1, 2, conn_2());
    flowMap.addConnection(2, 1, conn_3());

    flowMap.compress(4);

    flowMap.addConnection(0, 2, conn_1());
    flowMap.addConnection(2, 0, conn_2());
    flowMap.addConnection(3, 1, conn_3());

    flowMap.compress(4);

    {
        auto flows = flowMap.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -1.0, 5.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -1.0, 5.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(2, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), -0.7, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), -1.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), -2.1, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), -2.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), -3.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 1.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 1.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 5.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(2, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -0.5, 5.0e-6);
    }

    {
        auto flows = flowMap.getInterRegFlows(1, 3);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 0.6, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 1.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 1.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 5.0e-6);
    }
}

BOOST_AUTO_TEST_CASE(LinearBufferReadWrite)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap1 = Opm::data::InterRegFlowMap{};
    flowMap1.addConnection(0, 1, conn_1());
    flowMap1.addConnection(0, 1, conn_2());
    flowMap1.addConnection(0, 1, conn_3());

    flowMap1.addConnection(2, 1, conn_1());
    flowMap1.addConnection(1, 2, conn_2());
    flowMap1.addConnection(2, 1, conn_3());

    flowMap1.addConnection(0, 2, conn_1());
    flowMap1.addConnection(2, 0, conn_2());
    flowMap1.addConnection(3, 1, conn_3());

    flowMap1.compress(4);

    auto buffer = MessageBuffer{};
    flowMap1.write(buffer);

    auto flowMap2 = Opm::data::InterRegFlowMap{};
    flowMap2.read(buffer);

    flowMap2.compress(4);

    {
        auto flows = flowMap2.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -1.0, 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -1.0, 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(2, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), -0.7, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), -1.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), -2.1, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), -2.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), -3.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 1.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 1.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(2, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -0.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -0.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -0.5, 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(1, 3);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 0.6, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 1.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 1.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 5.0e-6);
    }
}

BOOST_AUTO_TEST_CASE(LinearBuffer_ManyReadWrite)
{
    using Component = Opm::data::InterRegFlowMap::ReadOnlyWindow::Component;
    using Direction = Opm::data::InterRegFlowMap::ReadOnlyWindow::Direction;

    auto flowMap1 = Opm::data::InterRegFlowMap{};
    flowMap1.addConnection(0, 1, conn_1());
    flowMap1.addConnection(0, 1, conn_2());
    flowMap1.addConnection(0, 1, conn_3());

    flowMap1.addConnection(2, 1, conn_1());
    flowMap1.addConnection(1, 2, conn_2());
    flowMap1.addConnection(2, 1, conn_3());

    flowMap1.addConnection(0, 2, conn_1());
    flowMap1.addConnection(2, 0, conn_2());
    flowMap1.addConnection(3, 1, conn_3());

    flowMap1.compress(4);

    auto buffer = MessageBuffer{};
    flowMap1.write(buffer);
    flowMap1.write(buffer);
    flowMap1.write(buffer);
    flowMap1.write(buffer);

    auto flowMap2 = Opm::data::InterRegFlowMap{};
    flowMap2.read(buffer);
    flowMap2.read(buffer);
    flowMap2.read(buffer);
    flowMap2.read(buffer);

    flowMap2.compress(4);

    {
        auto flows = flowMap2.getInterRegFlows(0, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 4 * 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 4 * 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 4 * 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4 * 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4 * 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 4 * 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 4 * 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 4 * 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4 * 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 4 * 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 4 * (-0.2), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 4 * (-0.4), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 4 * (-0.6), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 4 * (-0.8), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 4 * (-1.0), 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(1, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 4 * 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 4 * 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 4 * 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4 * 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4 * 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 4 * 1.1, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 4 * 2.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 4 * 3.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4 * 4.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 4 * 5.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 4 * (-0.2), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 4 * (-0.4), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 4 * (-0.6), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 4 * (-0.8), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 4 * (-1.0), 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(2, 1);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 4 * (-0.7), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 4 * (-1.4), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 4 * (-2.1), 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4 * (-2.8), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4 * (-3.5), 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 4 * 0.3, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 4 * 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 4 * 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4 * 1.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 4 * 1.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 4 * (-1.0), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 4 * (-2.0), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 4 * -3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 4 * (-4.0), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 4 * (-5.0), 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(2, 0);
        BOOST_REQUIRE_MESSAGE(flows.has_value(),
                              "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, -1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 4 * 0.9, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 4 * 1.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 4 * 2.7, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4 * 3.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4 * 4.5, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 4 * 1.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 4 * 2.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 4 * 3.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4 * 4.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 4 * 5.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 4 * (-0.1), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 4 * (-0.2), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 4 * (-0.3), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 4 * (-0.4), 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 4 * (-0.5), 5.0e-6);
    }

    {
        auto flows = flowMap2.getInterRegFlows(1, 3);
        BOOST_CHECK_MESSAGE(flows.has_value(),
                            "Registered region pair must have a value");

        const auto& [ iregFlow, sign ] = flows.value();
        BOOST_CHECK_EQUAL(sign, 1.0);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 4 * 0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 4 * 0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 4 * 0.6, 8.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4 * 0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 4 * 1.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 4 * 0.2, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 4 * 0.4, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 4 * 0.6, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4 * 0.8, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 4 * 1.0, 5.0e-6);

        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), 0.0, 5.0e-6);
        BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), 0.0, 5.0e-6);
    }
}

BOOST_AUTO_TEST_SUITE_END() // InterRegMap
