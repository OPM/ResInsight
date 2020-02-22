/*
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

#define BOOST_TEST_MODULE TEST_DERIVEDQUANTITIES

#include <boost/test/unit_test.hpp>

#include <opm/flowdiagnostics/DerivedQuantities.hpp>
#include <opm/flowdiagnostics/Toolbox.hpp>

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
        return std::vector<double>(n, 0.3);
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

BOOST_AUTO_TEST_SUITE(Test_DerivedQuantities)

BOOST_AUTO_TEST_CASE (Constructor)
{
    const auto cas = Setup(2, 2);

    Toolbox diagTool(cas.connectivity());

    diagTool.assignPoreVolume(cas.poreVolume());
    diagTool.assignConnectionFlux(cas.flux());
}


namespace {

    template <typename T>
    void element_is_close(const T& t1, const T& t2)
    {
        BOOST_CHECK_CLOSE(t1, t2, 1.0e-10);
    }

    // using DP = std::pair<double, double>;

    // template<>
    // void element_is_close<DP>(const DP& p1, const DP& p2)
    // {
    //     BOOST_CHECK_CLOSE(p1.first, p2.first, 1.0e-10);
    //     BOOST_CHECK_CLOSE(p1.second, p2.second, 1.0e-10);
    // }

    template <class Collection1, class Collection2>
    void check_is_close(const Collection1& c1, const Collection2& c2)
    {
        BOOST_REQUIRE_EQUAL(c1.size(), c2.size());

        if (! c1.empty()) {
            auto i1 = c1.begin(), e1 = c1.end();
            auto i2 = c2.begin();

            for (; i1 != e1; ++i1, ++i2) {
                element_is_close(*i1, *i2);
            }
        }
    }

    template <>
    void check_is_close<>(const Graph& c1, const Graph& c2)
    {
        BOOST_TEST_MESSAGE("Comparing first collections");
        check_is_close(c1.first, c2.first);
        BOOST_TEST_MESSAGE("Comparing second collections");
        check_is_close(c1.second, c2.second);
    }

} // Namespace Anonymous






BOOST_AUTO_TEST_CASE (OneDimCase)
{
    using namespace Opm::FlowDiagnostics;

    const auto cas = Setup(5, 1);
    const auto& graph = cas.connectivity();
    const auto& pv = cas.poreVolume();
    const auto& flux = cas.flux();

    // Create well in/out flows.
    std::map<CellSetID, CellSetValues> wellflow = { { CellSetID("I-1"), {{0, 0.3}} }, { CellSetID("P-1"), {{4, -0.3}} } };

    Toolbox diagTool(graph);
    diagTool.assignPoreVolume(pv);
    diagTool.assignConnectionFlux(flux);
    diagTool.assignInflowFlux(wellflow);

    auto inje = std::vector<CellSet>{CellSet(CellSetID("I-1"), {0})};

    auto prod = std::vector<CellSet>{CellSet(CellSetID("P-1"), {int(graph.numCells()) - 1})};

    {
        const auto fwd = diagTool.computeInjectionDiagnostics(inje);
        const auto rev = diagTool.computeProductionDiagnostics(prod);

        BOOST_TEST_MESSAGE("==== F-Phi graph");
        const Graph expectedFPhi{
            { 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 },
            { 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 }
        };
        BOOST_CHECK_THROW(flowCapacityStorageCapacityCurve({}, rev, pv), std::runtime_error);
        BOOST_CHECK_THROW(flowCapacityStorageCapacityCurve(fwd, {}, pv), std::runtime_error);
        BOOST_CHECK_THROW(flowCapacityStorageCapacityCurve(fwd, rev, {}), std::runtime_error);
        const auto fcapscap = flowCapacityStorageCapacityCurve(fwd, rev, pv);
        check_is_close(fcapscap, expectedFPhi);
        const auto fcapscap2 = flowCapacityStorageCapacityCurve(fwd.fd.timeOfFlight(), rev.fd.timeOfFlight(), pv);
        check_is_close(fcapscap2, expectedFPhi);

        BOOST_TEST_MESSAGE("==== Lorenz coefficient");
        const double expectedLorenz = 0.0;
        BOOST_CHECK_CLOSE(lorenzCoefficient(fcapscap), expectedLorenz, 1e-10);
        const Graph wrongGraph {
            { 0.0, 0.5, 1.0 },
            { 1.0, 1.0 }
        };
        BOOST_CHECK_THROW(lorenzCoefficient(wrongGraph), std::runtime_error);
        const Graph maxLorenzGraph {
            { 0.0, 1.0 },
            { 1.0, 1.0 }
        };
        BOOST_CHECK_CLOSE(lorenzCoefficient(maxLorenzGraph), 1.0, 1e-10);
        const Graph inbetweenLorenzGraph {
            { 0.0, 0.45, 1.0 },
            { 0.0, 0.75, 1.0 }
        };
        BOOST_CHECK_CLOSE(lorenzCoefficient(inbetweenLorenzGraph), 0.3, 1e-10);

        BOOST_TEST_MESSAGE("==== Sweep efficiency");
        const Graph expectedSweep{
            { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
            { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
        };
        BOOST_CHECK_THROW(sweepEfficiency(wrongGraph), std::runtime_error);
        check_is_close(sweepEfficiency(fcapscap), expectedSweep);
        const Graph expSweepMax {
            { 0.0 },
            { 0.0 }
        };
        check_is_close(sweepEfficiency(maxLorenzGraph), expSweepMax);
        const Graph expSweepInbetween { // Verified against MRST version
            { 0.0, 0.6, 2.2 },
            { 0.0, 0.6, 1.0 }
        };
        check_is_close(sweepEfficiency(inbetweenLorenzGraph), expSweepInbetween);

        const double expectedVol12 = 1.5;
        const double vol12 = injectorProducerPairVolume(fwd, rev, pv, CellSetID("I-1"), CellSetID("P-1"));
        BOOST_CHECK_CLOSE(vol12, expectedVol12, 1e-10);

        const auto pairflux = injectorProducerPairFlux(fwd, rev, CellSetID("I-1"), CellSetID("P-1"), wellflow);
        BOOST_CHECK_CLOSE(pairflux.first, 0.3, 1e-10);
        BOOST_CHECK_CLOSE(pairflux.second, -0.3, 1e-10);
    }

}





BOOST_AUTO_TEST_CASE (GeneralCase)
{
    BOOST_TEST_MESSAGE("==== F-Phi graph");

    std::vector<double> pv { 1.0, 2.0, 1.0 };
    std::vector<double> ftof { 0.0, 2.0, 1.0 };
    std::vector<double> rtof { 1.0, 2.0, 0.0 };
    const Graph expectedFPhi{
        { 0.0, 0.25, 0.5, 1.0 },
        { 0.0, 0.4, 0.8, 1.0 }
    };
    const auto fcapscap = flowCapacityStorageCapacityCurve(ftof, rtof, pv);
    check_is_close(fcapscap, expectedFPhi);

    BOOST_TEST_MESSAGE("==== Lorenz coefficient");
    const double expectedLorenz = 0.3;
    BOOST_CHECK_CLOSE(lorenzCoefficient(fcapscap), expectedLorenz, 1e-10);
}


BOOST_AUTO_TEST_SUITE_END()
