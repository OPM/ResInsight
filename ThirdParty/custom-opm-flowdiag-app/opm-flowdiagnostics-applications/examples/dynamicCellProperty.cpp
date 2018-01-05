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
#include <opm/utility/ECLUnitHandling.hpp>

#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

namespace {
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
}

struct CellState
{
    CellState(const Opm::ECLGraph&                    G,
              const Opm::ECLCaseUtilities::ResultSet& rset,
              const int                               cellID_);

    int                 cellID;
    std::vector<double> time;
    std::vector<double> Po;
    std::vector<double> Rs;
    std::vector<double> Rv;
};

CellState::CellState(const Opm::ECLGraph&                    G,
                     const Opm::ECLCaseUtilities::ResultSet& rset,
                     const int                               cellID_)
    : cellID(cellID_)
{
    const auto rsteps = rset.reportStepIDs();

    this->time.reserve(rsteps.size());
    this->Po  .reserve(rsteps.size());
    this->Rs  .reserve(rsteps.size());
    this->Rv  .reserve(rsteps.size());

    auto rstrt = std::unique_ptr<Opm::ECLRestartData>{};

    for (const auto& step : rsteps) {
        openRestartSet(rset, step, rstrt);

        rstrt->selectReportStep(step);

        this->time.push_back(example::simulationTime(*rstrt));

        {
            const auto& press =
                G.rawLinearisedCellData<double>(*rstrt, "PRESSURE");

            this->Po.push_back(press[cellID]);
        }

        {
            const auto& R =
                G.rawLinearisedCellData<double>(*rstrt, "RS");

            this->Rs.push_back(R.empty() ? 0.0 : R[cellID]);
        }

        {
            const auto& R =
                G.rawLinearisedCellData<double>(*rstrt, "RV");

            this->Rv.push_back(R.empty() ? 0.0 : R[cellID]);
        }
    }
}

struct Property
{
    std::string         name;
    std::vector<double> data;
};

namespace {

    // -----------------------------------------------------------------------
    // Gas Properties

    std::vector<double>
    Bg(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCC,
       const CellState&                          x)
    {
        using RC = Opm::ECLPVT::RawCurve;
        using PI = Opm::ECLPhaseIndex;

        return pvtCC.getDynamicPropertyNative(RC::FVF, PI::Vapour,
                                              x.cellID, x.Po, x.Rv);
    }

    std::vector<double>
    mu_g(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCC,
         const CellState&                          x)
    {
        using RC = Opm::ECLPVT::RawCurve;
        using PI = Opm::ECLPhaseIndex;

        return pvtCC.getDynamicPropertyNative(RC::Viscosity, PI::Vapour,
                                              x.cellID, x.Po, x.Rv);
    }

    // -----------------------------------------------------------------------
    // Oil Properties

    std::vector<double>
    Bo(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCC,
       const CellState&                          x)
    {
        using RC = Opm::ECLPVT::RawCurve;
        using PI = Opm::ECLPhaseIndex;

        return pvtCC.getDynamicPropertyNative(RC::FVF, PI::Liquid,
                                              x.cellID, x.Po, x.Rs);
    }

    std::vector<double>
    mu_o(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCC,
         const CellState&                          x)
    {
        using RC = Opm::ECLPVT::RawCurve;
        using PI = Opm::ECLPhaseIndex;

        return pvtCC.getDynamicPropertyNative(RC::Viscosity, PI::Liquid,
                                              x.cellID, x.Po, x.Rs);
    }

    // ---------------------------------------------------------------------
    // Command-line argument processing and property output.

    using DynProp = std::vector<double>
        (*)(const Opm::ECLPVT::ECLPvtCurveCollection& pvtCC,
            const CellState&                          x);

    std::map<std::string, DynProp> enumerateProperties()
    {
        return {
            { "Bg"   , &Bg   },
            { "mu_g" , &mu_g },
            { "Bo"   , &Bo   },
            { "mu_o" , &mu_o },
        };
    }

    void writeVector(const std::vector<double>& x, const std::string& var)
    {
        std::cout << var << " = [\n";

        for (const auto& xi : x) {
            std::cout << "  " << xi << '\n';
        }

        std::cout << "]\n\n";
    }

    void writeResults(const CellState& x, const std::vector<Property>& props)
    {
        writeVector(x.time, "time");
        writeVector(x.Po  , "Po");
        writeVector(x.Rs  , "Rs");
        writeVector(x.Rv  , "Rv");

        for (const auto& prop : props) {
            writeVector(prop.data, prop.name);
        }
    }

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
    const auto cellID = prm.getDefault("cell", 0);

    const auto rset  = example::identifyResultSet(prm);
    const auto init  = Opm::ECLInitFileData(rset.initFile());
    const auto graph = Opm::ECLGraph::load(rset.gridFile(), init);

    auto pvtCC = Opm::ECLPVT::ECLPvtCurveCollection(graph, init);
    if (prm.has("unit")) {
        pvtCC.setOutputUnits(makeUnits(prm.get<std::string>("unit"), init));
    }

    const auto x = CellState{ graph, rset, cellID };

    auto props = std::vector<Property>{};

    for (const auto& prop : enumerateProperties()) {
        if (prm.getDefault(prop.first, false)) {
            props.push_back(Property{ prop.first, (*prop.second)(pvtCC, x) });
        }
    }

    if (! props.empty()) {
        std::cout.precision(16);
        std::cout.setf(std::ios_base::scientific);

        writeResults(x, props);
    }
}
catch (const std::exception& e) {
    std::cerr << "Caught Exception: " << e.what() << '\n';

    return EXIT_FAILURE;
}
