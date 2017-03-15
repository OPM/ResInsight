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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif // HAVE_DYNAMIC_BOOST_TEST

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_FLOWDIAGNOSTICSTOOL

#include <boost/test/unit_test.hpp>

#include <opm/flowdiagnostics/Toolbox.hpp>

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/utility/numeric/RandomVector.hpp>

#include <algorithm>

using namespace Opm::FlowDiagnostics;

namespace
{
    std::size_t
    numIntConn(const std::size_t nx,
               const std::size_t ny)
    {
        return (nx - 1)*ny + nx*(ny - 1);
    }

    std::vector<int>
    internalConnections(const std::size_t nx,
                        const std::size_t ny)
    {
        auto cellID = [](const std::size_t start,
                         const std::size_t off)
        {
            return static_cast<int>(start + off);
        };

        auto neighbours = std::vector<int>{};
        neighbours.reserve(2 * numIntConn(nx, ny));

        // I connections
        {
            for (auto j = 0*ny; j < ny; ++j) {
                const auto start = j * nx;

                for (auto i = 0*nx + 1; i < nx; ++i) {
                    neighbours.push_back(cellID(start, i - 1));
                    neighbours.push_back(cellID(start, i - 0));
                }
            }
        }

        // J connections
        {
            for (auto j = 0*ny + 1; j < ny; ++j) {
                const auto start = (j - 1)*nx;

                for (auto i = 0*nx; i < nx; ++i) {
                    neighbours.push_back(cellID(start, i + 0 ));
                    neighbours.push_back(cellID(start, i + nx));
                }
            }
        }

        return neighbours;
    }

    std::vector<double>
    flowField(const std::vector<double>::size_type n)
    {
        static Opm::RandomVector genRandom{};

        return genRandom.normal(n);
    }

} // Namespace anonymous

class Setup
{
public:
    Setup(const std::size_t nx,
          const std::size_t ny);

    const ConnectivityGraph& connectivity() const;
    const std::vector<double>&    poreVolume()   const;
    const ConnectionValues&  flux()         const;

private:
    ConnectivityGraph g_;
    std::vector<double>    pvol_;
    ConnectionValues  flux_;
};

Setup::Setup(const std::size_t nx,
             const std::size_t ny)
    : g_   (nx * ny, internalConnections(nx, ny))
    , pvol_(g_.numCells(), 0.3)
    , flux_(ConnectionValues::NumConnections{ g_.numConnections() },
            ConnectionValues::NumPhases     { 1 })
{
    const auto flux = flowField(g_.numConnections());

    using ConnID = ConnectionValues::ConnID;

    const auto phaseID =
        ConnectionValues::PhaseID{ 0 };

    for (decltype(flux_.numConnections())
             conn = 0, nconn = flux_.numConnections();
         conn < nconn; ++conn)
    {
        flux_(ConnID{conn}, phaseID) = flux[conn];
    }
}

const ConnectivityGraph&
Setup::connectivity() const
{
    return g_;
}

const std::vector<double>&
Setup::poreVolume() const
{
    return pvol_;
}

const ConnectionValues&
Setup::flux() const
{
    return flux_;
}

BOOST_AUTO_TEST_SUITE(FlowDiagnostics_Toolbox)

BOOST_AUTO_TEST_CASE (Constructor)
{
    BOOST_TEST_MESSAGE("==============   Test: Constructor   ==============");

    const auto cas = Setup(2, 2);

    Toolbox diagTool(cas.connectivity());

    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(cas.flux());
}




namespace {

    template <class Collection1, class Collection2>
    void check_is_close(const Collection1& c1, const Collection2& c2)
    {
        BOOST_REQUIRE_EQUAL(c1.size(), c2.size());

        if (! c1.empty()) {
            auto i1 = c1.begin(), e1 = c1.end();
            auto i2 = c2.begin();

            for (; i1 != e1; ++i1, ++i2) {
                BOOST_CHECK_CLOSE(*i1, *i2, 1.0e-10);
            }
        }
    }

} // Namespace Anonymous






