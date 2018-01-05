/*
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

#define BOOST_TEST_MODULE TEST_ECLPROPTABLE

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/utility/ECLPropTable.hpp>

#include <exception>
#include <stdexcept>

namespace {
    template <class Collection1, class Collection2>
    void check_is_close(const Collection1& c1, const Collection2& c2)
    {
        BOOST_REQUIRE_EQUAL(c1.size(), c2.size());

        if (! c1.empty()) {
            auto i1 = c1.begin(), e1 = c1.end();
            auto i2 = c2.begin();

            for (; i1 != e1; ++i1, ++i2) {
                BOOST_CHECK_CLOSE(*i1, *i2, 1.0e-10);
            }
        }
    }

    Opm::ECLPropTableRawData
    toRawTableFormat(Opm::ECLPropTableRawData t)
    {
        // Note: Raw table format is nTab*nRows consecutive values for one
        // column followed by nTab*nRows consecutive values for the next
        // column &c.

        const auto d          = t.data;
        const auto rTabStride = t.numRows * t.numCols;
        const auto wColStride = t.numRows * t.numTables;

        for (auto c = 0*t.numCols; c < t.numCols; ++c) {
            const auto wStart = c * wColStride;

            for (auto k = 0*t.numTables; k < t.numTables; ++k) {
                const auto rStart = k * rTabStride;
                const auto wOff   = k * t.numRows;

                for (auto i = 0*t.numRows; i < t.numRows; ++i) {
                    t.data[wStart + wOff + i] =
                        d [rStart + i*t.numCols + c];
                }
            }
        }

        return t;
    }

    Opm::SatFuncInterpolant::ConvertUnits
    createDummyUnitConverter(const std::size_t ncol)
    {
        using Cvrt = Opm::SatFuncInterpolant::ConvertUnits::Converter;

        auto id = [](const double x) { return x; };

        return Opm::SatFuncInterpolant::ConvertUnits {
            Cvrt{ id },
            std::vector<Cvrt>(ncol, Cvrt{ id })
        };
    }
} // Namespace Anonymous

// =====================================================================
// Invalid tables (error handling/input validation)
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (InvalidTables)

BOOST_AUTO_TEST_CASE (EmptyTable)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
    };

    t.numPrimary = 1;
    t.numRows    = 0;
    t.numCols    = 3;
    t.numTables  = 1;

    BOOST_CHECK_THROW(Opm::SatFuncInterpolant(toRawTableFormat(t),
                                              createDummyUnitConverter(2)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (SingleNode)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.3 , 0.1 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 1;
    t.numCols    = 3;
    t.numTables  = 1;

    BOOST_CHECK_THROW(Opm::SatFuncInterpolant(toRawTableFormat(t),
                                              createDummyUnitConverter(2)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (NoResultColumns)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s
        0.2,
        0.3,
        0.7,
        0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 1;
    t.numTables  = 1;

    BOOST_CHECK_THROW(Opm::SatFuncInterpolant(toRawTableFormat(t),
                                              createDummyUnitConverter(0)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (EmptyTableLargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s    , kr      , pc
        -1.0e+20, -1.0e+20, 0.0,
        -1.0e+20, -1.0e+20, 0.0,
        -1.0e+20, -1.0e+20, 0.0,
        -1.0e+20, -1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 8;
    t.numCols    = 3;
    t.numTables  = 1;

    BOOST_CHECK_THROW(Opm::SatFuncInterpolant(toRawTableFormat(t),
                                              createDummyUnitConverter(2)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (SingleNodeLargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s    , kr      , pc
        -1.0e+20, -1.0e+20, 0.0,
        -1.0e+20, -1.0e+20, 0.0,
        -1.0e+20, -1.0e+20, 0.0,
        -1.0e+20, -1.0e+20, 0.0,
        0.3     ,  0.1    , 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
        1.0e+20 ,  1.0e+20, 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 9;
    t.numCols    = 3;
    t.numTables  = 1;

    BOOST_CHECK_THROW(Opm::SatFuncInterpolant(toRawTableFormat(t),
                                              createDummyUnitConverter(2)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (NoResultColumnsLargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s
        -1.0e+20,
        -1.0e+20,
        -1.0e+20,
        -1.0e+20,
        0.2,
        0.3,
        0.7,
        0.8,
        1.0e+20,
        1.0e+20,
        1.0e+20,
        1.0e+20,
    };

    t.numPrimary =  1;
    t.numRows    = 12;
    t.numCols    =  1;
    t.numTables  =  1;

    BOOST_CHECK_THROW(Opm::SatFuncInterpolant(toRawTableFormat(t),
                                              createDummyUnitConverter(2)),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Single table (i.e., a single region).
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (InterpolationSingleTable)

BOOST_AUTO_TEST_CASE (AtNodes)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 1;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s         = std::vector<double>{ 0.8, 0.3, 0.3, 0.2 };
    const auto kr_expect = std::vector<double>{ 0.5, 0.1, 0.1, 0.0 };
    const auto pc_expect = std::vector<double>{ 0.0, 0.0, 0.0, 0.0 };

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    // Check interpolation

    const auto kr = swfunc.interpolate(InTable{0}, ResultColumn{0}, s);
    const auto pc = swfunc.interpolate(InTable{0}, ResultColumn{1}, s);

    check_is_close(kr, kr_expect);
    check_is_close(pc, pc_expect);

    // Check error handling

    // Table ID out of range.
    BOOST_CHECK_THROW(swfunc.interpolate(InTable{10}, ResultColumn{0}, s),
                      std::invalid_argument);

    // Result Column ID out of range.
    BOOST_CHECK_THROW(swfunc.interpolate(InTable{0}, ResultColumn{2}, s),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE (AboveAndBelow)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 1;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s         = std::vector<double>{ 0.80000001, 0.9, 0.199999999, 0.1 };
    const auto kr_expect = std::vector<double>{ 0.5,        0.5, 0.0,         0.0 };
    const auto pc_expect = std::vector<double>{ 0.0,        0.0, 0.0,         0.0 };

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto kr = swfunc.interpolate(InTable{0}, ResultColumn{0}, s);
    const auto pc = swfunc.interpolate(InTable{0}, ResultColumn{1}, s);

    check_is_close(kr, kr_expect);
    check_is_close(pc, pc_expect);
}

BOOST_AUTO_TEST_CASE (Interpolation)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 1;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s = std::vector<double>{
        0.2000,
        0.2300,
        0.2600,
        0.2900,
        0.3200,
        0.3500,
        0.3800,
        0.4100,
        0.4400,
        0.4700,
        0.5000,
        0.5300,
        0.5600,
        0.5900,
        0.6200,
        0.6500,
        0.6800,
        0.7100,
        0.7400,
        0.7700,
        0.8000,
    };

    const auto kr_expect = std::vector<double>{
        0,
        0.0300,
        0.0600,
        0.0900,
        0.1160,
        0.1400,
        0.1640,
        0.1880,
        0.2120,
        0.2360,
        0.2600,
        0.2840,
        0.3080,
        0.3320,
        0.3560,
        0.3800,
        0.4040,
        0.4280,
        0.4520,
        0.4760,
        0.5000,
    };

    const auto pc_expect = std::vector<double>(s.size(), 0.0);

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto kr = swfunc.interpolate(InTable{0}, ResultColumn{0}, s);
    const auto pc = swfunc.interpolate(InTable{0}, ResultColumn{1}, s);

    check_is_close(kr, kr_expect);
    check_is_close(pc, pc_expect);
}

BOOST_AUTO_TEST_CASE (InterpolationLargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.8    , 0.5      , 0.0, //  8
        1.0e20 , 1.0e+100 , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
    };

    t.numPrimary =  1;
    t.numRows    = 15;
    t.numCols    =  3;
    t.numTables  =  1;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s = std::vector<double>{
        0.0000,
        0.1000,
        0.1500,
        0.1900,
        0.2000,
        0.2300,
        0.2600,
        0.2900,
        0.3200,
        0.3500,
        0.3800,
        0.4100,
        0.4400,
        0.4700,
        0.5000,
        0.5300,
        0.5600,
        0.5900,
        0.6200,
        0.6500,
        0.6800,
        0.7100,
        0.7400,
        0.7700,
        0.8000,
        0.8100,
        0.8500,
        0.9000,
        1.0000,
    };

    const auto kr_expect = std::vector<double>{
        0,
        0,
        0,
        0,
        0,
        0.0300,
        0.0600,
        0.0900,
        0.1160,
        0.1400,
        0.1640,
        0.1880,
        0.2120,
        0.2360,
        0.2600,
        0.2840,
        0.3080,
        0.3320,
        0.3560,
        0.3800,
        0.4040,
        0.4280,
        0.4520,
        0.4760,
        0.5000,
        0.5000,
        0.5000,
        0.5000,
        0.5000,
    };

    const auto pc_expect = std::vector<double>(s.size(), 0.0);

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto kr = swfunc.interpolate(InTable{0}, ResultColumn{0}, s);
    const auto pc = swfunc.interpolate(InTable{0}, ResultColumn{1}, s);

    check_is_close(kr, kr_expect);
    check_is_close(pc, pc_expect);
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Multiple tables (i.e., multiple regions).
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (InterpolationFourTables)

BOOST_AUTO_TEST_CASE (AtNodes)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // Table 0
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 1
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 2
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 3
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 4;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s         = std::vector<double>{ 0.8, 0.3, 0.3, 0.2 };
    const auto kr_expect = std::vector<double>{ 0.5, 0.1, 0.1, 0.0 };
    const auto pc_expect = std::vector<double>{ 0.0, 0.0, 0.0, 0.0 };

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    for (auto ti = 0*t.numTables; ti < t.numTables; ++ti) {
        const auto kr = swfunc.interpolate(InTable{ti}, ResultColumn{0}, s);
        const auto pc = swfunc.interpolate(InTable{ti}, ResultColumn{1}, s);

        check_is_close(kr, kr_expect);
        check_is_close(pc, pc_expect);

        // Check error handling

        // Table ID out of range.
        BOOST_CHECK_THROW(swfunc.interpolate(InTable{10}, ResultColumn{0}, s),
                          std::invalid_argument);

        // Result Column ID out of range.
        BOOST_CHECK_THROW(swfunc.interpolate(InTable{0}, ResultColumn{2}, s),
                          std::invalid_argument);
    }
}

BOOST_AUTO_TEST_CASE (AboveAndBelow)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // Table 0
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 1
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 2
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 3
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 4;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s         = std::vector<double>{ 0.80000001, 0.9, 0.199999999, 0.1 };
    const auto kr_expect = std::vector<double>{ 0.5,        0.5, 0.0,         0.0 };
    const auto pc_expect = std::vector<double>{ 0.0,        0.0, 0.0,         0.0 };

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    for (auto ti = 0*t.numTables; ti < t.numTables; ++ti) {
        const auto kr = swfunc.interpolate(InTable{ti}, ResultColumn{0}, s);
        const auto pc = swfunc.interpolate(InTable{ti}, ResultColumn{1}, s);

        check_is_close(kr, kr_expect);
        check_is_close(pc, pc_expect);
    }
}

BOOST_AUTO_TEST_CASE (Interpolation)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // Table 0
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 1
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 2
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // Table 3
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 4;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s = std::vector<double>{
        0.2000,
        0.2300,
        0.2600,
        0.2900,
        0.3200,
        0.3500,
        0.3800,
        0.4100,
        0.4400,
        0.4700,
        0.5000,
        0.5300,
        0.5600,
        0.5900,
        0.6200,
        0.6500,
        0.6800,
        0.7100,
        0.7400,
        0.7700,
        0.8000,
    };

    const auto kr_expect = std::vector<double>{
        0,
        0.0300,
        0.0600,
        0.0900,
        0.1160,
        0.1400,
        0.1640,
        0.1880,
        0.2120,
        0.2360,
        0.2600,
        0.2840,
        0.3080,
        0.3320,
        0.3560,
        0.3800,
        0.4040,
        0.4280,
        0.4520,
        0.4760,
        0.5000,
    };

    const auto pc_expect = std::vector<double>(s.size(), 0.0);

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    for (auto ti = 0*t.numTables; ti < t.numTables; ++ti) {
        const auto kr = swfunc.interpolate(InTable{ti}, ResultColumn{0}, s);
        const auto pc = swfunc.interpolate(InTable{ti}, ResultColumn{1}, s);

        check_is_close(kr, kr_expect);
        check_is_close(pc, pc_expect);
    }
}

BOOST_AUTO_TEST_CASE (InterpolationLargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // Table 0
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.7    , 0.15     , 0.0, //  8
        0.8    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15

        // Table 1
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.7    , 0.15     , 0.0, //  8
        0.8    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15

        // Table 2
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.7    , 0.15     , 0.0, //  8
        0.8    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15

        // Table 3
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.7    , 0.15     , 0.0, //  8
        0.8    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
    };

    t.numPrimary =  1;
    t.numRows    = 15;
    t.numCols    =  3;
    t.numTables  =  4;

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    const auto s = std::vector<double>{
        0.0000,
        0.1000,
        0.1500,
        0.1900,
        0.2000,
        0.2300,
        0.2600,
        0.2900,
        0.3200,
        0.3500,
        0.3800,
        0.4100,
        0.4400,
        0.4700,
        0.5000,
        0.5300,
        0.5600,
        0.5900,
        0.6200,
        0.6500,
        0.6800,
        0.7100,
        0.7400,
        0.7700,
        0.8000,
        0.8100,
        0.8500,
        0.9000,
        1.0000,
    };

    const auto kr_expect = std::vector<double>{
        0,
        0,
        0,
        0,
        0,
        3.0000e-02,
        6.0000e-02,
        9.0000e-02,
        1.0250e-01,
        1.0625e-01,
        1.1000e-01,
        1.1375e-01,
        1.1750e-01,
        1.2125e-01,
        1.2500e-01,
        1.2875e-01,
        1.3250e-01,
        1.3625e-01,
        1.4000e-01,
        1.4375e-01,
        1.4750e-01,
        1.8500e-01,
        2.9000e-01,
        3.9500e-01,
        5.0000e-01,
        5.0000e-01,
        5.0000e-01,
        5.0000e-01,
        5.0000e-01,
    };

    const auto pc_expect = std::vector<double>(s.size(), 0.0);

    using InTable      = Opm::SatFuncInterpolant::InTable;
    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    for (auto ti = 0*t.numTables; ti < t.numTables; ++ti) {
        const auto kr = swfunc.interpolate(InTable{ti}, ResultColumn{0}, s);
        const auto pc = swfunc.interpolate(InTable{ti}, ResultColumn{1}, s);

        check_is_close(kr, kr_expect);
        check_is_close(pc, pc_expect);
    }
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Single Table End-Points
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (SingleTableEndPoints)

BOOST_AUTO_TEST_CASE (SWFN_CritIsConn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 1;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2 };
    const auto scrit_expect = std::vector<double>{ 0.2 };
    const auto smax_expect  = std::vector<double>{ 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SWFN_CritIsConn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.8    , 0.5      , 0.0, //  8
        1.0e20 , 1.0e+100 , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
    };

    t.numPrimary =  1;
    t.numRows    = 15;
    t.numCols    =  3;
    t.numTables  =  1;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2 };
    const auto scrit_expect = std::vector<double>{ 0.2 };
    const auto smax_expect  = std::vector<double>{ 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SWFN)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 3;
    t.numTables  = 1;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2  };
    const auto scrit_expect = std::vector<double>{ 0.21 };
    const auto smax_expect  = std::vector<double>{ 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SWFN_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.21   , 0.0      , 0.0, //  7
        0.3    , 0.1      , 0.0, //  8
        0.8    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
        1.0e20 , 1.0e+100 , 0.0, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  1;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2 };
    const auto scrit_expect = std::vector<double>{ 0.21 };
    const auto smax_expect  = std::vector<double>{ 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SOF3_CritIsConn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2 };
    const auto scrit_krow_expect = std::vector<double>{ 0.2 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2 };
    const auto smax_expect       = std::vector<double>{ 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_CritIsConn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.3    ,  0.1     ,  0.5,      //  7
        0.8    ,  0.5     ,  0.8,      //  8
        1.0e20 ,  1.0e+100,  1.0e+100, //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
    };

    t.numPrimary =  1;
    t.numRows    = 15;
    t.numCols    =  3;
    t.numTables  =  1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2 };
    const auto scrit_krow_expect = std::vector<double>{ 0.2 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2 };
    const auto smax_expect       = std::vector<double>{ 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOGCR_is_Conn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.1,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 3;
    t.numTables  = 1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.21 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2  };
    const auto smax_expect       = std::vector<double>{ 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOGCR_is_Conn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.0     ,  0.1,      //  7
        0.3    ,  0.1     ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.21 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2  };
    const auto smax_expect       = std::vector<double>{ 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOWCR_is_Conn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.1 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 3;
    t.numTables  = 1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.2  };
    const auto scrit_krog_expect = std::vector<double>{ 0.21 };
    const auto smax_expect       = std::vector<double>{ 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOWCR_is_Conn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.1     ,  0.0,      //  7
        0.3    ,  0.1     ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.2  };
    const auto scrit_krog_expect = std::vector<double>{ 0.21 };
    const auto smax_expect       = std::vector<double>{ 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SCR_Not_Conn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2  , 0.0 , 0.0,
        0.205, 0.0 , 0.0,
        0.21 , 0.0 , 0.1,
        0.25 , 0.1 , 0.2,
        0.3  , 0.1 , 0.5,
        0.8  , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 6;
    t.numCols    = 3;
    t.numTables  = 1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2   };
    const auto scrit_krow_expect = std::vector<double>{ 0.21  };
    const auto scrit_krog_expect = std::vector<double>{ 0.205 };
    const auto smax_expect       = std::vector<double>{ 0.8   };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SCR_Not_Conn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.2    ,  0.0     ,  0.0     , //  5
        0.205  ,  0.0     ,  0.0     , //  6
        0.21   ,  0.0     ,  0.1     , //  7
        0.25   ,  0.1     ,  0.2     , //  8
        0.3    ,  0.1     ,  0.5     , //  9
        0.8    ,  0.5     ,  0.8     , // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  1;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2   };
    const auto scrit_krow_expect = std::vector<double>{ 0.21  };
    const auto scrit_krog_expect = std::vector<double>{ 0.205 };
    const auto smax_expect       = std::vector<double>{ 0.8   };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_SUITE_END ()

// =====================================================================
// Table End-Points in Multiple Tables
// ---------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (TableEndPointsMultiTable)

BOOST_AUTO_TEST_CASE (SWFN_CritIsConn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 4;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto scrit_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto smax_expect  = std::vector<double>{ 0.8, 0.8, 0.8, 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SWFN_CritIsConn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.8    , 0.5      , 0.0, //  8
        1.0e20 , 1.0e+100 , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15

        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.8    , 0.5      , 0.0, //  8
        1.0e20 , 1.0e+100 , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15

        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.8    , 0.5      , 0.0, //  8
        1.0e20 , 1.0e+100 , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15

        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.3    , 0.1      , 0.0, //  7
        0.8    , 0.5      , 0.0, //  8
        1.0e20 , 1.0e+100 , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
    };

    t.numPrimary =  1;
    t.numRows    = 15;
    t.numCols    =  3;
    t.numTables  =  4;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto scrit_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto smax_expect  = std::vector<double>{ 0.8, 0.8, 0.8, 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SWFN)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,

        // s, kr  , pc
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.0,
        0.3 , 0.1 , 0.0,
        0.8 , 0.5 , 0.0,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 3;
    t.numTables  = 4;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2 , 0.2 , 0.2 , 0.2  };
    const auto scrit_expect = std::vector<double>{ 0.21, 0.21, 0.21, 0.21 };
    const auto smax_expect  = std::vector<double>{ 0.8 , 0.8 , 0.8 , 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SWFN_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    // 1e20 is a sentinel value that counts as row "ignored".
    t.data = std::vector<double>{
        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        -1.0e20, -1.0e+100, 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.21   , 0.0      , 0.0, //  7
        0.3    , 0.1      , 0.0, //  8
        0.7    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
        1.0e20 , 1.0e+100 , 0.0, // 16

        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        0.1    , 0.0      , 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.21   , 0.0      , 0.0, //  7
        0.3    , 0.1      , 0.0, //  8
        0.8    , 0.5      , 0.0, //  9
        1.0e20 , 1.0e+100 , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
        1.0e20 , 1.0e+100 , 0.0, // 16

        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        -1.0e20, -1.0e+100, 0.0, //  4
        0.1    , 0.0      , 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.21   , 0.0      , 0.0, //  7
        0.3    , 0.1      , 0.0, //  8
        0.5    , 0.35     , 0.0, //  9
        0.9    , 0.5      , 0.0, // 10
        1.0e20 , 1.0e+100 , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
        1.0e20 , 1.0e+100 , 0.0, // 16

        // s   , kr       , pc
        -1.0e20, -1.0e+100, 0.0, //  1
        -1.0e20, -1.0e+100, 0.0, //  2
        -1.0e20, -1.0e+100, 0.0, //  3
        0.0    , 0.0      , 0.0, //  4
        0.1    , 0.0      , 0.0, //  5
        0.2    , 0.0      , 0.0, //  6
        0.21   , 0.0      , 0.0, //  7
        0.3    , 0.1      , 0.0, //  8
        0.5    , 0.35     , 0.0, //  9
        0.8    , 0.5      , 0.0, // 10
        0.95   , 0.5      , 0.0, // 11
        1.0e20 , 1.0e+100 , 0.0, // 12
        1.0e20 , 1.0e+100 , 0.0, // 13
        1.0e20 , 1.0e+100 , 0.0, // 14
        1.0e20 , 1.0e+100 , 0.0, // 15
        1.0e20 , 1.0e+100 , 0.0, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  4;

    // Table end-points
    const auto sconn_expect = std::vector<double>{ 0.2 , 0.1 , 0.1 , 0.0  };
    const auto scrit_expect = std::vector<double>{ 0.21, 0.21, 0.21, 0.21 };
    const auto smax_expect  = std::vector<double>{ 0.7 , 0.8 , 0.9 , 0.95 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn = swfunc.connateSat();
    const auto scrit = swfunc.criticalSat(ResultColumn{0});
    const auto smax  = swfunc.maximumSat();

    check_is_close(sconn, sconn_expect);
    check_is_close(scrit, scrit_expect);
    check_is_close(smax , smax_expect );
}

BOOST_AUTO_TEST_CASE (SOF3_CritIsConn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 3;
    t.numCols    = 3;
    t.numTables  = 4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto scrit_krow_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto smax_expect       = std::vector<double>{ 0.8, 0.8, 0.8, 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_CritIsConn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.3    ,  0.1     ,  0.5,      //  7
        0.8    ,  0.5     ,  0.8,      //  8
        1.0e20 ,  1.0e+100,  1.0e+100, //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.1    ,  0.0     ,  0.0,      //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.3    ,  0.1     ,  0.5,      //  7
        0.8    ,  0.5     ,  0.8,      //  8
        1.0e20 ,  1.0e+100,  1.0e+100, //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        0.0    ,  0.0     ,  0.0,      //  4
        0.15   ,  0.0     ,  0.0,      //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.3    ,  0.1     ,  0.5,      //  7
        0.7    ,  0.35    ,  0.8,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.1    ,  0.0     ,  0.0,      //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.3    ,  0.1     ,  0.5,      //  7
        0.8    ,  0.5     ,  0.8,      //  8
        0.9    ,  0.85    ,  0.8,      //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
    };

    t.numPrimary =  1;
    t.numRows    = 15;
    t.numCols    =  3;
    t.numTables  =  4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2, 0.1, 0.0, 0.1 };
    const auto scrit_krow_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2, 0.2, 0.2, 0.2 };
    const auto smax_expect       = std::vector<double>{ 0.8, 0.8, 0.8, 0.9 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOGCR_is_Conn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.1,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.1,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.1,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.0 , 0.1,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 3;
    t.numTables  = 4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2 , 0.2 , 0.2 , 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.21, 0.21, 0.21, 0.21 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2 , 0.2 , 0.2 , 0.2  };
    const auto smax_expect       = std::vector<double>{ 0.8 , 0.8 , 0.8 , 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOGCR_is_Conn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.0     ,  0.1,      //  7
        0.3    ,  0.1     ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.0     ,  0.1,      //  7
        0.25   ,  0.0     ,  0.5,      //  8
        0.3    ,  0.1     ,  0.5,      //  9
        0.8    ,  0.5     ,  0.8,      // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.1    ,  0.0     ,  0.0,      //  5
        0.2    ,  0.0     ,  0.00001,  //  6
        0.21   ,  0.0     ,  0.1,      //  7
        0.3    ,  0.1     ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        0.9    ,  0.75    ,  0.9,      // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.0     ,  0.1,      //  7
        0.3    ,  0.0     ,  0.25,     //  8
        0.4    ,  0.00001 ,  0.30,     //  9
        0.5    ,  0.1     ,  0.5,      // 10
        0.8    ,  0.5     ,  0.8,      // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2 , 0.2 , 0.1 , 0.2 };
    const auto scrit_krow_expect = std::vector<double>{ 0.21, 0.25, 0.21, 0.3 };
    const auto scrit_krog_expect = std::vector<double>{ 0.2 , 0.2 , 0.1 , 0.2 };
    const auto smax_expect       = std::vector<double>{ 0.8 , 0.8 , 0.9 , 0.8 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOWCR_is_Conn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.1 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.1 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.1 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,

        // s, krow, krog
        0.2 , 0.0 , 0.0,
        0.21, 0.1 , 0.0,
        0.3 , 0.1 , 0.5,
        0.8 , 0.5 , 0.8,
    };

    t.numPrimary = 1;
    t.numRows    = 4;
    t.numCols    = 3;
    t.numTables  = 4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2 , 0.2 , 0.2 , 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.2 , 0.2 , 0.2 , 0.2  };
    const auto scrit_krog_expect = std::vector<double>{ 0.21, 0.21, 0.21, 0.21 };
    const auto smax_expect       = std::vector<double>{ 0.8 , 0.8 , 0.8 , 0.8  };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SOWCR_is_Conn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.1     ,  0.0,      //  7
        0.3    ,  0.1     ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        1.0e20 ,  1.0e+100,  1.0e+100, // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.1     ,  0.0,      //  7
        0.25   ,  0.15    ,  0.0,      //  8
        0.3    ,  0.1     ,  0.5,      //  9
        0.8    ,  0.5     ,  0.8,      // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.0    ,  0.0     ,  0.0,      //  5
        0.1    ,  0.000001,  0.0,      //  6
        0.21   ,  0.1     ,  0.0,      //  7
        0.3    ,  0.15    ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        0.82   ,  0.6     ,  0.8,      // 10
        0.9    ,  0.8     ,  0.89,     // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        -1.0e20, -1.0e+100, -1.0e+100, //  5
        0.2    ,  0.0     ,  0.0,      //  6
        0.21   ,  0.1     ,  0.0,      //  7
        0.3    ,  0.1     ,  0.5,      //  8
        0.8    ,  0.5     ,  0.8,      //  9
        0.80001,  0.500001,  0.8,      // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2 , 0.2 , 0.0 , 0.2     };
    const auto scrit_krow_expect = std::vector<double>{ 0.2 , 0.2 , 0.0 , 0.2     };
    const auto scrit_krog_expect = std::vector<double>{ 0.21, 0.25, 0.21, 0.21    };
    const auto smax_expect       = std::vector<double>{ 0.8 , 0.8 , 0.9 , 0.80001 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SCR_Not_Conn)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s, krow, krog
        0.2  , 0.0 , 0.0,
        0.205, 0.0 , 0.0,
        0.21 , 0.0 , 0.1,
        0.25 , 0.1 , 0.2,
        0.3  , 0.1 , 0.5,
        0.8  , 0.5 , 0.8,

        // s, krow, krog
        0.2  , 0.0 , 0.0,
        0.206, 0.0 , 0.0,
        0.211, 0.0 , 0.1,
        0.25 , 0.1 , 0.2,
        0.3  , 0.1 , 0.5,
        0.8  , 0.5 , 0.8,

        // s, krow, krog
        0.2  , 0.0 , 0.0,
        0.207, 0.0 , 0.0,
        0.212, 0.0 , 0.1,
        0.25 , 0.1 , 0.2,
        0.3  , 0.1 , 0.5,
        0.85 , 0.6 , 0.8,

        // s, krow, krog
        0.2  , 0.0 , 0.0,
        0.209, 0.0 , 0.0,
        0.225, 0.0 , 0.1,
        0.25 , 0.1 , 0.2,
        0.3  , 0.1 , 0.5,
        0.9  , 0.8 , 0.9,
    };

    t.numPrimary = 1;
    t.numRows    = 6;
    t.numCols    = 3;
    t.numTables  = 4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2  , 0.2  , 0.2  , 0.2   };
    const auto scrit_krow_expect = std::vector<double>{ 0.21 , 0.211, 0.212, 0.225 };
    const auto scrit_krog_expect = std::vector<double>{ 0.205, 0.206, 0.207, 0.209 };
    const auto smax_expect       = std::vector<double>{ 0.8  , 0.8  , 0.85 , 0.9   };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_CASE (SOF3_SCR_Not_Conn_LargeNodeAlloc)
{
    auto t = Opm::ECLPropTableRawData{};

    t.data = std::vector<double>{
        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.2    ,  0.0     ,  0.0     , //  5
        0.205  ,  0.0     ,  0.0     , //  6
        0.21   ,  0.0     ,  0.1     , //  7
        0.25   ,  0.1     ,  0.2     , //  8
        0.3    ,  0.1     ,  0.5     , //  9
        0.8    ,  0.5     ,  0.8     , // 10
        1.0e20 ,  1.0e+100,  1.0e+100, // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        0.1    ,  0.0     ,  0.0     , //  4
        0.2    ,  0.0     ,  0.0     , //  5
        0.205  ,  0.0     ,  0.0     , //  6
        0.21   ,  0.0     ,  0.1     , //  7
        0.25   ,  0.1     ,  0.2     , //  8
        0.3    ,  0.15    ,  0.5     , //  9
        0.5    ,  0.25    ,  0.6     , // 10
        0.8    ,  0.5     ,  0.8     , // 11
        0.9    ,  0.75    ,  0.9     , // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.2    ,  0.0     ,  0.0     , //  5
        0.205  ,  0.000001,  0.000001, //  6
        0.21   ,  0.000002,  0.1     , //  7
        0.25   ,  0.1     ,  0.2     , //  8
        0.3    ,  0.1     ,  0.5     , //  9
        0.8    ,  0.5     ,  0.8     , // 10
        0.825  ,  0.75    ,  0.825   , // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16

        // s   ,  krow    ,  krog
        -1.0e20, -1.0e+100, -1.0e+100, //  1
        -1.0e20, -1.0e+100, -1.0e+100, //  2
        -1.0e20, -1.0e+100, -1.0e+100, //  3
        -1.0e20, -1.0e+100, -1.0e+100, //  4
        0.2    ,  0.0     ,  0.0     , //  5
        0.205  ,  0.0     ,  0.0     , //  6
        0.21   ,  0.0     ,  0.0     , //  7
        0.25   ,  0.0     ,  0.0     , //  8
        0.3    ,  0.0     ,  0.000001, //  9
        0.8    ,  0.0     ,  0.8     , // 10
        0.99   ,  0.000001,  0.99    , // 11
        1.0e20 ,  1.0e+100,  1.0e+100, // 12
        1.0e20 ,  1.0e+100,  1.0e+100, // 13
        1.0e20 ,  1.0e+100,  1.0e+100, // 14
        1.0e20 ,  1.0e+100,  1.0e+100, // 15
        1.0e20 ,  1.0e+100,  1.0e+100, // 16
    };

    t.numPrimary =  1;
    t.numRows    = 16;
    t.numCols    =  3;
    t.numTables  =  4;

    // Table end-points
    const auto sconn_expect      = std::vector<double>{ 0.2  , 0.1  , 0.2  , 0.2  };
    const auto scrit_krow_expect = std::vector<double>{ 0.21 , 0.21 , 0.2  , 0.8  };
    const auto scrit_krog_expect = std::vector<double>{ 0.205, 0.205, 0.2  , 0.25 };
    const auto smax_expect       = std::vector<double>{ 0.8  , 0.9  , 0.825, 0.99 };

    // Note: Need to convert input table to column major (Fortran) order
    // because that is the format in which PropTable1D expects the tabular
    // data.
    const auto swfunc = Opm::SatFuncInterpolant {
        toRawTableFormat(t),
        createDummyUnitConverter(t.numCols - 1)
    };

    // Check number of tables
    BOOST_CHECK_EQUAL(swfunc.numTables(), t.numTables);

    using ResultColumn = Opm::SatFuncInterpolant::ResultColumn;

    const auto sconn      = swfunc.connateSat();
    const auto scrit_krow = swfunc.criticalSat(ResultColumn{0});
    const auto scrit_krog = swfunc.criticalSat(ResultColumn{1});
    const auto smax       = swfunc.maximumSat();

    check_is_close(sconn     , sconn_expect     );
    check_is_close(scrit_krow, scrit_krow_expect);
    check_is_close(scrit_krog, scrit_krog_expect);
    check_is_close(smax      , smax_expect      );
}

BOOST_AUTO_TEST_SUITE_END ()
