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

#ifndef OPM_ECLFLUXCALC_HEADER_INCLUDED
#define OPM_ECLFLUXCALC_HEADER_INCLUDED

#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLPhaseIndex.hpp>
#include <opm/utility/ECLPvtGas.hpp>
#include <opm/utility/ECLPvtOil.hpp>
#include <opm/utility/ECLPvtWater.hpp>
#include <opm/utility/ECLRegionMapping.hpp>
#include <opm/utility/ECLSaturationFunc.hpp>

#include <memory>
#include <vector>

namespace Opm
{

    class ECLRestartData;
    class ECLInitFileData;

    /// Class for computing connection fluxes in the absence of flux output.
    class ECLFluxCalc
    {
    public:
        /// Construct from ECLGraph and Run Initialization Data.
        ///
        /// \param[in] graph Connectivity data, as well as providing a means
        ///    to read data from the restart file.
        ///
        /// \param[in] init ECLIPSE result set static initialization data
        ///    ("INIT" file).
        ///
        /// \param[in] grav Gravity constant (9.80665 m/s^2 at Tellus equator).
        ///
        /// \param[in] useEPS Whether or not to include effects of
        ///    saturation function end-point scaling if activated in the
        ///    result set.
        ECLFluxCalc(const ECLGraph&        graph,
                    const ECLInitFileData& init,
                    const double           grav,
                    const bool             useEPS);

        /// Retrive phase flux on all connections defined by \code
        /// graph.neighbours() \endcode.
        ///
        /// \param[in] rstrt ECL Restart data set from which to extract
        ///            relevant data per cell.
        ///
        /// \param[in] phase Canonical phase for which to retrive flux.
        ///
        /// \return Flux values corresponding to selected phase.
        ///         Empty if required data is missing.
        ///         Numerical values in SI units (rm^3/s).
        std::vector<double>
        flux(const ECLRestartData& rstrt,
             const ECLPhaseIndex   phase) const;

        /// Retrive phase mass flux on all connections defined by \code
        /// graph.neighbours() \endcode.
        ///
        /// \param[in] rstrt ECL Restart data set from which to extract
        ///            relevant data per cell.
        ///
        /// \param[in] phase Canonical phase for which to retrive flux.
        ///
        /// \return Mass flux values corresponding to selected phase.
        ///         Empty if required data is missing.
        ///         Numerical values in SI units (kg/s).
        std::vector<double>
        massflux(const ECLRestartData& rstrt,
                 const ECLPhaseIndex   phase) const;

        /// Return type for the phaseProperties() method,
        /// encapsulates dynamic properties for a single
        /// phase.
        struct DynamicData
        {
            std::vector<double> pressure;
            std::vector<double> mobility;
            std::vector<double> density;
            std::vector<double> saturation;
        };

        /// Retrive dynamical properties of a single phase on all cells.
        ///
        /// \param[in] rstrt ECL Restart data set from which to extract
        ///            relevant data per cell.
        ///
        /// \param[in] phase Canonical phase for which to retrive properties.
        ///
        /// \return DynamicData struct containing cell-values for phase properties.
        ///         Numerical values in SI units (kg/s).
        DynamicData phaseProperties(const ECLRestartData& rstrt,
                                    const ECLPhaseIndex   phase) const;

        /// Retrive the constant surface density of a phase.
        ///
        /// \param[in] phase Canonical phase for which to retrive the surface density.
        ///
        /// \return Density of given phase at surface conditions.
        ///         Numerical value in SI units (kg/m^3).
        double surfaceDensity(const ECLPhaseIndex phase) const;


    private:


        double singleFlux(const int connection,
                          const DynamicData& dyn_data) const;

        double singleMassFlux(const int connection,
                              const DynamicData& dyn_data) const;


        bool phaseIsActive(const ECLPhaseIndex phase) const;

        DynamicData gasPVT(const ECLRestartData& rstrt,
                           DynamicData&&         dyn_data) const;

        DynamicData oilPVT(const ECLRestartData& rstrt,
                           DynamicData&&         dyn_data) const;

        DynamicData watPVT(DynamicData&& dyn_data) const;

        void computePhaseMobility(const int                  regID,
                                  const std::vector<double>& mu,
                                  DynamicData&               dyn_data) const;

        template <typename T>
        std::vector<T>
        gatherRegionSubset(const int             reg,
                           const std::vector<T>& x) const
        {
            auto y = std::vector<T>{};

            if (x.empty()) {
                return y;
            }

            for (const auto& ix : this->rmap_.getRegionIndices(reg)) {
                y.push_back(x[ix]);
            }

            return y;
        }

        template <typename T>
        void scatterRegionResults(const int             reg,
                                  const std::vector<T>& x_reg,
                                  std::vector<T>&       x) const
        {
            auto i = static_cast<decltype(x_reg.size())>(0);

            for (const auto& ix : this->rmap_.getRegionIndices(reg)) {
                x[ix] = x_reg[i++];
            }
        }

        template <class RegOp>
        void regionLoop(RegOp&& regOp) const
        {
            for (const auto& regID : this->rmap_.activeRegions()) {
                regOp(regID);
            }
        }

        const ECLGraph& graph_;
        ECLSaturationFunc satfunc_;
        ECLRegionMapping rmap_;
        std::vector<int> neighbours_;
        std::vector<double> transmissibility_;
        std::vector<double> gravDz_;

        bool disgas_{false};
        bool vapoil_{false};

        std::unique_ptr<ECLPVT::Gas> pvtGas_;
        std::unique_ptr<ECLPVT::Oil> pvtOil_;
        std::unique_ptr<ECLPVT::Water> pvtWat_;
    };

} // namespace Opm

#endif // OPM_ECLFLUXCALC_HEADER_INCLUDED
