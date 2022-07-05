/*
  Copyright (c) 2021 Equinor ASA

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

#ifndef OPM_AGGREGATE_AQUIFER_DATA_HPP
#define OPM_AGGREGATE_AQUIFER_DATA_HPP

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/WindowedArray.hpp>

#include <opm/output/data/Aquifer.hpp>

#include <vector>

namespace Opm {
    class AquiferConfig;
    class EclipseGrid;
    class SummaryState;
    class UnitSystem;
} // Opm

namespace Opm { namespace RestartIO { namespace Helpers {

    class AggregateAquiferData
    {
    public:
        /// Constructor.
        ///
        /// \param[in] aqDims Aquifer dimensions including number of active
        ///    aquifers, maximum aquifer IDs, and number of data items per
        ///    aquifer (or connection) in the various output arrays.
        ///
        /// \param[in] aqConfig Aquifer configuration object.  Keeps track
        ///    of aquifer types (Carter-Tracy vs. Fetkovich) and provides
        ///    read-only access to the individual aquifer objects.
        ///
        /// \param[in] grid Simulation grid.  Needed to map active to
        ///    Cartesian cell indices and to extract (I,J,K) index tuples of
        ///    the active cells.
        explicit AggregateAquiferData(const InteHEAD::AquiferDims& aqDims,
                                      const AquiferConfig&         aqConfig,
                                      const EclipseGrid&           grid);

        /// Linearise dynamic information pertinent to analytic aquifers
        /// into internal arrays.
        ///
        /// \param[in] aqConfig Aquifer configuration object.  Keeps track
        ///    of aquifer types (Carter-Tracy vs. Fetkovich) and provides
        ///    read-only access to the individual aquifer objects.
        ///
        /// \param[in] aquData Dynamic aquifer data, including time
        ///    constants, water mass densities, water viscosities, and
        ///    initial aquifer pressures.
        ///
        /// \param[in] summaryState Current state of summary variables.
        ///    Expected to contain at least the summary variables AAQP
        ///    (aquifer pressure), AAQR (aquifer flow rate), and AAQT (total
        ///    produced inflow volume from aquifer).
        ///
        /// \param[in] usys Unit system.  Needed to convert quantities from
        ///    internal to output units.
        void captureDynamicdAquiferData(const AquiferConfig&  aqConfig,
                                        const data::Aquifers& aquData,
                                        const SummaryState&   summaryState,
                                        const UnitSystem&     usys);

        /// Retrieve the maximum active aquifer ID over all analytic
        /// aquifers.
        ///
        /// Controls output of restart information pertaining to analytic
        /// aquifer connections.
        int maximumActiveAnalyticAquiferID() const
        {
            return this->maxActiveAnalyticAquiferID_;
        }

        /// Retrieve Integer Aquifer Data Array.
        const std::vector<int>& getIntegerAquiferData() const
        {
            return this->integerAnalyticAq_.data();
        }

        /// Retrieve Floating-Point (Real) Aquifer Data Array.
        const std::vector<float>& getSinglePrecAquiferData() const
        {
            return this->singleprecAnalyticAq_.data();
        }

        /// Retrieve Floating-Point (Double Precision) Aquifer Data Array.
        const std::vector<double>& getDoublePrecAquiferData() const
        {
            return this->doubleprecAnalyticAq_.data();
        }

        /// Retrieve Integer Aquifer Data Array for Numeric Aquifers.
        const std::vector<int>& getNumericAquiferIntegerData() const
        {
            return this->integerNumericAq_.data();
        }

        /// Retrieve Double Precision Aquifer Data Array for Numeric Aquifers.
        const std::vector<double>& getNumericAquiferDoublePrecData() const
        {
            return this->doubleprecNumericAq_.data();
        }

        /// Retrieve Integer Aquifer Connection Data Array (analytic aquifers)
        ///
        /// \param[in] aquiferID Aquifer for which to retrieve integer
        ///    connection data array.  Expected to be in the range
        ///    [1..maximumActiveAnalyticAquiferID()] (inclusive).
        const std::vector<int>& getIntegerAquiferConnectionData(const int aquiferID) const
        {
            return this->integerAnalyticAquiferConn_[aquiferID - 1].data();
        }

        /// Retrieve Floating-Point (Real) Aquifer Connection Data Array (analytic aquifers)
        ///
        /// \param[in] aquiferID Aquifer for which to retrieve single
        ///    precision floating point connection data array.  Expected to
        ///    be in the range [1..maximumActiveAnalyticAquiferID()]
        ///    (inclusive).
        const std::vector<float>& getSinglePrecAquiferConnectionData(const int aquiferID) const
        {
            return this->singleprecAnalyticAquiferConn_[aquiferID - 1].data();
        }

        /// Retrieve Floating-Point (Double Precision) Aquifer Connection
        /// Data Array (analytic aquifers)
        ///
        /// \param[in] aquiferID Aquifer for which to retrieve double
        ///    precision floating point connection data array.  Expected to
        ///    be in the range [1..maximumActiveAnalyticAquiferID()]
        ///    (inclusive).
        const std::vector<double>& getDoublePrecAquiferConnectionData(const int aquiferID) const
        {
            return this->doubleprecAnalyticAquiferConn_[aquiferID - 1].data();
        }

    private:
        int maxActiveAnalyticAquiferID_{0};

        std::vector<int> numActiveConn_{};
        std::vector<double> totalInflux_{};

        /// Aggregate 'IAAQ' array (Integer) for all analytic aquifers.
        WindowedArray<int> integerAnalyticAq_;

        /// Aggregate 'SAAQ' array (Real) for all analytic aquifers.
        WindowedArray<float> singleprecAnalyticAq_;

        /// Aggregate 'XAAQ' array (Double Precision) for all analytic aquifers.
        WindowedArray<double> doubleprecAnalyticAq_;

        /// Aggregate 'IAQN' array (integer) for all numeric aquifers.
        WindowedArray<int> integerNumericAq_;

        /// Aggregate 'RAQN' array (Double Precision) for all numeric aquifers.
        WindowedArray<double> doubleprecNumericAq_;

        /// Aggregate ICAQ array (Integer) for all analytic aquifer
        /// connections.  Separate array for each aquifer.
        std::vector<WindowedArray<int>> integerAnalyticAquiferConn_;

        /// Aggregate SCAQ array (Real) for all analytic aquifer
        /// connections.  Separate array for each aquifer.
        std::vector<WindowedArray<float>> singleprecAnalyticAquiferConn_;

        /// Aggregate ACAQ array (Double Precision) for all analytic aquifer
        /// connections.  Separate array for each aquifer.
        std::vector<WindowedArray<double>> doubleprecAnalyticAquiferConn_;
    };

}}} // Opm::RestartIO::Helpers

#endif // OPM_AGGREGATE_WELL_DATA_HPP
