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

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>
#include <opm/common/utility/FileSystem.hpp>

#define BOOST_TEST_MODULE Test EclIO
#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <tuple>

#include "tests/WorkArea.hpp"


using Opm::EclIO::ESmry;
using Opm::EclIO::ExtESmry;

template<typename InputIterator1, typename InputIterator2>
bool
range_equal(InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2, InputIterator2 last2)
{
    while(first1 != last1 && first2 != last2)
    {
        if(*first1 != *first2) return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

bool compare_files(const std::string& filename1, const std::string& filename2)
{
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    std::istreambuf_iterator<char> end;

    return range_equal(begin1, end, begin2, end);
}


template <typename T>
bool operator==(const std::vector<T> & t1, const std::vector<T> & t2)
{
    return std::equal(t1.begin(), t1.end(), t2.begin(), t2.end());
}



void getRefSmryVect(std::vector <float> &time_ref, std::vector <float> &wgpr_prod_ref, std::vector <float> &wbhp_prod_ref, std::vector <float> &wbhp_inj_ref,
 std::vector <float> &fgor_ref, std::vector <float> &bpr_111_ref, std::vector <float> &bpr_10103_ref) {

      // reference vectors check against resinsight, right click and show plot data.

        time_ref = {1,4,13,31,59,90,120,151,181,212,243,273,304,334,365,396,424,455,485,516,546,577,608,638,669,699,730,761,789,820,850,
             881,911,942,973,1003,1034,1064,1095,1126,1154,1185,1215,1246,1276,1307,1338,1368,1399,1429,1460,1491,1519,1550,1580,1611,1641,1672,1703,1733,
             1764,1794,1825,1856,1884,1915,1945,1976,2006,2037,2068,2098,2129,2159,2190,2221,2249,2280,2310,2341,2371,2402,2433,2463,2494,2524,2555,2586,
             2614,2645,2675,2706,2736,2767,2798,2828,2859,2889,2920,2951,2979,3010,3040,3071,3101,3132,3163,3193,3224,3254,3285,3316,3344,3375,3405,3436,
             3466,3497,3528,3558,3589,3619,3650};



        wgpr_prod_ref = { 25400,25400.01,25400,24868.41,24679.34,24722.67,24727.77,24727.34,24727.29,24728.52,24730.62,24733.06,24735.76,
             24738.4,24741.04,24743.66,24745.98,24748.4,24750.63,24752.83,24756.07,24761.09,24764.1,24784,27066.45,33575.16,43705.94,54388.21,65340.68,
             77829.57,88535.32,98802.72,108186,117395.1,125395.9,128689.4,129727.7,130422.1,130712.2,130653.5,130509.7,130192.4,129639.2,128959.1,128221.1,
             127395.6,126528,125665.9,124767.1,123897.2,123015.2,122150.7,121380.2,120541.4,119750.7,119008.2,118720,118329.8,117777.3,117232.2,116687.7,
             116198.9,115723,115269.7,114877.9,114498.6,113968.2,113365.6,112830.9,112342.7,111902.2,111506.3,111429.6,111820.9,112200.7,112393.1,112350.7,
             112093,111747.9,111399.3,111158.4,110974.7,110812.2,110704,110753.9,110856.4,110930.2,111033.1,111159.7,111290.7,111532.7,111754.7,111923.3,
             112092.7,112282.4,112476.8,112690.1,112907.5,113139.5,113373.6,113588.7,113835.7,114116.7,114427.3,114735.8,115058.2,115383.3,115700.7,116031.4,
             116362.3,116715.3,117065.4,117363.8,117677.7,117947.1,118135.7,118303.8,118480.7,118663,118842.3,119024.9,119195,119350.3};

        wbhp_prod_ref = {2904.77,2667.102,2430.112,2295.094,2233.452,2252.221,2311.47,2386.978,2464.336,2544.591,2623.208,2697.206,
             2771.272,2840.809,2910.732,2978.707,3038.701,3104.135,3166.967,3231.747,3294.372,3358.698,3434.863,3509.692,3539.855,3415.612,3121.1,2793.217,2517.024,
             2246.404,2005.897,1766.418,1534.75,1295.477,1072.804,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,
             1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,
             1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,
             1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};

        wbhp_inj_ref = {8253.15,8014.97,7329.12,6983.16,6684.92,6575.38,6528.93,6476.34,6453.38,6449.5,6458.79,6480.98,6510.48,6542.09,6577.34,
             6614.92,6650.01,6689.56,6728.47,6769.33,6808.93,6849.93,6890.88,6930.45,6970.21,7004.62,7031.64,7047.83,7052.25,7043.31,7022.21,6988.23,6944.89,
             6889.79,6825.3,6757.2,6684.39,6613.54,6540.99,6469.54,6406.19,6337.45,6272.51,6207.1,6145.47,6083.56,6023.49,5967.08,5910.65,5857.73,5804.9,5753.85,
             5709.25,5661.67,5617.34,5573.38,5532.49,5491.8,5452.55,5415.84,5379.28,5345.09,5311.01,5277.73,5248.81,5217.83,5188.74,5159.81,5132.76,5105.75,5079.57,
             5054.97,5030.16,5006.49,4982.17,4958,4936.49,4913.21,4891.25,4869.29,4848.68,4827.92,4807.7,4788.72,4769.62,4751.56,4733.24,4715.3,4699.48,4682.35,
             4666.14,4649.83,4634.43,4618.94,4603.88,4589.81,4575.75,4562.72,4549.85,4537.6,4526.95,4515.46,4504.59,4493.46,4482.87,4471.97,4461.14,4450.65,4439.73,
             4429.22,4418.21,4407.14,4397.07,4385.85,4374.93,4363.59,4352.5,4341.05,4329.71,4318.7,4307.34,4296.4,4285.14};

        fgor_ref = { 1.27,1.27,1.27,1.24342,1.23397,1.23613,1.23639,1.23637,1.23636,1.23643,1.23653,1.23665,1.23679,1.23692,1.23705,1.23718,1.2373,
             1.23742,1.23753,1.23764,1.2378,1.23805,1.2382,1.2392,1.35323,1.67876,2.18511,2.71941,3.26676,3.888,4.42677,4.93806,5.4093,5.86975,6.2698,6.60301,
             6.91579,7.19277,7.44386,7.66559,7.8618,8.06106,8.22055,8.37112,8.50559,8.63422,8.75421,8.86387,8.97159,9.07182,9.17264,9.27578,9.36818,9.4675,9.56145,
             9.6552,9.81817,9.96311,10.0741,10.1726,10.2715,10.3662,10.4631,10.5602,10.648,10.7503,10.812,10.8645,10.9203,10.985,11.055,11.1259,11.2608,11.4882,
             11.7386,11.9596,12.1154,12.2423,12.3375,12.4268,12.5285,12.6496,12.7744,12.9017,13.0636,13.2416,13.4274,13.6136,13.7875,13.9853,14.2141,14.4478,
             14.6595,14.8698,15.081,15.29,15.5087,15.7207,15.9375,16.1507,16.3397,16.5477,16.7576,16.9828,17.2054,17.4386,17.6743,17.9048,18.1461,18.3837,18.6359,18.8934,19.1273,19.3865,19.6289,19.8495,20.0575,20.2783,20.5075,20.7371,20.9815,21.2229,21.4733};

        bpr_111_ref = {5192.06,5606.51,5909.88,6057.86,6080.45,6080.23,6081.54,6084.29,6097.11,6119.32,6147.81,6181.28,6218.68,6256.71,6297.53,
             6339.85,6378.65,6421.72,6463.58,6507.1,6548.96,6591.99,6634.73,6675.85,6717.01,6752.59,6780.59,6797.41,6802.21,6793.55,6772.59,6738.64,6695.22,
             6639.93,6575.16,6506.73,6433.56,6362.35,6289.45,6217.66,6154.01,6084.96,6019.74,5954.06,5892.19,5830.05,5769.77,5713.17,5656.56,5603.5,5550.54,
             5499.38,5454.69,5407.03,5362.65,5318.65,5277.74,5237.04,5197.81,5161.12,5124.6,5090.45,5056.44,5023.23,4994.12,4962.81,4933.43,4904.23,4876.93,
             4849.68,4823.29,4798.5,4773.5,4749.65,4725.15,4700.79,4679.13,4655.68,4633.58,4611.48,4590.75,4569.88,4549.56,4530.49,4511.31,4493.19,4474.8,4456.8,
             4440.94,4423.76,4407.52,4391.18,4375.77,4360.26,4345.2,4331.13,4317.09,4304.08,4291.25,4279.05,4268.46,4257.02,4246.22,4235.16,4224.65,4213.81,
             4203.05,4192.62,4181.76,4171.3,4160.34,4149.32,4139.29,4128.11,4117.22,4105.91,4094.85,4083.42,4072.1,4061.11,4049.77,4038.84,4027.6};

        bpr_10103_ref = { 4583.96,4323.13,4063.73,3936.52,3884.46,3904.56,3969.39,4052.03,4136.77,4224.77,4311.05,4392.34,4473.78,4550.3,4627.31,
             4702.24,4768.42,4840.68,4910.14,4981.81,5051.17,5122.48,5206.98,5290.2,5339.93,5335.32,5290.28,5224.73,5142.4,5051.97,4968.24,4882.83,4793.78,4697.18,
             4600.77,4541.35,4502.17,4466.12,4428.31,4389.78,4355.46,4318.33,4282.8,4246.64,4212.19,4177.25,4143.1,4110.87,4078.55,4048.17,4017.89,3989.55,3964.72,
             3938.13,3913.35,3888.53,3864.13,3839.42,3816.21,3794.87,3773.77,3754.37,3735.2,3716.65,3700.38,3683.04,3667.23,3651.1,3635.94,3620.84,3606.19,3592.37,
             3578.56,3565.21,3550.34,3535.32,3521.95,3507.4,3493.98,3481.08,3469.23,3457.1,3445.23,3434.31,3423.9,3413.55,3402.49,3392.2,3383.36,3373.38,3363.72,
             3353.81,3344.61,3335.8,3327.55,3319.76,3312.03,3304.96,3298.14,3291.74,3286.38,3280.83,3275.92,3271,3266.36,3261.69,3257.16,3252.91,3248.64,3244.7,
             3240.73,3236.73,3232.96,3228.72,3224.57,3220.09,3215.85,3211.46,3207.02,3202.65,3198.03,3193.45,3188.54};

}

std::vector<float> getFrom(const std::vector<float> &ref_vect,int from){

    std::vector<float> vect;

    for (unsigned int i=from; i<ref_vect.size();i++){
       vect.push_back(ref_vect[i]);
    }

    return vect;
}

BOOST_AUTO_TEST_CASE(TestExtESmry_1) {
    WorkArea work;
    work.copyIn("SPE1CASE1.SMSPEC");
    work.copyIn("SPE1CASE1.UNSMRY");

    ESmry smry1("SPE1CASE1.SMSPEC");

    smry1.make_esmry_file();

    ExtESmry esmry1("SPE1CASE1.ESMRY");

    auto ntsteps = esmry1.numberOfTimeSteps();
    BOOST_CHECK_EQUAL(ntsteps, 123);

    std::vector <float> time_ref, wgpr_prod_ref, wbhp_prod_ref, wbhp_inj_ref, fgor_ref, bpr_111_ref, bpr_10103_ref;

    getRefSmryVect(time_ref, wgpr_prod_ref, wbhp_prod_ref, wbhp_inj_ref,fgor_ref, bpr_111_ref, bpr_10103_ref);

    auto time = esmry1.get("TIME");

    std::vector<float> smryVect = smry1.get("TIME");
    BOOST_CHECK_EQUAL(smryVect == time_ref, true);

    const auto dates = esmry1.dates();

    for (std::size_t index = 0; index < dates.size(); index++) {
        auto diff = dates[index]- smry1.startdate();
        auto diff_seconds = std::chrono::duration_cast<std::chrono::seconds>(diff).count();
        BOOST_CHECK_CLOSE(diff_seconds, 24*3600 * smryVect[index], 1e-6);
    }

    smryVect = esmry1.get("WGPR:PROD");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], wgpr_prod_ref[i], 0.01);

    smryVect = esmry1.get("WBHP:PROD");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], wbhp_prod_ref[i], 0.01);

    smryVect = esmry1.get("WBHP:INJ");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], wbhp_inj_ref[i], 0.01);

    smryVect = esmry1.get("FGOR");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], fgor_ref[i], 0.01);

    smryVect = esmry1.get("BPR:1,1,1");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], bpr_111_ref[i], 0.01);

    smryVect = esmry1.get("BPR:10,10,3");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], bpr_10103_ref[i], 0.01);

    ExtESmry esmry2("SPE1CASE1.ESMRY");
    auto fopr2a = esmry2.get("FOPR");
    esmry2.loadData({"FOPR"});
    auto fopr2b = esmry2.get("FOPR");

    BOOST_CHECK_EQUAL(fopr2a.size(), fopr2b.size());

    ExtESmry esmry3("SPE1CASE1.ESMRY");
    auto fopr3a = esmry3.get("FOPR");
    esmry3.loadData();
    auto fopr3b = esmry3.get("FOPR");

    BOOST_CHECK_EQUAL(fopr3a.size(), fopr3b.size());

    ExtESmry esmry4("SPE1CASE1.ESMRY");

    // vector list holding two elements of FOPR, should not be loaded twice
    std::vector<std::string> vect_list{"FOPR", "WBHP:PROD", "FOPR", "FGOR"};
    esmry4.loadData(vect_list);
    auto fopr4a = esmry4.get("FOPR");
    auto fgor4a = esmry4.get("FGOR");

    BOOST_CHECK_EQUAL(fopr4a.size(), fgor4a.size());
}

