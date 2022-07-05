/*
  Copyright 2021 Equinor ASA.

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

#include <opm/io/eclipse/RestartFileView.hpp>

#define BOOST_TEST_MODULE Test_RestartFileView
#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/ERst.hpp>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <random>
#include <tuple>
#include <type_traits>

namespace {

template <typename T>
T calcSum(const std::vector<T>& x)
{
    return std::accumulate(x.begin(), x.end(), T(0));
}

std::unique_ptr<Opm::EclIO::RestartFileView>
openRestart(const std::string& filename,
            const int          report_step)
{
    auto rst = std::make_shared<Opm::EclIO::ERst>(filename);
    return std::make_unique<Opm::EclIO::RestartFileView>
        (std::move(rst), report_step);
}
}

BOOST_AUTO_TEST_SUITE(Restart_File_View)

BOOST_AUTO_TEST_CASE(Load_Step_10)
{
    const std::vector<int> ref_icon_10 = {
        1,10,10,3,0,1,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,
        0,1,1,1,1,0,1,0,0,0,0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,
    };

    const std::vector<std::string> ref_zwel_10 = {"PROD","","","INJ","",""};

    const auto rst1 = openRestart("SPE1_TESTCASE.UNRST", 10);

    BOOST_CHECK_EQUAL(rst1->simStep(), 9ull);
    BOOST_CHECK_EQUAL(rst1->reportStep(), 10);

    BOOST_REQUIRE_MESSAGE(rst1->hasKeyword<int>("ICON"), "Restart file view must have ICON");
    BOOST_REQUIRE_MESSAGE(rst1->hasKeyword<float>("PRESSURE"), "Restart file view must have PRESSURE");
    BOOST_REQUIRE_MESSAGE(rst1->hasKeyword<double>("XGRP"), "Restart file view must have XGRP");
    BOOST_REQUIRE_MESSAGE(rst1->hasKeyword<std::string>("ZWEL"), "Restart file view must have ZWEL");

    const auto icon = rst1->getKeyword<int>("ICON");
    const auto pres = rst1->getKeyword<float>("PRESSURE");
    const auto xgrp = rst1->getKeyword<double>("XGRP");
    const auto zwel = rst1->getKeyword<std::string>("ZWEL");

    BOOST_CHECK_MESSAGE(icon == ref_icon_10, "ICON must equal reference");
    BOOST_CHECK_EQUAL(pres.size(), 300ull);
    BOOST_CHECK_CLOSE(calcSum(pres), 1.68803e+06, 1e-3);

    BOOST_CHECK_EQUAL(xgrp.size(), 360ull);
    BOOST_CHECK_CLOSE(calcSum(xgrp), 1.81382e+08, 1e-3);

    BOOST_CHECK_MESSAGE(zwel == ref_zwel_10, "ZWEL must equal reference");
}

BOOST_AUTO_TEST_SUITE_END()
