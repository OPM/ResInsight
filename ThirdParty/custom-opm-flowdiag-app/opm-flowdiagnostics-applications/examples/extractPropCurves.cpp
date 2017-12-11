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
#include <opm/utility/ECLPropertyUnitConversion.hpp>
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
    void printGraph(OStream&                                        os,
                    const std::string&                              name,
                    const std::vector<Opm::FlowDiagnostics::Graph>& graphs)
    {
        const auto oprec  = os.precision(16);
        const auto oflags = os.setf(std::ios_base::scientific);

        auto k = 1;
        for (const auto& graph : graphs) {
            const auto& x = graph.first;
            const auto& y = graph.second;

            os << name << '{' << k << "} = [\n";

            for (auto n = x.size(), i = 0*n; i < n; ++i) {
                os << x[i] << ' ' << y[i] << '\n';
            }

            os << "];\n\n";
            k += 1;
        }

        os.setf(oflags);
        os.precision(oprec);
    }

    template <class OStream>
    void printGraph(OStream&                                  os,
                    const std::string&                        name,
                    const std::vector<Opm::ECLPVT::PVTGraph>& graphs)
    {
        const auto oprec  = os.precision(16);
        const auto oflags = os.setf(std::ios_base::scientific);

        auto k = 1;
        for (const auto& graph : graphs) {
            const auto& p = graph.press;
            const auto& R = graph.mixRat;
            const auto& f = graph.value;

            os << name << '{' << k << "} = [\n";

            for (auto n = p.size(), i = 0*n; i < n; ++i) {
                os << p[i] << ' ' << R[i] << ' ' << f[i] << '\n';
            }

            os << "];\n\n";
            k += 1;
        }

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

        printGraph(std::cout, "krg", graph);
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

        printGraph(std::cout, "krog", graph);
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

        printGraph(std::cout, "krow", graph);
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

        printGraph(std::cout, "krw", graph);
    }

    // -----------------------------------------------------------------
    // Capillary pressure

    void pcgo(const Opm::ECLSaturationFunc& sfunc,
              const int                     activeCell,
              const bool                    useEPS)
    {
        using RC = Opm::ECLSaturationFunc::RawCurve;

        auto func = std::vector<RC>{};
        func.reserve(1);

        // Request pcgo (gas/oil capillary pressure (Pg-Po) in G/O system)
        func.push_back(RC{
            RC::Function::CapPress,
            RC::SubSystem::OilGas,
            Opm::ECLPhaseIndex::Vapour
        });

        const auto graph =
            sfunc.getSatFuncCurve(func, activeCell, useEPS);

        printGraph(std::cout, "pcgo", graph);
    }

    void pcow(const Opm::ECLSaturationFunc& sfunc,
              const int                     activeCell,
              const bool                    useEPS)
    {
        using RC = Opm::ECLSaturationFunc::RawCurve;

        auto func = std::vector<RC>{};
        func.reserve(1);

        // Request pcow (oil/water capillary pressure (Po-Pw) in O/W system)
        func.push_back(RC{
            RC::Function::CapPress,
            RC::SubSystem::OilWater,
            Opm::ECLPhaseIndex::Aqua
        });

        const auto graph =
            sfunc.getSatFuncCurve(func, activeCell, useEPS);

        printGraph(std::cout, "pcow", graph);
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

    // -----------------------------------------------------------------
    // Saturated states (RvSat(Pg) and RsSat(Po))

    void rvSat(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCurves,
               const int                                 activeCell)
    {
        using RC = Opm::ECLPVT::RawCurve;
        using PI = Opm::ECLPhaseIndex;

        const auto graph = pvtCurves
            .getPvtCurve(RC::SaturatedState, PI::Vapour, activeCell);

        printGraph(std::cout, "rvSat", graph);
    }

    void rsSat(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCurves,
               const int                                 activeCell)
    {
        using RC = Opm::ECLPVT::RawCurve;
        using PI = Opm::ECLPhaseIndex;

        const auto graph = pvtCurves
            .getPvtCurve(RC::SaturatedState, PI::Liquid, activeCell);

        printGraph(std::cout, "rsSat", graph);
    }

    // -----------------------------------------------------------------

    std::unique_ptr<const Opm::ECLUnits::UnitSystem>
    makeUnits(const std::string&          unit,
              const Opm::ECLInitFileData& init)
    {
        if ((unit == "si") || (unit == "SI") || (unit == "internal")) {
            return {};          // No conversion needed.
        }

        if ((unit == "metric") || (unit == "Metric") || (unit == "METRIC")) {
            return Opm::ECLUnits::metricUnitConventions();
        }

        if ((unit == "field") || (unit == "Field") || (unit == "FIELD")) {
            return Opm::ECLUnits::fieldUnitConventions();
        }

        if ((unit == "lab") || (unit == "Lab") || (unit == "LAB")) {
            return Opm::ECLUnits::labUnitConventions();
        }

        if ((unit == "pvt-m") || (unit == "PVT-M") || (unit == "PVTM")) {
            return Opm::ECLUnits::pvtmUnitConventions();
        }

        std::cerr << "Unit convention '" << unit << "' not recognized\n"
                  << "Using 'native' (input/serialised) conventions.\n";

        return Opm::ECLUnits::serialisedUnitConventions(init);
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

    auto sfunc = Opm::ECLSaturationFunc(graph, init, useEPS);
    auto pvtCC = Opm::ECLPVT::ECLPvtCurveCollection(graph, init);

    if (prm.has("unit")) {
        auto units = makeUnits(prm.get<std::string>("unit"), init);
        
        sfunc.setOutputUnits(units->clone());
        pvtCC.setOutputUnits(std::move(units));
    }

    // -----------------------------------------------------------------
    // Relative permeability

    if (prm.getDefault("krg" , false)) { krg (sfunc, cellID, useEPS); }
    if (prm.getDefault("krog", false)) { krog(sfunc, cellID, useEPS); }
    if (prm.getDefault("krow", false)) { krow(sfunc, cellID, useEPS); }
    if (prm.getDefault("krw" , false)) { krw (sfunc, cellID, useEPS); }

    // -----------------------------------------------------------------
    // Capillary pressure
    if (prm.getDefault("pcgo", false)) { pcgo(sfunc, cellID, useEPS); }
    if (prm.getDefault("pcow", false)) { pcow(sfunc, cellID, useEPS); }

    // -----------------------------------------------------------------
    // PVT Curves

    if (prm.getDefault("Bg"  , false)) { Bg  (pvtCC, cellID); }
    if (prm.getDefault("mu_g", false)) { mu_g(pvtCC, cellID); }
    if (prm.getDefault("Bo"  , false)) { Bo  (pvtCC, cellID); }
    if (prm.getDefault("mu_o", false)) { mu_o(pvtCC, cellID); }

    if (prm.getDefault("rvSat", false)) { rvSat(pvtCC, cellID); }
    if (prm.getDefault("rsSat", false)) { rsSat(pvtCC, cellID); }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}
