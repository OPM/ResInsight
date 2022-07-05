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

#define BOOST_TEST_MODULE data_InterRegFlow

#include <boost/test/unit_test.hpp>

#include <opm/output/data/InterRegFlow.hpp>

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

BOOST_AUTO_TEST_SUITE(InterReg_View)

BOOST_AUTO_TEST_CASE(Construct)
{
    using IRegFlow = Opm::data::InterRegFlow<decltype(std::declval<std::vector<double>>().begin())>;
    using IRegRate = IRegFlow::FlowRates;
    using Component = IRegFlow::Component;
    using Direction = IRegFlow::Direction;

    auto store = std::vector<double>(IRegFlow::bufferSize(), 0.0);

    {
        auto iregFlow = IRegFlow{ store.begin(), store.begin() }; // empty
        BOOST_CHECK_MESSAGE(iregFlow.empty(), "[begin, begin) must be an empty range");
    }

    {
        auto iregFlow = IRegFlow{ store.begin(), store.begin() + static_cast<std::size_t>(Component::NumComponents) };
        BOOST_CHECK_MESSAGE(!iregFlow.isValid(), "Small range must be invalid");
    }

    auto iregFlow = IRegFlow{ store.begin(), store.end() };
    BOOST_REQUIRE_MESSAGE(iregFlow.isValid(), "Valid range must be valid");

    auto rate = IRegRate{};
    rate[Component::Oil] = 1.0;
    rate[Component::Gas] = 2.0;
    rate[Component::Water] = 3.0;
    rate[Component::Disgas] = 4.0;
    rate[Component::Vapoil] = 5.0;

    iregFlow.addFlow(1.0, rate); // 1->2

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

    iregFlow.addFlow(-1.0, rate); // 2->1

    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 0.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 0.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 0.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 0.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 0.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 1.0e-6);

    iregFlow.addFlow(1.0, rate); // 1->2

    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Positive), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Positive), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Oil, Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Gas, Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Water, Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Disgas, Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(Component::Vapoil, Direction::Negative), -5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(store[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[9], - 5.0, 1.0e-6);
}

