/*
   Copyright 2016 Statoil ASA.

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

#ifndef ECLFILESCOMPARATOR_HPP
#define ECLFILESCOMPARATOR_HPP

#include "Deviation.hpp"

#include <map>
#include <stddef.h>
#include <string>
#include <vector>

namespace Opm { namespace EclIO {
    class EGrid;
}} // namespace Opm::ecl

class ECLFilesComparator {
public:
    ECLFilesComparator(const std::string& basename1,
                       const std::string& basename2,
                       double absTolerance, double relTolerance);

    void throwOnErrors(bool dothrow) {
        throwOnError = dothrow;
    }

    void doAnalysis(bool analize) {
        analysis = analize;
    }

    size_t getNoErrors() const {
        return num_errors;
    }

    //! \brief Returns the absolute tolerance stored as a private member variable in the class
    double getAbsTolerance() const {
        return absTolerance;
    }
    //! \brief Returns the relative tolerance stored as a private member variable in the class
    double getRelTolerance() const {
        return relTolerance;
    }

    //! \brief Calculate deviations for two values.
    //! \details Using absolute values of the input arguments: If one of the values are non-zero, the Deviation::abs returned is the difference between the two input values. In addition, if both values are non-zero, the Deviation::rel returned is the absolute deviation divided by the largest value.
    static Deviation calculateDeviations(double val1, double val2);
    //! \brief Calculate median of a vector.
    //! \details Returning the median of the input vector, i.e. the middle value of the sorted vector if the number of elements is odd or the mean of the two middle values if the number of elements are even. Copy is intentional.
    static double median(std::vector<double> vec);
    //! \brief Calculate average of a vector.
    //! \details Returning the average of the input vector, i.e. the sum of all values divided by the number of elements.
    static double average(const std::vector<double>& vec);

protected:
    bool throwOnError = true; //!< Throw on first error
    bool analysis = false; //!< Perform full error analysis
    std::map<std::string, std::vector<Deviation>> deviations;
    mutable size_t num_errors = 0;

    std::string rootName1, rootName2;

    template <typename T>
    void printValuesForCell(const std::string& keyword,
                            const std::string& reference,
                            size_t kw_size,
                            size_t cell,
                            Opm::EclIO::EGrid *grid,
                            const T& value1,
                            const T& value2) const;

private:
    double absTolerance      = 0;
    double relTolerance      = 0;
};

#endif
