/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.

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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MESSAGELIMITER_TESTS

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/common/OpmLog/MessageLimiter.hpp>


using namespace Opm;


BOOST_AUTO_TEST_CASE(ConstructionAndLimits)
{
    MessageLimiter m1;
    BOOST_CHECK_EQUAL(m1.messageLimit(), MessageLimiter::NoLimit);
    MessageLimiter m2(0);
    BOOST_CHECK_EQUAL(m2.messageLimit(), 0);
    MessageLimiter m3(1);
    BOOST_CHECK_EQUAL(m3.messageLimit(), 1);
    MessageLimiter m4(-4);
    BOOST_CHECK_EQUAL(m4.messageLimit(), MessageLimiter::NoLimit);
}

BOOST_AUTO_TEST_CASE(Response)
{
    {
        // No limits.
        MessageLimiter m;
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::PrintMessage);
    }

    {
        // Limit == 0.
        MessageLimiter m(0);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::JustOverLimit);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::JustOverLimit);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::OverLimit);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::OverLimit);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::OverLimit);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::OverLimit);
    }

    {
        // Limit == 1.
        MessageLimiter m(1);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::PrintMessage);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::JustOverLimit);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::JustOverLimit);
        BOOST_CHECK(m.handleMessageTag("tag1") == MessageLimiter::Response::OverLimit);
        BOOST_CHECK(m.handleMessageTag("tag2") == MessageLimiter::Response::OverLimit);
    }
}
