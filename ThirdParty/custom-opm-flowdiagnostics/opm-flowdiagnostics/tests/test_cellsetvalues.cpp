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

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_CELLSETVALUES

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/flowdiagnostics/CellSetValues.hpp>

using Opm::FlowDiagnostics::CellSetValues;

BOOST_AUTO_TEST_SUITE(CellSet_Values)

BOOST_AUTO_TEST_CASE (Constructor)
{
    {
        CellSetValues s{};
    }

    {
        auto s = CellSetValues{ 100 };
    }
}

BOOST_AUTO_TEST_CASE (AssignValues)
{
    auto s = CellSetValues{ 100 };

    for (decltype(s.cellValueCount())
             i = 0, n = 100;
         i < n; ++i)
    {
        s.addCellValue(100 - i, i * 10.0);
    }

    BOOST_CHECK_EQUAL(s.cellValueCount(), 100);

    {
        const auto a = s.cellValue(0);

        BOOST_CHECK_EQUAL(a.first , 100);
        BOOST_CHECK_CLOSE(a.second, 0.0, 1.0e-10);
    }

    {
        const auto a = s.cellValue(s.cellValueCount() - 1);

        BOOST_CHECK_EQUAL(a.first , 1);
        BOOST_CHECK_CLOSE(a.second, 990.0, 1.0e-10);
    }

    {
        const auto a = s.cellValue(50);

        BOOST_CHECK_EQUAL(a.first , 50);
        BOOST_CHECK_CLOSE(a.second, 500.0, 1.0e-10);
    }
}

BOOST_AUTO_TEST_SUITE_END()
