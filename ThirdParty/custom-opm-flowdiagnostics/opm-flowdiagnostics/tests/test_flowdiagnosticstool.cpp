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
    const auto cas = Setup(2, 2);

    Toolbox diagTool(cas.connectivity());

    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(cas.flux());
}

BOOST_AUTO_TEST_CASE (InjectionDiagnostics)
{
    const auto cas = Setup(2, 2);

    Toolbox diagTool(cas.connectivity());

    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(cas.flux());

    auto start = std::vector<CellSet>{};
    {
        start.emplace_back();

        auto& s = start.back();

        s.identify(CellSetID("I-1"));
        s.insert(0);
    }

    {
        start.emplace_back();

        auto& s = start.back();

        s.identify(CellSetID("I-2"));
        s.insert(cas.connectivity().numCells() - 1);
    }

    const auto fwd = diagTool
        .computeInjectionDiagnostics(start);

    // Global ToF field (accumulated from all injectors)
    {
        const auto tof = fwd.fd.timeOfFlight();

        BOOST_CHECK_EQUAL(tof.size(), cas.connectivity().numCells());
    }

    // Verify set of start points.
    {
        const auto startpts = fwd.fd.startPoints();

        BOOST_CHECK_EQUAL(startpts.size(), start.size());

        for (const auto& pt : startpts) {
            auto pos =
                std::find_if(start.begin(), start.end(),
                    [&pt](const CellSet& s)
                    {
                        return s.id().to_string() == pt.to_string();
                    });

            // ID of 'pt' *MUST* be in set of identified start points.
            BOOST_CHECK(pos != start.end());
        }
    }

    // Tracer-ToF
    {
        const auto tof = fwd.fd
            .timeOfFlight(CellSetID("I-1"));

        for (decltype(tof.cellValueCount())
                 i = 0, n = tof.cellValueCount();
             i < n; ++i)
        {
            const auto v = tof.cellValue(i);

            BOOST_TEST_MESSAGE("[" << i << "] -> ToF["
                               << v.first << "] = "
                               << v.second);
        }
    }

    // Tracer Concentration
    {
        const auto conc = fwd.fd
            .concentration(CellSetID("I-2"));

        BOOST_TEST_MESSAGE("conc.cellValueCount() = " <<
                           conc.cellValueCount());

        for (decltype(conc.cellValueCount())
                 i = 0, n = conc.cellValueCount();
             i < n; ++i)
        {
            const auto v = conc.cellValue(i);

            BOOST_TEST_MESSAGE("[" << i << "] -> Conc["
                               << v.first << "] = "
                               << v.second);
        }
    }
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
    CellSetValues wellflow;
    wellflow.addCellValue(0, 0.3);
    wellflow.addCellValue(4, -0.3);

    Toolbox diagTool(graph);
    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(flux);
    diagTool.assignInflowFlux(wellflow);

    auto start = std::vector<CellSet>{};
    {
        start.emplace_back();

        auto& s = start.back();

        s.identify(CellSetID("I-1"));
        s.insert(0);
    }

    {
        start.emplace_back();

        auto& s = start.back();

        s.identify(CellSetID("I-2"));
        s.insert(cas.connectivity().numCells() - 1);
    }

    const auto fwd = diagTool.computeInjectionDiagnostics(start);
    const auto rev = diagTool.computeProductionDiagnostics(start);

    // Global ToF field (accumulated from all injectors)
    {
        const auto tof = fwd.fd.timeOfFlight();

        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 0.5, 1.5, 2.5, 3.5, 0.0 };
        check_is_close(tof, expected);
    }

    // Global ToF field (accumulated from all producers)
    {
        const auto tof = rev.fd.timeOfFlight();

        BOOST_REQUIRE_EQUAL(tof.size(), cas.connectivity().numCells());
        std::vector<double> expected = { 0.0, 3.5, 2.5, 1.5, 0.5 };
        check_is_close(tof, expected);
    }

    // Verify set of start points.
    {
        const auto startpts = fwd.fd.startPoints();

        BOOST_CHECK_EQUAL(startpts.size(), start.size());

        for (const auto& pt : startpts) {
            auto pos =
                std::find_if(start.begin(), start.end(),
                    [&pt](const CellSet& s)
                    {
                        return s.id().to_string() == pt.to_string();
                    });

            // ID of 'pt' *MUST* be in set of identified start points.
            BOOST_CHECK(pos != start.end());
        }
    }

    // Tracer-ToF
    {
        const auto tof = fwd.fd
            .timeOfFlight(CellSetID("I-2"));

        for (decltype(tof.cellValueCount())
                 i = 0, n = tof.cellValueCount();
             i < n; ++i)
        {
            const auto v = tof.cellValue(i);

            BOOST_TEST_MESSAGE("[" << i << "] -> ToF["
                               << v.first << "] = "
                               << v.second);
        }
    }

    // Tracer Concentration
    {
        const auto conc = fwd.fd
            .concentration(CellSetID("I-2"));

        BOOST_TEST_MESSAGE("conc.cellValueCount() = " <<
                           conc.cellValueCount());

        for (decltype(conc.cellValueCount())
                 i = 0, n = conc.cellValueCount();
             i < n; ++i)
        {
            const auto v = conc.cellValue(i);

            BOOST_TEST_MESSAGE("[" << i << "] -> Conc["
                               << v.first << "] = "
                               << v.second);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
