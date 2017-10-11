/*
  Copyright 2017 SINTEF ICT, Applied Mathematics.
  Copyright 2017 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

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

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_REGION_MAPPING

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/ECLRegionMapping.hpp>

#include <cstddef>
#include <exception>
#include <initializer_list>
#include <numeric>
#include <stdexcept>

namespace {
    std::vector<int> pvtnum(const std::size_t n = 10)
    {
        return std::vector<int>(n, 1);
    }

    std::vector<int> satnum()
    {
        return std::vector<int> {
            1, 1, 1, 2, 2,
            3, 3, 3, 2, 2,
        };
    }

    std::vector<int> linear(const std::vector<int>::size_type n)
    {
        auto i = std::vector<int>(n);

        std::iota(std::begin(i), std::end(i), 0);

        return i;
    }

    template <class Coll1, class Coll2>
    void equal_collection(const Coll1& c1, const Coll2& c2)
    {
        BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(c1), std::end(c1),
                                      std::begin(c2), std::end(c2));
    }
}

BOOST_AUTO_TEST_SUITE (Full_Region_Mapping)

BOOST_AUTO_TEST_CASE (Constructor_Failure)
{
    using RM = ::Opm::ECLRegionMapping;

    BOOST_CHECK_THROW(RM{ std::vector<int>{} },
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (Single_Region)
{
    const auto rm = ::Opm::ECLRegionMapping{ pvtnum(5) };

    // All cells in single region => active regions == single ID.
    {
        const auto expect_actreg = std::vector<int>{1};
        equal_collection(rm.activeRegions(), expect_actreg);
    }

    // Defaulted index subset => Index vector [0 .. reg.size()-1]
    {
        const auto expect_ix = std::vector<int>{ 0, 1, 2, 3, 4, };
        equal_collection(rm.regionSubset(), expect_ix);
    }

    // All cells in single region => region's subset of index vector is
    // [0 .. regionSubset().size()-1]
    {
        const auto expect_regix = std::vector<int>{ 0, 1, 2, 3, 4, };
        equal_collection(rm.getRegionIndices(1), expect_regix);
    }

    // Invalid region ID (outside configured subset) => logic_error.
    BOOST_CHECK_THROW(rm.getRegionIndices(1729),
                      std::logic_error);
}

BOOST_AUTO_TEST_CASE (Multiple_Regions)
{
    const auto rm = ::Opm::ECLRegionMapping{ satnum() };

    // Active regions returned in sorted order
    {
        const auto expect_actreg = std::vector<int>{1, 2, 3};
        equal_collection(rm.activeRegions(), expect_actreg);
    }

    // Defaulted index subset => Index vector [0 .. reg.size()-1]
    {
        const auto expect_ix = std::vector<int>{
            0, 1, 2, 3, 4,
            5, 6, 7, 8, 9,
        };
        equal_collection(rm.regionSubset(), expect_ix);
    }

    // Cells in multiple regions => Must verify correct subset mappings.
    {
        const auto expect_regix_1 = std::vector<int>{ 0, 1, 2, };
        const auto expect_regix_2 = std::vector<int>{ 3, 4,
                                                      8, 9 };
        const auto expect_regix_3 = std::vector<int>{ 5, 6, 7, };

        equal_collection(rm.getRegionIndices(1), expect_regix_1);
        equal_collection(rm.getRegionIndices(2), expect_regix_2);
        equal_collection(rm.getRegionIndices(3), expect_regix_3);
    }

    // Invalid region ID (outside configured subset) => logic_error.
    BOOST_CHECK_THROW(rm.getRegionIndices(1701),
                      std::logic_error);
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================

BOOST_AUTO_TEST_SUITE (Subset_Region_Mapping)

BOOST_AUTO_TEST_CASE (Single_Region_Subset)
{
    const auto rm = ::Opm::ECLRegionMapping{
        pvtnum(5), std::vector<int>{ 1, 3, 4 }
    };

    // All cells in single region => active regions == single ID.
    {
        const auto expect_actreg = std::vector<int>{1};
        equal_collection(rm.activeRegions(), expect_actreg);
    }

    // Explicit index subset => Index vector equal to this subset.
    {
        const auto expect_ix = std::vector<int>{ 1, 3, 4, };
        equal_collection(rm.regionSubset(), expect_ix);
    }

    // All cells in single region => region's subset of index vector is
    // [0 .. regionSubset().size()-1]
    {
        const auto expect_regix = std::vector<int>{ 0, 1, 2, };
        equal_collection(rm.getRegionIndices(1), expect_regix);
    }
}

BOOST_AUTO_TEST_CASE (Single_Region_MultiSampledSubset)
{
    const auto cellIDs =
        std::vector<int>{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    const auto rm = ::Opm::ECLRegionMapping{
        pvtnum(5), cellIDs
    };

    // All cells in single region => active regions == single ID.
    {
        const auto expect_actreg = std::vector<int>{1};
        equal_collection(rm.activeRegions(), expect_actreg);
    }

    // Explicit index subset => Index vector equal to this subset.
    {
        equal_collection(rm.regionSubset(), cellIDs);
    }

    // All cells in single region => region's subset of index vector is
    // [0 .. regionSubset().size()-1]
    {
        const auto expect_regix = linear(cellIDs.size());
        equal_collection(rm.getRegionIndices(1), expect_regix);
    }
}

BOOST_AUTO_TEST_CASE (Multi_Region_Subset)
{
    const auto cellIDs = std::vector<int> {
        0, 1, /* 2, */ /* 3, */ 4,
        5, 6, /* 7, */ /* 8, */ 9,
    };

    const auto rm = ::Opm::ECLRegionMapping{
        satnum(), cellIDs
    };

    // Active regions returned in sorted order (cell subset covers all
    // regions).
    {
        const auto expect_actreg = std::vector<int>{1, 2, 3};
        equal_collection(rm.activeRegions(), expect_actreg);
    }

    // Explicit index subset => Index vector must match this subset.
    {
        equal_collection(rm.regionSubset(), cellIDs);
    }

    // Cells in multiple regions => Must verify correct subset mappings.
    {
        const auto expect_regix_1 = std::vector<int>{ 0, 1, };
        const auto expect_regix_2 = std::vector<int>{ 2,
                                                      5, };
        const auto expect_regix_3 = std::vector<int>{ 3, 4, };

        equal_collection(rm.getRegionIndices(1), expect_regix_1);
        equal_collection(rm.getRegionIndices(2), expect_regix_2);
        equal_collection(rm.getRegionIndices(3), expect_regix_3);
    }
}

