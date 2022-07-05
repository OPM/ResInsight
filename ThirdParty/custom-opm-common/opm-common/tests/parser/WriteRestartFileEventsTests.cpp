/*
  Copyright 2021 Equinor.

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

#define BOOST_TEST_MODULE Restart File Events
#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Schedule/WriteRestartFileEvents.hpp>

#include <cstddef>

BOOST_AUTO_TEST_SUITE(Basic_Operations)

BOOST_AUTO_TEST_CASE(Default_Constructor)
{
    const auto events = Opm::WriteRestartFileEvents{};

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(0),
                        "Default constructed events object must not "
                        "have events at report step zero");

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(11),
                        "Default constructed events object must not "
                        "have events at report step 11");

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(1729),
                        "Default constructed events object must not "
                        "have events at report step 1729");
}

BOOST_AUTO_TEST_CASE(Add_Events)
{
    auto events = Opm::WriteRestartFileEvents{};

    events.addRestartOutput(11);
    events.addRestartOutput(22);
    events.addRestartOutput(33);
    events.addRestartOutput(59);
    events.addRestartOutput(64);
    BOOST_CHECK_MESSAGE(events.writeRestartFile(11),
                        "Events object must have restart event at "
                        "report step 11 after 'add'");

    BOOST_CHECK_MESSAGE(events.writeRestartFile(22),
                        "Events object must have restart event at "
                        "report step 22 after 'add'");

    BOOST_CHECK_MESSAGE(events.writeRestartFile(33),
                        "Events object must have restart event at "
                        "report step 33 after 'add'");

    BOOST_CHECK_MESSAGE(events.writeRestartFile(59),
                        "Events object must have restart event at "
                        "report step 59 after 'add'");

    BOOST_CHECK_MESSAGE(events.writeRestartFile(64),
                        "Events object must have restart event at "
                        "report step 64 after 'add'");

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(0),
                        "Events object must have NOT restart event at "
                        "report step 0 after 'add(64)'");

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(42),
                        "Events object must have NOT restart event at "
                        "report step 42 after 'add(64)'");

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(65),
                        "Events object must have NOT restart event at "
                        "report step 65 after 'add(64)'");

    BOOST_CHECK_MESSAGE(!events.writeRestartFile(1729),
                        "Events object must have NOT restart event at "
                        "report step 1729 after 'add(64)'");
}

BOOST_AUTO_TEST_CASE(Clear_Remaining_Events)
{
    auto events = Opm::WriteRestartFileEvents{};

    events.addRestartOutput(11);
    events.addRestartOutput(22);
    events.addRestartOutput(33);
    events.addRestartOutput(59);

    events.clearRemainingEvents(33);

    BOOST_CHECK_MESSAGE(events.writeRestartFile(11),
                        "Events object must have restart event at "
                        "report step 11 after 'add'");

    BOOST_CHECK_MESSAGE(events.writeRestartFile(22),
                        "Events object must have restart event at "
                        "report step 22 after 'add'");

    for (auto i = 33; i < 64; ++i) {
        BOOST_CHECK_MESSAGE(!events.writeRestartFile(i),
                            "Events object must have NOT restart event at "
                            "report step " << i << " after 'clearRemainingEvents(33)'");
    }

    events.addRestartOutput(33);
    events.clearRemainingEvents(34);

    BOOST_CHECK_MESSAGE(events.writeRestartFile(33),
                        "Events object must have restart event at "
                        "report step 33 after 'clearRemainingEvents(34)'");

    for (auto i = 34; i < 64; ++i) {
        BOOST_CHECK_MESSAGE(!events.writeRestartFile(i),
                            "Events object must have NOT restart event at "
                            "report step " << i << " after 'clearRemainingEvents(34)'");
    }
}

BOOST_AUTO_TEST_CASE(Last_Restart_Event_Before)
{
    auto events = Opm::WriteRestartFileEvents{};

    {
        const auto event = events.lastRestartEventBefore(271828);
        BOOST_CHECK_MESSAGE(!event.has_value(), "There must be no output events before report step 271828");
    }

    events.addRestartOutput(11);
    events.addRestartOutput(22);
    events.addRestartOutput(33);
    events.addRestartOutput(59);

    {
        const auto event = events.lastRestartEventBefore(11);
        BOOST_CHECK_MESSAGE(!event.has_value(), "There must be no output events before report step 11");
    }

    {
        const auto event = events.lastRestartEventBefore(12);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 12");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{11});
    }

    {
        const auto event = events.lastRestartEventBefore(22);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 22");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{11});
    }

    {
        const auto event = events.lastRestartEventBefore(23);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 23");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{22});
    }

    {
        const auto event = events.lastRestartEventBefore(28);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 28");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{22});
    }

    {
        const auto event = events.lastRestartEventBefore(33);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 33");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{22});
    }

    {
        const auto event = events.lastRestartEventBefore(34);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 34");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{33});
    }

    {
        const auto event = events.lastRestartEventBefore(50);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 50");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{33});
    }

    {
        const auto event = events.lastRestartEventBefore(59);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 59");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{33});
    }

    {
        const auto event = events.lastRestartEventBefore(60);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 60");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{59});
    }

    {
        const auto event = events.lastRestartEventBefore(64);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 64");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{59});
    }

    {
        const auto event = events.lastRestartEventBefore(1729);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 1729");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{59});
    }

    events.addRestartOutput(42);

    {
        const auto event = events.lastRestartEventBefore(42);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 42");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{33});
    }

    {
        const auto event = events.lastRestartEventBefore(43);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 43");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{42});
    }

    events.addRestartOutput(128);

    {
        const auto event = events.lastRestartEventBefore(1729);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 1729");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{128});
    }

    {
        const auto event = events.lastRestartEventBefore(128);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 128");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{59});
    }

    {
        const auto event = events.lastRestartEventBefore(129);
        BOOST_CHECK_MESSAGE(event.has_value(), "There must be an output event before report step 129");

        BOOST_CHECK_EQUAL(event.value(), std::size_t{128});
    }
}

BOOST_AUTO_TEST_SUITE_END()    // Basic_Operations
