/*
   Copyright 2016 Statoil ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include "EclFilesComparator.hpp"

#include <opm/common/ErrorMacros.hpp>
#include <opm/io/eclipse/EGrid.hpp>

#include <stdio.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <set>
#include <type_traits>
#include <vector>

// helper macro to handle error throws or not
#define HANDLE_ERROR(type, message) \
  { \
    if (throwOnError) \
      OPM_THROW(type, message); \
    else { \
      std::cerr << message << std::endl; \
      ++num_errors; \
    } \
  }

using Opm::EclIO::EGrid;

template <typename T>
void ECLFilesComparator::printValuesForCell(const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, EGrid *grid, const T& value1, const T& value2) const {
    if (grid) {
        int nActive = grid->activeCells();
        int nTot = grid->totalNumberOfCells();

        if (static_cast<int>(kw_size) == nActive) {
            auto ijk = grid->ijk_from_active_index(cell);

            ijk[0]++, ijk[1]++, ijk[2]++;

            std::cout << std::endl
                      << "\nKeyword: " << keyword << ", origin "  << reference << "\n"
                      << "Global index (zero based)   = "  << cell << "\n"
                      << "Grid coordinate             = (" << ijk[0] << ", " << ijk[1] << ", " << ijk[2] << ")" << "\n"
                      << "(first value, second value) = (" << value1 << ", " << value2 << ")\n\n";
            return;
        }

        if (static_cast<int>(kw_size) == nTot) {

            auto ijk = grid->ijk_from_global_index(cell);

            ijk[0]++, ijk[1]++, ijk[2]++;

            std::cout << std::endl
                      << "\nKeyword: " << keyword << ", origin "  << reference << "\n\n"
                      << "Global index (zero based)   = "  << cell << "\n"
                      << "Grid coordinate             = (" << ijk[0] << ", " << ijk[1] << ", " << ijk[2] << ")" << "\n"
                      << "(first value, second value) = (" << value1 << ", " << value2 << ")\n\n";
            return;
        }
    }

    std::cout << std::endl
              << "\nKeyword: " << keyword << ", origin "  << reference << "\n\n"
              << "Value index                 = "  << cell << "\n"
              << "(first value, second value) = (" << value1 << ", " << value2 << ")\n\n";
}

template void ECLFilesComparator::printValuesForCell<bool>       (const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, EGrid *grid, const bool&        value1, const bool&        value2) const;
template void ECLFilesComparator::printValuesForCell<int>        (const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, EGrid *grid, const int&         value1, const int&         value2) const;
template void ECLFilesComparator::printValuesForCell<double>     (const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, EGrid *grid, const double&      value1, const double&      value2) const;
template void ECLFilesComparator::printValuesForCell<std::string>(const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, EGrid *grid, const std::string& value1, const std::string& value2) const;
// Hack to work around case where std::vector<bool>::const_reference is not a bool. If it is we will initialize printValuesForCell<char> otherwise printValuesForCell<std::vector<bool>::const_reference>
using boolConstReference = typename std::vector<bool>::const_reference;
using boolTypeHelper = typename std::remove_const<typename std::remove_reference<boolConstReference>::type>::type;
using boolType = typename std::conditional<std::is_same<boolTypeHelper, bool>::value, char, boolTypeHelper>::type;
template void ECLFilesComparator::printValuesForCell<boolType>       (const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, EGrid *grid, const boolType& value1, const boolType& value2) const;

ECLFilesComparator::ECLFilesComparator(const std::string& basename1,
                                       const std::string& basename2,
                                       double absToleranceArg, double relToleranceArg) :
    rootName1(basename1), rootName2(basename2),
    absTolerance(absToleranceArg), relTolerance(relToleranceArg) {
}


Deviation ECLFilesComparator::calculateDeviations(double val1, double val2) {
    val1 = std::abs(val1);
    val2 = std::abs(val2);
    Deviation deviation;
    if (val1 != 0 || val2 != 0) {
        deviation.abs = std::abs(val1 - val2);
        if (val1 != 0 && val2 != 0) {
            deviation.rel = deviation.abs/(std::max(val1, val2));
        }
    }
    return deviation;
}


double ECLFilesComparator::median(std::vector<double> vec) {
    if (vec.empty()) {
        return 0;
    }
    else {
        size_t n = vec.size() / 2;
        std::nth_element(vec.begin(), vec.begin() + n, vec.end());
        if (vec.size() % 2 == 0) {
            return 0.5 * (vec[n-1] + vec[n]);
        }
        else {
            return vec[n];
        }
    }
}


double ECLFilesComparator::average(const std::vector<double>& vec) {
    if (vec.empty()) {
        return 0;
    }
    double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
    return sum/vec.size();
}
