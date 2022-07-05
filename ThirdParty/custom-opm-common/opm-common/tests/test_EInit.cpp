/*
   +   Copyright 2019 Equinor ASA.
   +
   +   This file is part of the Open Porous Media project (OPM).
   +
   +   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   +   the Free Software Foundation, either version 3 of the License, or
   +   (at your option) any later version.
   +
   +   OPM is distributed in the hope that it will be useful,
   +   but WITHOUT ANY WARRANTY; without even the implied warranty of
   +   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   +   GNU General Public License for more details.
   +
   +   You should have received a copy of the GNU General Public License
   +   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   +   */

#include "config.h"

#include <opm/io/eclipse/EInit.hpp>

#define BOOST_TEST_MODULE Test EclIO
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <iterator>

#include <opm/common/utility/FileSystem.hpp>

using namespace Opm::EclIO;


template <typename T>
bool operator==(const std::vector<T> & t1, const std::vector<T> & t2)
{
    return std::equal(t1.begin(), t1.end(), t2.begin(), t2.end());
}


BOOST_AUTO_TEST_CASE(TestEInit_1) {

    std::string testInitFile = "LGR_TESTMOD.INIT";

    const std::vector<std::string> ref_lgr_list = {"LGR1", "LGR2"};

    const std::vector<std::string> ref_global_names = {"CON", "DEPTH", "DOUBHEAD", "DX",
        "DY", "DZ", "ENDNUM", "EQLNUM", "FIPNUM", "INTEHEAD", "KRG", "KRGR", "KRO", "KRORG",
        "KRORW", "KRW", "KRWR", "LOGIHEAD", "MINPVV", "MULTPV", "MULTX", "MULTY", "MULTZ",
        "NTG", "PCG", "PCW", "PERMX", "PERMY", "PERMZ", "PORO", "PORV", "PVTNUM", "SATNUM",
        "SGCR", "SGL", "SGLPC", "SGU", "SOGCR", "SOWCR", "SWATINIT", "SWCR", "SWL", "SWLPC",
        "SWU", "TAB", "TABDIMS", "TOPS", "TRANNNC", "TRANX", "TRANY", "TRANZ" };

    const std::vector<int> ref_global_size = {3, 30, 229, 30, 30, 30, 30, 30, 30, 411,
        30, 30, 30, 30, 30, 30, 30, 121, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
        30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 6821, 100, 30, 8, 30, 30, 30 };

    const std::vector<std::string> ref_lgr1_names = {"DEPTH", "DOUBHEAD", "DX", "DY", "DZ",
        "ENDNUM", "EQLNUM", "FIPNUM", "INTEHEAD", "KRG", "KRGR", "KRO", "KRORG", "KRORW",
        "KRW", "KRWR", "LGRHEADD", "LGRHEADI", "LGRHEADQ", "LOGIHEAD", "MINPVV", "MULTPV",
        "MULTX", "MULTY", "MULTZ", "NTG", "PCG", "PCW", "PERMX", "PERMY", "PERMZ", "PORO",
        "PORV", "PVTNUM", "SATNUM", "SGCR", "SGL", "SGLPC", "SGU", "SOGCR", "SOWCR",
        "SWATINIT", "SWCR", "SWL", "SWLPC", "SWU", "TOPS", "TRANGL", "TRANNNC", "TRANX",
        "TRANY", "TRANZ" };

    const std::vector<int> ref_lgr1_size = {128, 229, 128, 128, 128, 128, 128, 128, 411, 128,
        128, 128, 128, 128, 128, 128, 5, 45, 5, 121, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 84, 12, 128, 128, 128 };

    const std::vector<std::string> ref_lgr2_names = {"DEPTH", "DOUBHEAD", "DR", "DTHETA", "DZ",
        "EDTTRANX", "EDTTRANY", "EDTTRANZ", "ENDNUM", "EQLNUM", "FIPNUM", "INTEHEAD", "KRG",
        "KRGR", "KRO", "KRORG", "KRORW", "KRW", "KRWR", "LGRHEADD", "LGRHEADI", "LGRHEADQ",
        "LOGIHEAD", "MINPVV", "MULTPV", "MULTR", "MULTTHT", "MULTZ", "NTG", "PCG", "PCW", "PERMR",
        "PERMTHT", "PERMZ", "PORO", "PORV", "PVTNUM", "SATNUM", "SGCR", "SGL", "SGLPC", "SGU",
        "SOGCR", "SOWCR", "SWATINIT", "SWCR", "SWL", "SWLPC", "SWU", "TOPS", "TRANGL", "TRANNNC",
        "TRANR", "TRANTHT", "TRANZ" };

    const std::vector<int> ref_lgr2_size = { 192, 229, 192, 192, 192, 192, 192, 192, 192, 192,
        192, 411, 192, 192, 192, 192, 192, 192, 192, 5, 45, 5, 121, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
        192, 192, 192, 192, 192, 58, 60, 192, 192, 192 };

    EInit init1(testInitFile);

    auto lgr_list = init1.list_of_lgrs();

    BOOST_CHECK_EQUAL(ref_lgr_list == lgr_list, true);
    auto global_arrays = init1.list_arrays();

    BOOST_CHECK_EQUAL(global_arrays.size() == ref_global_names.size(), true);
    BOOST_CHECK_EQUAL(global_arrays.size() == ref_global_size.size(), true);


    {
        std::vector<std::string> tmpg0;
        std::vector<int> tmpg2;

        for (auto element : global_arrays){
            tmpg0.push_back(std::get<0>(element));
            tmpg2.push_back(std::get<2>(element));
        }

        BOOST_CHECK_EQUAL(ref_global_names == tmpg0, true);
        BOOST_CHECK_EQUAL(ref_global_size == tmpg2, true);
    }


    BOOST_CHECK_THROW(init1.list_arrays("XXXX") , std::invalid_argument );


    auto lgr1_arrays = init1.list_arrays("LGR1");

    BOOST_CHECK_EQUAL(lgr1_arrays.size() == ref_lgr1_names.size(), true);
    BOOST_CHECK_EQUAL(lgr1_arrays.size() == ref_lgr1_size.size(), true);


    {
        std::vector<std::string> tmplgr1_0;
        std::vector<int> tmplgr1_2;

        for (auto element : lgr1_arrays){
            tmplgr1_0.push_back(std::get<0>(element));
            tmplgr1_2.push_back(std::get<2>(element));
        }

        BOOST_CHECK_EQUAL(ref_lgr1_names == tmplgr1_0, true);
        BOOST_CHECK_EQUAL(ref_lgr1_size == tmplgr1_2, true);
    }


    auto lgr2_arrays = init1.list_arrays("LGR2");

    BOOST_CHECK_EQUAL(lgr2_arrays.size() == ref_lgr2_names.size(), true);
    BOOST_CHECK_EQUAL(lgr2_arrays.size() == ref_lgr2_size.size(), true);


    {
        std::vector<std::string> tmplgr2_0;
        std::vector<int> tmplgr2_2;

        for (auto element : lgr2_arrays){
            tmplgr2_0.push_back(std::get<0>(element));
            tmplgr2_2.push_back(std::get<2>(element));
        }

        BOOST_CHECK_EQUAL(ref_lgr2_names == tmplgr2_0, true);
        BOOST_CHECK_EQUAL(ref_lgr2_size == tmplgr2_2, true);
    }
}