BOOST_AUTO_TEST_CASE (OneDimCase)
{
    BOOST_TEST_MESSAGE("==============   Test: OneDimCase   ==============");
    using namespace Opm::FlowDiagnostics;

    const auto cas = Setup(5, 1);
    const auto& graph = cas.connectivity();

    // Create fluxes.
    ConnectionValues flux(ConnectionValues::NumConnections{ graph.numConnections() },
                          ConnectionValues::NumPhases     { 1 });
    const size_t nconn = cas.connectivity().numConnections();
    for (size_t conn = 0; conn < nconn; ++conn) {
        flux(ConnectionValues::ConnID{conn}, ConnectionValues::PhaseID{0}) = 0.3;
    }

    // Create well in/out flows.
    CellSetValues wellflow = { {0, 0.3}, {4, -0.3} };

    Toolbox diagTool(graph);
    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(flux);
    diagTool.assignInflowFlux(wellflow);

    // Check that inconsistent start set specifications will throw.
    {
        const int first_cell = 0;
        const int last_cell = cas.connectivity().numCells() - 1;
        auto start = std::vector<CellSet>{ CellSet(CellSetID("I-1"), {first_cell}),
                                           CellSet(CellSetID("I-2"), {last_cell}) };
        BOOST_CHECK_THROW(diagTool.computeInjectionDiagnostics(start), std::runtime_error);
        BOOST_CHECK_THROW(diagTool.computeProductionDiagnostics(start), std::runtime_error);
    }

    const int first_cell = 0;
    const int last_cell = cas.connectivity().numCells() - 1;
    auto start_fwd = std::vector<CellSet>{ CellSet(CellSetID("I-1"), {first_cell}) };
    auto start_rev = std::vector<CellSet>{ CellSet(CellSetID("I-2"), {last_cell}) };
    const auto fwd = diagTool.computeInjectionDiagnostics(start_fwd);
    const auto rev = diagTool.computeProductionDiagnostics(start_rev);

    // Global ToF field (accumulated from all injectors)
    {
        const auto tof = fwd.fd.timeOfFlight();

        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 1.0, 2.0, 3.0, 4.0, 5.0 };
        check_is_close(tof, expected);
    }

    // Global ToF field (accumulated from all producers)
    {
        const auto tof = rev.fd.timeOfFlight();

        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 5.0, 4.0, 3.0, 2.0, 1.0 };
        check_is_close(tof, expected);
    }

    // Verify set of start points.
    {
        const auto startpts = fwd.fd.startPoints();

        BOOST_CHECK_EQUAL(startpts.size(), start_fwd.size());

        for (const auto& pt : startpts) {
            auto pos =
                std::find_if(start_fwd.begin(), start_fwd.end(),
                    [&pt](const CellSet& s)
                    {
                        return s.id().to_string() == pt.to_string();
                    });

            // ID of 'pt' *MUST* be in set of identified start points.
            BOOST_CHECK(pos != start_fwd.end());
        }
    }

    // Tracer-ToF
    {
        const auto tof = fwd.fd
            .timeOfFlight(CellSetID("BogusID"));

        std::vector<std::pair<int, double>> expected = {};
        BOOST_REQUIRE_EQUAL(tof.size(), expected.size());

        int i = 0;
        for (const auto& v : tof) {
            BOOST_TEST_MESSAGE("ToF[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Tracer Concentration
    {
        const auto conc = fwd.fd
            .concentration(CellSetID("BogusID"));

        std::vector<std::pair<int, double>> expected = {};
        BOOST_REQUIRE_EQUAL(conc.size(), expected.size());

        int i = 0;
        for (const auto& v : conc) {
            BOOST_TEST_MESSAGE("Conc[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }

    }


    // Tracer-ToF
    {
        const auto tof = fwd.fd
            .timeOfFlight(CellSetID("I-1"));

        std::vector<double> expected = { 1.0, 2.0, 3.0, 4.0, 5.0 };
        BOOST_REQUIRE_EQUAL(tof.size(), expected.size());

        for (const auto& v : tof) {
            BOOST_TEST_MESSAGE("ToF[" << v.first << "] = " << v.second);
            BOOST_CHECK_CLOSE(v.second, expected[v.first], 1.0e-10);
        }
    }

    // Tracer Concentration
    {
        const auto conc = fwd.fd
            .concentration(CellSetID("I-1"));

        std::vector<double> expected = { 1.0, 1.0, 1.0, 1.0, 1.0 };
        BOOST_REQUIRE_EQUAL(conc.size(), expected.size());

        for (const auto& v : conc) {
            BOOST_TEST_MESSAGE("Conc[" << v.first << "] = " << v.second);
            BOOST_CHECK_CLOSE(v.second, expected[v.first], 1.0e-10);
        }
    }


    // Add a start point in the middle.
    const int middle_cell = 2;
    start_fwd.emplace_back(CellSet(CellSetID("Middle"), {middle_cell}));
    BOOST_CHECK_THROW(diagTool.computeInjectionDiagnostics(start_fwd), std::runtime_error);
}

// Arrows indicate a flux of 0.3, O is a source of 0.3
// and X is a sink of 0.3 (each cell has a pore volume of 0.3).
//  ----------------------------
//  |       |        |         |
//  |   O   ->       ->        |
//  |       |        ->        |
//  |       |        |    ||   |
//  -------------^--------VV----
//  |       |    |   |         |
//  |       |        |         |
//  |   O   ->       |   XX    |
//  |       |        |         |
//  ----------------------------
// Cell indices:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   3   |    4   |    5    |
//  |       |        |         |
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   0   |    1   |    2    |
//  |       |        |         |
//  ----------------------------
// Expected global injection TOF:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   1.0 |    2.0 |    2.5  |
//  |       |        |         |
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   1.0 |    2.0 |    3.0  |
//  |       |        |         |
//  ----------------------------
// Expected global production TOF:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   2.5 |    1.5 |    1.0  |
//  |       |        |         |
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   3.5 |    2.5 |    0.5  |
//  |       |        |         |
//  ----------------------------
BOOST_AUTO_TEST_CASE (LocalSolutions)
{
    BOOST_TEST_MESSAGE("==============   Test: LocalSolutions   ==============");
    using namespace Opm::FlowDiagnostics;

    const auto cas = Setup(3, 2);
    const auto& graph = cas.connectivity();

    // Create fluxes.
    ConnectionValues flux(ConnectionValues::NumConnections{ graph.numConnections() },
                          ConnectionValues::NumPhases     { 1 });
    const size_t nconn = cas.connectivity().numConnections();
    for (size_t conn = 0; conn < nconn; ++conn) {
        BOOST_TEST_MESSAGE("Connection " << conn << " connects cells "
                           << graph.connection(conn).first << " and "
                           << graph.connection(conn).second);
    }

    using C = ConnectionValues::ConnID;
    using P = ConnectionValues::PhaseID;
    flux(C{0}, P{0}) = 0.3;
    flux(C{1}, P{0}) = 0.0;
    flux(C{2}, P{0}) = 0.3;
    flux(C{3}, P{0}) = 0.6;
    flux(C{4}, P{0}) = 0.0;
    flux(C{5}, P{0}) = 0.3;
    flux(C{6}, P{0}) = -0.6;

    // Create well in/out flows.
    CellSetValues wellflow = { {0, 0.3}, {3, 0.3}, {2, -0.6} };

    Toolbox diagTool(graph);
    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(flux);
    diagTool.assignInflowFlux(wellflow);

    auto injstart = std::vector<CellSet>{ CellSet(CellSetID("I-1"), {0}),
                                          CellSet(CellSetID("I-2"), {3}) };
    auto prdstart = std::vector<CellSet>{ CellSet(CellSetID("P-1"), {2}) };

    const auto fwd = diagTool.computeInjectionDiagnostics(injstart);
    const auto rev = diagTool.computeProductionDiagnostics(prdstart);

    // Global ToF field (accumulated from all injectors)
    {
        const auto tof = fwd.fd.timeOfFlight();

        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 1.0, 2.0, 3.0, 1.0, 2.0, 2.5 };
        check_is_close(tof, expected);
    }

    // Global ToF field (accumulated from all producers)
    {
        const auto tof = rev.fd.timeOfFlight();

        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 3.5, 2.5, 0.5, 2.5, 1.5, 1.0 };
        check_is_close(tof, expected);
    }

    // Verify set of start points.
    {
        using VCS = std::vector<Opm::FlowDiagnostics::CellSet>;
        using VCSI = std::vector<Opm::FlowDiagnostics::CellSetID>;
        using P = std::pair<VCS, VCSI>;
        std::vector<P> pairs { P{ injstart, fwd.fd.startPoints() }, P{ prdstart, rev.fd.startPoints() } };
        for (const auto& p : pairs) {
            const auto& s1 = p.first;
            const auto& s2 = p.second;
            BOOST_CHECK_EQUAL(s1.size(), s2.size());
            for (const auto& pt : s2) {
                // ID of 'pt' *MUST* be in set of identified start points.
                auto pos = std::find_if(s1.begin(), s1.end(),
                                        [&pt](const CellSet& s)
                                        {
                                            return s.id().to_string() == pt.to_string();
                                        });
                BOOST_CHECK(pos != s1.end());
            }
        }
    }

    // Local I-1 tracer concentration.
    {
        const auto conc = fwd.fd.concentration(CellSetID("I-1"));
        std::vector<std::pair<int, double>> expected = { {0, 1.0}, {1, 1.0}, {2, 0.5}, {4, 0.5}, {5, 0.5} };
        BOOST_REQUIRE_EQUAL(conc.size(), expected.size());

        int i = 0;
        for (const auto& v : conc) {
            BOOST_TEST_MESSAGE("Conc[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Local I-1 tof.
    {
        const auto tof = fwd.fd.timeOfFlight(CellSetID("I-1"));
        std::vector<std::pair<int, double>> expected = { {0, 1.0}, {1, 2.0}, {2, 3.5}, {4, 2.5}, {5, 3.0} };
        BOOST_REQUIRE_EQUAL(tof.size(), expected.size());

        int i = 0;
        for (const auto& v : tof) {
            BOOST_TEST_MESSAGE("ToF[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Local I-2 tracer concentration.
    {
        const auto conc = fwd.fd.concentration(CellSetID("I-2"));
        std::vector<std::pair<int, double>> expected = { {2, 0.5}, {3, 1.0}, {4, 0.5}, {5, 0.5} };
        BOOST_REQUIRE_EQUAL(conc.size(), expected.size());

        int i = 0;
        for (const auto& v : conc) {
            BOOST_TEST_MESSAGE("Conc[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Local I-2 tof.
    {
        const auto tof = fwd.fd.timeOfFlight(CellSetID("I-2"));
        std::vector<std::pair<int, double>> expected = { {2, 2.5}, {3, 1.0}, {4, 1.5}, {5, 2.0} };
        BOOST_REQUIRE_EQUAL(tof.size(), expected.size());

        int i = 0;
        for (const auto& v : tof) {
            BOOST_TEST_MESSAGE("ToF[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

}



// Arrows indicate a flux of 0.3, O is a source of 0.3
// and X is a sink of 0.3 (each cell has a pore volume of 0.3).
//  ----------------------------
//  |       |        |         |
//  |   O   ->   O   ->   XX   |
//  | "I-1" |  "I-2" ->  "P-1" |
//  |       |        |         |
//  ----------------------------
// Cell indices:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   0   |    1   |    2    |
//  |       |        |         |
//  ----------------------------
// Expected global injection TOF:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   1.0 |    1.0 |    1.5  |
//  |       |        |         |
//  ----------------------------
// Expected global production TOF:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   2.0 |    1.0 |    0.5  |
//  |       |        |         |
//  ----------------------------
// Expected local tracer I-1:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   1.0 |    0.5 |    0.5  |
//  |       |        |         |
//  ----------------------------
// Expected local tracer I-2:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   0.0 |    0.5 |    0.5  |
//  |       |        |         |
//  ----------------------------
// Expected local tof I-1:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |   1.0 |    1.5 |    2.0  |
//  |       |        |         |
//  ----------------------------
// Expected local tof I-2:
//  ----------------------------
//  |       |        |         |
//  |       |        |         |
//  |       |    0.5 |    1.0  |
//  |       |        |         |
//  ----------------------------
BOOST_AUTO_TEST_CASE (LocalSolutionsWithMidflowSource)
{
    BOOST_TEST_MESSAGE("==============   Test: LocalSolutionsWithMidflowSource   ==============");
    using namespace Opm::FlowDiagnostics;

    const auto cas = Setup(3, 1);
    const auto& graph = cas.connectivity();

    // Create fluxes.
    ConnectionValues flux(ConnectionValues::NumConnections{ graph.numConnections() },
                          ConnectionValues::NumPhases     { 1 });
    const size_t nconn = cas.connectivity().numConnections();
    for (size_t conn = 0; conn < nconn; ++conn) {
        BOOST_TEST_MESSAGE("Connection " << conn << " connects cells "
                           << graph.connection(conn).first << " and "
                           << graph.connection(conn).second);
    }
    using C = ConnectionValues::ConnID;
    using P = ConnectionValues::PhaseID;
    flux(C{0}, P{0}) = 0.3;
    flux(C{1}, P{0}) = 0.6;

    // Create well in/out flows.
    CellSetValues wellflow = { {0, 0.3}, {1, 0.3}, {2, -0.6} };

    Toolbox diagTool(graph);
    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(flux);
    diagTool.assignInflowFlux(wellflow);

    auto injstart = std::vector<CellSet>{ CellSet(CellSetID("I-1"), {0}),
                                          CellSet(CellSetID("I-2"), {1}) };
    auto prdstart = std::vector<CellSet>{ CellSet(CellSetID("P-1"), {2}) };

    const auto fwd = diagTool.computeInjectionDiagnostics(injstart);
    const auto rev = diagTool.computeProductionDiagnostics(prdstart);
    // Global ToF field (accumulated from all injectors)
    {
        BOOST_TEST_MESSAGE("== Global injector ToF");
        const auto tof = fwd.fd.timeOfFlight();
        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 1.0, 1.0, 1.5 };
        check_is_close(tof, expected);
    }

    // Global ToF field (accumulated from all producers)
    {
        BOOST_TEST_MESSAGE("== Global producer ToF");
        const auto tof = rev.fd.timeOfFlight();
        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 2.0, 1.0, 0.5 };
        check_is_close(tof, expected);
    }

    // Verify set of start points.
    {
        using VCS = std::vector<Opm::FlowDiagnostics::CellSet>;
        using VCSI = std::vector<Opm::FlowDiagnostics::CellSetID>;
        using P = std::pair<VCS, VCSI>;
        std::vector<P> pairs { P{ injstart, fwd.fd.startPoints() }, P{ prdstart, rev.fd.startPoints() } };
        for (const auto& p : pairs) {
            const auto& s1 = p.first;
            const auto& s2 = p.second;
            BOOST_CHECK_EQUAL(s1.size(), s2.size());
            for (const auto& pt : s2) {
                // ID of 'pt' *MUST* be in set of identified start points.
                auto pos = std::find_if(s1.begin(), s1.end(),
                                        [&pt](const CellSet& s)
                                        {
                                            return s.id().to_string() == pt.to_string();
                                        });
                BOOST_CHECK(pos != s1.end());
            }
        }
    }

    // Local I-1 tracer concentration.
    {
        BOOST_TEST_MESSAGE("== I-1 tracer");
        const auto conc = fwd.fd.concentration(CellSetID("I-1"));
        std::vector<std::pair<int, double>> expected = { {0, 1.0}, {1, 0.5}, {2, 0.5} };
        BOOST_REQUIRE_EQUAL(conc.size(), expected.size());

        int i = 0;
        for (const auto& v : conc) {
            BOOST_TEST_MESSAGE("Conc[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Local I-1 tof.
    {
        BOOST_TEST_MESSAGE("== I-1 tof");
        const auto tof = fwd.fd.timeOfFlight(CellSetID("I-1"));
        std::vector<std::pair<int, double>> expected = { {0, 1.0}, {1, 1.5}, {2, 2.0} };
        BOOST_REQUIRE_EQUAL(tof.size(), expected.size());

        int i = 0;
        for (const auto& v : tof) {
            BOOST_TEST_MESSAGE("ToF[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Local I-2 tracer concentration.
    {
        BOOST_TEST_MESSAGE("== I-2 tracer");
        const auto conc = fwd.fd.concentration(CellSetID("I-2"));
        std::vector<std::pair<int, double>> expected = { {1, 0.5}, {2, 0.5} };
        BOOST_REQUIRE_EQUAL(conc.size(), expected.size());

        int i = 0;
        for (const auto& v : conc) {
            BOOST_TEST_MESSAGE("Conc[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }

    // Local I-2 tof.
    {
        BOOST_TEST_MESSAGE("== I-2 tof");
        const auto tof = fwd.fd.timeOfFlight(CellSetID("I-2"));
        std::vector<std::pair<int, double>> expected = { {1, 0.5}, {2, 1.0} };
        BOOST_REQUIRE_EQUAL(tof.size(), expected.size());

        int i = 0;
        for (const auto& v : tof) {
            BOOST_TEST_MESSAGE("ToF[" << v.first << "] = " << v.second);
            BOOST_CHECK_EQUAL(v.first, expected[i].first);
            BOOST_CHECK_CLOSE(v.second, expected[i].second, 1.0e-10);
            ++i;
        }
    }
}



BOOST_AUTO_TEST_SUITE_END()
