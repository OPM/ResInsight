/*
  Copyright 2015, 2016, 2017 SINTEF ICT, Applied Mathematics.
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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <opm/flowdiagnostics/DerivedQuantities.hpp>
#include <algorithm>
#include <numeric>

namespace Opm
{
namespace FlowDiagnostics
{

    namespace {
        /// Helper function for flowCapacityStorageCapacityCurve().
        template <class InputValues, class ExtractElement>
        std::vector<double>
        cumulativeNormalized(const InputValues& input,
                             ExtractElement&& extract)
        {
            // Extract quantity.
            auto q = std::vector<double>{};
            q.reserve(input.size() + 1);
            q.push_back(0.0);
            for (const auto& e : input) {
                q.push_back(extract(e));
            }

            // Accumulate and normalize.
            std::partial_sum(q.begin(), q.end(), q.begin());
            const auto t = q.back();
            for (auto& qi : q) {
                qi /= t;
            }

            return q;
        }

    } // anonymous namespace



    /// The F-Phi curve.
    ///
    /// The F-Phi curve is an analogue to the fractional flow
    /// curve in a 1D displacement. It can be used to compute
    /// other interesting diagnostic quantities such as the Lorenz
    /// coefficient. For a technical description see Shavali et
    /// al. (SPE 146446), Shook and Mitchell (SPE 124625).
    Graph flowCapacityStorageCapacityCurve(const Toolbox::Forward& injector_solution,
                                           const Toolbox::Reverse& producer_solution,
                                           const std::vector<double>& pv,
                                           const double max_pv_fraction)
    {
        return flowCapacityStorageCapacityCurve(injector_solution.fd.timeOfFlight(),
                                                producer_solution.fd.timeOfFlight(),
                                                pv,
                                                max_pv_fraction);
    }




    /// The F-Phi curve.
    ///
    /// The F-Phi curve is an analogue to the fractional flow
    /// curve in a 1D displacement. It can be used to compute
    /// other interesting diagnostic quantities such as the Lorenz
    /// coefficient. For a technical description see Shavali et
    /// al. (SPE 146446), Shook and Mitchell (SPE 124625).
    Graph flowCapacityStorageCapacityCurve(const std::vector<double>& injector_tof,
                                           const std::vector<double>& producer_tof,
                                           const std::vector<double>& pv,
                                           const double max_pv_fraction)
    {
        if (pv.size() != injector_tof.size() || pv.size() != producer_tof.size()) {
            throw std::runtime_error("flowCapacityStorageCapacityCurve(): "
                                     "Input solutions must have same size.");
        }

        // Compute max pv cutoff.
        const double total_pv = std::accumulate(pv.begin(), pv.end(), 0.0);
        const double max_pv = max_pv_fraction * total_pv;

        // Sort according to total travel time.
        const int n = pv.size();
        typedef std::pair<double, double> D2;
        std::vector<D2> time_and_pv(n);
        for (int ii = 0; ii < n; ++ii) {
            if (pv[ii] > max_pv) {
                time_and_pv[ii].first = 1e100;
                time_and_pv[ii].second = 0.0;
            } else {
                time_and_pv[ii].first = injector_tof[ii] + producer_tof[ii]; // Total travel time.
                time_and_pv[ii].second = pv[ii];
            }
        }
        std::sort(time_and_pv.begin(), time_and_pv.end());

        auto Phi = cumulativeNormalized(time_and_pv, [](const D2& i) { return i.second; });
        auto F = cumulativeNormalized(time_and_pv, [](const D2& i) { return i.second / i.first; });

        return Graph{std::move(Phi), std::move(F)};
    }




    /// The Lorenz coefficient from the F-Phi curve.
    ///
    /// The Lorenz coefficient is a measure of heterogeneity. It
    /// is equal to twice the area between the F-Phi curve and the
    /// F = Phi line.  The coefficient can vary from zero to
    /// one. If the coefficient is zero (so the F-Phi curve is a
    /// straight line) we have perfect piston-like displacement
    /// while a coefficient of one indicates infinitely
    /// heterogenous displacement (essentially no sweep).
    ///
    /// Note: The coefficient is analogous to the Gini coefficient
    /// of economic theory, where the name Lorenz curve is applied
    /// to what we call the F-Phi curve.
    double lorenzCoefficient(const Graph& flowcap_storagecap_curve)
    {
        const auto& storagecap = flowcap_storagecap_curve.first;
        const auto& flowcap = flowcap_storagecap_curve.second;
        if (flowcap.size() != storagecap.size()) {
            throw std::runtime_error("lorenzCoefficient(): Inconsistent sizes in input graph.");
        }
        double integral = 0.0;
        // Trapezoid quadrature of the curve F(Phi).
        const int num_intervals = flowcap.size() - 1;
        for (int ii = 0; ii < num_intervals; ++ii) {
            const double len = storagecap[ii+1] - storagecap[ii];
            integral += (flowcap[ii] + flowcap[ii+1]) * len / 2.0;
        }
        return 2.0 * (integral - 0.5);
    }




    /// Compute sweep efficiency versus dimensionless time (pore
    /// volumes injected).
    ///
    /// The sweep efficiency is analogue to 1D displacement using
    /// the F-Phi curve as flux function.
    Graph sweepEfficiency(const Graph& flowcap_storagecap_curve)
    {
        const auto& storagecap = flowcap_storagecap_curve.first;
        const auto& flowcap = flowcap_storagecap_curve.second;
        if (flowcap.size() != storagecap.size()) {
            throw std::runtime_error("sweepEfficiency(): Inconsistent sizes in input graph.");
        }

        // Compute tD and Ev simultaneously,
        // skipping identical Phi data points.
        const int n = flowcap.size();
        std::vector<double> Ev;
        std::vector<double> tD;
        tD.reserve(n);
        Ev.reserve(n);
        tD.push_back(0.0);
        Ev.push_back(0.0);
        for (int ii = 1; ii < n; ++ii) { // Note loop limits.
            const double fd = flowcap[ii] - flowcap[ii-1];
            const double sd = storagecap[ii] - storagecap[ii-1];
            if (fd != 0.0) {
                tD.push_back(sd/fd);
                Ev.push_back(storagecap[ii] + (1.0 - flowcap[ii]) * tD.back());
            }
        }

        return std::make_pair(tD, Ev);
    }





    /// Compute pore volume associated with an injector-producer pair.
    double injectorProducerPairVolume(const Toolbox::Forward& injector_solution,
                                      const Toolbox::Reverse& producer_solution,
                                      const std::vector<double>& pore_volume,
                                      const CellSetID& injector,
                                      const CellSetID& producer)
    {
        const auto& inj_tracer = injector_solution.fd.concentration(injector);
        const auto& prod_tracer = producer_solution.fd.concentration(producer);

        double volume = 0.0;
        for (const auto& inj_data : inj_tracer) {
            const int cell = inj_data.first;
            const auto prod_data = prod_tracer.find(cell);
            if (prod_data != prod_tracer.end()) {
                volume += pore_volume[cell] * inj_data.second * prod_data->second;
            }
        }
        return volume;
    }





    namespace {

        // Helper for injectorProducerPairFlux().
        double pairFlux(const CellSetValues& tracer,
                        const CellSetValues& inflow_flux,
                        const bool require_inflow)
        {
            double flux = 0.0;
            for (const auto inflow : inflow_flux) {
                const int cell = inflow.first;
                const auto tracer_iter = tracer.find(cell);
                if (tracer_iter != tracer.end()) {
                    // Tracer present in cell.
                    const double source = inflow.second;
                    if ((source > 0.0) == require_inflow) {
                        // Source term has correct sign.
                        flux += source * tracer_iter->second;
                    }
                }
            }
            return flux;
        }

    } // anonymous namespace




    /// Compute fluxes associated with an injector-producer pair.
    ///
    /// The first flux returned is the injection flux associated with the given producers,
    /// (equal to the accumulated product of producer tracer values at the injector cells
    /// with the injection fluxes), the second is the production flux associated with the
    /// given injectors. In general, they will only be the same (up to sign) for
    /// incompressible cases.
    ///
    /// Note: since we consider injecting fluxes positive and producing fluxes negative
    /// (for the inflow_flux), the first returned element will be positive and the second
    /// will be negative.
    std::pair<double, double>
    injectorProducerPairFlux(const Toolbox::Forward& injector_solution,
                             const Toolbox::Reverse& producer_solution,
                             const CellSetID& injector,
                             const CellSetID& producer,
                             const std::map<CellSetID, CellSetValues>& inflow_flux)
    {
        const auto& inj_tracer = injector_solution.fd.concentration(injector);
        const auto& prod_tracer = producer_solution.fd.concentration(producer);
        const auto inj_set_iter = inflow_flux.find(injector);
        if (inj_set_iter == inflow_flux.end()) {
            throw std::runtime_error("injectorProducerPairFlux(): Could not find requeste injector set in inflow fluxes.");
        }
        const auto prod_set_iter = inflow_flux.find(producer);
        if (prod_set_iter == inflow_flux.end()) {
            throw std::runtime_error("injectorProducerPairFlux(): Could not find requested producer set in inflow fluxes.");
        }
        const double inj_flux = pairFlux(prod_tracer, inj_set_iter->second, true);
        const double prod_flux = pairFlux(inj_tracer, prod_set_iter->second, false);
        return { inj_flux, prod_flux };
    }


} // namespace FlowDiagnostics
} // namespace Opm
