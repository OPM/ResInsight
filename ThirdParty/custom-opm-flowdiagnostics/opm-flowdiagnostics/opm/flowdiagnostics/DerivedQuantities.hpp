/*
  Copyright 2016 Statoil ASA.
  Copyright 2015, 2016 SINTEF ICT, Applied Mathematics.

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

#ifndef OPM_DERIVEDQUANTITIES_HEADER_INCLUDED
#define OPM_DERIVEDQUANTITIES_HEADER_INCLUDED

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/flowdiagnostics/Solution.hpp>
#include <opm/flowdiagnostics/Toolbox.hpp>
#include <utility>
#include <vector>

namespace Opm
{
namespace FlowDiagnostics
{
    /// Class used to return graph objects. For a graph g, the
    /// abscissa (x) values should go in g.first and the ordinate (y)
    /// values in g.second.
    using Graph = std::pair< std::vector<double>, std::vector<double> >;

    /// The F-Phi curve.
    ///
    /// The F-Phi curve is an analogue to the fractional flow
    /// curve in a 1D displacement. It can be used to compute
    /// other interesting diagnostic quantities such as the Lorenz
    /// coefficient. For a technical description see Shavali et
    /// al. (SPE 146446), Shook and Mitchell (SPE 124625).
    ///
    /// Single cells with a very large pore volume can be filtered out
    /// before creating the curve. The 'max_pv_fraction' parameter
    /// gives a fraction such that, if a cell's fraction of the total
    /// pore volume is above this number, that cell will be
    /// ignored. This can be used to disregard numerical aquifers for
    /// example.
    ///
    /// Returns F (flow capacity) as a function of Phi (storage capacity),
    /// that is for the returned Graph g, g.first is Phi and g.second is F.
    Graph flowCapacityStorageCapacityCurve(const Toolbox::Forward& injector_solution,
                                           const Toolbox::Reverse& producer_solution,
                                           const std::vector<double>& pore_volume,
                                           const double max_pv_fraction = 1.0);

    /// This overload gets the injector and producer time-of-flight
    /// directly instead of extracting it from the solution
    /// objects. It otherwise behaves identically to the other
    /// overload.
    Graph flowCapacityStorageCapacityCurve(const std::vector<double>& injector_tof,
                                           const std::vector<double>& producer_tof,
                                           const std::vector<double>& pore_volume,
                                           const double max_pv_fraction = 1.0);

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
    double lorenzCoefficient(const Graph& flowcap_storagecap_curve);

    /// Compute sweep efficiency versus dimensionless time (pore
    /// volumes injected).
    ///
    /// The sweep efficiency is analogue to 1D displacement using
    /// the F-Phi curve as flux function.
    Graph sweepEfficiency(const Graph& flowcap_storagecap_curve);

    /// Compute pore volume associated with an injector-producer pair.
    double injectorProducerPairVolume(const Toolbox::Forward& injector_solution,
                                      const Toolbox::Reverse& producer_solution,
                                      const std::vector<double>& pore_volume,
                                      const CellSetID& injector,
                                      const CellSetID& producer);

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
                             const std::map<CellSetID, CellSetValues>& inflow_flux);


} // namespace FlowDiagnostics
} // namespace Opm



#endif // OPM_DERIVEDQUANTITIES_HEADER_INCLUDED