BOOST_AUTO_TEST_CASE(Copy_Move_Assign_Add)
{
    using IRegFlow = Opm::data::InterRegFlow<decltype(std::declval<std::vector<double>>().begin())>;
    using IRegRate = IRegFlow::FlowRates;
    using Component = IRegFlow::Component;
    using Direction = IRegFlow::Direction;

    auto store = std::vector<double>(2 * IRegFlow::bufferSize(), 0.0);

    {
        auto iregFlow = IRegFlow{ store.begin(), store.begin() }; // empty
        BOOST_CHECK_MESSAGE(iregFlow.empty(), "[begin, begin) must be an empty range");
    }

    {
        auto iregFlow = IRegFlow{ store.begin(), store.end() };
        BOOST_CHECK_MESSAGE(!iregFlow.isValid(), "Large range must be invalid");
    }

    auto iregFlow_1 = IRegFlow{ store.begin(), store.begin() + IRegFlow::bufferSize() };
    auto iregFlow_2 = IRegFlow{ store.begin() + IRegFlow::bufferSize(), store.end() };
    BOOST_REQUIRE_MESSAGE(iregFlow_1.isValid(), "Valid range 1 must be valid");
    BOOST_REQUIRE_MESSAGE(iregFlow_2.isValid(), "Valid range 2 must be valid");

    auto rate = IRegRate{};
    rate[Component::Oil] = 1.0;
    rate[Component::Gas] = 2.0;
    rate[Component::Water] = 3.0;
    rate[Component::Disgas] = 4.0;
    rate[Component::Vapoil] = 5.0;

    iregFlow_1.addFlow(  1.0, rate); // 1->2
    iregFlow_1.addFlow(- 1.0, rate); // 2->1
    iregFlow_1.addFlow(  1.0, rate); // 1->2

    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Oil), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Gas), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Water), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Disgas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Vapoil), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Oil, Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Gas, Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Water, Direction::Positive), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Disgas, Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Vapoil, Direction::Positive), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Oil, Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Gas, Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Water, Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Disgas, Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Vapoil, Direction::Negative), -5.0, 1.0e-6);

    const auto iregFlow_3 = std::move(iregFlow_1);
    BOOST_CHECK_MESSAGE(! iregFlow_1.isValid(), "Moved-from range must be invalid");
    BOOST_CHECK_MESSAGE(  iregFlow_3.isValid(), "Move-constructed range must be valid");

    BOOST_CHECK_CLOSE(store[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[9], - 5.0, 1.0e-6);

    iregFlow_2 = iregFlow_3;
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Oil), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Gas), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Water), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Disgas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Vapoil), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Oil, Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Gas, Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Water, Direction::Positive), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Disgas, Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Vapoil, Direction::Positive), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Oil, Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Gas, Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Water, Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Disgas, Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Vapoil, Direction::Negative), -5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(store[10 + 0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 9], - 5.0, 1.0e-6);

    iregFlow_2 += iregFlow_3;
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Oil), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Gas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Water), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Disgas), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Vapoil), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Oil, Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Gas, Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Water, Direction::Positive), 12.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Disgas, Direction::Positive), 16.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Vapoil, Direction::Positive), 20.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Oil, Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Gas, Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Water, Direction::Negative), -6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Disgas, Direction::Negative), -8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_2.flow(Component::Vapoil, Direction::Negative), -10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(store[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[9], - 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(store[10 + 0],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 1], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 2],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 3], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 4],  12.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 5], - 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 6],  16.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 7], - 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 8],  20.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 9], -10.0, 1.0e-6);

    iregFlow_1 = std::move(iregFlow_2);
    BOOST_CHECK_MESSAGE(! iregFlow_2.isValid(), "Moved-from source range must be invalid");
    BOOST_CHECK_MESSAGE(  iregFlow_1.isValid(), "Moved-assigned destination range must be valid");
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Oil), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Gas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Water), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Disgas), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Vapoil), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Oil, Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Gas, Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Water, Direction::Positive), 12.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Disgas, Direction::Positive), 16.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Vapoil, Direction::Positive), 20.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Oil, Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Gas, Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Water, Direction::Negative), -6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Disgas, Direction::Negative), -8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow_1.flow(Component::Vapoil, Direction::Negative), -10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(store[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[9], - 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(store[10 + 0],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 1], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 2],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 3], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 4],  12.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 5], - 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 6],  16.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 7], - 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 8],  20.0, 1.0e-6);
    BOOST_CHECK_CLOSE(store[10 + 9], -10.0, 1.0e-6);
}

BOOST_AUTO_TEST_CASE(Assign_Different_Iterator)
{
    using IRegFlow = Opm::data::InterRegFlow<decltype(std::declval<std::vector<double>>().begin())>;

    using ArrayBuffer = std::array<double, IRegFlow::bufferSize()>;
    using IRegFlow_ArrayBacked = Opm::data::InterRegFlow<decltype(std::declval<ArrayBuffer>().begin())>;
    using IRegRate = IRegFlow_ArrayBacked::FlowRates;
    using Component = IRegFlow_ArrayBacked::Component;

    auto buffer = ArrayBuffer{};
    buffer.fill(0.0);

    auto iregFlow_array = IRegFlow_ArrayBacked{ buffer.begin(), buffer.end() };

    auto rate = IRegRate{};
    rate[Component::Oil] = 1.0;
    rate[Component::Gas] = 2.0;
    rate[Component::Water] = 3.0;
    rate[Component::Disgas] = 4.0;
    rate[Component::Vapoil] = 5.0;

    iregFlow_array.addFlow(  1.0, rate); // 1->2
    iregFlow_array.addFlow(- 1.0, rate); // 2->1
    iregFlow_array.addFlow(  1.0, rate); // 1->2

    auto range = std::vector<double>(IRegFlow::bufferSize(), 0.0);
    auto iregFlow = IRegFlow{ range.begin(), range.end() };

    iregFlow = iregFlow_array;

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil, IRegFlow::Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas, IRegFlow::Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water, IRegFlow::Direction::Positive), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas, IRegFlow::Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil, IRegFlow::Direction::Positive), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil, IRegFlow::Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas, IRegFlow::Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water, IRegFlow::Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas, IRegFlow::Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil, IRegFlow::Direction::Negative), -5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(range[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[9], - 5.0, 1.0e-6);
}

