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

#ifndef OPM_AGGREGATE_UDQ_DATA_HPP
#define OPM_AGGREGATE_UDQ_DATA_HPP

#include <opm/output/eclipse/WindowedArray.hpp>
#include <opm/io/eclipse/PaddedOutputString.hpp>

#include <cstddef>
#include <string>
#include <vector>
#include <map>

namespace Opm {
    class Schedule;
    class UDQInput;
    class UDQActive;
    class UDQState;
} // Opm

namespace Opm { namespace RestartIO { namespace Helpers {

class igphData {
public:
    const std::vector<int> ig_phase(const Opm::Schedule& sched, const std::size_t simStep, const std::vector<int>& inteHead);
};

class AggregateUDQData
{
public:
    explicit AggregateUDQData(const std::vector<int>& udqDims);

void captureDeclaredUDQData(const Opm::Schedule&                 sched,
                       const std::size_t                    simStep,
                       const Opm::UDQState&                 udqState,
                       const std::vector<int>&              inteHead);

    const std::vector<int>& getIUDQ() const
    {
        return this->iUDQ_.data();
    }

    const std::vector<int>& getIUAD() const
    {
        return this->iUAD_.data();
    }

    const std::vector<EclIO::PaddedOutputString<8>>& getZUDN() const
    {
        return this->zUDN_.data();
    }

    const std::vector<EclIO::PaddedOutputString<8>>& getZUDL() const
    {
        return this->zUDL_.data();
    }

    const std::vector<int>& getIGPH() const
    {
        return this->iGPH_.data();
    }

    const std::vector<int>& getIUAP() const
    {
        return this->iUAP_.data();
    }

    const std::vector<double>& getDUDW() const
    {
        return this->dUDW_.data();
    }

        const std::vector<double>& getDUDG() const
    {
        return this->dUDG_.data();
    }

    const std::vector<double>& getDUDF() const
    {
        return this->dUDF_.data();
    }

private:
    /// Aggregate 'IUDQ' array (Integer) for all UDQ data  (3 integers pr UDQ)
    WindowedArray<int> iUDQ_;

    /// Aggregate 'IUAD' array (Integer) for all UDQ data  (5 integers pr UDQ that is used for various well and group controls)
    WindowedArray<int> iUAD_;


    /// Aggregate 'ZUDN' array (Character) for all UDQ data. (2 * 8 chars pr UDQ -> UNIT keyword)
    WindowedArray<EclIO::PaddedOutputString<8>> zUDN_;

    /// Aggregate 'ZUDL' array (Character) for all UDQ data.  (16 * 8 chars pr UDQ DEFINE "Data for operation - Msth Expression)
    WindowedArray<EclIO::PaddedOutputString<8>> zUDL_;

    /// Aggregate 'IGPH' array (Integer) for all UDQ data  (3 - zeroes - as of current understanding)
    WindowedArray<int> iGPH_;

    /// Aggregate 'IUAP' array (ICharArrayNullTermnteger) for all UDQ data  (1 integer pr UDQ constraint used)
    WindowedArray<int> iUAP_;

    /// Aggregate 'DUDW' array (Double Precision) for all UDQ data. (Dimension = max no wells * noOfUDQ's)
    WindowedArray<double> dUDW_;

    /// Aggregate 'DUDG' array (Double Precision) for all UDQ data. (Dimension = (max no groups + 1) * noOfUDQ's)
    WindowedArray<double> dUDG_;

    /// Aggregate 'DUDF' array (Double Precision) for all UDQ data.  (Dimension = Number of FU - UDQ's, with value equal to the actual constraint)
    WindowedArray<double> dUDF_;


};

}}} // Opm::RestartIO::Helpers

#endif //OPM_AGGREGATE_WELL_DATA_HPP
