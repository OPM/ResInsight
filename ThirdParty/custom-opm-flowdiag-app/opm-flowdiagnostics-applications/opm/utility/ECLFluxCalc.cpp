/*
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

#include <opm/utility/ECLFluxCalc.hpp>
#include <opm/utility/ECLPvtCommon.hpp>
#include <opm/utility/ECLUnitHandling.hpp>
#include <opm/utility/ECLSaturationFunc.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <ert/ecl/ecl_kw_magic.h>

namespace {

    std::vector<double>
    computeGravDZ(const std::vector<int>&    neigh,
                  const double               grav,
                  const std::vector<double>& depth)
    {
        const auto nf = neigh.size() / 2;

        auto gdz = std::vector<double>{};
        gdz.reserve(nf);

        for (auto f = 0*nf; f < nf; ++f) {
            const auto c1 = neigh[2*f + 0];
            const auto c2 = neigh[2*f + 1];

            gdz.push_back(grav * (depth[c2] - depth[c1]));
        }

        return gdz;
    }

    std::vector<int>
    pvtnumVector(const ::Opm::ECLGraph&        G,
                 const ::Opm::ECLInitFileData& init)
    {
        auto pvtnum = G.rawLinearisedCellData<int>(init, "PVTNUM");

        if (pvtnum.empty()) {
            // PVTNUM missing in one or more of the grids managed by 'G'.
            // Put all cells in PVTNUM region 1.
            pvtnum.assign(G.numCells(), 1);
        }

        return pvtnum;
    }

    std::vector<double>
    depthVector(const ::Opm::ECLGraph&        G,
                const ::Opm::ECLInitFileData& init)
    {
        // Note: ECLGraph does not support unit conversion of INIT data so
        // we need to perform the requisite conversions ourselves.
        auto depth = G.rawLinearisedCellData<double>(init, "DEPTH");

        if (depth.empty()) {
            // DEPTH missing in one or more of the grids managed by 'G'.
            // Put all cells at zero depth, which turns off gravity.
            depth.assign(G.numCells(), 0.0);
        }

        const auto& ih = init.keywordData<int>(INTEHEAD_KW);
        const auto usys = ::Opm::ECLUnits::
            createUnitSystem(ih[ INTEHEAD_UNIT_INDEX ]);

        const auto depthscale = usys->depth();

        for (auto& zi : depth) {
            zi = ::Opm::unit::convert::from(zi, depthscale);
        }

        return depth;
    }

    std::vector<double>
    disgasVector(const ::Opm::ECLGraph&       G,
                 const bool                   is_liveoil,
                 const ::Opm::ECLRestartData& rstrt)
    {
        auto disgas = std::vector<double>{};

        if (! is_liveoil) {
            // Oil model does not use dissolved gas.  We don't need to
            // provide Rs data, so return zero (simplifies calling code).

            disgas.assign(G.numCells(), 0.0);

            return disgas;
        }

        // Gas model does use vaporised oil.  Extract Rs data or throw if
        // unavailable.
        disgas = G.linearisedCellData(rstrt, "RS",
                                      &::Opm::ECLUnits::UnitSystem::
                                      dissolvedGasOilRat);

        if (disgas.empty()) {
            throw std::invalid_argument {
                "Restart Data Does Not Provide "
                "Dissolved Gas/Oil Ratio Data "
                "for Live Oil PVT Model"
            };
        }

        return disgas;
    }

    std::vector<double>
    vapoilVector(const ::Opm::ECLGraph&       G,
                 const bool                   is_wetgas,
                 const ::Opm::ECLRestartData& rstrt)
    {
        auto vapoil = std::vector<double>{};

        if (! is_wetgas) {
            // Gas model does not use vaporised oil.  We don't need to
            // provide Rv data, so return zero (simplifies calling code).

            vapoil.assign(G.numCells(), 0.0);

            return vapoil;
        }

        // Gas model does use vaporised oil.  Extract Rv data or throw if
        // unavailable.
        vapoil = G.linearisedCellData(rstrt, "RV",
                                      &::Opm::ECLUnits::UnitSystem::
                                      vaporisedOilGasRat);

        if (vapoil.empty()) {
            throw std::invalid_argument {
                "Restart Data Does Not Provide "
                "Vaporised Oil/Gas Ratio Data "
                "for Wet Gas PVT Model"
            };
        }

        return vapoil;
    }

    template <class PVTPtr>
    void verify_active_phase(const PVTPtr&      pvt,
                             const std::string& phase)
    {
        if (! pvt) {
            throw std::logic_error {
                "Cannot Compute " + phase +
                " PVT Unless "    + phase +
                " is an Active Phase"
            };
        }
    }

} // Anonymous

namespace Opm
{

    ECLFluxCalc::ECLFluxCalc(const ECLGraph&        graph,
                             const ECLInitFileData& init,
                             const double           grav,
                             const bool             useEPS)
        : graph_(graph)
        , satfunc_(graph, init, useEPS)
        , rmap_(pvtnumVector(graph, init))
        , neighbours_(graph.neighbours())
        , transmissibility_(graph.transmissibility())
        , gravDz_(computeGravDZ(neighbours_, grav, depthVector(graph, init)))
        , pvtGas_(ECLPVT::CreateGasPVTInterpolant::fromECLOutput(init))
        , pvtOil_(ECLPVT::CreateOilPVTInterpolant::fromECLOutput(init))
        , pvtWat_(ECLPVT::CreateWaterPVTInterpolant::fromECLOutput(init))
    {
        const auto& lh = init.keywordData<bool>(LOGIHEAD_KW);

        this->disgas_ = lh[ LOGIHEAD_RS_INDEX ]; // Live Oil?
        this->vapoil_ = lh[ LOGIHEAD_RV_INDEX ]; // Wet Gas?
    }





    std::vector<double>
    ECLFluxCalc::flux(const ECLRestartData& rstrt,
                      const ECLPhaseIndex   phase) const
    {
        // Obtain dynamic data.
        const auto dyn_data = this->phaseProperties(rstrt, phase);

        // Compute fluxes per connection.
        const int num_conn = transmissibility_.size();
        std::vector<double> fluxvec(num_conn);
        for (int conn = 0; conn < num_conn; ++conn) {
            fluxvec[conn] = singleFlux(conn, dyn_data);
        }
        return fluxvec;
    }


    std::vector<double>
    ECLFluxCalc::massflux(const ECLRestartData& rstrt,
                          const ECLPhaseIndex   phase) const
    {
        // Obtain dynamic data.
        const auto dyn_data = this->phaseProperties(rstrt, phase);

        // Compute fluxes per connection.
        const int num_conn = transmissibility_.size();
        std::vector<double> fluxvec(num_conn);
        for (int conn = 0; conn < num_conn; ++conn) {
            fluxvec[conn] = singleMassFlux(conn, dyn_data);
        }
        return fluxvec;
    }



    double ECLFluxCalc::singleFlux(const int connection,
                                   const DynamicData& dyn_data) const
    {
        const int c1 = neighbours_[2*connection];
        const int c2 = neighbours_[2*connection + 1];

        // Phase pressure in connecting cells.
        const auto p1 = dyn_data.pressure[c1];
        const auto p2 = dyn_data.pressure[c2];

        // Phase density at interface: Arith. avg. of cell values.
        const auto rho =
            (dyn_data.density[c1] + dyn_data.density[c2]) / 2.0;

        // Phase potential drop across interface.
        const auto dh = p1 - p2 + rho*this->gravDz_[connection];

        // Phase mobility at interface: Upstream weighting (phase pot).
        const auto ucell = (dh < 0.0) ? c2 : c1;
        const auto mob   = dyn_data.mobility[ucell];

        // Background (static) transmissibility.
        const auto T = this->transmissibility_[connection];

        return mob * T * dh;
    }

    double ECLFluxCalc::singleMassFlux(const int connection,
                                       const DynamicData& dyn_data) const
    {
        const int c1 = neighbours_[2*connection];
        const int c2 = neighbours_[2*connection + 1];

        // Phase pressure in connecting cells.
        const auto p1 = dyn_data.pressure[c1];
        const auto p2 = dyn_data.pressure[c2];

        // Phase density at interface: Arith. avg. of cell values.
        const auto rho =
            (dyn_data.density[c1] + dyn_data.density[c2]) / 2.0;

        // Phase potential drop across interface.
        const auto dh = p1 - p2 + rho*this->gravDz_[connection];

        // Phase mobility at interface: Upstream weighting (phase pot).
        const auto ucell = (dh < 0.0) ? c2 : c1;
        const auto mob   = dyn_data.mobility[ucell];

        // Background (static) transmissibility.
        const auto T = this->transmissibility_[connection];

        // Upstream weighted phase density.
        const auto urho = dyn_data.density[ucell];

        return urho * mob * T * dh;
    }



    ECLFluxCalc::DynamicData
    ECLFluxCalc::phaseProperties(const ECLRestartData& rstrt,
                                 const ECLPhaseIndex   phase) const
    {
        auto dyn_data = DynamicData{};

        // Step 1 of Phase Pressure Calculation.
        // Retrieve oil pressure directly from result set.
        dyn_data.pressure = this->graph_
            .linearisedCellData(rstrt, "PRESSURE",
                                &ECLUnits::UnitSystem::pressure);

        // Step 1 of Mobility Calculation.
        // Store phase's relative permeability values.
        dyn_data.mobility =
            this->satfunc_.relperm(this->graph_, rstrt, phase);

        // Step 1 of Mass Density (Reservoir Conditions) Calculation.
        // Allocate space for storing the cell values.
        dyn_data.density.assign(this->graph_.numCells(), 0.0);

        switch (phase) {
        case ECLPhaseIndex::Aqua:
            dyn_data.saturation = this->graph_.rawLinearisedCellData<double>(rstrt, "SWAT");
            return this->watPVT(std::move(dyn_data));

        case ECLPhaseIndex::Liquid:
            dyn_data.saturation = this->graph_.rawLinearisedCellData<double>(rstrt, "SOIL");
            if (!dyn_data.saturation.empty()) {
                return this->oilPVT(rstrt, std::move(dyn_data));
            } else {
                // SOIL vector not provided. Compute from SWAT and/or SGAS.
                // may read two times
                auto sw = this->graph_.rawLinearisedCellData<double>(rstrt, "SWAT");
                auto sg = this->graph_.rawLinearisedCellData<double>(rstrt, "SGAS");
                std::vector<double>& so = dyn_data.saturation;
                so.assign(this->graph_.numCells(), 1.0);
                auto adjust_So_for_other_phase =
                    [&so](const std::vector<double>& s)
                {
                    std::transform(std::begin(so), std::end(so),
                                   std::begin(s) ,
                                   std::begin(so), std::minus<double>());
                };
                if (sg.size() == this->graph_.numCells()) {
                    adjust_So_for_other_phase(sg);
                }

                if (sw.size() == this->graph_.numCells()) {
                    adjust_So_for_other_phase(sw);
                }
                return this->oilPVT(rstrt, std::move(dyn_data));
            }

        case ECLPhaseIndex::Vapour:
            dyn_data.saturation = this->graph_.rawLinearisedCellData<double>(rstrt, "SGAS");
            return this->gasPVT(rstrt, std::move(dyn_data));
        }

        throw std::invalid_argument {
            "phaseProperties(): Invalid Phase Identifier"
        };
    }

    double ECLFluxCalc::surfaceDensity(const ECLPhaseIndex   phase) const{
        switch (phase) {
        case ECLPhaseIndex::Aqua:
            return this->pvtWat_->surfaceMassDensity(0);

        case ECLPhaseIndex::Liquid:
            return this->pvtOil_->surfaceMassDensity(0);

        case ECLPhaseIndex::Vapour:
            return this->pvtGas_->surfaceMassDensity(0);
        }
    }



    ECLFluxCalc::DynamicData
    ECLFluxCalc::gasPVT(const ECLRestartData& rstrt,
                        DynamicData&&         dyn_data) const
    {
        verify_active_phase(this->pvtGas_, "Gas");

        const auto rv = vapoilVector(this->graph_, this->vapoil_, rstrt);

        this->regionLoop([this, &rv, &dyn_data]
            (const int regID)
        {
            // Note: This function assumes that 'regID' is a traditional
            // ECL-style one-based region ID such as PVTNUM.  Subtract one,
            // where approriate, to generate zero-based region indices.

            const auto Rv = ECLPVT::Gas::VaporizedOil {
                this->gatherRegionSubset(regID, rv)
            };

            const auto Pg = ECLPVT::Gas::GasPressure {
                // Cheating.  This is Po.
                this->gatherRegionSubset(regID, dyn_data.pressure)
            };

            // Mass Density at Reservoir Conditions.  Relies on setup code
            // having allocated sufficient space.
            {
                const auto rhoOS = this->vapoil_
                    ? this->pvtOil_->surfaceMassDensity(regID - 1)
                    : 0.0;

                const auto rhoGS =
                    this->pvtGas_->surfaceMassDensity(regID - 1);

                const auto Bg = this->pvtGas_
                    ->formationVolumeFactor(regID - 1, Rv, Pg);

                auto rhoGr = std::vector<double>{};
                rhoGr.reserve(Bg.size());

                std::transform(std::begin(Bg),
                               std::end  (Bg),
                               std::begin(Rv.data),
                               std::back_inserter(rhoGr),
                    [rhoOS, rhoGS]
                    (const double Bg_i, const double Rv_i)
                {
                    return (rhoOS*Rv_i + rhoGS) / Bg_i;
                });

                this->scatterRegionResults(regID, rhoGr, dyn_data.density);
            }

            // Convert relative permeability values into mobility values
            // (divide by phase viscosity).  Relies on setup code having
            // computed relative permeability for the phase.
            {
                const auto mu = this->pvtGas_->viscosity(regID - 1, Rv, Pg);

                this->computePhaseMobility(regID, mu, dyn_data);
            }
        });

        return std::move(dyn_data);
    }





    ECLFluxCalc::DynamicData
    ECLFluxCalc::oilPVT(const ECLRestartData& rstrt,
                        DynamicData&&         dyn_data) const
    {
        verify_active_phase(this->pvtOil_, "Oil");

        const auto rs = disgasVector(this->graph_, this->disgas_, rstrt);

        this->regionLoop([this, &rs, &dyn_data]
            (const int regID)
        {
            // Note: This section assumes that 'regID' is a traditional
            // ECL-style one-based region ID such as PVTNUM.  Subtract one,
            // where approriate, to generate zero-based region indices.

            const auto Rs = ECLPVT::Oil::DissolvedGas {
                this->gatherRegionSubset(regID, rs)
            };

            const auto Po = ECLPVT::Oil::OilPressure {
                // Recall: dyn_data.pressure is Po directly from 'rstrt'.
                this->gatherRegionSubset(regID, dyn_data.pressure)
            };

            // Mass Density at Reservoir Conditions.  Relies on setup code
            // having allocated sufficient space.
            {
                const auto rhoOS =
                    this->pvtOil_->surfaceMassDensity(regID - 1);

                const auto rhoGS = this->disgas_
                    ? this->pvtGas_->surfaceMassDensity(regID - 1)
                    : 0.0;

                const auto Bo = this->pvtOil_
                    ->formationVolumeFactor(regID - 1, Rs, Po);

                auto rhoOr = std::vector<double>{};
                rhoOr.reserve(Bo.size());

                std::transform(std::begin(Bo),
                               std::end  (Bo),
                               std::begin(Rs.data),
                               std::back_inserter(rhoOr),
                    [rhoOS, rhoGS]
                    (const double Bo_i, const double Rs_i)
                {
                    return (rhoOS + rhoGS*Rs_i) / Bo_i;
                });

                this->scatterRegionResults(regID, rhoOr, dyn_data.density);
            }

            // Convert relative permeability values into mobility values
            // (divide by phase viscosity).  Relies on setup code having
            // computed relative permeability for the phase.
            {
                const auto mu = this->pvtOil_->viscosity(regID - 1, Rs, Po);

                this->computePhaseMobility(regID, mu, dyn_data);
            }
        });

        return std::move(dyn_data);
    }





    ECLFluxCalc::DynamicData
    ECLFluxCalc::watPVT(DynamicData&& dyn_data) const
    {
        verify_active_phase(this->pvtWat_, "Water");

        this->regionLoop([this, &dyn_data]
            (const int regID)
        {
            // Note: This section assumes that 'regID' is a traditional
            // ECL-style one-based region ID such as PVTNUM.  Subtract one,
            // where approriate, to generate zero-based region indices.

            const auto Pw = ECLPVT::Water::WaterPressure {
                // Cheating.  This is Po.
                this->gatherRegionSubset(regID, dyn_data.pressure)
            };

            // Mass Density at Reservoir Conditions.  Relies on setup code
            // having allocated sufficient space.
            {
                const auto rhoWS =
                    this->pvtWat_->surfaceMassDensity(regID - 1);

                const auto Bw = this->pvtWat_
                    ->formationVolumeFactor(regID - 1, Pw);

                auto rhoWr = std::vector<double>{};
                rhoWr.reserve(Bw.size());

                std::transform(std::begin(Bw),
                               std::end  (Bw),
                               std::back_inserter(rhoWr),
                    [rhoWS](const double Bw_i)
                {
                    return rhoWS / Bw_i;
                });

                this->scatterRegionResults(regID, rhoWr, dyn_data.density);
            }

            // Convert relative permeability values into mobility values
            // (divide by phase viscosity).  Relies on setup code having
            // computed relative permeability for the phase.
            {
                const auto mu = this->pvtWat_->viscosity(regID - 1, Pw);

                this->computePhaseMobility(regID, mu, dyn_data);
            }
        });

        return std::move(dyn_data);
    }





    void
    ECLFluxCalc::computePhaseMobility(const int                  regID,
                                      const std::vector<double>& mu,
                                      DynamicData&               dyn_data) const
    {
        auto kr = this->gatherRegionSubset(regID, dyn_data.mobility);

        std::transform(std::begin(kr), std::end  (kr),
                       std::begin(mu), std::begin(kr),
                       std::divides<double>());

        this->scatterRegionResults(regID, kr, dyn_data.mobility);
    }

} // namespace Opm
