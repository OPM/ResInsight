/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

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

#include <config.h>

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_CELLSET

#include <boost/test/unit_test.hpp>

#include <opm/flowdiagnostics/CellSet.hpp>

#include <algorithm>


using Opm::FlowDiagnostics::CellSet;
using Opm::FlowDiagnostics::CellSetID;


BOOST_AUTO_TEST_SUITE(CellSetIDTest)

BOOST_AUTO_TEST_CASE (Construct)
{
    {
        const auto i = CellSetID{};

        BOOST_CHECK_EQUAL(i.to_string(), "");
    }

    {
        const auto name = std::string("Injection");

        const auto i = CellSetID(name);

        BOOST_CHECK_EQUAL(i.to_string(), name);
    }
    {
        const auto i1 = CellSetID("I-1");
        const auto i2 = CellSetID("I-2");
        BOOST_CHECK_EQUAL(i1 < i2, true);
        BOOST_CHECK_EQUAL(i1 < i2, i1.to_string() < i2.to_string());
    }
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(CellSetTest)

BOOST_AUTO_TEST_CASE (Constructor)
{
    {
        const auto name = std::string("Test-Ctor");

        auto s = CellSet{CellSetID(name)};

        BOOST_CHECK_EQUAL(s.id().to_string(), name);
    }
}

BOOST_AUTO_TEST_CASE (AssignCells)
{
    const auto cells = std::vector<int>
        { 0, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

    {
        // Using insert() to populate.
        auto s = CellSet{CellSetID("TestSet")};
        for (const auto& cell : cells) {
            s.insert(cell);
        }

        auto out = std::vector<int>(s.begin(), s.end());
        {
            std::sort(out.begin(), out.end());
        }

        BOOST_CHECK_EQUAL_COLLECTIONS(out  .begin(), out  .end(),
                                      cells.begin(), cells.end());
    }

    {
        // Using direct constructor to populate.
        auto s = CellSet{CellSetID("TestSet"), cells};

        auto out = std::vector<int>(s.begin(), s.end());
        {
            std::sort(out.begin(), out.end());
        }

        BOOST_CHECK_EQUAL_COLLECTIONS(out  .begin(), out  .end(),
                                      cells.begin(), cells.end());
    }
}


BOOST_AUTO_TEST_CASE (Duplicates)
{
    auto s = CellSet{CellSetID("TestSet")};

    const auto cells = std::vector<int>
        { 0, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

    for (auto i = 0; i < 2; ++i) {
        for (const auto& cell : cells) {
            s.insert(cell);
        }
    }

    auto out = std::vector<int>(s.begin(), s.end());
    {
        std::sort(out.begin(), out.end());
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(out  .begin(), out  .end(),
                                  cells.begin(), cells.end());
}


BOOST_AUTO_TEST_CASE (DuplicatesDirectConstruction)
{
    const auto cells = std::vector<int>
        { 0, 100, 100, 100, 2, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

    const auto expected = std::vector<int>
        { 0, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };

    auto s = CellSet{CellSetID("TestSet"), cells};

    auto out = std::vector<int>(s.begin(), s.end());
    {
        std::sort(out.begin(), out.end());
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(out     .begin(), out     .end(),
                                  expected.begin(), expected.end());
}


BOOST_AUTO_TEST_SUITE_END()
