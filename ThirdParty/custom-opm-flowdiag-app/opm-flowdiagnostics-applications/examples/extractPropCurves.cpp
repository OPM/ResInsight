/*
  Copyright 2017 SINTEF ICT, Applied Mathematics.
  Copyright 2017 Statoil ASA.

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

#include <examples/exampleSetup.hpp>

#include <opm/utility/ECLCaseUtilities.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLPvtCurveCollection.hpp>
#include <opm/utility/ECLResultData.hpp>
#include <opm/utility/ECLSaturationFunc.hpp>

#include <cstddef>
#include <exception>
#include <iomanip>
#include <ios>
#include <vector>

#include <boost/filesystem.hpp>

namespace {
    template <class OStream>
    void printGraph(OStream&                           os,
                    const std::string&                 name,
                    const Opm::FlowDiagnostics::Graph& graph)
    {
        const auto& x = graph.first;
        const auto& y = graph.second;

        const auto oprec  = os.precision(16);
        const auto oflags = os.setf(std::ios_base::scientific);

        os << name << " = [\n";

        for (auto n = x.size(), i = 0*n; i < n; ++i) {
            os << x[i] << ' ' << y[i] << '\n';
        }

        os << "];\n\n";

        os.setf(oflags);
        os.precision(oprec);
    }

    // -----------------------------------------------------------------
    // Relative permeability

    void krg(const Opm::ECLSaturationFunc& sfunc,
             const int                     activeCell,
             const bool                    useEPS)
    {
        using RC = Opm::ECLSaturationFunc::RawCurve;

        auto func = std::vector<RC>{};
        func.reserve(1);

        // Request krg (gas rel-perm in oil-gas system)
        func.push_back(RC{
            RC::Function::RelPerm,
            RC::SubSystem::OilGas,
            Opm::ECLPhaseIndex::Vapour
        });

        const auto graph =
            sfunc.getSatFuncCurve(func, activeCell, useEPS);

        printGraph(std::cout, "krg", graph[0]);
    }

    void krog(const Opm::ECLSaturationFunc& sfunc,
              const int                     activeCell,
              const bool                    useEPS)
    {
        using RC = Opm::ECLSaturationFunc::RawCurve;

        auto func = std::vector<RC>{};
        func.reserve(1);

        // Request krog (oil rel-perm in oil-gas system)
        func.push_back(RC{
            RC::Function::RelPerm,
            RC::SubSystem::OilGas,
            Opm::ECLPhaseIndex::Liquid
        });

        const auto graph =
            sfunc.getSatFuncCurve(func, activeCell, useEPS);

        printGraph(std::cout, "krog", graph[0]);
    }

    void krow(const Opm::ECLSaturationFunc& sfunc,
              const int                     activeCell,
              const bool                    useEPS)
    {
        using RC = Opm::ECLSaturationFunc::RawCurve;

        auto func = std::vector<RC>{};
        func.reserve(1);

        // Request krow (oil rel-perm in oil-water system)
        func.push_back(RC{
            RC::Function::RelPerm,
            RC::SubSystem::OilWater,
            Opm::ECLPhaseIndex::Liquid
        });

        const auto graph =
            sfunc.getSatFuncCurve(func, activeCell, useEPS);

        printGraph(std::cout, "krow", graph[0]);
    }

    void krw(const Opm::ECLSaturationFunc& sfunc,
             const int                     activeCell,
             const bool                    useEPS)
    {
        using RC = Opm::ECLSaturationFunc::RawCurve;

        auto func = std::vector<RC>{};
        func.reserve(1);

        // Request krw (water rel-perm in oil-water system)
        func.push_back(RC{
            RC::Function::RelPerm,
            RC::SubSystem::OilWater,
            Opm::ECLPhaseIndex::Aqua
        });

        const auto graph =
            sfunc.getSatFuncCurve(func, activeCell, useEPS);

        printGraph(std::cout, "krw", graph[0]);
    }

    // -----------------------------------------------------------------
    // PVT Curves

    void Bg(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCurves,
            const int                                 activeCell)
    {
        using RC = Opm::ECLPVT::RawCurve;

        const auto graph = pvtCurves
            .getPvtCurve(RC::FVF, Opm::ECLPhaseIndex::Vapour, activeCell);

        printGraph(std::cout, "Bg", graph);
    }

    void mu_g(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCurves,
              const int                                 activeCell)
    {
        using RC = Opm::ECLPVT::RawCurve;

        const auto graph = pvtCurves
            .getPvtCurve(RC::Viscosity, Opm::ECLPhaseIndex::Vapour, activeCell);

        printGraph(std::cout, "mu_g", graph);
    }

    void Bo(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCurves,
            const int                                 activeCell)
    {
        using RC = Opm::ECLPVT::RawCurve;

        const auto graph = pvtCurves
            .getPvtCurve(RC::FVF, Opm::ECLPhaseIndex::Liquid, activeCell);

        printGraph(std::cout, "Bo", graph);
    }

    void mu_o(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCurves,
              const int                                 activeCell)
    {
        using RC = Opm::ECLPVT::RawCurve;

        const auto graph = pvtCurves
            .getPvtCurve(RC::Viscosity, Opm::ECLPhaseIndex::Liquid, activeCell);

        printGraph(std::cout, "mu_o", graph);
    }
} // namespace Anonymous

int main(int argc, char* argv[])
try {
    const auto prm    = example::initParam(argc, argv);
    const auto useEPS = prm.getDefault("useEPS", false);
    const auto cellID = prm.getDefault("cell", 0);

    const auto rset  = example::identifyResultSet(prm);
    const auto init  = Opm::ECLInitFileData(rset.initFile());
    const auto graph = Opm::ECLGraph::load(rset.gridFile(), init);
    const auto sfunc = Opm::ECLSaturationFunc(graph, init, useEPS);
    const auto pvtCC = Opm::ECLPVT::ECLPvtCurveCollection(graph, init);

    // -----------------------------------------------------------------
    // Relative permeability

    if (prm.getDefault("krg" , false)) { krg (sfunc, cellID, useEPS); }
    if (prm.getDefault("krog", false)) { krog(sfunc, cellID, useEPS); }
    if (prm.getDefault("krow", false)) { krow(sfunc, cellID, useEPS); }
    if (prm.getDefault("krw" , false)) { krw (sfunc, cellID, useEPS); }

    // -----------------------------------------------------------------
    // PVT Curves

    if (prm.getDefault("Bg"  , false)) { Bg  (pvtCC, cellID); }
    if (prm.getDefault("mu_g", false)) { mu_g(pvtCC, cellID); }
    if (prm.getDefault("Bo"  , false)) { Bo  (pvtCC, cellID); }
    if (prm.getDefault("mu_o", false)) { mu_o(pvtCC, cellID); }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}
