/*
  Copyright 2021 Equinor

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

#define BOOST_TEST_MODULE Aggregate_Aquifer_Data

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateAquiferData.hpp>

#include <opm/output/data/Aquifer.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/aquifer.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/input/eclipse/EclipseState/Aquifer/Aquancon.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/AquiferCT.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/Aquifetp.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/AquiferConfig.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/EclipseState/Tables/FlatTable.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/common/utility/TimeService.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

    class AquiferConnections
    {
    public:
        using Connections = std::vector<Opm::Aquancon::AquancCell>;
        using AllConnections = std::unordered_map<int, Connections>;

        void addConnection(const int                   aquiferID,
                           const std::size_t           cartesianCell,
                           const double                influxCoefficient,
                           const double                effectiveFaceArea,
                           const Opm::FaceDir::DirEnum direction)
        {
            this->all_connections_[aquiferID]
                .emplace_back(aquiferID, cartesianCell, influxCoefficient,
                              effectiveFaceArea, direction);
        }

        const AllConnections& getAllConnections() const
        {
            return this->all_connections_;
        }

    private:
        AllConnections all_connections_;
    };

    double prodIndexUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().to_si(M::liquid_productivity_index, 1.0);
    }

    double temperatureUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().to_si(M::temperature, 1.0);
    }

    double pressureUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().to_si(M::pressure, 1.0);
    }

    double compressibilityUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().from_si(M::pressure, 1.0);
    }

    double volumeUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().to_si(M::liquid_surface_volume, 1.0);
    }

    double depthUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().to_si(M::length, 1.0);
    }

    double permeabilityUnit()
    {
        using M = Opm::UnitSystem::measure;
        return Opm::UnitSystem::newMETRIC().to_si(M::permeability, 1.0);
    }

    Opm::TableManager waterProperties()
    {
        const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
TABDIMS
  1  2  50  60  2 60 /

AQUDIMS
  1*  1*  3  100  3  1000 /

PROPS
PVTW
  279.0  1.038  5.0E-05  0.318  0.0001 /
  279.0  1.038  5.0E-05  0.118  0.0001 /

DENSITY
  856.50000  1053.0  0.85204 /
  856.50000  1033.0  0.85204 /

AQUTAB
   0.01   0.112
1000.00 666.982 /

   0.01   0.112
1000.00 250.578 /
)");

        return Opm::TableManager { deck };
    }

    std::pair<std::vector<double>, std::vector<double>> CTInfluenceFunction_1()
    {
        return {
            std::vector<double> { // tD
                0.010, 0.050, 0.100, 0.150, 0.200, 0.250, 0.300, 0.400,
                0.500, 0.600, 0.700, 0.800, 0.900, 1.000, 1.500, 2.000,
                2.500, 3.000, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000,
                10.00, 15.00, 20.00, 25.00, 30.00, 40.00, 50.00, 60.00,
                70.00, 80.00, 90.00, 100.0, 150.0, 200.0, 250.0, 300.0,
                400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000,
            }
            ,
            std::vector<double> { // pD
                0.112, 0.229, 0.315, 0.376, 0.424, 0.469, 0.503, 0.564,
                0.616, 0.659, 0.702, 0.735, 0.772, 0.802, 0.927, 1.020,
                1.101, 1.169, 1.275, 1.362, 1.436, 1.500, 1.556, 1.604,
                1.651, 1.829, 1.960, 2.067, 2.147, 2.282, 2.388, 2.476,
                2.550, 2.615, 2.672, 2.723, 2.921, 3.064, 3.173, 3.263,
                3.406, 3.516, 3.608, 3.684, 3.750, 3.809, 3.86,
            }
        };
    }

    std::pair<std::vector<double>, std::vector<double>> CTInfluenceFunction_3()
    {
        return {
            std::vector<double> { // tD
                0.01, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.40, 0.50, 0.52,
                0.54, 0.56, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95,
                1.00, 1.20, 1.40, 1.60, 2.00, 3.00, 4.00, 5.00, 10.00,
                20.00, 30.00, 50.00, 100.00, 200.00, 300.00, 500.00,
                1000.00,
            },
            std::vector<double> { // pD
                0.112, 0.229, 0.315, 0.376, 0.424, 0.469, 0.503, 0.564, 0.616,
                0.627, 0.636, 0.645, 0.662, 0.683, 0.703, 0.721, 0.740, 0.758,
                0.776, 0.791, 0.806, 0.865, 0.920, 0.973, 1.076, 1.328, 1.578,
                1.828, 3.078, 5.578, 8.078, 13.078, 25.578, 50.578, 75.578,
                125.578, 250.578,
            }
        };
    }

    void connectCarterTracy(AquiferConnections& aquancon)
    {
        using FDir = Opm::FaceDir::DirEnum;

        {
            const auto aquiferID = 1;
            const auto cartCell1 = std::size_t{699}; // one-based IJK = (20,5,7)
            const auto cartCell2 = std::size_t{799}; // one-based IJK = (20,5,8)
            const auto cartCell3 = std::size_t{899}; // one-based IJK = (20,5,9)
            const auto effFaceArea = 4.0 * 0.2;     // DY * DZ
            const auto influxCoeff = effFaceArea;

            aquancon.addConnection(aquiferID, cartCell1, 1.0*influxCoeff,  6.0*effFaceArea, FDir::XMinus);
            aquancon.addConnection(aquiferID, cartCell2, 2.0*influxCoeff, 12.0*effFaceArea, FDir::YMinus);
            aquancon.addConnection(aquiferID, cartCell3, 3.0*influxCoeff, 18.0*effFaceArea, FDir::ZMinus);
        }

        {
            const auto aquiferID = 6;
            const auto cartCell = std::size_t{579}; // one-based IJK = (20,4,6)
            const auto effFaceArea = 4.0 * 0.2;     // DY * DZ
            const auto influxCoeff = 1.0;           // [m^2]

            aquancon.addConnection(aquiferID, cartCell, influxCoeff, effFaceArea, FDir::XPlus);
        }
    }

    Opm::AquiferCT createCarterTracy()
    {
        const auto porosity        = 0.3;
        const auto compr           = 3.0e-5*compressibilityUnit();
        const auto datumDepth      = 2000.0*depthUnit();
        const auto initialPressure = 269.0*pressureUnit();
        const auto initialTemperature = 50*temperatureUnit();

        const auto angle = 360.0; // degrees
        const auto thickness = 10.0*depthUnit();

        auto properties = std::vector<Opm::AquiferCT::AQUCT_data>{};

        {
            const auto aquiferID = 1;
            const auto influenceFunction = 1;
            const auto pvtTable = 2;
            const auto ro = 800.0*depthUnit();
            const auto ka = 1000.0*permeabilityUnit();

            const auto& [tD, pD] = CTInfluenceFunction_1();

            auto& ct = properties
                .emplace_back(aquiferID, influenceFunction, pvtTable,
                              porosity, datumDepth, compr, ro, ka,
                              thickness, angle / 360.0, initialPressure, initialTemperature);

            ct.dimensionless_time = tD;
            ct.dimensionless_pressure = pD;

            ct.finishInitialisation(waterProperties());
        }

        {
            const auto aquiferID = 6;
            const auto influenceFunction = 3;
            const auto pvtTable = 1;
            const auto ro = 900.0*depthUnit();
            const auto ka = 5000.0*permeabilityUnit();

            const auto& [tD, pD] = CTInfluenceFunction_3();

            auto& ct = properties
                .emplace_back(aquiferID, influenceFunction, pvtTable,
                              porosity, datumDepth, compr, ro, ka,
                              thickness, angle / 360.0, initialPressure, initialTemperature);

            ct.dimensionless_time = tD;
            ct.dimensionless_pressure = pD;

            ct.finishInitialisation(waterProperties());
        }

        return Opm::AquiferCT(properties);
    }

    void connectFetkovic(AquiferConnections& aquancon)
    {
        using FDir = Opm::FaceDir::DirEnum;

        {
            const auto aquiferID = 2;
            const auto cartCell = std::size_t{619}; // one-based IJK = (20,1,7)
            const auto effFaceArea = 4.0 * 0.2;     // DY * DZ
            const auto influxCoeff = effFaceArea;

            aquancon.addConnection(aquiferID, cartCell, influxCoeff, effFaceArea, FDir::YPlus);
        }

        {
            const auto aquiferID = 4;
            const auto cartCell = std::size_t{419}; // one-based IJK = (20,1,5)
            const auto effFaceArea = 4.0 * 0.2;     // DY * DZ
            const auto influxCoeff = 1.0;           // [m^2]

            aquancon.addConnection(aquiferID, cartCell, influxCoeff, effFaceArea, FDir::ZPlus);
        }
    }

    Opm::Aquifetp createFetkovich()
    {
        const auto compr           = 1.5312e-4*compressibilityUnit();
        const auto datumDepth      = 2000.0*depthUnit();
        const auto initialPressure = 250.0*pressureUnit();

        auto properties = std::vector<Opm::Aquifetp::AQUFETP_data>{};

        {
            const auto aquiferID     = 2;
            const auto pvtTable      = 2;
            const auto prodIndex     = 495.0*prodIndexUnit();
            const auto initialVolume = 5.0e+10*volumeUnit();

            auto& fetp = properties
                .emplace_back(aquiferID, pvtTable, prodIndex, compr,
                              initialVolume, datumDepth, initialPressure);

            fetp.finishInitialisation(waterProperties());
        }

        {
            const auto aquiferID     = 4;
            const auto pvtTable      = 1;
            const auto prodIndex     = 910.0*prodIndexUnit();
            const auto initialVolume = 2.0e+10*volumeUnit();

            auto& fetp = properties
                .emplace_back(aquiferID, pvtTable, prodIndex, compr,
                              initialVolume, datumDepth, initialPressure);

            fetp.finishInitialisation(waterProperties());
        }

        return Opm::Aquifetp(properties);
    }

    Opm::AquiferConfig createAquiferConfig()
    {
        auto aquancon = AquiferConnections{};
        connectCarterTracy(aquancon);
        connectFetkovic(aquancon);

        return {
            createFetkovich(), createCarterTracy(), Opm::Aquancon(aquancon.getAllConnections())
        };
    }

    Opm::AquiferConfig createNumericAquiferConfig()
    {
        const auto deck = Opm::Parser{}.parseString(R"(
START             -- 0
10 MAY 2007 /
RUNSPEC

DIMENS
 10 10 10 /
REGDIMS
  3/
AQUDIMS
4 4 1* 1* 3 200 1* 1* /

GRID
DXV
   10*400 /
DYV
   10*400 /
DZV
   10*400 /
TOPS
   100*2202 /
PERMX
  1000*0.25 /
COPY
  PERMX PERMY /
  PERMX PERMZ /
/
PORO
   1000*0.15 /
AQUNUM
--ID      I J K     Area    Len   Phi  K     Depth  P0  PVT  SAT
  1       1 1 1      15000  5000  0.3  30    2700   285        /
  1       2 1 1     160000  6000  0.4  400   2705   295 1*   3 /
  1       3 1 1     150000  7000  0.5  5000  2710   1*  2      /
  2       4 1 1     140000  9000  0.3  300   2715   250 2    3 / aq cell
/
AQUCON
-- #    I1 I2  J1 J2   K1  K2    Face
   1    1  1   16 18   19  20   'I-'    /
   1    2  2   16 18   19  20   'I-'    /
   1    3  3   16 18   19  20   'I-'    /
   2    4  4   16 18   19  20   'I-'    /
/

END
)");

        return Opm::EclipseState { deck }.aquifer();
    }

    Opm::EclipseGrid createGrid()
    {
        auto grid = Opm::EclipseGrid { 20, 5, 10, 5.0, 4.0, 0.2 };

        auto actnum = std::vector<int>(grid.getCartesianSize(), 1);

        actnum[grid.getGlobalIndex( 1, 1, 1)] = 0;
        actnum[grid.getGlobalIndex( 2, 1, 1)] = 0;
        actnum[grid.getGlobalIndex(19, 1, 6)] = 0;
        actnum[grid.getGlobalIndex(19, 2, 6)] = 0;

        grid.resetACTNUM(actnum);

        return grid;
    }

    Opm::RestartIO::InteHEAD::AquiferDims syntheticAquiferDimensions()
    {
        auto aqDims = Opm::RestartIO::InteHEAD::AquiferDims{};

        aqDims.numAquifers = 4; // 1, 2, 4, 6
        aqDims.maxNumAquifers = 10; // >= 6
        aqDims.maxNumAquiferConn = 5;       // >= 3
        aqDims.maxNumActiveAquiferConn = 3; // ID = 1
        aqDims.maxAquiferID = 6;

        return aqDims;
    }

    Opm::RestartIO::InteHEAD::AquiferDims syntheticNumericAquiferDimensions()
    {
        auto aqDims = Opm::RestartIO::InteHEAD::AquiferDims{};

        aqDims.numNumericAquiferRecords = 4;

        return aqDims;
    }

    Opm::RestartIO::InteHEAD::AquiferDims parseAquiferDimensions()
    {
        const auto deck = Opm::Parser{}.parseString(R"(RUNSPEC
DIMENS
  20 5 10 /

AQUDIMS
-- MXNAQN   MXNAQC   NIFTBL  NRIFTB   NANAQU    NNCAMAX
    4       4        5       100      5         1000 /

GRID

DXV
  20*100.0 /

DYV
  5*50.0 /

DZV
  10*10.0 /

DEPTHZ
  126*2000.0 /

PORO
  1000*0.25 /

EQUALS
  'PORO' 0.0 2 3 2 2 2 2 /
  'PORO' 0.0 20 20 2 3 7 7 /
/
AQUNUM
  4       1 1 1      15000  5000  0.3  30  2700  / aq cell
  5       2 1 1     150000  9000  0.3  30  2700  / aq cell
  6       3 1 1     150000  9000  0.3  30  2700  / aq cell
  7       4 1 1     150000  9000  0.3  30  2700  / aq cell
/
AQUCON
-- #    I1 I2  J1 J2   K1  K2    Face
   4    1  1   16 18   19  20   'I-'    / connecting cells
   5    2  2   16 18   19  20   'I-'    / connecting cells
   6    3  3   16 18   19  20   'I-'    / connecting cells
   7    4  4   16 18   19  20   'I-'    / connecting cells
/

SOLUTION

AQUFETP
-- Aqu        depth     Pr      vol        Comp         PI        PVTW
   2           2000.0   250.0   5.0E+10   1.5312E-4    495         2     /
   4           2000.0   250.0   2.0E+10   1.5312E-4    910         1     /
/

AQUANCON
-- Aq#     I1   I2    J1   J2   K1  K2  FACE
    2      20   20     1    5    7   9   'I+'  1*   1.0 NO /
    4      20   20     1    5    5   6   'I+'  1.0  1.0 NO /
/

)");

        const auto es = Opm::EclipseState{ deck };

        return Opm::RestartIO::inferAquiferDimensions(es);
    }

    Opm::SummaryState sim_state()
    {
        auto state = Opm::SummaryState{Opm::TimeService::now()};

        state.update("AAQP:1", 123.456);
        state.update("AAQR:1", 234.567);
        state.update("AAQT:1", 3456.789);
        state.update("AAQPD:1", 4.567);
        state.update("AAQTD:1", 50.607);

        state.update("AAQP:2", 121.212);
        state.update("AAQR:2", 222.333);
        state.update("AAQT:2", 333.444);

        state.update("AAQP:4", 555.444);
        state.update("AAQR:4", 333.222);
        state.update("AAQT:4", 222.111);

        state.update("AAQP:6", 456.123);
        state.update("AAQR:6", 34.567);
        state.update("AAQT:6", 4444.5555);
        state.update("AAQPD:6", 50.706);
        state.update("AAQTD:6", 100.321);

        return state;
    }

    Opm::SummaryState aqunum_sim_state()
    {
        auto state = Opm::SummaryState{Opm::TimeService::now()};

        state.update("ANQP:1", 123.456);
        state.update("ANQR:1", 234.567);
        state.update("ANQT:1", 3456.789);

        state.update("ANQP:2", 121.212);
        state.update("ANQR:2", 222.333);
        state.update("ANQT:2", 333.444);

        return state;
    }

    Opm::data::Aquifers aquiferValues(const Opm::AquiferConfig& aquConfig)
    {
        auto aquiferValues = Opm::data::Aquifers{};

        for (const auto& aquct : aquConfig.ct()) {
            auto& aquifer = aquiferValues[aquct.aquiferID];

            aquifer.aquiferID = aquct.aquiferID;
            aquifer.initPressure = aquct.initial_pressure.value();
            aquifer.datumDepth = aquct.datum_depth;

            auto* aquCT = aquifer.typeData.create<Opm::data::AquiferType::CarterTracy>();

            aquCT->timeConstant = aquct.timeConstant();
            aquCT->influxConstant = aquct.influxConstant();
            aquCT->waterDensity = aquct.waterDensity();
            aquCT->waterViscosity = aquct.waterViscosity();
        }

        for (const auto& aqufetp : aquConfig.fetp()) {
            auto& aquifer = aquiferValues[aqufetp.aquiferID];

            aquifer.aquiferID = aqufetp.aquiferID;
            aquifer.initPressure = aqufetp.initial_pressure.value();
            aquifer.datumDepth = aqufetp.datum_depth;

            auto* aquFet = aquifer.typeData.create<Opm::data::AquiferType::Fetkovich>();

            aquFet->initVolume = aqufetp.initial_watvolume;
            aquFet->prodIndex = aqufetp.prod_index;
            aquFet->timeConstant = aqufetp.timeConstant();
        }

        return aquiferValues;
    }

    Opm::data::Aquifers numericAquiferValues(const Opm::AquiferConfig& aquConfig)
    {
        const auto equilibratedAquiferCellPressure = 300.0*pressureUnit();

        auto aquiferValues = Opm::data::Aquifers{};

        for (const auto& [aquiferID, aquNum] : aquConfig.numericalAquifers().aquifers()) {
            const auto numCells = aquNum.numCells();

            auto& aquifer = aquiferValues[aquiferID];
            aquifer.aquiferID = static_cast<int>(aquiferID);

            auto* aquNumData = aquifer.typeData.create<Opm::data::AquiferType::Numerical>();
            aquNumData->initPressure.reserve(numCells);

            for (auto cellIndex = 0*numCells; cellIndex < numCells; ++cellIndex) {
                const auto* aqCell = aquNum.getCellPrt(cellIndex);

                const auto p0 = aqCell->init_pressure.has_value()
                    ? aqCell->init_pressure.value()
                    : equilibratedAquiferCellPressure;

                aquNumData->initPressure.push_back(p0);
            }
        }

        return aquiferValues;
    }

    template <class Coll1, class Coll2>
    void check_is_close(const Coll1& coll1, const Coll2& coll2, const double tol)
    {
        BOOST_REQUIRE_EQUAL(std::size(coll1), std::size(coll2));

        if (coll1.empty()) { return; }

        auto c1 = std::begin(coll1);
        auto e1 = std::end  (coll1);
        auto c2 = std::begin(coll2);
        for (; c1 != e1; ++c1, ++c2) {
            BOOST_CHECK_CLOSE(*c1, *c2, tol);
        }
    }
} // namespace

BOOST_AUTO_TEST_SUITE(Aggregate_Aquifer_Data)

BOOST_AUTO_TEST_CASE(AquiferDimensions)
{
    const auto aqDims = parseAquiferDimensions();

    BOOST_CHECK_EQUAL(aqDims.numAquifers, 2); // 2 unique aquifer IDs (2 and 4)
    BOOST_CHECK_EQUAL(aqDims.maxNumAquifers, 5);           // AQUDIMS(5)
    BOOST_CHECK_EQUAL(aqDims.maxNumAquiferConn, 1000);     // AQUDIMS(6)
    BOOST_CHECK_EQUAL(aqDims.maxNumActiveAquiferConn, 13); // In aquifer ID 2
    BOOST_CHECK_EQUAL(aqDims.maxAquiferID, 4);             // Maximum aquifer ID
    BOOST_CHECK_EQUAL(aqDims.numNumericAquiferRecords, 4); // Number of lines of AQUNUM data

    // # Data items per analytic aquifer
    BOOST_CHECK_EQUAL(aqDims.numIntAquiferElem, 18);       // # Integer
    BOOST_CHECK_EQUAL(aqDims.numRealAquiferElem, 24);      // # Single precision
    BOOST_CHECK_EQUAL(aqDims.numDoubAquiferElem, 10);      // # Double precision

    // # Data items per analytic aquifer *connection*
    BOOST_CHECK_EQUAL(aqDims.numIntConnElem, 7);           // # Integer
    BOOST_CHECK_EQUAL(aqDims.numRealConnElem, 2);          // # Single precision
    BOOST_CHECK_EQUAL(aqDims.numDoubConnElem, 4);          // # Double precision

    // # Data items per numeric aquifer
    BOOST_CHECK_EQUAL(aqDims.numNumericAquiferIntElem, 10);    // # Integer
    BOOST_CHECK_EQUAL(aqDims.numNumericAquiferDoubleElem, 13); // # Double precision
}

BOOST_AUTO_TEST_CASE(Static_Information_Analytic_Aquifers)
{
    const auto aqDims = syntheticAquiferDimensions();
    const auto aqConfig = createAquiferConfig();

    const auto aquiferData = Opm::RestartIO::Helpers::
        AggregateAquiferData{ aqDims, aqConfig, createGrid() };

    BOOST_CHECK_EQUAL(aquiferData.maximumActiveAnalyticAquiferID(), 6);

    // ICAQ:1
    {
        const auto expect = std::vector<int> {
            // Connection 0
            20,  5,  7, 993,   1,   0, 0,
            // Connection 1
            20,  5,  8, 994,   3,   0, 0,
            // Connection 2
            20,  5,  9, 995,   5,   0, 0,
        };

        const auto& icaq = aquiferData.getIntegerAquiferConnectionData(1);

        BOOST_CHECK_EQUAL_COLLECTIONS(icaq.begin(), icaq.end(), expect.begin(), expect.end());
    }

    // SCAQ:1
    {
        const auto expect = std::vector<float> {
            // Connection 0
            1.0f/6.0f,  1.0f,
            // Connection 1
            1.0f/3.0f,  2.0f,
            // Connection 2
            1.0f/2.0f,  3.0f,
        };

        const auto& scaq = aquiferData.getSinglePrecAquiferConnectionData(1);

        check_is_close(scaq, expect, 1.0e-7);
    }

    // ICAQ:2
    {
        const auto expect = std::vector<int> {
            // Connection 0
            20,  1,  7, 955,   4,   0, 0,
            // Connection 1 (nonexistent)
            0, 0, 0, 0, 0, 0, 0,
            // Connection 2 (nonexistent)
            0, 0, 0, 0, 0, 0, 0,
        };

        const auto& icaq = aquiferData.getIntegerAquiferConnectionData(2);

        BOOST_CHECK_EQUAL_COLLECTIONS(icaq.begin(), icaq.end(), expect.begin(), expect.end());
    }

    // SCAQ:2
    {
        const auto expect = std::vector<float> {
            // Connection 0
            1.0f,  1.0f,
            // Connection 1 (nonexistent)
            0.0f,  0.0f,
            // Connection 2 (nonexistent)
            0.0f,  0.0f,
        };

        const auto& scaq = aquiferData.getSinglePrecAquiferConnectionData(2);

        check_is_close(scaq, expect, 1.0e-7);
    }

    // ICAQ:3 (not activated/connected)
    {
        const auto expect = std::vector<int> {
            // Connection 0
            0, 0, 0, 0, 0, 0, 0,
            // Connection 1
            0, 0, 0, 0, 0, 0, 0,
            // Connection 2
            0, 0, 0, 0, 0, 0, 0,
        };

        const auto& icaq = aquiferData.getIntegerAquiferConnectionData(3);

        BOOST_CHECK_EQUAL_COLLECTIONS(icaq.begin(), icaq.end(), expect.begin(), expect.end());
    }

    // SCAQ:3 (not activated/connected)
    {
        const auto expect = std::vector<float> {
            // Connection 0
            0.0f,  0.0f,
            // Connection 1
            0.0f,  0.0f,
            // Connection 2
            0.0f,  0.0f,
        };

        const auto& scaq = aquiferData.getSinglePrecAquiferConnectionData(3);

        check_is_close(scaq, expect, 1.0e-7);
    }

    // ICAQ:4
    {
        const auto expect = std::vector<int> {
            // Connection 0
            20,  1,  5, 953,   6,   0, 0,
            // Connection 1 (nonexistent)
            0, 0, 0, 0, 0, 0, 0,
            // Connection 2 (nonexistent)
            0, 0, 0, 0, 0, 0, 0,
        };

        const auto& icaq = aquiferData.getIntegerAquiferConnectionData(4);

        BOOST_CHECK_EQUAL_COLLECTIONS(icaq.begin(), icaq.end(), expect.begin(), expect.end());
    }

    // SCAQ:4
    {
        const auto expect = std::vector<float> {
            // Connection 0
            1.0f,  0.8f,
            // Connection 1 (nonexistent)
            0.0f,  0.0f,
            // Connection 2 (nonexistent)
            0.0f,  0.0f,
        };

        const auto& scaq = aquiferData.getSinglePrecAquiferConnectionData(4);

        check_is_close(scaq, expect, 1.0e-7);
    }

    // ICAQ:5 (not activated/connected)
    {
        const auto expect = std::vector<int> {
            // Connection 0
            0, 0, 0, 0, 0, 0, 0,
            // Connection 1
            0, 0, 0, 0, 0, 0, 0,
            // Connection 2
            0, 0, 0, 0, 0, 0, 0,
        };

        const auto& icaq = aquiferData.getIntegerAquiferConnectionData(5);

        BOOST_CHECK_EQUAL_COLLECTIONS(icaq.begin(), icaq.end(), expect.begin(), expect.end());
    }

    // SCAQ:5 (not activated/connected)
    {
        const auto expect = std::vector<float> {
            // Connection 0
            0.0f,  0.0f,
            // Connection 1
            0.0f,  0.0f,
            // Connection 2
            0.0f,  0.0f,
        };

        const auto& scaq = aquiferData.getSinglePrecAquiferConnectionData(5);

        check_is_close(scaq, expect, 1.0e-7);
    }

    // ICAQ:6
    {
        const auto expect = std::vector<int> {
            // Connection 0
            20,  4,  6, 982,   2,   0, 0,
            // Connection 1 (nonexistent)
            0, 0, 0, 0, 0, 0, 0,
            // Connection 2 (nonexistent)
            0, 0, 0, 0, 0, 0, 0,
        };

        const auto& icaq = aquiferData.getIntegerAquiferConnectionData(6);

        BOOST_CHECK_EQUAL_COLLECTIONS(icaq.begin(), icaq.end(), expect.begin(), expect.end());
    }

    // SCAQ:6
    {
        const auto expect = std::vector<float> {
            // Connection 0
            1.0f,  0.8f,
            // Connection 1 (nonexistent)
            0.0f,  0.0f,
            // Connection 2 (nonexistent)
            0.0f,  0.0f,
        };

        const auto& scaq = aquiferData.getSinglePrecAquiferConnectionData(6);

        check_is_close(scaq, expect, 1.0e-7);
    }
}

BOOST_AUTO_TEST_CASE(Dynamic_Information_Analytic_Aquifers)
{
    const auto aqDims = syntheticAquiferDimensions();
    const auto aqConfig = createAquiferConfig();

    auto aquiferData = Opm::RestartIO::Helpers::
        AggregateAquiferData{ aqDims, aqConfig, createGrid() };

    aquiferData.captureDynamicdAquiferData(aqConfig, aquiferValues(aqConfig), sim_state(),
                                           Opm::UnitSystem::newMETRIC());

    // IAAQ
    {
        const auto expect = std::vector<int> {
            // Aquifer 1
            3, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
            // Aquifer 2
            1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            // Aquifer 3
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // Aquifer 4
            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            // Aquifer 5
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // Aquifer 6
            1, 1, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 0, 0, 0, 0, 0, 0,
        };

        const auto& iaaq = aquiferData.getIntegerAquiferData();

        BOOST_CHECK_EQUAL_COLLECTIONS(iaaq.begin(), iaaq.end(), expect.begin(), expect.end());
    }

    // SAAQ
    {
        const auto expect = std::vector<float> {
            // Aquifer 1
            3.0e-5f, 800.0f, 1000.0f, 0.3f, 269.0f, 2000.0f, 10.0f, 1.0f,  //  0.. 7
            994.6857f, 0.117882f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,      //  8..15
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                // 16..23

            // Aquifer 2
            1.5312e-4f, 5.0e10f, 495.0f, 1.546666666666667e+04f, 250.0f, 2000.0f, 0.0f, 0.0f, //  0.. 7 (24..31)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  8..15 (32..39)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 16..23 (40..47)

            // Aquifer 3
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  0.. 7 (48..55)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  8..15 (56..63)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 16..23 (64..71)

            // Aquifer 4
            1.5312e-4f, 2.0e10f, 910.0f, 3.365274725274725e+03f, 250.0f, 2000.0f, 0.0f, 0.0f, //  0.. 7 (72..79)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  8..15 (80..87)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 16..23 (88..95)

            // Aquifer 5
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  0.. 7 ( 96..103)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  8..15 (104..111)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 16..23 (112..119)

            // Aquifer 6
            3.0e-5f, 900.0f, 5000.0f, 0.3f, 269.0f, 2000.0f, 10.0f, 1.0f, //  0.. 7 (120..127)
            1013.943893f, 0.317682f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //  8..15 (128..135)
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 16..23 (136..143)
        };

        const auto& saaq = aquiferData.getSinglePrecAquiferData();

        check_is_close(saaq, expect, 1.0e-7);
    }

    // XAAQ
    {
        const auto expect = std::vector<double> {
            // Aquifer 1
            234.567, 123.456, 3456.789, 4.8, 12.558192939329325, 361.9008, 0.0, 0.0, 50.607, 4.567,

            // Aquifer 2
            222.333, 121.212, 333.444, 0.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,

            // Aquifer 3
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,

            // Aquifer 4
            333.222, 555.444, 222.111, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,

            // Aquifer 5
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,

            // Aquifer 6
            34.567, 456.123, 4444.5555, 1.0, 18.409712143375852, 458.0307, 0.0, 0.0, 100.321, 50.706,
        };

        const auto& xaaq = aquiferData.getDoublePrecAquiferData();

        check_is_close(xaaq, expect, 1.0e-7);
    }
}

BOOST_AUTO_TEST_CASE(Dynamic_Information_Numeric_Aquifers)
{
    const auto aqDims = syntheticNumericAquiferDimensions();
    const auto aqConfig = createNumericAquiferConfig();

    BOOST_REQUIRE_MESSAGE(aqConfig.hasNumericalAquifer(), "Aquifer configuration object must have numeric aquifers");

    auto aquiferData = Opm::RestartIO::Helpers::
        AggregateAquiferData{ aqDims, aqConfig, createGrid() };

    {
        BOOST_CHECK_EQUAL(aquiferData.maximumActiveAnalyticAquiferID(), 0);
        BOOST_CHECK_MESSAGE(aquiferData.getIntegerAquiferData().empty(), "IAAQ must be empty");
        BOOST_CHECK_MESSAGE(aquiferData.getSinglePrecAquiferData().empty(), "SAAQ must be empty");
        BOOST_CHECK_MESSAGE(aquiferData.getDoublePrecAquiferData().empty(), "XAAQ must be empty");

        BOOST_CHECK_EQUAL(aquiferData.getNumericAquiferIntegerData().size(), 4u * 10);
        BOOST_CHECK_EQUAL(aquiferData.getNumericAquiferDoublePrecData().size(), 4u * 13);
    }

    aquiferData.captureDynamicdAquiferData(aqConfig,
                                           numericAquiferValues(aqConfig),
                                           aqunum_sim_state(),
                                           Opm::UnitSystem::newMETRIC());

    // IAQN
    {
        const auto expect = std::vector<int> {
            1,  1, 1, 1,  1, 1, 0, 0, 0, 0, //  0.. 9 (record 0)
            1,  2, 1, 1,  1, 3, 0, 0, 0, 0, // 10..19 (record 1)
            1,  3, 1, 1,  2, 1, 0, 0, 0, 0, // 20..29 (record 2)
            2,  4, 1, 1,  2, 3, 0, 0, 0, 0, // 30..39 (record 3)
        };

        const auto& iaqn = aquiferData.getNumericAquiferIntegerData();
        BOOST_CHECK_EQUAL_COLLECTIONS(iaqn.begin(), iaqn.end(), expect.begin(), expect.end());
    }

    // RAQN
    {
        const auto pv = std::vector<double> {
             15.0e3 * 5.0e3 * 0.3,
            160.0e3 * 6.0e3 * 0.4,
            150.0e3 * 7.0e3 * 0.5,
            140.0e3 * 9.0e3 * 0.3,
        };

        const auto expect = std::vector<double> {
             15.0e3, 5.0e3, 0.3,   30.0, 2700.0, 285.0,   1.0, 1.0, 1.0,   pv[0], 234.567, 3456.789, 123.456, //  0..12 (record 0)
            160.0e3, 6.0e3, 0.4,  400.0, 2705.0, 295.0,   1.0, 1.0, 1.0,   pv[1],   0.0  ,    0.0  ,   0.0  , // 13..25 (record 1)
            150.0e3, 7.0e3, 0.5, 5000.0, 2710.0, 300.0,   1.0, 1.0, 1.0,   pv[2],   0.0  ,    0.0  ,   0.0  , // 26..38 (record 2)
            140.0e3, 9.0e3, 0.3,  300.0, 2715.0, 250.0,   1.0, 1.0, 1.0,   pv[3], 222.333,  333.444, 121.212, // 39..51 (record 3)
        };

        const auto& raqn = aquiferData.getNumericAquiferDoublePrecData();
        check_is_close(raqn, expect, 1.0e-10);
    }
}

BOOST_AUTO_TEST_SUITE_END()