BOOST_AUTO_TEST_CASE(TestEInit_2) {

    std::string testInitFile = "LGR_TESTMOD.INIT";

    const std::array<int, 3> ref_dim_global = {2,3,5};
    const std::array<int, 3> ref_dim_lgr1 = {4,8,4};
    const std::array<int, 3> ref_dim_lgr2 = {6,8,4};

    EInit init1(testInitFile);

    BOOST_CHECK_THROW(init1.grid_dimension("XXXX") , std::invalid_argument );

    const std::array<int, 3> nijk = init1.grid_dimension();
    BOOST_CHECK_EQUAL(nijk == ref_dim_global, true);

    const std::array<int, 3> nijk_global = init1.grid_dimension("global");
    BOOST_CHECK_EQUAL(nijk_global == ref_dim_global, true);

    const std::array<int, 3> nijk_lgr1 = init1.grid_dimension("LGR1");
    BOOST_CHECK_EQUAL(nijk_lgr1 == ref_dim_lgr1, true);

    const std::array<int, 3> nijk_lgr2 = init1.grid_dimension("LGR2");
    BOOST_CHECK_EQUAL(nijk_lgr2 == ref_dim_lgr2, true);

    BOOST_CHECK_EQUAL(init1.activeCells(), 30);
    BOOST_CHECK_THROW(init1.activeCells("XXXX") , std::invalid_argument );

    BOOST_CHECK_EQUAL(init1.activeCells("global"), 30);
    BOOST_CHECK_EQUAL(init1.activeCells("LGR1"), 128);
    BOOST_CHECK_EQUAL(init1.activeCells("LGR2"), 192);

    BOOST_CHECK_EQUAL(init1.hasLGR("XXX"), false);
    BOOST_CHECK_EQUAL(init1.hasLGR("global"), false);

    BOOST_CHECK_EQUAL(init1.hasLGR("LGR1"), true);
    BOOST_CHECK_EQUAL(init1.hasLGR("LGR2"), true);
}

