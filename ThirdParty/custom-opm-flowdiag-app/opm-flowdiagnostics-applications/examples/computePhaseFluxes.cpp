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
#include <opm/utility/ECLResultData.hpp>

#include <chrono>
#include <exception>
#include <ios>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/system/error_code.hpp>

namespace {
    template <class OStream, class Task>
    void timeIt(OStream& os, Task&& task)
    {
        const auto start = ::std::chrono::steady_clock::now();

        task();

        const auto stop = ::std::chrono::steady_clock::now();

        os << std::setprecision(3) << std::scientific
           << std::chrono::duration<double>(stop - start).count()
           << " [s]" << std::endl;
    }

    std::string phaseName(const Opm::ECLPhaseIndex p)
    {
        switch (p) {
        case Opm::ECLPhaseIndex::Aqua:   return "water";
        case Opm::ECLPhaseIndex::Liquid: return "oil";
        case Opm::ECLPhaseIndex::Vapour: return "gas";
        }

        throw std::invalid_argument {
            "Invalid Phase ID"
        };
    }

    int numDigits(const std::vector<int>& steps)
    {
        if (steps.empty()) {
            return 1;
        }

        const auto m =
            *std::max_element(std::begin(steps), std::end(steps));

        if (m == 0) {
            return 1;
        }

        assert (m > 0);

        return std::floor(std::log10(static_cast<double>(m))) + 1;
    }

    void openRestartSet(const Opm::ECLCaseUtilities::ResultSet& rset,
                        const int                               step,
                        std::unique_ptr<Opm::ECLRestartData>&   rstrt)
    {
        if (! (rset.isUnifiedRestart() && rstrt)) {
            // Not a unified restart file or this is the first time we're
            // seeing the result set.
            rstrt.reset(new Opm::ECLRestartData(rset.restartFile(step)));
        }
    }

    void savePhaseVector(const std::string&         quant,
                         const std::string&         phase,
                         const std::string&         type,
                         const int                  step,
                         const int                  ndgt,
                         const std::vector<double>& pflux)
    {
        namespace fs = boost::filesystem;

        const auto dir = fs::path{ quant } / phase;

        boost::system::error_code ec{};
        if (fs::create_directories(dir, ec) ||
            (ec.value() == boost::system::errc::errc_t::success))
        {
            auto fn = dir;
            {
                std::ostringstream os;

                os << type << '-'
                   << std::setw(ndgt) << std::setfill('0')
                   << step << ".txt";

                fn /= os.str();
            }

            fs::ofstream os(fn);
            if (os) {
                os.precision(16);
                os.setf(std::ios::scientific);

                for (const auto& qi : pflux) {
                    os << qi << '\n';
                }
            }
        }
    }

    void saveNeighbours(const ::Opm::ECLGraph& G)
    {
        std::ofstream os("neigh.txt");

        if (!os) { return; }

        const auto& neigh = G.neighbours();

        for (auto nconn = neigh.size() / 2,
                  conn  = 0*nconn; conn < nconn; ++conn)
        {
            os << neigh[2*conn + 0 ] << ' ' << neigh[2*conn + 1] << '\n';
        }
    }

    void computePhaseFluxes(const Opm::ECLGraph&                    G,
                            const Opm::ECLCaseUtilities::ResultSet& rset,
                            const Opm::ECLFluxCalc&                 fcalc,
                            const int                               step,
                            const int                               ndgt,
                            std::unique_ptr<Opm::ECLRestartData>&   rstrt)
    {
        openRestartSet(rset, step, rstrt);

        if (! (rstrt && rstrt->selectReportStep(step))) {
            std::cout << "    Failed (No Such Report Step)\n";
        }

        for (const auto& phase : G.activePhases()) {
            const auto pname = phaseName(phase);
            auto       pflux = std::vector<double>{};

            timeIt(std::cout, [&fcalc, &rstrt, phase, &pname, &pflux]()
            {
                pflux = fcalc.flux(*rstrt, phase);

                std::cout << "    - " << std::right
                          << std::setw(5) << std::setfill(' ')
                          << pname << ": ";
            });

            if (! pflux.empty()) {
                savePhaseVector("flux", pname, "calc", step, ndgt, pflux);
            }

            // Extract reference fluxes if available.
            pflux = G.flux(*rstrt, phase);
            if (! pflux.empty()) {
                savePhaseVector("flux", pname, "ref", step, ndgt, pflux);
            }
        }
    }
} // namespace Anonymous

int main(int argc, char* argv[])
try {
    const auto prm    = example::initParam(argc, argv);
    const auto grav   = prm.getDefault("grav"  , 9.80665);
    const auto useEPS = prm.getDefault("useEPS", false);

    const auto rset  = example::identifyResultSet(prm);
    const auto steps = rset.reportStepIDs();
    const auto ndgt  = numDigits(steps);
    const auto init  = Opm::ECLInitFileData(rset.initFile());
    const auto graph = Opm::ECLGraph::load(rset.gridFile(), init);
    const auto fcalc = Opm::ECLFluxCalc(graph, init, grav, useEPS);

    if (prm.getDefault("emitNeigh", false)) {
        saveNeighbours(graph);
    }

    auto rstrt = std::unique_ptr<Opm::ECLRestartData>{};

    for (const auto& step : steps) {
        timeIt(std::cout, [&rset, &graph, &rstrt, &fcalc, ndgt, step]()
        {
            std::cout << "Phase Fluxes for Report Step "
                      << std::setw(ndgt) << std::setfill('0')
                      << step << std::endl;

            computePhaseFluxes(graph, rset, fcalc, step, ndgt, rstrt);

            std::cout << "  - Step Time: ";
        });

        std::cout << std::endl;
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}
