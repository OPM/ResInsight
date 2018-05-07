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

#define BOOST_TEST_MODULE TEST_CONNECTIONVALUES

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/flowdiagnostics/ConnectionValues.hpp>

BOOST_AUTO_TEST_SUITE(Connection_Values)


using Opm::FlowDiagnostics::ConnectionValues;


BOOST_AUTO_TEST_CASE (Constructor)
{
    using NConn = ConnectionValues::NumConnections;
    using NPhas = ConnectionValues::NumPhases;

    const auto nconn = NConn{4};
    const auto nphas = NPhas{2};

    auto v = ConnectionValues(nconn, nphas);

    BOOST_CHECK_EQUAL(v.numPhases()     , nphas.total);
    BOOST_CHECK_EQUAL(v.numConnections(), nconn.total);
}

BOOST_AUTO_TEST_CASE (AssignValues)
{
    using NConn = ConnectionValues::NumConnections;
    using NPhas = ConnectionValues::NumPhases;

    const auto nconn = NConn{4};
    const auto nphas = NPhas{2};

    using ConnID = ConnectionValues::ConnID;
    using PhasID = ConnectionValues::PhaseID;

    auto v = ConnectionValues(nconn, nphas);

    {
        for (decltype(v.numConnections())
                 conn = 0, numconn = v.numConnections();
             conn < numconn; ++conn)
        {
            for (decltype(v.numPhases())
                     phas = 0, numphas = v.numPhases();
                 phas < numphas; ++phas)
            {
                v(ConnID{conn}, PhasID{phas}) =
                    conn*numphas + phas;
            }
        }
    }

    {
        const auto w = v;

        BOOST_CHECK_CLOSE(w(ConnID{0}, PhasID{0}), 0.0, 1.0e-10);

        BOOST_CHECK_CLOSE(w(ConnID{ nconn.total - 1 },
                            PhasID{ 0 }), 6.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_SUITE_END()