BOOST_AUTO_TEST_CASE(TestEInit_3) {

    // testing getInitData member function
    // main purpase of these tests, is to ensure that the correct array is picked

    std::string testInitFile = "LGR_TESTMOD.INIT";

    EInit init1(testInitFile);

    // logihead item 5, logihead[4], is true if radial grid is used. This is the case for LGR2

    // bool data type
    auto logi_data_global= init1.getInitData<bool>("LOGIHEAD");
    BOOST_CHECK_EQUAL(logi_data_global[4], false);

    BOOST_CHECK_THROW(init1.getInitData<bool>("LOGIHEAD", "XXXX") , std::invalid_argument );

    auto logi_data_lgr1= init1.getInitData<bool>("LOGIHEAD", "LGR1");
    BOOST_CHECK_EQUAL(logi_data_lgr1[4], false);

    auto logi_data_lgr2= init1.getInitData<bool>("LOGIHEAD", "LGR2");
    BOOST_CHECK_EQUAL(logi_data_lgr2[4], true);


    // double data type

    auto doub_data_global= init1.getInitData<double>("DOUBHEAD");

    BOOST_CHECK_THROW(init1.getInitData<double>("DOUBHEAD", "XXXX") , std::invalid_argument );

    auto doub_data_lgr1= init1.getInitData<double>("DOUBHEAD", "LGR1");
    auto doub_data_lgr2= init1.getInitData<double>("DOUBHEAD", "LGR2");

    // item 4 in doubhead has been modified in this file test file
    // for LGR1 and LGR2 to ensure that at least one elemenet is different

    BOOST_REQUIRE_CLOSE (doub_data_global[3], 0.10000000149012, 1e-5);
    BOOST_REQUIRE_CLOSE (doub_data_lgr1[3]  , 0.10011100149012, 1e-5);
    BOOST_REQUIRE_CLOSE (doub_data_lgr2[3]  , 0.10022200149012, 1e-5);

    // int data type

    auto fipnum_global= init1.getInitData<int>("FIPNUM");

    BOOST_CHECK_THROW(init1.getInitData<int>("FIPNUM", "XXXX") , std::invalid_argument );

    auto fipnum_lgr1= init1.getInitData<int>("FIPNUM", "LGR1");
    auto fipnum_lgr2= init1.getInitData<int>("FIPNUM", "LGR2");

    BOOST_CHECK_EQUAL(fipnum_global.size(), 30);
    BOOST_CHECK_EQUAL(fipnum_lgr1.size(), 128);
    BOOST_CHECK_EQUAL(fipnum_lgr2.size(), 192);

    // float data type

    auto porv_global= init1.getInitData<float>("PORV");

    BOOST_CHECK_THROW(init1.getInitData<float>("PORV", "XXXX") , std::invalid_argument );

    auto porv_lgr1= init1.getInitData<float>("PORV", "LGR1");
    auto porv_lgr2= init1.getInitData<float>("PORV", "LGR2");

    BOOST_CHECK_EQUAL(porv_global.size(), 30);
    BOOST_CHECK_EQUAL(porv_lgr1.size(), 128);
    BOOST_CHECK_EQUAL(porv_lgr2.size(), 192);

    BOOST_CHECK_THROW(init1.getInitData<float>("DTHETA", "LGR1") , std::invalid_argument );
    BOOST_CHECK_THROW(init1.getInitData<float>("DY", "LGR2") , std::invalid_argument );

    auto dx_lgr1= init1.getInitData<float>("DX", "LGR1");
    auto dx_global= init1.getInitData<float>("DX");
    auto dtheta_lgr2= init1.getInitData<float>("DTHETA", "LGR2");

    for (auto dx : dx_lgr1)
        BOOST_REQUIRE_CLOSE (dx  , 25.0, 0.1);

    for (auto dx : dx_global)
        BOOST_REQUIRE_CLOSE (dx  , 100.0, 0.1);

    for (auto dtheta : dtheta_lgr2)
        BOOST_REQUIRE_CLOSE (dtheta  , 45.0, 0.1);
}