BOOST_AUTO_TEST_CASE(Assign_Different_ElmT)
{
    using IRegFlow = Opm::data::InterRegFlow<decltype(std::declval<std::vector<double>>().begin())>;
    using IRegFlow_float = Opm::data::InterRegFlow<decltype(std::declval<std::vector<float>>().begin())>;

    using IRegRate = IRegFlow_float::FlowRates;
    using Component = IRegFlow_float::Component;

    auto buffer = std::vector<float>(IRegFlow_float::bufferSize(), 0.0f);
    auto iregFlow_float = IRegFlow_float{ buffer.begin(), buffer.end() };

    auto rate = IRegRate{};
    rate[Component::Oil] = 1.0f;
    rate[Component::Gas] = 2.0f;
    rate[Component::Water] = 3.0f;
    rate[Component::Disgas] = 4.0f;
    rate[Component::Vapoil] = 5.0f;

    iregFlow_float.addFlow(  1.0, rate); // 1->2
    iregFlow_float.addFlow(- 1.0, rate); // 2->1
    iregFlow_float.addFlow(  1.0, rate); // 1->2

    auto range = std::vector<double>(IRegFlow::bufferSize(), 0.0);
    auto iregFlow = IRegFlow{ range.begin(), range.end() };

    iregFlow = iregFlow_float;

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil, IRegFlow::Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas, IRegFlow::Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water, IRegFlow::Direction::Positive), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas, IRegFlow::Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil, IRegFlow::Direction::Positive), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil, IRegFlow::Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas, IRegFlow::Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water, IRegFlow::Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas, IRegFlow::Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil, IRegFlow::Direction::Negative), -5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(range[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[9], - 5.0, 1.0e-6);
}

BOOST_AUTO_TEST_CASE(Assign_Different_ElmT_Different_Iterator)
{
    using IRegFlow = Opm::data::InterRegFlow<decltype(std::declval<std::vector<double>>().begin())>;

    using ArrayBuffer = std::array<float, IRegFlow::bufferSize()>;
    using IRegFlow_ArrayBacked = Opm::data::InterRegFlow<decltype(std::declval<ArrayBuffer>().begin())>;
    using IRegRate = IRegFlow_ArrayBacked::FlowRates;
    using Component = IRegFlow_ArrayBacked::Component;

    auto buffer = ArrayBuffer{};
    buffer.fill(0.0f);
    auto iregFlow_array = IRegFlow_ArrayBacked{ buffer.begin(), buffer.end() };

    auto rate = IRegRate{};
    rate[Component::Oil] = 1.0f;
    rate[Component::Gas] = 2.0f;
    rate[Component::Water] = 3.0f;
    rate[Component::Disgas] = 4.0f;
    rate[Component::Vapoil] = 5.0f;

    iregFlow_array.addFlow(  1.0, rate); // 1->2
    iregFlow_array.addFlow(- 1.0, rate); // 2->1
    iregFlow_array.addFlow(  1.0, rate); // 1->2

    auto range = std::vector<double>(IRegFlow::bufferSize(), 0.0);
    auto iregFlow = IRegFlow{ range.begin(), range.end() };

    iregFlow = iregFlow_array;

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil), 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water), 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil), 5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil, IRegFlow::Direction::Positive), 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas, IRegFlow::Direction::Positive), 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water, IRegFlow::Direction::Positive), 6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas, IRegFlow::Direction::Positive), 8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil, IRegFlow::Direction::Positive), 10.0, 1.0e-6);

    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Oil, IRegFlow::Direction::Negative), -1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Gas, IRegFlow::Direction::Negative), -2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Water, IRegFlow::Direction::Negative), -3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Disgas, IRegFlow::Direction::Negative), -4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(iregFlow.flow(IRegFlow::Component::Vapoil, IRegFlow::Direction::Negative), -5.0, 1.0e-6);

    BOOST_CHECK_CLOSE(range[0],   2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[1], - 1.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[2],   4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[3], - 2.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[4],   6.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[5], - 3.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[6],   8.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[7], - 4.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[8],  10.0, 1.0e-6);
    BOOST_CHECK_CLOSE(range[9], - 5.0, 1.0e-6);
}

BOOST_AUTO_TEST_SUITE_END() // InterReg_View