BOOST_AUTO_TEST_CASE(TestExtESmry_2) {

    // using a syntetic restart file.
    //
    //   # 1 copied SPE1CASE1 to TMP.DATA
    //      - added two extra summary vectors (FOPT and FGPR)
    //      - deleted 4 vectors WOIR:PROD, WOIR:INJ, WOIT:PROD and WOIT:INJ
    //   # 2 run full simulation with TMP.DATA
    //      - exact same solution as SPE1CASE1
    //   # 3 renamed TMP1.SMSPEC to SPE1CASE1_RST60.SMSPEC + manual modifications
    //      - update RESTART keyword ('SPE1CASE'  '1       '  ... )
    //      - updated DIMENS, last item equal to 60 => restart from report report step 60
    //   # 4 copy TMP1.UNSMRY to SPE1CASE1_RST60.UNSMRY + manual modifications
    //      - delete all data from ministep 0 to ministep 62, (ministep 62 = report step 60 in this run)

    // this is what the summary file from the restart run would be if the restart was 100% perfect.
    // changing summary keywords to make the file realistic.

    WorkArea work;
    std::vector <float> time_ref, wgpr_prod_ref, wbhp_prod_ref, wbhp_inj_ref, fgor_ref, bpr_111_ref, bpr_10103_ref;

    getRefSmryVect(time_ref, wgpr_prod_ref, wbhp_prod_ref, wbhp_inj_ref,fgor_ref, bpr_111_ref, bpr_10103_ref);

    // defaulting second argument, loadBaseRunData. Only data from the restarted run
    // will be loaded. No data from base run (SPE1CASE1 in this case)

    work.copyIn("SPE1CASE1.SMSPEC");
    work.copyIn("SPE1CASE1.UNSMRY");
    work.copyIn("SPE1CASE1_RST60.ESMRY");
    ESmry smry1("SPE1CASE1.SMSPEC");
    smry1.make_esmry_file();

    ExtESmry esmry1("SPE1CASE1_RST60.ESMRY");

    BOOST_CHECK_EQUAL(esmry1.all_steps_available(), true);

    auto ntsteps = esmry1.numberOfTimeSteps();
    BOOST_CHECK_EQUAL(ntsteps, 60);

    BOOST_CHECK_THROW( esmry1.get_unit("NO_SUCH_KEY"), std::invalid_argument);
    BOOST_CHECK_EQUAL( esmry1.get_unit("TIME"), "DAYS");
    BOOST_CHECK_EQUAL( esmry1.get_unit("WOPR:PROD"), "STB/DAY");

    std::vector<float> smryVect = esmry1.get("TIME");
    std::vector<float> time_ref_rst60 = getFrom(time_ref,63);

    BOOST_CHECK_EQUAL(smryVect==time_ref_rst60, true);

    smryVect = esmry1.get("WGPR:PROD");
    std::vector<float> ref_rst60 = getFrom(wgpr_prod_ref,63);

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], ref_rst60[i], 0.01);

    smryVect = esmry1.get("WBHP:PROD");
    ref_rst60 = getFrom(wbhp_prod_ref,63);

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], ref_rst60[i], 0.01);


    smryVect = esmry1.get("WBHP:INJ");
    ref_rst60 = getFrom(wbhp_inj_ref,63);

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], ref_rst60[i], 0.01);

    smryVect = esmry1.get("FGOR");
    ref_rst60 = getFrom(fgor_ref,63);

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], ref_rst60[i], 0.01);

    smryVect = esmry1.get("BPR:1,1,1");
    ref_rst60 = getFrom(bpr_111_ref,63);

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], ref_rst60[i], 0.01);

    smryVect = esmry1.get("BPR:10,10,3");
    ref_rst60 = getFrom(bpr_10103_ref,63);

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], ref_rst60[i], 0.01);
}

