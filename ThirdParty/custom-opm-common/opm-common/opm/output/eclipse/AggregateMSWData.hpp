/*
  Copyright (c) 2018 Statoil ASA

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

#ifndef OPM_AGGREGATE_MSW_DATA_HPP
#define OPM_AGGREGATE_MSW_DATA_HPP

#include <opm/output/data/Wells.hpp>
#include <opm/output/eclipse/WindowedArray.hpp>

#include <string>
#include <vector>

namespace Opm {
    class Phases;
    class Schedule;
    class EclipseGrid;
    class UnitSystem;
    class SummaryState;
} // Opm

namespace Opm { namespace RestartIO { namespace Helpers {

    struct BranchSegmentPar {
      int outletS;
      int noSegInBranch;
      int firstSeg;
      int lastSeg;
      int branch;
    };

    struct SegmentSetSourceSinkTerms {
      std::vector<double> qosc;
      std::vector<double> qwsc;
      std::vector<double> qgsc;
    };

    struct SegmentSetFlowRates {
      std::vector<double> sofr;
      std::vector<double> swfr;
      std::vector<double> sgfr;
    };

    class AggregateMSWData
    {
    public:
        explicit AggregateMSWData(const std::vector<int>& inteHead);

        void captureDeclaredMSWData(const Opm::Schedule& sched,
                                     const std::size_t    rptStep,
				     const Opm::UnitSystem& units,
				     const std::vector<int>& inteHead,
				     const Opm::EclipseGrid&  grid,
				     const Opm::SummaryState& smry,
				     const Opm::data::Wells&  wr
				   );

        /// Retrieve Integer Multisegment well data Array.
        const std::vector<int>& getISeg() const
        {
            return this->iSeg_.data();
        }

        /// Retrieve Double precision segment data Array.
        const std::vector<double>& getRSeg() const
        {
            return this->rSeg_.data();
        }

        /// Retrieve Integer multisegment well data Array for lateral branches  (ILBS)
        const std::vector<int>& getILBs() const
        {
            return this->iLBS_.data();
        }

        /// Retrieve Integer multisegment well data Array for lateral branches (ILBR)
        const std::vector<int>& getILBr() const
        {
            return this->iLBR_.data();
        }


    private:
        /// Aggregate 'ISEG' array (Integer) for all multisegment wells
        WindowedArray<int> iSeg_;

        /// Aggregate 'RSEG' array (Double) for all multisegment wells
        WindowedArray<double> rSeg_;

        /// Aggregate 'ILBS' array (Integer) for all multisegment wells
        WindowedArray<int> iLBS_;

        /// Aggregate 'ILBR' array (Integer) for all multisegment wells
        WindowedArray<int> iLBR_;

    };

}}} // Opm::RestartIO::Helpers

#endif // OPM_AGGREGATE_WELL_DATA_HPP