BOOST_AUTO_TEST_CASE (Multi_Region_MultiSampledSubset)
{
    const auto cellIDs = std::vector<int> {
        0, 0, 0, 0, 0, 0,       //  0 ..  5
        9, 9, 8, 8, 3, 4,       //  6 .. 11
        5, 5, 2, 2, 7, 7,       // 12 .. 17
        0, 0, 0, 0, 0, 0,       // 18 .. 23
    };

    const auto rm = ::Opm::ECLRegionMapping{
        satnum(), cellIDs
    };

    // Active regions returned in sorted order (cell subset covers all
    // regions).
    {
        const auto expect_actreg = std::vector<int>{1, 2, 3};
        equal_collection(rm.activeRegions(), expect_actreg);
    }

    // Explicit index subset => Index vector must match this subset.
    {
        equal_collection(rm.regionSubset(), cellIDs);
    }

    // Cells in multiple regions => Must verify correct subset mappings.
    {
        // Note: Index subsets appear in sorted order by construction.

        const auto expect_regix_1 = std::vector<int>{
             0,  1,  2,  3,  4,  5, //  0 ..  5
         //                             6 .. 11
                    14, 15,         // 12 .. 17
            18, 19, 20, 21, 22, 23, // 18 .. 23
        };

        const auto expect_regix_2 = std::vector<int>{
            //                          0 ..  5
             6,  7,  8,  9, 10, 11, //  6 .. 11
            //                         12 .. 17
            //                         18 .. 23
        };

        const auto expect_regix_3 = std::vector<int>{
            //                              0 ..  5
            //                              6 .. 11
            12, 13, /* 14, 15 */ 16, 17 // 12 .. 17
            //                             18 .. 23
        };

        equal_collection(rm.getRegionIndices(1), expect_regix_1);
        equal_collection(rm.getRegionIndices(2), expect_regix_2);
        equal_collection(rm.getRegionIndices(3), expect_regix_3);
    }
}

BOOST_AUTO_TEST_SUITE_END ()
