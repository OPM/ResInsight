/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#include <config.h>

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_CONNECTIVITY_GRAPH

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/flowdiagnostics/ConnectivityGraph.hpp>

using Opm::FlowDiagnostics::ConnectivityGraph;

BOOST_AUTO_TEST_SUITE(Two_By_Two)

BOOST_AUTO_TEST_CASE (Constructor)
{
    const auto g =
        ConnectivityGraph(4,
                          { 0 , 1 ,
                            0 , 2 ,
                            1 , 3 ,
                            2 , 3 });

    BOOST_CHECK_EQUAL(g.numCells(),       4);
    BOOST_CHECK_EQUAL(g.numConnections(), 4);
}

BOOST_AUTO_TEST_CASE (ConnectionList)
{
    const auto g =
        ConnectivityGraph(4,
                          { 0, 1 ,
                            0, 2 ,
                            1, 3 ,
                            2, 3 });

    {
        const auto c = g.connection(0);

        BOOST_CHECK_EQUAL(c.first , 0);
        BOOST_CHECK_EQUAL(c.second, 1);
    }

    {
        const auto c = g.connection(1);

        BOOST_CHECK_EQUAL(c.first , 0);
        BOOST_CHECK_EQUAL(c.second, 2);
    }

    {
        const auto c = g.connection(2);

        BOOST_CHECK_EQUAL(c.first , 1);
        BOOST_CHECK_EQUAL(c.second, 3);
    }

    {
        const auto c = g.connection(3);

        BOOST_CHECK_EQUAL(c.first , 2);
        BOOST_CHECK_EQUAL(c.second, 3);
    }
}

BOOST_AUTO_TEST_SUITE_END()