BOOST_AUTO_TEST_CASE(TestESmry_3) {

    // using a syntetic restart file.
    //
    //   # 1 copied SPE1CASE1 to TMP.DATA
    //      - added two extra summary vectors (FOPT and FGPR)
    //      - deleted 4 vectors WOIR:PROD, WOIR:INJ, WOIT:PROD and WOIT:INJ
    //   # 2 run full simulation with TMP.DATA
    //      - exact same solution as SPE1CASE1
    //   # 3 renamed TMP1.SMSPEC to SPE1CASE1_RST60.SMSPEC + manual modifications
    //      - update RESTART keyword ('SPE1CASE'  '1       '  ... )
    //      - updated DIMENS, last item equal to 60 => restart from report report step 60
    //   # 4 copy TMP1.UNSMRY to SPE1CASE1_RST60.UNSMRY + manual modifications
    //      - delete all data from ministep 0 to ministep 62, (ministep 62 = report step 60 in this run)

    // this is what the summary file from the restart run would be if the restart was 100% perfect.
    // changing summary keywords to make the file realistic.
    WorkArea work;
    work.copyIn("SPE1CASE1.SMSPEC");
    work.copyIn("SPE1CASE1.UNSMRY");
    work.copyIn("SPE1CASE1_RST60.ESMRY");

    std::vector <float> time_ref, wgpr_prod_ref, wbhp_prod_ref, wbhp_inj_ref, fgor_ref, bpr_111_ref, bpr_10103_ref;

    getRefSmryVect(time_ref, wgpr_prod_ref, wbhp_prod_ref, wbhp_inj_ref,fgor_ref, bpr_111_ref, bpr_10103_ref);

    // second argument, loadBaseRunData = true. Both data from restarted run and base run loaded
    // vectors should be equal to reference vectors (from SPE1CASE1)


    ESmry smry1("SPE1CASE1.SMSPEC");
    smry1.make_esmry_file();

    ExtESmry esmry1("SPE1CASE1_RST60.ESMRY", true);

    auto ntsteps = esmry1.numberOfTimeSteps();
    BOOST_CHECK_EQUAL(ntsteps, 123);

    std::vector<float> smryVect = esmry1.get("TIME");
    BOOST_CHECK_EQUAL(smryVect==time_ref, true);

    smryVect = esmry1.get("WGPR:PROD");
    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], wgpr_prod_ref[i], 0.01);


    smryVect = esmry1.get("WBHP:PROD");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], wbhp_prod_ref[i], 0.01);

    smryVect = esmry1.get("WBHP:INJ");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], wbhp_inj_ref[i], 0.01);

    smryVect = esmry1.get("FGOR");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], fgor_ref[i], 0.01);

    smryVect = esmry1.get("BPR:1,1,1");

    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], bpr_111_ref[i], 0.01);

    smryVect = esmry1.get("BPR:10,10,3");
    for (unsigned int i=0;i< smryVect.size();i++)
        BOOST_REQUIRE_CLOSE (smryVect[i], bpr_10103_ref[i], 0.01);

    auto fopt = esmry1.get("FOPT");

    // fopt vector not present in base run, should get zeros as not
    // for time step 0, 1, ... 62

    for (size_t n = 0; n < 63; n++)
        BOOST_CHECK_EQUAL(fopt[n], 0.0);

    std::vector<float> fopt_rst_ref = { 3.19319e+07, 3.2234e+07, 3.25642e+07, 3.28804e+07, 3.32039e+07, 3.35138e+07,
            3.38309e+07, 3.41447e+07, 3.44453e+07, 3.47521e+07, 3.50441e+07, 3.53404e+07, 3.56317e+07,
            3.58914e+07, 3.61752e+07, 3.6447e+07, 3.67249e+07, 3.6991e+07, 3.7263e+07, 3.75319e+07, 3.77893e+07,
            3.80521e+07, 3.83033e+07, 3.85594e+07, 3.88122e+07, 3.9038e+07, 3.92847e+07, 3.95201e+07, 3.97599e+07,
            3.99889e+07, 4.02226e+07, 4.04534e+07, 4.06741e+07, 4.08993e+07, 4.11148e+07, 4.13349e+07, 4.15525e+07,
            4.17471e+07, 4.19604e+07, 4.21647e+07, 4.23735e+07, 4.25736e+07, 4.27781e+07, 4.29805e+07, 4.31744e+07,
            4.33726e+07, 4.35625e+07, 4.37566e+07, 4.39487e+07, 4.41205e+07, 4.43087e+07, 4.4489e+07, 4.46735e+07,
            4.48504e+07, 4.50315e+07, 4.52109e+07, 4.53828e+07, 4.55587e+07, 4.57272e+07, 4.58995e+07 };


    for (size_t n = 63; n < fopt.size(); n++)
        BOOST_REQUIRE_CLOSE(fopt[n], fopt_rst_ref[n-63], 0.01);
}
